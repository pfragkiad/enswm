//include guard
#ifndef COVIDTRACE_H
#define COVIDTRACE_H

#include "macaddress.h"
#include <stdlib.h>  //rand
#include <stdbool.h> //bool
#include <stdio.h>   //printf

//TimerSettings-------------------------------------------
long currentTime;

const int MAX_SPEEDUP;
typedef struct TimerSettings_t
{
    //all time settings in ms (for simulation purposes)
    long SearchInterval;
    long NearContactMinTime;
    long NearContactMaxTime;
    long TestTimeInterval;
    long MaxRememberNearContactTime;
    long TerminateSimulationTime;
    int SpeedUp;

} TimerSettings;
TimerSettings getTimerSettings(int speedUp);
TimerSettings timerSettings;
void printTimerSettings(TimerSettings* timerSettings);
//--------------------------------------------------------

typedef struct MacAddressStats_t
{
    macaddress Address;
    long FirstRegisteredTime;
    long LastRegisteredTime;
    bool IsNearContact;
} MacAddressStats;

//This is the main array of registered MAC addresses. These are the addresses chosen by the BTnearMe function.
//They are chosen from the pool of the allMacAddresses array, which is loaded in the beginning of the simulation.
MacAddressStats *registeredMacAddresses;
size_t registeredMacAddressesCount;

//This is a value in the range [0.0f, 1.0f]. It defines the probability of a single Bernoulli trial of having a COVID case.
float covidPercentageProbability;

void initializeSimulation(const char *macAddressesFile, int speedUp, float covidPercentage);

//-----

//Returns a random MAC address from the already loaded array.
macaddress BTnearMe();

//Returns a random COVID result based on the given covidProbability.
bool testCOVID();

void uploadContacts(macaddress *nearAddresses, int nearAddressesCount);

#endif