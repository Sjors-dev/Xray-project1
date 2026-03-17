#ifndef DOSEADMIN_INTERNAL_H
#define DOSEADMIN_INTERNAL_H


#define HASHTABLE_SIZE			(256)
#define MAX_DOSES (10)

typedef struct {
    int year;
    int month;
    int day;
} Date;

typedef struct {
    int dose;
    char doseType[256];
    Date *doseDate;
} Dosage;

typedef struct Patient {
    char name[256];
    int age;
    Dosage dosages[MAX_DOSES]; // lijst van dosages
    int doseCount;             // hoeveel er gevuld zijn
} Patient;

extern Patient * hashTable[HASHTABLE_SIZE];


//hashfunction!!
uint8_t hashFunction(char * patientName);

//print print print
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
