#ifndef SERIAL_H
#define SERIAL_H

#define SERIAL_OUT          0x01
#define SERIAL_IN           0x02

#define SERIAL_PARITY_NONE  0
#define SERIAL_PARITY_EVEN  1
#define SERIAL_PARITY_ODD   2

#define SERIAL_INVALID_FD -1

#include <stdio.h>

typedef struct
{
    short port;
    long baud;
    int parity;
    int stopbits;
    int databits;
} serialinfo_s;

typedef struct
{
    union {
        int          fd;
        void*        handle;
    };
    serialinfo_s info;
} serial_s;

int  serial_count  ();
int  serial_names  (char** list);
int  serial_init   (serial_s* serial);
void serial_cleanup(serial_s* serial);
int  serial_open   (serial_s* serial, const char* port);
void serial_close  (serial_s* serial);
int  serial_read   (serial_s* serial, void* buf, size_t count);
int  serial_write  (serial_s* serial, void* buf, size_t count);
int  serial_recv   (serial_s* serial, int to_sec, void* buf, size_t bufsz);
int  serial_isopen (serial_s* serial);
int  serial_setinfo(serial_s* serial, serialinfo_s* info);
void serial_flush  (serial_s* serial, int dir);
void serial_drain  (serial_s* serial);
void serial_debug  (serial_s* serial, FILE* f);

#endif  /* SERIAL_H */
