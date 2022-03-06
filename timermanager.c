#include "timermanager.h"

// TimerId-Action association type
typedef struct
{
    timer_t *timerId;
    TimerHandler handler;
} timerAction_t;

// Timer actions associative array
timerAction_t *actions = NULL;

// Length of current actions array
size_t actionsCount = 0U;

// Mutex object to allow thread-safe management of the actions array.
pthread_mutex_t lockActionsMutex;

// Ensures that Timer operations are thread safe.
bool InitializeTimerSync()
{
    if (pthread_mutex_init(&lockActionsMutex, NULL) != 0)
    {
        perror("Synchronization timer context has failed.\n");
        return false;
    }
    return true;
}

// Internal handler of all alarm signals (ONLY A SINGLE alarm handler is supported per process in POSIX systems)
void alarmHandler(int signal, siginfo_t *info, void *ptr)
{
    // get timerId
    timer_t *timerId = info->si_value.sival_ptr;

    // call associated timer handler
    for (size_t iTime = 0; iTime < actionsCount; iTime++)
        if ((actions + iTime)->timerId == timerId)
        {
            (actions + iTime)->handler();
            return;
        }
}

// Initialize and Start Timer with a specific Interval and Handler.
// Returns true if the Timer was created and started successfully.
timer_t* CreateTimer(long interval, enum IntervalType intervalType, TimerHandler handler, bool isOneShot)
{
    long intervalInNanoSeconds = interval;
    if (intervalType == MILLISECONDS)
        intervalInNanoSeconds *= 1000000L;

    // allocate a timerId variable in order to avoid losing its scope
    timer_t *timerId;
    timerId = malloc(sizeof(timer_t));

    // ensure that the actions array is updated accordingly
    pthread_mutex_lock(&lockActionsMutex);

    // prepare timer action array to store timerId-handler association
    if (actionsCount == 0)
        actions = malloc(sizeof(timerAction_t));
    else
        actions = realloc(actions, (actionsCount + 1) * sizeof(timerAction_t));

    // add handler and timerId pointer to the actions associated array
    actions[actionsCount].timerId = timerId;
    actions[actionsCount].handler = handler;
    actionsCount++;

    // release the mutex - action array is now safe
    pthread_mutex_unlock(&lockActionsMutex);

    // setup alarm signal action options
    struct sigaction actionOptions;
    actionOptions.sa_flags = SA_SIGINFO;
    actionOptions.sa_sigaction = alarmHandler;
    sigaction(SIGALRM, &actionOptions, 0);

    // setup signal event options
    struct sigevent signalOptions;
    signalOptions.sigev_notify = SIGEV_SIGNAL;
    signalOptions.sigev_signo = SIGALRM;
    signalOptions.sigev_value.sival_ptr = timerId;

    // create and start timer
    int result = timer_create(CLOCK_REALTIME, &signalOptions, timerId);
    if (result != 0)
    {
        perror("Could not create timer!\n");
        return false;
    }

    // setup timer options
    struct itimerspec timerOptions;
    const long ns = 1000000000L; // nanoseconds per second
    timerOptions.it_value.tv_sec = intervalInNanoSeconds / ns;
    timerOptions.it_value.tv_nsec = intervalInNanoSeconds % ns;
    timerOptions.it_interval.tv_sec = isOneShot ? 0 : intervalInNanoSeconds / ns;
    timerOptions.it_interval.tv_nsec = isOneShot ? 0 : intervalInNanoSeconds % ns;

    // initialize timer
    result = timer_settime(*timerId, 0, &timerOptions, NULL);
    if (result != 0)
    {
        perror("Could not start timer!\n");
        return NULL;
    }

    return timerId;
}

//Stops a specific timer.
void StopTimer(timer_t *timerId)
{
    timer_delete(*timerId);
    *timerId = NULL;
}

// Release all Timer resources
void DisposeTimers()
{
    // release timer resources
    for (size_t iTime = 0; iTime < actionsCount; iTime++)
    {
        timer_t *timerId = (actions + iTime)->timerId;
        if (*timerId != NULL)
            StopTimer(timerId);
        free(timerId);
    }

    // release timerid-action array
    free(actions);
    actions = NULL;
    actionsCount = 0;

    pthread_mutex_destroy(&lockActionsMutex);
}