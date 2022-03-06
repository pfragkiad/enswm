#ifndef SIMULATION_H
#define SIMULATION_H

#include <stdio.h>   //printf, setbuf, puts
#include <stdbool.h> //bool
#include <stdlib.h>  //srand, atoi, exit, EXIT_FAILURE, malloc
#include <string.h>  //strcmp
#include <time.h>    //time

#include <pthread.h>
#include <unistd.h> //sleep

//#include "alarms.h"
#include "timermanager.h"

#include "macaddress.h"
#include "covidTrace.h"

#include <sys/time.h> //gettimeofday


void runSimulation();

#endif