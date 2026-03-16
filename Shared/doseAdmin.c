#include "doseAdmin.h"
#include "doseAdmin_internal.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

Patient *hashTable[HASHTABLE_SIZE]; // echte definitie
void PrintHashTable(void);



// hashen :))
uint8_t hashFunction(char *patientName)
{
    unsigned int hash_value = 0;

    for (int i = 0; patientName[i] != '\0'; i++)
    {
        hash_value = hash_value * 31 + patientName[i]; // beter verdeeld
    }

    return hash_value % HASHTABLE_SIZE; // pas modulo toe voor de tabelgrootte
}

void testHashFunction()
{
    AddPatient("Jan de Vries", 32, 67);
    AddPatient("Sofie Jansen", 28, 75);
    AddPatient("Mark van Dijk", 45, 80);
    AddPatient("Lisa Bakker", 36, 55);
    AddPatient("Tom Visser", 50, 90);
    AddPatient("Emma Willems", 22, 60);
    AddPatient("Joris Peters", 40, 70);
    AddPatient("Nina de Jong", 29, 65);
    AddPatient("Pieter Mulder", 33, 85);
    AddPatient("Lotte van Leeuwen", 31, 72);
}

void *getHashTable()
{
    // hier is je hashtable geen probleem
    return hashTable;
}

void CreatePatientDoseAdmin()
{
    // alles null en een johndoe toevoegen want waarom niet? hell yeah
    RemoveAllDataFromPatientDoseAdmin();
    AddPatient("John Doe", 18, 0);
}

void RemoveAllDataFromPatientDoseAdmin()
{
    // alles verwijderen...? rip patienten i guess
    for (int i = 0; i < HASHTABLE_SIZE; i++)
    {
        hashTable[i] = NULL;
    }
    return;
}

// print print print print
void PrintHashTable()
{
   //testHashFunction();     (spreiding van hashen testen)
    bool emptyBool = false;
    printf("Table: \n");
    for (int i = 0; i < HASHTABLE_SIZE; i++)
    {
        if (hashTable[i] == NULL)
        {
            if (emptyBool == false)
            {
                printf("\t%i Start\n ", i);
                emptyBool = true;
            }
        }
        else
        {
            if (emptyBool == true)
            {
                printf("\t| \n");
            }
            printf("\t%i\t%s\n", i, hashTable[i]->name);
        }
    }
}

int8_t AddPatient(char *patientName, int patientAge, int patientDoseage)
{
    // check lengte en uniekheid
    if (patientName == NULL)
        return -3;
    if (strlen(patientName) > MAX_PATIENTNAME_SIZE)
        return -3;
    int checkPresent = IsPatientPresent(patientName);

    uint8_t index = hashFunction(patientName);
    uint8_t startIndex = index;

    // maak hash index aan
    while (hashTable[index] != NULL)
    {
        index = (index + 1) % HASHTABLE_SIZE;
        if (index == startIndex)
            return -4; // hele table vol :(
    }

    // niet uniek
    if (checkPresent == -1)
    {
        printf("Patient bestaat al in tabel? wtf loll");
        return -1;
    }

    // hij is uniek!
    else
    {

        Patient *p = malloc(sizeof(Patient)); // memory allocate op basis van grootte struct

        if (!p)
            return -2; // ehh geen patient?

        strncpy(p->name, patientName, (MAX_PATIENTNAME_SIZE - 1)); // kopieer struct op basis van input name

        p->age = patientAge; // sla leeftijd en shit op
        p->doseDate = NULL;  // add this
        p->doseage = patientDoseage;

        hashTable[index] = p; // sla op :)
    }
    return 0;
}

// select een patient :)
Patient *SelectPatient(char *patientName)
{
    uint8_t index = hashFunction(patientName);
    uint8_t startIndex = index;

    while (hashTable[index] != NULL)
    {
        if (strcmp(hashTable[index]->name, patientName) == 0)
        {
            return hashTable[index];
        }
        index = (index + 1) % HASHTABLE_SIZE;
        if (index == startIndex) break; // hele tabel doorlopen
    }
    return NULL;
}

int8_t AddPatientDose(char *patientName, Date *date, uint16_t dose)
{
    if(IsPatientPresent(patientName) == 1){
        Patient *tmp = SelectPatient(patientName);
        if (tmp == NULL) return -1;

        Date *dateCopy = malloc(sizeof(Date));  // heap allocation
        if (!dateCopy) return -2;

        *dateCopy = *date;  // copy the contents, not the pointer

        // free previous date if one existed
        if (tmp->doseDate != NULL) {
            free(tmp->doseDate);
        }

        tmp->doseage = dose;
        tmp->doseDate = dateCopy;  // safe: persists after function returns
    

    }
    
    return 1;
}

int8_t PatientDoseInPeriod(char *patientName,
                           Date *startDate, Date *endDate, uint32_t *totalDose)
{
    return -1;
}

int8_t RemovePatient(char *patientName)
{
    uint8_t index = 0;
    index = hashFunction(patientName);
    free(hashTable[index]->doseDate);  // free date first
    free(hashTable[index]);            // then free patient
    hashTable[index] = NULL;

    return -1;
}

int8_t IsPatientPresent(char *patientName)
{

    uint8_t index = hashFunction(patientName);

    if (hashTable[index] != NULL && strcmp(hashTable[index]->name, patientName) == 0)
    {
        return 1;
    }

    return 0;
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
