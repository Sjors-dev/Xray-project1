#ifndef DOSEADMIN_H
#define DOSEADMIN_H
#include <stdint.h>
#include <stddef.h>


#define MAX_PATIENTNAME_SIZE	(80)
#define HASHTABLE_SIZE			(256)


/*************************************************************************************** 
 * Creates and initializes the patient dose administration. No patient data will be 
 * present after creation
 *  
 */
void CreatePatientDoseAdmin(); 
					   

/***************************************************************************************
 * Removes all patient data from the administration, but remark that JohnDoe must remain
 * present (with no dose data)
 * 
 */
void RemoveAllDataFromPatientDoseAdmin();


/***************************************************************************************
 * Adds the patient in the administration
 *
 * Returns -1 when the passed patientName is already present 
 * Returns -2 when allocation of memory failed
 * Returns -3 when string length of patientName exceeds MAX_PATIENTNAME_SIZE
 * Returns  0 when the patient is successfully added
 * 
 * It is a precondition that patientName is not NULL and is \0 terminated
 */
int8_t AddPatient(char * patientName);


/***************************************************************************************
 * Selects the patient as the active patient (selected patient)
 *
 * Returns -1 when the passed patientName is not present 
 * Returns -2 when string length of patientName exceeds MAX_PATIENTNAME_SIZE
 * Returns  0 when the patient is successfully selected
 * 
 * It is a precondition that patientName is not NULL and is \0 terminated
 */
int8_t SelectPatient(char * patientName);


typedef struct {
	uint8_t   day;    // value in range [1, 31]
	uint8_t   month;  // value in range [1, 12]
	uint16_t  year;   // value in range [1900, 2500]
} Date;

/***************************************************************************************
 * Adds the dose a patient received during an examination at a particular date in 
 * the administration. Dose is added to the selected patient.
 * 
 * Returns -1 when allocation of memory failed
 * Returns -2 when string length of patientName exceeds MAX_PATIENTNAME_SIZE
 * Returns  0 when the data is successfully copied into the administration
 * 
 * It is also a precondition that date is not NULL
 * It is also a precondition that dose is not 0
 */
int8_t AddPatientDose(Date* date, uint16_t dose);


/***************************************************************************************
 * Returns the total dose a patient received in passed period.
 * 
 * Returns -1 when the passed patientName is unknown
 * Returns -2 when string length of patientName exceeds MAX_PATIENTNAME_SIZE
 * Returns  0 when the totalDose is  updated successfully
 * 
 * It is a precondition that patientName is not NULL and is \0 terminated
 * It is also a precondition that both dates and totalDose are not NULL
 */
int8_t PatientDoseInPeriod(char * patientName, 
                           Date* startDate, Date* endDate, uint32_t* totalDose);


/***************************************************************************************
 * Removes the patient from the administration
 * 
 * Returns -1 when the passed patientName is not present 
 * Returns -2 when string length of patientName exceeds MAX_PATIENTNAME_SIZE
 * Returns  0 when the patient data is successfully removed from the administration
 * 
 * It is a precondition that patientName is not NULL and is \0 terminated
 */
int8_t RemovePatient(char * patientName);


/***************************************************************************************
 * Checks if the passed patientName is present in the administration
 * 
 * Returns -1 when the passed patientName is not present 
 * Returns -2 when string length of patientName exceeds MAX_PATIENTNAME_SIZE
 * Returns 0 when the patientName is present
 * 
 * It is a precondition that patientName is not NULL and is \0 terminated
 */
int8_t IsPatientPresent(char * patientName);


/***************************************************************************************
 * Returns the number of dose measurements done for a given patient
 *
 * Returns -1 when the passed patientName is not present 
 * Returns -2 when string length of patientName exceeds MAX_PATIENTNAME_SIZE
 * Returns 0 otherwise, the number of measurements is put in nrOfMeasurements
 * 
 * It is a precondition that patientName is not NULL and is \0 terminated
 */
int8_t GetNumberOfMeasurements(char * patientName, 
                               size_t * nrOfMeasurements);
								

/***************************************************************************************
 * Writes all patient data to a text file in the table
 * 
 * Returns 0 on success
 * Returns -1 on faillure
 */
int8_t WriteToFile(char * filePath);



/***************************************************************************************
 * Reads all patient data from a text file, and put the data in an empty table.
 * 
 * Returns 0 on success
 * Returns -1 on faillure
 */
int8_t ReadFromFile(char * filePath);

#endif
