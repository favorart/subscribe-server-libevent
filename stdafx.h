#ifndef _STDAFX_H_
#define _STDAFX_H_

#define _XOPEN_SOURCE
#define  WAIT_ANY (-1)
//-----------------------------------------
#include <sys/types.h>

#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
// #include <arpa/inet.h>

#include <unistd.h>
#include <limits.h>
#include <fcntl.h>
//-----------------------------------------
#include <event.h>
//-----------------------------------------
#include <time.h>
#include <errno.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
//-----------------------------------------
#define SWAP(X,Y) \
    do { unsigned char _buf[sizeof (*(X))]; \
         memmove(_buf, (X), sizeof (_buf)); \
         memmove((X),  (Y), sizeof (_buf)); \
         memmove((Y), _buf, sizeof (_buf)); \
       } while (0)
//-----------------------------------------
#define IN
#define OUT
//-----------------------------------------
#endif // _STDAFX_H_
