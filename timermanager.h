#ifndef TIMERMANAGER_H
#define TIMERMANAGER_H

#include <time.h>    //timer_settime, timer_create
#include <stdlib.h>  //malloc
#include <signal.h>  //siginfo_t
#include <stdio.h>   //perror
#include <stdbool.h> //bool

#include <pthread.h>

// Action called after each Timer interval has passed.
typedef void (*TimerHandler)();

// Ensures that Timer operations are thread safe. Should be called before any CreateTimer call.
bool InitializeTimerSync();

enum IntervalType { MILLISECONDS, NANOSECONDS} ;

// Initializes and starts a Timer with a specific Interval and Handler.
// isOneShot: 
// Returns: the Timer ID or NULL on error.
timer_t* CreateTimer(long interval, enum IntervalType intervalType, TimerHandler handler, bool isOneShot);

// Stops a specicfic Timer.
void StopTimer(timer_t *timerId);

// Release all Timer resources
void DisposeTimers();

#endif