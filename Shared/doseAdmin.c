#include "doseAdmin.h"
#include "doseAdmin_internal.h"

uint8_t hashFunction(char * patientName)
{
	// add here the code of your hash function
	return 0;
}

void * getHashTable() 
{
	// add here the code that returns the entry to your hash table
	return NULL;
}

void CreatePatientDoseAdmin()
{
	 return;
}
 
void RemoveAllDataFromPatientDoseAdmin()
{
	 return;
}
 
int8_t AddPatient(char * patientName)
{
	 return -1;
}

int8_t SelectPatient(char * patientName)
{
	 return -1;
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
	 return -1;
}
 
int8_t IsPatientPresent(char * patientName)
{
	 return -1;
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
