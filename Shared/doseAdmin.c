#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include "doseAdmin.h"
#include "doseAdmin_internal.h"

static Patient *hashTable[HASHTABLE_SIZE]; // echte definitie

static Patient *FindPatientInBucket(uint8_t index, char *patientName)
{
    // Loop door 1 bucket-lijst en pak de eerste exacte naam-match.
    Patient *current = hashTable[index];
    while (current != NULL)
    {
        if (strcmp(current->name, patientName) == 0)
        {
            return current;
        }
        current = current->next;
    }

    return NULL;
}

// hashen
uint8_t hashFunction(char *patientName)
{
    uint16_t len = strlen(patientName);
    uint32_t hash_value = 0;

    for (int i = 0; i < len; i++)
    {
        hash_value = hash_value * 31 + patientName[i]; // beter verdeeld, met priemgetal
    }

    return hash_value % HASHTABLE_SIZE; // pas modulo toe voor de tabelgrootte (geeft restwaarde)
}

void *GetHashTable()
{
    return hashTable;
}

void CreatePatientDoseAdmin()
{
    // alles null en een johndoe toevoegen
    RemoveAllDataFromPatientDoseAdmin();
    AddPatient("John Doe");

    // set jon doe als selected patient
}

void RemoveAllDataFromPatientDoseAdmin()
{
    for (int i = 0; i < HASHTABLE_SIZE; i++)
    {
        //Check nexts en zet null
        Patient *current = hashTable[i];
        while (current != NULL)
        {
            Patient *next = current->next;
            free(current);
            current = next;
        }
        hashTable[i] = NULL;
    }
}



int8_t AddPatient(char *patientName)
{
    if (patientName == NULL)
        return -3;
    if (strlen(patientName) > MAX_PATIENTNAME_SIZE)
        return -3;

    uint8_t index = hashFunction(patientName);
    if (FindPatientInBucket(index, patientName) != NULL)
    {
        return -1;
    }

    Patient *p = calloc(1, sizeof(Patient)); // memory allocate op basis van grootte struct

    if (!p)
        return -2; 

    strncpy(p->name, patientName, (MAX_PATIENTNAME_SIZE - 1)); // kopieer struct op basis van input name

    // Zet nieuwe patient vooraan in de bucket.
    p->next = hashTable[index];
    hashTable[index] = p; // sla op

    return 0;
}


// select een patient
Patient *SelectPatient(char *patientName) // geen return van patient ptr
{
    if (patientName == NULL)
    {
        return NULL;
    }

    uint8_t index = hashFunction(patientName);
    return FindPatientInBucket(index, patientName);
}

int8_t AddPatientDose(char *patientName, Date *date, uint16_t dose)   // geen patientName
{
    if (IsPatientPresent(patientName) == 1)
    {
        Patient *tmp = SelectPatient(patientName);
        if (tmp == NULL)
        {
            return -1;
        }

        if (tmp->doseCount >= MAX_DOSES)
        {
            printf("Max doses bereikt :(");
            return -1;
        }

        tmp->dosages[tmp->doseCount].dose = dose;      // in tmp (pointer naar patient) doseages (array) pak de eerste lege plek (bijgehouden door dosecount) en pas dose aan (in dus die lege plek)
        tmp->dosages[tmp->doseCount].doseDate = *date; // gewoon kopiëren
        tmp->doseCount++;

        return 0;
    }

    return -1;
}

int8_t PatientDoseInPeriod(char *patientName, Date *startDate, Date *endDate, uint32_t *totalDose)
{
    return -1;
}

int8_t RemovePatient(char *patientName)
{
    if (patientName == NULL)
    {
        return -1;
    }

    uint8_t index = hashFunction(patientName);
    Patient *current = hashTable[index];
    Patient *previous = NULL;

    
    while (current != NULL)
    {
        if (strcmp(current->name, patientName) == 0)
        {
            if (previous == NULL)
            {
                hashTable[index] = current->next;
            }
            else
            {
                previous->next = current->next;
            }

            free(current);
            return 0;
        }

        previous = current;
        current = current->next;
    }

    return -1;
}

int8_t IsPatientPresent(char *patientName)
{
    return SelectPatient(patientName) != NULL; // hashtable index is niet leeg. en hashtable index name is zelfde als inputnaam
}

int8_t GetNumberOfMeasurements(char *patientName,
                               size_t *nrOfMeasurements)
{
    return -1;
}

void GetHashPerformance(size_t *totalNumberOfPatients, double *averageNumberOfPatients,
                        double *standardDeviation)
{
    return;
}

int8_t WriteToFile(char *filePath)
{
    return -1;
}

int8_t ReadFromFile(char *filePath)
{
    return -1;
}
