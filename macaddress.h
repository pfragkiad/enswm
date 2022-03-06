

//header guard
#ifndef MACADDRESS_H
#define MACADDRESS_H

#include <sys/types.h>
#include <stdbool.h> //bool

//The uint48 requested type for the MAC Address
typedef struct uint48_t
{
    u_int8_t x[6];
} uint48, *puint48;

//Use macaddress as an alias for the uint48 pointer type
typedef puint48 macaddress;

int macAddressesCount;
macaddress allMacAddresses;


//Prints a MAC Address to stdout using hexadecimal notation.
//  m: The MAC Address to be printed.
void printMacAddress(macaddress m, bool useEndLine);

//Saves random MAC Addresses to a file.
//
//  filename: The output file path.
//  count: The number of the MAC Addresses to generate.
void saveRandomMacAddresses(const char *filename, int macAddressesCount);


//Loads random MAC Addresses from a file.
//
//  filename: The input file path.
//  count: Pointer to a variable that will receive the number of MAC Addresses as retrieved from the file.
macaddress readMacAddresses(const char *filename, int *count);

#endif