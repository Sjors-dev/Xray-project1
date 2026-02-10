#ifndef DOSEADMIN_INTERNAL_H
#define DOSEADMIN_INTERNAL_H

/* The functions listed in this file need to implemented in doseAdmin.c.
 *
 * This headerfile is also included in the doseAdmin test code, so that 
 * the test code is aware of the internal structure of the stored data
 */
 

#define HASHTABLE_SIZE			(256)

/*
	Define here the data structure of your hash table
*/

/***************************************************************************************
 * Returns a number in the range [0, HASHTABLE_SIZE), i.e. the entry in the hash table
 * 
 * It is a precondition that patientName is not NULL, is \0 terminated, and length
 * does not exceed MAX_PATIENTNAME_SIZE
 */
uint8_t hashFunction(char * patientName);

/***************************************************************************************
 * Returns the entry to the hash table
 *
 * Returns now a void pointer, but feel free to change it to for instance a pointer to a 
 * patient struct.
 */
void * getHashTable();

/***************************************************************************************
 * Returns the total number of patients in the table, the average number of patients in 
 * a table entry and standard deviation of an table entry. 
 * This function is used to check if the hash function is good enough (i.e. distributes 
 * the patients equally over the entries (i.e. small standard deviation)).
 * 
 */
void GetHashPerformance(size_t *totalNumberOfPatients, double *averageNumberOfPatients,
                        double *standardDeviation);

#endif
