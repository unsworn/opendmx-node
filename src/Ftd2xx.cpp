/*
 * serial.c    for ftd2xx raw
 *
 * serial port routines
 *
 * Adapted from sio.c
 * (C)1999 Stefano Busti
 * (C)2012 Nicklas Marelis
 */
 
#include <stdio.h>
#include <string.h>               
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/select.h>
#include <fcntl.h>
#include <dirent.h>
#include "serial.h"

#include <ftd2xx.h>

struct serialhandle_s {
    FT_HANDLE ft;
};

int 
serial_count()
{
    DWORD numDevs = 0;
    if (FT_ListDevices(&numDevs, NULL, FT_LIST_NUMBER_ONLY) != FT_OK) {
        fprintf(stderr, "FT_LIST_NUMBER_ONLY fails\n");
    }
    return (int)numDevs;
}

int
serial_list(serialinfo_list_s* list)
{
    char** names;
    DWORD i=0,numDevs = serial_count();
    
    if (numDevs <= 0)
        return 1;
    
    list->size = numDevs;
    
    names = (char **) malloc ( numDevs * sizeof(char*) );
    
    for (i = 0 ; i < numDevs ; i++)
        names[i] = (char*) malloc (64 * sizeof(char));
        
    if (FT_ListDevices(names, &numDevs, FT_LIST_ALL|FT_OPEN_BY_SERIAL_NUMBER) != FT_OK) {
        fprintf(stderr, "FT_OPEN_BY_SERIAL_NUMBER fails\n");
    }
    
    list->info = (serialinfo_s*) malloc (list->size * sizeof(serialinfo_s));
    
    for (i=0 ; i < numDevs ; i++) {
        list->info[i].name = names[i];
        list->info[i].port = i;
        list->info[i].baud = 230400;
        list->info[i].parity = SERIAL_PARITY_NONE;
        list->info[i].stopbits = 1;
        list->info[i].databits = 8;
    }
    
    free(names);
    
    return 0;
}

void
serial_names_free(serialinfo_list_s* list)
{
    int i;
    if (!list->info)
        return ;
    for (i=0 ; i < list->size ; i++)
        free(list->info[i].name);
    free(list->info);
}

int
serial_init(serial_s* serial)
{
    serial->port = (struct serialhandle_s*) malloc(sizeof(struct serialhandle_s));
    serial->port->ft = NULL;
    serial->info.baud = 230400;
    serial->info.parity = SERIAL_PARITY_NONE;
    serial->info.stopbits = 1;
    serial->info.databits = 8;
    return 0;
}

void 
serial_cleanup(serial_s* serial)
{
    if (serial_isopen(serial))
        serial_close(serial);  
    free(serial->port);
}

int 
serial_open(serial_s* serial, const char* port)
{            
    FT_HANDLE ft;
    UCHAR i_parity;
    
    if (serial_isopen(serial))
        return -1;
        
    if (FT_OpenEx((void*)port, FT_OPEN_BY_SERIAL_NUMBER, &ft) != FT_OK) {
        fprintf(stderr, "FT_OPEN_BY_SERIAL_NUMBER %s fails\n", port);
        return -1;
    }
    
    if (FT_SetBaudRate(ft, serial->info.baud) != FT_OK) {
        fprintf(stderr, "FT_SetBaudRate fails\n");
    }
    
    switch(serial->info.parity)
    {
    case SERIAL_PARITY_NONE:
        i_parity = FT_PARITY_NONE;
        break;
        
    case SERIAL_PARITY_EVEN:
        i_parity = FT_PARITY_EVEN;
        break;

    case SERIAL_PARITY_ODD:
        i_parity = FT_PARITY_ODD;
        break;
    default:
        i_parity = FT_PARITY_NONE;
        break;
    }
    
    
    if(FT_SetDataCharacteristics(
        ft, 
        serial->info.databits, 
        serial->info.stopbits,
        i_parity) != FT_OK) {
            fprintf(stderr, "FT_SetDataCharacteristics fails\n");
    }

    // all setup
    serial->port->ft = ft;
    
    return 0;
    
}

void 
serial_close(serial_s *serial)
{
    if (serial_isopen(serial))
        if (FT_Close(serial->port->ft) != FT_OK)
            fprintf(stderr, "FT_Close fails\n");
    serial->port->ft = NULL;
}

int 
serial_read(serial_s *serial, void *buf, size_t count)
{
    DWORD nBytesRead = 0;
    if (FT_Read(serial->port->ft, buf, count, &nBytesRead) != FT_OK) {
        fprintf(stderr, "FT_Read fails\n");                           
        return -1;
    }
    return nBytesRead;
}

int 
serial_write(serial_s *serial, void *buf, size_t count)
{        
    DWORD nBytesWritten = 0;
    if (FT_Write(serial->port->ft, buf, count, &nBytesWritten) != FT_OK) {
        fprintf(stderr, "FT_Write fails\n");
        return -1;
    }
    return nBytesWritten;
}

int
serial_recv(serial_s* serial, int to_sec, void* buf, size_t bufsz)
{
    fprintf(stderr, "recv() not implemented\n");
    return 0;
}

int 
serial_isopen(serial_s *serial)
{
    return (serial->port->ft != NULL);
}

int 
serial_setinfo(serial_s *serial, serialinfo_s *info)
{
    serial->info = *info;
    return 0;
}
    
void 
serial_flush(serial_s *serial, int dir)
{
}

void 
serial_drain(serial_s *serial)
{
}

void 
serial_debug(serial_s *serial, FILE *f)
{
    fprintf(f, "serial {\n");
    fprintf(f, "\tfd = %d\n", serial_isopen(serial) ? serial->port->ft != NULL : 0);
    fprintf(f, "\tbaud = %ld\n", serial->info.baud);    
    fprintf(f, "\tparity = %d\n", serial->info.parity); 
    fprintf(f, "\tstopbits = %d\n", serial->info.stopbits);
    fprintf(f, "\tdatabits = %d\n", serial->info.databits); 
    fprintf(f, "\topen = %d\n", serial_isopen(serial)); 
    fprintf(f, "}\n\n");
}
