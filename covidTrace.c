

#include "covidTrace.h"

#include <stdio.h>  //puts, printf
#include <stdlib.h> //exit, EXIT_FAILURE

size_t registeredMacAddressesCount = 0u;

long currentTime = 0;
const int MAX_SPEEDUP = 10 * 1000;

void initializeSimulation(const char *macAddressesFile, int speedUp, float covidPercentage)
{
    covidPercentageProbability = covidPercentage;
    printf("Speedup: %d\n", speedUp);
    printf("Covid probability: %3.1f\n", covidPercentage);

    // load macaddresses from given file
    allMacAddresses = readMacAddresses(macAddressesFile, &macAddressesCount);
    printf("Number of MAC addresses in pool: %d\n", macAddressesCount);
#ifdef FULL_REPORT
    puts("MAC addresses:");
    for (size_t i = 0; i < macAddressesCount; i++)
        printMacAddress(allMacAddresses + i, true);
#endif

    // initialize timer settings (should go inside initializeSimulation)
    timerSettings = getTimerSettings(speedUp);

#ifdef FULL_REPORT
    printTimerSettings(&timerSettings);
#endif
}

TimerSettings getTimerSettings(int speedUp)
{
    const long s = 1000L;   // ms per second
    const long m = 60L * s; // ms per minute
    const long h = 60L * m; // ms per hour
    const long d = 24L * h; // ms per day

    if (speedUp > MAX_SPEEDUP)
    {
        fprintf(stderr, "Speedup %d is greater than %d.\n", speedUp, MAX_SPEEDUP);
        exit(EXIT_FAILURE);
    }

    TimerSettings t;
    t.SpeedUp = speedUp;
    t.SearchInterval = 10L * s / speedUp;             // 10" (speedUp=1)
    t.NearContactMinTime = 4L * m / speedUp;          // 4' (speedUp=1)
    t.NearContactMaxTime = 20L * m / speedUp;         // 20' (speedUp=1)
    t.TestTimeInterval = 4L * h / speedUp;            // 4 h (speedUp=1)
    t.MaxRememberNearContactTime = 14L * d / speedUp; // 14 d (speedUp=1)
    t.TerminateSimulationTime = 30L * d / speedUp;    // 30 d
    return t;
}

void printTimerSettings(TimerSettings *t)
{
    puts("Timer Settings:");
    printf("  Speedup:\t\t\t\t%d\n", t->SpeedUp);
    printf("  Search Interval:\t\t\tSim: %ld s\tReal: %ld ms\n", t->SpeedUp * t->SearchInterval / 1000L, t->SearchInterval);
    printf("  Near Contact Min Time:\t\tSim: %ld s\tReal: %ld ms\n", t->SpeedUp * t->NearContactMinTime / 1000L, t->NearContactMinTime);
    printf("  Near Contact Max Time:\t\tSim: %ld s\tReal: %ld ms\n", t->SpeedUp * t->NearContactMaxTime / 1000L, t->NearContactMaxTime);
    printf("  Test Time Interval:\t\t\tSim: %ld s\tReal: %ld ms\n", t->SpeedUp * t->TestTimeInterval / 1000L, t->TestTimeInterval);
    printf("  Max Remember Near Contact Time:\tSim: %ld s\tReal: %ld ms\n", t->SpeedUp * t->MaxRememberNearContactTime / 1000L, t->MaxRememberNearContactTime);
    printf("  Max Simulation Time:\t\t\tSim: %ld s\tReal: %ld ms\n", t->SpeedUp * t->TerminateSimulationTime / 1000L, t->TerminateSimulationTime);
}

// Returns a random MAC address from the (already) loaded array.
macaddress BTnearMe()
{
    return allMacAddresses + rand() % macAddressesCount;
}

bool testCOVID()
{
    // get a value in the [0.0f , 1.0f] range
    float r = (float)rand() / RAND_MAX;
    return r <= covidPercentageProbability;
}

void uploadContacts(macaddress *nearAddresses, int nearAddressesCount)
{
    const char *filename = "latest_near_contacts.dat";

    // open the file and get a file handle
    FILE *hFile = fopen(filename, "wb");

    // write the number of the entries at the beginning of the file
    fwrite(&nearAddressesCount, sizeof(int), 1, hFile);

    for (size_t i = 0; i < nearAddressesCount; i++)
        fwrite(nearAddresses[i], sizeof(uint48), 1, hFile);

    // flush and close the file
    fclose(hFile);
}
