

#include <stdlib.h> //malloc
#include <stdio.h>  //fopen, fclose, printf
#include <time.h>   //time
#include <unistd.h>
#include <stdbool.h> //bool

#include "macaddress.h"

void printMacAddress(macaddress m, bool useEndLine)
{
    printf("  %02X %02X %02X %02X %02X %02X%s",
            m->x[0], m->x[1], m->x[2], m->x[3], m->x[4], m->x[5],
            useEndLine?"\n":"");
}

void saveRandomMacAddresses(const char *filename, int macAddressesCount)
{
    //initialize the seed for the pseudo-random number generator
    srand(time(NULL));

    //open the file and get a file handle
    FILE *hFile = fopen(filename, "wb");

    //write the number of the entries at the beginning of the file
    fwrite(&macAddressesCount, sizeof(int), 1, hFile);

    for (size_t i = 0; i < macAddressesCount; i++)
    {
        // //create a temporary mac address
        // puint48 newMacAddress = (puint48)malloc(sizeof(uint48));
        // //rand is guaranteed to return a value within the 0-32767 (RAND_MAX) range
        // for (int ib = 0; ib < 6; ib++)
        // {
        //     newMacAddress->x[ib] = rand() % 0xFF;
        //     //printf("%2X ", newMacAddress->x[ib]);
        // } //puts("");
        // //write to file
        // fwrite(newMacAddress, sizeof(uint48), 1, hFile);
        // //release from memory
        // free(newMacAddress);

        //the fastest way is to write 6 random bytes immediately to the file
        //for each MAC address (avoid any allocation of memory)
        for (size_t ib = 0; ib < 6; ib++)
        {
            //create a random byte (unsigned) value from 0-255
            u_int8_t b;
            b = (u_int8_t)(rand() % 0xFF);
            fwrite(&b, (size_t)1, 1, hFile);
        }
    }

    //flush and close the stream
    fclose(hFile);
}

macaddress readMacAddresses(const char *filename, int *pCount)
{
    FILE *hFile = fopen(filename, "rb");

    //read the number of MAC addresses entries first
    fread(pCount, sizeof(int), 1, hFile);

    //allocate memory for *pCount mac addresses
    macaddress macAddresses = malloc(*pCount * sizeof(uint48));

    //and read them all with a single call!
    fread(macAddresses, sizeof(uint48), *pCount, hFile);

    //close the file prior to returning the MAC addresses
    fclose(hFile);

    return macAddresses;
}