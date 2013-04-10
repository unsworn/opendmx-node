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

#include "Constants.h"
#include "DmxThread.h"

using std::string;

/*
 * Create a new DmxThread object
 */
DmxThread::DmxThread(const string &path)
    : Thread(),
      m_path(path),
      m_term(false) 
{
    serial_init(&m_fd);
}

bool DmxThread::Open() {
    
    serialinfo_list_s list;
    
    if (m_path.empty()) {
        
        serial_list(&list);
        
        if (list.size == 0) {
            fprintf(stderr, "Open() no devices found\n");
            serial_list_free(&list);
            return false;
        }
        
        m_path.assign( list.info[0].name );
        
        fprintf(stderr, "Open() autoselect: %s\n", m_path.c_str());
        
    }
    
    return (serial_open(&m_fd, m_path.c_str()) == SERIAL_OK);
}
/*
 * Run this thread
 */
void *DmxThread::Run() {
    uint8_t buffer[DMX_UNIVERSE_SIZE+1];
    unsigned int length = DMX_UNIVERSE_SIZE;
    struct timeval tv;
    struct timespec ts;

    // should close other fd here

    // start code
    buffer[0] = 0x00;
    
    if (!serial_isopen(&m_fd))
        if (serial_open(&m_fd, m_path.c_str()))
            fprintf(stderr, "open serial failed\n");

    while (true) {
        {
            MutexLocker lock(&m_term_mutex);
            if (m_term)
                break;
        }

        if (!serial_isopen(&m_fd)) {
            if (gettimeofday(&tv, NULL) < 0) {
                fprintf(stderr, "gettimeofday error\n");
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

            if (serial_open(&m_fd, m_path.c_str()))
                fprintf(stderr, "Open %s: %s\n", m_path.c_str(), strerror(errno));

        } else {
            length = DMX_UNIVERSE_SIZE;
            {
                MutexLocker locker(&m_mutex);
                m_buffer.Get(buffer + 1, &length);
            }

            if (serial_write(&m_fd, buffer, length + 1) < 0) {
                // if you unplug the dongle
                fprintf(stderr,"Error writing to device: %s\n", strerror(errno));
                serial_close(&m_fd);
            }
            //fprintf(stderr, "Run() wrote %d bytes\n", length);
        }
    }
    return NULL;
}

void DmxThread::Dispose() 
{
    serial_cleanup(&m_fd);
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
