#include <papi.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include <sys/types.h>
#include <signal.h>
#include <string.h>
#include <pthread.h>
#include <sys/mman.h>
#include <fcntl.h>

#define NUMEVENTS 3
#define FILESIZE (NUMEVENTS * sizeof(long long int))

int countPapi(int PID);
