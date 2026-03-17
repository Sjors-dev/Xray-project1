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



//even testen :p
void testHashFunction()
{
    AddPatient("Jan de Vries", 32);
    AddPatient("Jan de Vries", 45); 

    AddPatient("Sofie Jansen", 28);
    AddPatient("Sofie Janssen", 34); 

    AddPatient("Mark van Dijk", 45);
    AddPatient("Mark van Dyke", 41); 

    AddPatient("Lisa Bakker", 36);
    AddPatient("Liza Bakker", 30); 

    AddPatient("Tom Visser", 50);
    AddPatient("Thomas Visser", 27);

    AddPatient("Emma Willems", 22);
    AddPatient("Emma Willems", 29); 

    AddPatient("Joris Peters", 40);
    AddPatient("Joris Peeters", 38); 

    AddPatient("Nina de Jong", 29);
    AddPatient("Nina Jong", 31); 

    AddPatient("Pieter Mulder", 33);

    AddPatient("Lotte van Leeuwen", 31);
    AddPatient("Lotte Leeuwen", 26); 
}

void *getHashTable()
{
    // hier is je hashtable, geen probleem
    return hashTable;
}

void CreatePatientDoseAdmin()
{
    // alles null en een johndoe toevoegen want waarom niet? hell yeah
    RemoveAllDataFromPatientDoseAdmin();
    AddPatient("John Doe", 18);
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
    printf("Table Start: \n");
    for (int i = 0; i < HASHTABLE_SIZE; i++)
    {
        if (hashTable[i] != NULL)
        {     
            printf("\t%i\t%s\n", i, hashTable[i]->name);
            printf("\t| \n");
            
        }
    }
}

int8_t AddPatient(char *patientName, int patientAge)
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
    { 
        Patient *tmp = SelectPatient(patientName);
        if (tmp == NULL){
            return -1;
        }

        if (tmp->doseCount >= MAX_DOSES){
            printf("Max doses bereikt");
            return -1;
        }
        


        Date *dateCopy = malloc(sizeof(Date)); // maak pointer aan voor de date van nu. allocate memory voor grootte hiervan
        if (!dateCopy)
        return -2;

        *dateCopy = *date; // copy the contents, not the pointer

        tmp->dosages[tmp->doseCount].dose = dose;           //in tmp doseages (array) pak de eerste lege plek. (bijgehouden door dosecount) en pas dose aan
        tmp->dosages[tmp->doseCount].doseDate = dateCopy;
        tmp->doseCount++;
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
    if(!IsPatientPresent(patientName)){
        return -1;
    }
    uint8_t index = 0;
    index = hashFunction(patientName);
    hashTable[index] = NULL;
     free(hashTable[index]);           //free patient x

    return -1;
}

int8_t IsPatientPresent(char *patientName)
{
    uint8_t index = hashFunction(patientName);
    if (hashTable[index] != NULL && strcmp(hashTable[index]->name, patientName) == 0) //hashtable index is niet leeg. en hashtable index name is zelfde als inputnaam
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
