#include "simulation.h"

const char *btnearmeFilename = "btnearme_times.dat";

long usedCpuTime = 0L;

pthread_mutex_t keepSimulationMutex;
pthread_cond_t keepSimulationCondition;
pthread_mutex_t registeredMacAddressesMutex;
pthread_mutex_t performanceFileMutex;

//------------------------------------------------
// Calls BTnearMe, registers a new MAC address or updates the time of an old MAC address.

// The function is used before every BTnearMe call
void writeCurrentTime()
{
    struct timeval realCurrentTime;
    gettimeofday(&realCurrentTime, NULL);

    FILE *hBtnearmeFile = fopen(btnearmeFilename, "ab");
    fwrite(&realCurrentTime, sizeof(struct timeval), 1, hBtnearmeFile);
    fclose(hBtnearmeFile);
}

void addPerformanceTime(struct timeval *tStart)
{
    struct timeval tEnd;
    gettimeofday(&tEnd, NULL); // used for CPU utilization
    pthread_mutex_lock(&performanceFileMutex);
    usedCpuTime += tEnd.tv_sec * 1000000 + tEnd.tv_usec - (tStart->tv_sec * 1000000 + tStart->tv_usec);
    pthread_mutex_unlock(&performanceFileMutex);
}

void registerNewAddress(macaddress currentMacAddress)
{
    registeredMacAddressesCount++;
    MacAddressStats *m;

    // add a new entry to the registered addresses array
    if (registeredMacAddressesCount == 1)
    {
        // initialize array
        registeredMacAddresses = malloc(sizeof(MacAddressStats));
        m = registeredMacAddresses;
    }
    else
    {
        // append a new entry
        registeredMacAddresses = realloc(registeredMacAddresses, registeredMacAddressesCount * sizeof(MacAddressStats));
        m = registeredMacAddresses + registeredMacAddressesCount - 1;
    }
    // set all the remaining properties for the newly arrived item
    m->Address = currentMacAddress;
    m->FirstRegisteredTime = m->LastRegisteredTime = currentTime;
    m->IsNearContact = false;

#ifdef FULL_REPORT
    printf(">  %li: ", registeredMacAddressesCount - 1);
    printMacAddress(m->Address, false);
    printf(" Age: 0, Time to forget: %ld [NEW]\n", timerSettings.NearContactMaxTime);
#endif
}

void forgetAddress(size_t index)
{
    if (registeredMacAddressesCount > 1)
    {
        // to avoid using a linked list and also avoid expensive memory operations by rebuilding the array,
        // we move the last item to the position of the removed item (the order of the mac addresses is insignificant)
        //(this is not needed if it is already the last item in the list)
        if (index != registeredMacAddressesCount - 1)
            registeredMacAddresses[index] = registeredMacAddresses[registeredMacAddressesCount - 1];
        // memcpy(registeredMacAddresses + im, registeredMacAddresses + registeredMacAddressesCount - 1, sizeof(MacAddressStats));

        // finally reallocate in order release the memory of the last item
        registeredMacAddresses = realloc(registeredMacAddresses, (--registeredMacAddressesCount) * sizeof(MacAddressStats));
    }
    else // it was the last entry so release the whole array
    {
        registeredMacAddressesCount = 0;
        free(registeredMacAddresses);
        registeredMacAddresses = NULL;
    }
}

void registerAddress(macaddress currentMacAddress)
{

#ifdef FULL_REPORT
    printf("REGISTERED ADDRESSES: %li\n", registeredMacAddressesCount);
#endif
    if (registeredMacAddressesCount == 0u)
    {
        registerNewAddress(currentMacAddress);
        return;
    }

    // keep a flag to track if the current mac address is already registered
    bool isRegistered = false;

    // check the status of all registered mac addresses
    for (int im = registeredMacAddressesCount - 1; im >= 0; im--)
    {
        MacAddressStats *m = registeredMacAddresses + im;
        long dt = currentTime - m->LastRegisteredTime;

        // check for old MAC addresses that are already in the registered addresses list
        // remove all REGISTERED OLD macaddresses and CHECK for NEW one
        if (m->Address != currentMacAddress)
        {
            // a non-near contact is forgotten if we exceed 20' (NearContactMaxTime)
            // a near contact is forgotten if we exceed 14 days (MaxRememberNearContactTime)
            bool toForget = !m->IsNearContact ? dt > timerSettings.NearContactMaxTime : dt > timerSettings.MaxRememberNearContactTime;

            if (toForget)
            {
#ifdef FULL_REPORT
                // print late address info
                printf("  %i: ", im);
                printMacAddress(m->Address, false);
                printf(" Age: %ld [FORGET]\n", dt);
#endif
                forgetAddress(im);
            }
#ifdef FULL_REPORT
            else
            {
                long timeToForget = !m->IsNearContact ? timerSettings.NearContactMaxTime - dt : timerSettings.MaxRememberNearContactTime - dt;
                // print late address info
                printf("  %i: ", im);
                printMacAddress(m->Address, false);
                printf(" Age: %ld, Time to forget: %ld%s\n", dt, timeToForget, m->IsNearContact ? " [CLOSE]" : " [FAR]");
            }
#endif
        }
        else // the Address is already REGISTERED, but is current again
        {
            m->LastRegisteredTime = currentTime;

            if (!m->IsNearContact) // check a non-near contact if it is now a near contact
            {
                // check REGISTERED MAC for a NEAR contact
                // note that if we come here there is no way that we have passed the NearContactMaxTime (20')
                // because the mac address would be forgotten based on the previous block check
                m->IsNearContact = currentTime - m->FirstRegisteredTime >= timerSettings.NearContactMinTime;

#ifdef FULL_REPORT
                printf("  %i: ", im);
                printMacAddress(m->Address, false);
                if (m->IsNearContact) // then it is Near so we wait for 14 days (MaxRememberNearContactTime)
                    printf(" Age: 0, Time to forget: %ld [CURRENT->CLOSE]\n", timerSettings.MaxRememberNearContactTime);
                else // not near yet so we wait for 20' (NearContactMaxTime)
                    printf(" Age: 0, Time to forget: %ld [CURRENT]\n", timerSettings.NearContactMaxTime);
#endif
            }

            // set the flag in order to avoid registering a new address after the loop
            isRegistered = true;
        }
    }

    // add a new entry to the registeredMacAddresses array
    if (!isRegistered)
        registerNewAddress(currentMacAddress);
}

void newSearch()
{
    struct timeval tStart;
    gettimeofday(&tStart, NULL); // used for CPU utilization (tic)

    currentTime += timerSettings.SearchInterval;

#ifdef FULL_REPORT
    long simulatedTimeInSeconds = (long)(currentTime * timerSettings.SpeedUp / 1000l);
    printf("\nTime: %ld s, ", simulatedTimeInSeconds);
#endif

    writeCurrentTime();
    macaddress nextMacAddress = BTnearMe();

    addPerformanceTime(&tStart); // used for CPU utilization (toc)

#ifdef FULL_REPORT
    printf("Next Address: ");
    printMacAddress(nextMacAddress, true);
#endif

    // many types of operations affect the content of the registeredMacAddresses array so we lock it
    // to ensure the consistency of all macaddress records within the array in case the BTNearMe check needs to access it.
    pthread_mutex_lock(&registeredMacAddressesMutex);
    gettimeofday(&tStart, NULL); // used for CPU utilization (tic)
    registerAddress(nextMacAddress);
    addPerformanceTime(&tStart); // used for CPU utilization (toc)
    pthread_mutex_unlock(&registeredMacAddressesMutex);
}

void *newSearchThreadHandler(void *arg)
{
    timer_t *timerId = CreateTimer(timerSettings.SearchInterval, MILLISECONDS, &newSearch, /*isOneShot*/ false);

    // keep thread/timer alive before the end of simulation
    pthread_mutex_lock(&keepSimulationMutex);
    pthread_cond_wait(&keepSimulationCondition, &keepSimulationMutex);
    pthread_mutex_unlock(&keepSimulationMutex);

    // stop internal timer
    StopTimer(timerId);
}

//------------------------------------------------

//------------------------------------------------
void runCovidTest()
{
    bool haveCovid = testCOVID(); //we assume that this time is insignificant

    if (haveCovid)
    {
        // get all near contacts

        // lock the array temporarily in order to retrieve current near contacts
        pthread_mutex_lock(&registeredMacAddressesMutex);

        struct timeval tStart;
        gettimeofday(&tStart, NULL); // used for CPU utilization (tic)

        // get the number of near contacts first
        size_t nearContactsCount = 0u;
        for (size_t i = 0u; i < registeredMacAddressesCount; i++)
            if (registeredMacAddresses[i].IsNearContact)
                nearContactsCount++;

        if (nearContactsCount > 0u)
        {
            macaddress *nearContacts = malloc(nearContactsCount * sizeof(macaddress));
            size_t iNearContact = 0u;

            // now get the near contacts
            for (size_t i = 0u; i < registeredMacAddressesCount; i++)
                if (registeredMacAddresses[i].IsNearContact)
                    nearContacts[iNearContact++] = registeredMacAddresses[i].Address;

            pthread_mutex_unlock(&registeredMacAddressesMutex);

            // and write them to the near contacts target file
            uploadContacts(nearContacts, nearContactsCount);

#ifdef FULL_REPORT
            printf("Covid Test Positive. Write %li near contacts to file.\n", nearContactsCount);
#endif
        }
        else
        {
            pthread_mutex_unlock(&registeredMacAddressesMutex);

            puts("No near contacts found!");
        }

        addPerformanceTime(&tStart);// used for CPU utilization (toc)
    }
}

void *runCovidTestThreadHandler(void *arg)
{
    timer_t *timerId = CreateTimer(timerSettings.TestTimeInterval, MILLISECONDS, &runCovidTest, /*isOneShot*/ false);

    // keep thread/timer alive before the end of simulation
    pthread_mutex_lock(&keepSimulationMutex);
    pthread_cond_wait(&keepSimulationCondition, &keepSimulationMutex);
    pthread_mutex_unlock(&keepSimulationMutex);

    // stop internal timer
    StopTimer(timerId);
}
//------------------------------B------------------

//-----------------------------------------------
// When fired the simulation ends.
void simulationTermination()
{
    pthread_cond_broadcast(&keepSimulationCondition);
}

void *simulationTerminationThreadHandler(void *arg)
{
    CreateTimer(timerSettings.TerminateSimulationTime, MILLISECONDS, &simulationTermination, /*isOneShot*/ true);

    // keep thread alive before the end of simulation
    pthread_mutex_lock(&keepSimulationMutex);
    pthread_cond_wait(&keepSimulationCondition, &keepSimulationMutex);
    pthread_mutex_unlock(&keepSimulationMutex);
}
//--------------------------------------------------------

void runSimulation()
{
    currentTime = 0;
    usedCpuTime = 0;
    
    struct timeval tStart;
    gettimeofday(&tStart, NULL); // used for CPU utilization (tic)
    pthread_mutex_init(&performanceFileMutex, NULL);  

    
    // intitialize thread timer resources (just a mutex)
    InitializeTimerSync();

    // initialize times file for btnearme call
    FILE *hBtnearmeFile = fopen(btnearmeFilename, "wb");
    int magicNumber = 0x123456;
    fwrite(&magicNumber, sizeof(int), 1, hBtnearmeFile);
    // flush all write operations
    fclose(hBtnearmeFile);

    pthread_mutex_init(&keepSimulationMutex, NULL);
    pthread_cond_init(&keepSimulationCondition, NULL);
    pthread_mutex_init(&registeredMacAddressesMutex, NULL);

    // typically this thread is not needed during continuous operation
    pthread_t simulationTerminationThread;
    if (pthread_create(&simulationTerminationThread, NULL, simulationTerminationThreadHandler, NULL))
    {
        perror("Failed to create main Simulation Termination Thread!");
        exit(EXIT_FAILURE);
    }

    pthread_t newSearchThread;
    if (pthread_create(&newSearchThread, NULL, newSearchThreadHandler, NULL))
    {
        perror("Failed to create New Search Thread!");
        exit(EXIT_FAILURE);
    }

    pthread_t runCovidTestSimulationThread;
    if (pthread_create(&runCovidTestSimulationThread, NULL, runCovidTestThreadHandler, NULL))
    {
        perror("Failed to create main COVID Test Thread!");
        exit(EXIT_FAILURE);
    }

    addPerformanceTime(&tStart); // used for CPU utilization (toc) (we measure the thread/timer creation time here)
   
    // wait for all threads to terminate!
    pthread_join(newSearchThread, NULL);
    pthread_join(runCovidTestSimulationThread, NULL);
    pthread_join(simulationTerminationThread, NULL);

   //the LAST entry of the time duration includes the TOTAL duration of the simulation after termination of the threads
    
    struct timeval tEnd;
    gettimeofday(&tEnd, NULL); // used for CPU utilization
    long totalSimulationTime = tEnd.tv_sec * 1000000 + tEnd.tv_usec - (tStart.tv_sec * 1000000 + tStart.tv_usec);

    printf("Total time: %ld, CPU time: %ld, Utilization: %5.1f %%\n",totalSimulationTime, usedCpuTime, (float)usedCpuTime/totalSimulationTime*100 );
 
    DisposeTimers();
    pthread_mutex_destroy(&performanceFileMutex);
    pthread_mutex_destroy(&registeredMacAddressesMutex);
    pthread_mutex_destroy(&keepSimulationMutex);
    pthread_cond_destroy(&keepSimulationCondition);
}