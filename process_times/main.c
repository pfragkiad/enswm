#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <sys/stat.h>

void exportBtNearDurations()
{
    char *filename = "btnearme_times.dat";
    char *durationsFilename = "durations.txt";

    // get number of records
    struct stat stats;
    if (stat(filename, &stats) == -1)
    {
        perror("Could not retrieve file stats.");
        return EXIT_FAILURE;
    }
    printf("File size [bytes]: %ld\n", stats.st_size);

    long recordsCount = (stats.st_size - sizeof(int)) / (sizeof(struct timeval));
    printf("Number of records: %ld\n", recordsCount);

    FILE *hFile = fopen(filename, "rb");
    int magicNumber;
    fread(&magicNumber, sizeof(int), 1, hFile); // 4 bytes
    printf("Magic number: %04X\n", magicNumber);

    struct timeval *times; // 16 bytes
    times = malloc(recordsCount * sizeof(struct timeval));
    // read all times
    fread(times, sizeof(struct timeval), recordsCount, hFile);
    fclose(hFile);

    // now export stats!
    FILE *hFile2 = fopen(durationsFilename, "w");
    for (int i = 1; i < recordsCount; i++)
    {
        long dt = times[i].tv_sec * 1000000 + times[i].tv_usec -
                  (times[i - 1].tv_sec * 1000000 + times[i - 1].tv_usec);
        fprintf(hFile2, "%ld\r\n", dt);
    }

    fclose(hFile2);
}


int main(int argc, char **argv)
{
    exportBtNearDurations();

    return EXIT_SUCCESS;
}
