/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 * OpenDMX driver from OLA. 
 * adapted for node.js with fixed darwin support
 * Copyright (C) 2010 Simon Newton              
 * Copyright (C) 2013 Nicklas Marelius
 */

#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <string>
#include <vector>

#include "Constants.h"
#include "DmxThread.h"
              
#if DEBUG_FTD2XX
#define DEBUG(...) fprintf(stderr, __VA_ARGS__)
#else
#define DEBUG(...)
#endif

using std::string;
using std::vector;

typedef struct __Ft_Device_Info {
    int  device;
    char description[64];
    char serial[64];
} ft_device_t;
    
bool 
ft_list_devices(ft_device_t** list) {
    FT_STATUS err;
    DWORD i, numDevs, sz;

    char **arr;
    
    if ((err = FT_ListDevices(&numDevs, NULL, FT_LIST_NUMBER_ONLY)) != FT_OK) {
        DEBUG("ft_list_devices() failed!\n");
        return false;
    }
    
    DEBUG("attached dmx devices: %d\n", numDevs);
    
    if (numDevs == 0) {
        fprintf(stderr, "no dmx interface found\n");
        return false;  
    }
    
    (*list) = (ft_device_t*) malloc( (numDevs + 1) * sizeof(ft_device_t) );
    
    arr = (char **) malloc ( numDevs * sizeof(char*) );
    
    for (i=0 ; i < numDevs ; i++) {
        (*list)[i].device = i;
        arr[i] = (char*) malloc (64 * sizeof(char));
    }

    sz = numDevs;
    if ((err = FT_ListDevices(arr, &sz, FT_LIST_ALL|FT_OPEN_BY_DESCRIPTION)) == FT_OK) {
        
        for (i=0 ; i < sz ; i++) {
            //DEBUG("ft_list_devices() by desc (%d, %s)\n", i, arr[i]);
            memset( (*list)[i].description, 0, 64 );
            strcpy( (*list)[i].description, arr[i] );
        }
    }
    sz = numDevs;
    if ((err = FT_ListDevices(arr, &sz, FT_LIST_ALL|FT_OPEN_BY_SERIAL_NUMBER)) == FT_OK) {
        
        for (i=0 ; i < sz ; i++) {
            //DEBUG("ft_list_devices() by serial (%d, %s)\n", i, arr[i]);
            memset( (*list)[i].serial, 0, 64);
            strcpy( (*list)[i].serial, arr[i] );
        }
    }

    (*list)[numDevs].device = -1;
    
    for (i=0 ; i < numDevs ; i++)
        free( arr[i] );
    
    free(arr);
    
    return true;
}
/*
 * Create a new DmxThread object
 */
DmxThread::DmxThread(const string &path)
    : Thread(),
    m_fd(INVALID_FD),
    m_path(path),
    m_term(false),
    m_handle(NULL) {
}

bool
DmxThread::Open() {
    DWORD ft_version;
    
    DEBUG("\n\nDmxThread::Open()\n");
                           
    if (FT_GetLibraryVersion(&ft_version) == FT_OK) {
        DEBUG("ftd2xx version: (%d.%d.%d.%d)\n",
                (int)((char)((ft_version & 0xFF000000) >> 24)),
                (int)((char)((ft_version & 0x00FF0000) >> 16)),
                (int)((char)((ft_version & 0x0000FF00) >> 8)),
                (int)((char)((ft_version & 0x000000FF))));
    }
        
    ft_device_t* devices;
    
    if (!ft_list_devices(&devices)) {
        fprintf(stderr, "ft_list_devices() fails\n");
        return false;
    }

    ft_device_t* ptr = devices;
    
    while (ptr->device != -1) {
        DEBUG("Device (%s, serial: %s, number: %d)\n", ptr->description, ptr->serial, ptr->device);
        
        if (m_path.length() == 0 || m_path == ptr->description || m_path == ptr->serial) {
            m_fd = ptr->device;
            break;
        }
        ptr++;
    }
    
    free(devices);
    
    if (m_fd == INVALID_FD) {
        fprintf(stderr, "no such device: (%s)\n", m_path.c_str());
        return false;
    }
    
    DEBUG("DmxThread::Open(device=%d) == SUCCESS\n", m_fd);
    return true;
}
void
DmxThread::Dispose() {
    DEBUG("Dispose!\n");
    if (m_handle != NULL) {
        DEBUG("Dispose, Closing device\n");
        if (FT_Close(m_handle) != FT_OK)
            fprintf(stderr, "Close failed on cleanup\n");
        m_handle = NULL;
        m_fd = INVALID_FD;
    }
}
/*
 * Run this thread
 */
void *DmxThread::Run() {
  uint8_t buffer[DMX_UNIVERSE_SIZE+1];
  unsigned int length = DMX_UNIVERSE_SIZE;
  struct timeval tv;
  struct timespec ts;
  DWORD wr;
  
  // should close other fd here
  
  // start code
  buffer[0] = 0x00;
  
  DEBUG("Starting DmxThread()\n");
  
  if (m_fd == INVALID_FD) {
      fprintf(stderr, "Cant open NULL\n");
      return NULL;
  }
  
  if (m_handle != NULL) {
      fprintf(stderr, "Already opened, do not call Run() twice\n");
      return NULL;
  }
  
  DEBUG("Open(%d)\n", m_fd);
  
  if (FT_Open(m_fd, &m_handle) != FT_OK) {
      fprintf(stderr, "Failed to open device!!\n");
      return NULL;
  }
  
  DEBUG("InitFt_\n");
  
  InitFt();
  
  DEBUG("Started...\n");
  
  while (true) {
    
    {
      MutexLocker lock(&m_term_mutex);
      if (m_term)
        break;
    }

    if (m_handle == NULL) {
        
        if (gettimeofday(&tv, NULL) < 0) {
            std::cerr << "gettimeofday error";
            break;
        }
        
        ts.tv_sec = tv.tv_sec + 1;
        ts.tv_nsec = tv.tv_usec * 1000;

        // wait for either a signal that we should terminate, or ts seconds
        m_term_mutex.Lock();
        if (m_term)
            break;
            
        m_term_cond.TimedWait(&m_term_mutex, &ts);
        m_term_mutex.Unlock();

        if (FT_Open(m_fd, &m_handle) != FT_OK) {
            fprintf(stderr, "Open (2)\n");
            m_handle = NULL;
        }
        
        InitFt();
          
    } else {
        
        length = DMX_UNIVERSE_SIZE;
        {
            MutexLocker locker(&m_mutex);
            m_buffer.Get(buffer + 1, &length);
        }
        
        if (FT_Write(m_handle, buffer, length + 1, &wr) != FT_OK) {
            // if you unplug the dongle
            fprintf(stderr,"Error writing to device\n" );
            
            DEBUG("Abort, Closing device\n");
            
            if (FT_Close(m_handle) != FT_OK)
                fprintf(stderr, "Close failed\n");
                
            m_handle = NULL;
        }
        
        if (wr != length + 1) {
            fprintf(stderr, "Error not all bytes written\n");
        } else {
            DEBUG("Wrote %d bytes\n", wr);
        }
    }
    
  }
  
  if (m_handle != NULL) {
      DEBUG("Closing device\n");
      if (FT_Close(m_handle) != FT_OK)
          fprintf(stderr, "Close failed on cleanup\n");
      m_handle = NULL;
      m_fd = INVALID_FD;
  }
  
  return NULL;
}


/*
 * Stop the thread
 */
bool DmxThread::Stop() {
  {
    MutexLocker locker(&m_mutex);
    m_term = true;
  }
  m_term_cond.Signal();
  return Join();
}


/*
 * Store the data in the shared buffer
 *
 */
bool DmxThread::WriteDmx(const DmxBuffer &buffer) {
  MutexLocker locker(&m_mutex);
  // avoid the reference counting
  m_buffer.Set(buffer);
  return true;
}

void DmxThread::InitFt() {
    
    if (m_handle == NULL)
        return ;
    
    if (FT_SetBaudRate(m_handle, 115200) != FT_OK)
        fprintf(stderr, "set baudrate failed\n");
    
}