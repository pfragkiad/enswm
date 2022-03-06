//#define FULL_REPORT

#include "simulation.h"

int main(int argc, char **argv)
{
    // prevent caching for the buffer of the standard output
    setbuf(stdout, 0);

   // define the mac addresses source file
    const char *filename = "macaddresses.dat";

    //--------------------- write scenario (write to file)
    int speedUp = 1; //default operation (no speedup)
    if (argc > 1)
    {
        // syntax example to write 100 records:
        //./<executable> write 100
        if (strcmp(argv[1], "write") == 0)
        {
            macAddressesCount = atoi(argv[2]);
            saveRandomMacAddresses(filename, macAddressesCount);
            return EXIT_SUCCESS;
        }
        //./<executable> writeandload 100
        else if (strcmp(argv[1], "writeandload") == 0)
        {
            macAddressesCount = atoi(argv[2]);
            saveRandomMacAddresses(filename, macAddressesCount);
            // execution continues
        }
        //override speedup
        //./<executable> speedup 100
        else if(strcmp(argv[1],"speedup")==0) 
            speedUp = atoi(argv[2]); //execution continues
    }

    //-----------------------continue simulation

    // if no arguments are given then we load the current file
    // read mac addresses from file given above
    initializeSimulation(filename, /*speedUp*/ speedUp,/*covidPercentageProbability*/ 0.1f);

    puts("Running simulation...");
    runSimulation();

    // release resources at the end (we should not arrive here during the "continuous" operation)
    free(allMacAddresses);
    puts("Simulation ended.");

    return EXIT_SUCCESS;
}
