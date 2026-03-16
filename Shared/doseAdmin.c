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
    AddPatient("Jan de Vries", 45, 80); 

    AddPatient("Sofie Jansen", 28, 75);
    AddPatient("Sofie Janssen", 34, 70); 

    AddPatient("Mark van Dijk", 45, 80);
    AddPatient("Mark van Dyke", 41, 78); 

    AddPatient("Lisa Bakker", 36, 55);
    AddPatient("Liza Bakker", 30, 58); 

    AddPatient("Tom Visser", 50, 90);
    AddPatient("Thomas Visser", 27, 65);

    AddPatient("Emma Willems", 22, 60);
    AddPatient("Emma Willems", 29, 62); 

    AddPatient("Joris Peters", 40, 70);
    AddPatient("Joris Peeters", 38, 72); 

    AddPatient("Nina de Jong", 29, 65);
    AddPatient("Nina Jong", 31, 66); 

    AddPatient("Pieter Mulder", 33, 85);

    AddPatient("Lotte van Leeuwen", 31, 72);
    AddPatient("Lotte Leeuwen", 26, 68); 
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
        free(hashTable[i]); // dan free patient
    }
    return;
}

// print print print print
void PrintHashTable()
{
    testHashFunction();    // (spreiding van hashen testen)
    bool emptyBool = true;
    printf("Table Start: \n");
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
    Patient *matches[20]; // array van max 20 resultaten
    int matchCount = 0;     

    for (int i = 0; i < HASHTABLE_SIZE; i++)
    {
        if (hashTable[i] != NULL)
        {
            if (strstr(hashTable[i]->name, patientName) != NULL)        //strstr zoekt strings in strings. dus hij zoekt voor patientname (input) in de hashtable array.
            {
                matches[matchCount] = hashTable[i];
                matchCount++;
            }
        }
    }

    if (matchCount == 0)
    {
        return NULL;
    }

    if (matchCount == 1)
    {
        return matches[0];
    }

    printf("Meerdere patiënten gevonden:\n");

    for (int i = 0; i < matchCount; i++)        //loop door aantal matchcounts (dus matches)
    {
        printf("[%d] %s (age %d)\n", i + 1, matches[i]->name, matches[i]->age);     //print naam en leeftijd van match
    }

    int choice;
    printf("Kies patiënt: ");
    scanf("%d", &choice);

    if (choice < 1 || choice > matchCount)
    {
        return NULL;
    }

    return matches[choice - 1];
}

int8_t AddPatientDose(char *patientName, Date *date, uint16_t dose)
{
    if (IsPatientPresent(patientName) == 1)
    { // bestaat patient?
        Patient *tmp = SelectPatient(patientName);
        if (tmp == NULL)
            return -1;

        Date *dateCopy = malloc(sizeof(Date)); // maak pointer aan voor de date van nu. allocate memory voor grootte hiervan
        if (!dateCopy)
            return -2;

        *dateCopy = *date; // copy the contents, not the pointer

        // free vorige date als die bestond
        if (tmp->doseDate != NULL)
        {
            free(tmp->doseDate);
        }

        tmp->doseage = dose;
        tmp->doseDate = dateCopy;
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
    free(hashTable[index]->doseDate); // free date eerst
    free(hashTable[index]);           // dan free patient
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
