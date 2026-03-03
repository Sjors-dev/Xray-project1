#include "doseAdmin.h"
#include "doseAdmin_internal.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

Patient* hashTable[HASHTABLE_SIZE]; // echte definitie
void PrintHashTable(void);

//hashen :))
uint8_t hashFunction(char * patientName)
{
	int length = strlen(patientName);    
	unsigned int hash_value = 0;

    for (int i = 0; i < length; i++){
        hash_value = patientName[i];
        hash_value = ((hash_value * patientName[i]) + patientName[i]) % HASHTABLE_SIZE;
    }

    return hash_value;
}

void * getHashTable() 
{
    return hashTable;
}

void CreatePatientDoseAdmin()
{
    for (int i = 0; i < HASHTABLE_SIZE; i++){
        hashTable[i] = NULL;
    }
}
 
void RemoveAllDataFromPatientDoseAdmin()
{
	 return;
}
 
void PrintHashTable()
{
    bool emptyBool = false;
    printf("Table: \n");
    for (int i = 0; i < HASHTABLE_SIZE; i++){
        if (hashTable[i] == NULL){
            if(emptyBool == false){
                printf("\t%i ^\n ", i);
                emptyBool = true;
            }
        }
        else{
            if(emptyBool == true){
                printf("\t| \n");
              
            }
            printf("\t%i\t%s\n", i, hashTable[i]->name);
        }   
    }
}


int8_t AddPatient(char * patientName, int patientAge)
{
    if (patientName == NULL) return -1;

    uint8_t index = hashFunction(patientName);
    uint8_t startIndex = index;

    while (hashTable[index] != NULL) {
        index = (index + 1) % HASHTABLE_SIZE; 
        if (index == startIndex) return -1; // hele table vol :(
    }

    Patient *p = malloc(sizeof(Patient));
    if (!p) return -1;

    strcpy(p->name, patientName);
    p->age = patientAge;

    hashTable[index] = p;
    return 0;
}


//select een patient :)
Patient* SelectPatient(char * patientName)
{
    int index = hashFunction(patientName);
    if(hashTable[index] != NULL &&
       strcmp(hashTable[index]->name, patientName) == 0){
        return hashTable[index];
    }
    return NULL;
}
 
int8_t AddPatientDose(Date* date, uint16_t dose)
{
	 return -1;
}
 
int8_t PatientDoseInPeriod(char * patientName, 
                           Date* startDate, Date* endDate, uint32_t* totalDose)
{
	 return -1;
}
 
int8_t RemovePatient(char * patientName)
{
    uint8_t index = 0;
    index = hashFunction(patientName);
    hashTable[index] = NULL;
    

	 return -1;
}
 
int8_t IsPatientPresent(char * patientName)
{
    uint8_t index = hashFunction(patientName);

    if (hashTable[index] != NULL &&
        strcmp(hashTable[index]->name, patientName) == 0){
        return 1;
    }

    return 0;
}

int8_t GetNumberOfMeasurements(char * patientName, 
                               size_t * nrOfMeasurements)
{
	 return -1;
}

void GetHashPerformance(size_t *totalNumberOfPatients, double *averageNumberOfPatients,
                        double *standardDeviation)
{
	 return;
}
				
int8_t WriteToFile(char * filePath)
{
	 return -1;
}

int8_t ReadFromFile(char * filePath)
{
	 return -1;
}
