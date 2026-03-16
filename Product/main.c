#include <stdio.h>
#include "menu.h"
#include <fcntl.h>
#include "doseAdmin.h"
#include "CentralAcquisitionProxy.h"
#include <stdint.h>
#include <string.h>
#include <time.h>

#define MAX_NAME 256

void *PrintHashTable(void);
Patient *selected = NULL;

typedef enum
{
    NOT_CONNECTED_WITH_CENTRAL_ACQUISITION,
    CONNECTED_WITH_CENTRAL_ACQUISITION
} CENTRAL_ACQUISITION_CONNECTION_STATE;

CENTRAL_ACQUISITION_CONNECTION_STATE initConnection()
{
    if (connectWithCentralAcquisition())
    {
        return CONNECTED_WITH_CENTRAL_ACQUISITION;
    }

    printf("\n\nConnecting with CentralAcquisition Failed. No problem, you can continue with \n");
    printf("the functionality that does not depend on that connection!\n");

    return NOT_CONNECTED_WITH_CENTRAL_ACQUISITION;
}

void userInputName(char *buffer)
{
    printf("Input name: \n");
    fgets(buffer, MAX_NAME, stdin);
    buffer[strcspn(buffer, "\n")] = 0;
}

void handleDoseReception(CENTRAL_ACQUISITION_CONNECTION_STATE state)
{
    if (state == CONNECTED_WITH_CENTRAL_ACQUISITION)
    {
        uint32_t doseData;
        if (getDoseDataFromCentralAcquisition(&doseData))
        {
            // handle dose here
        }
    }
}

void handleAddPatient()
{
    char inputName[MAX_NAME];
    int inputAge;

    userInputName(inputName);

    printf("Patient age: ");
    scanf("%d", &inputAge);

    printf("Patient dosage: ");

    AddPatient(inputName, inputAge, 0);
}

void handleDeletePatient()
{
    char inputName[MAX_NAME];
    int inputChoise;

    userInputName(inputName);

    Patient *tmp = SelectPatient(inputName);

    if (tmp == NULL)
    {
        printf("patient not found, check spelling.");
        return;
    }

    printf("Are you sure you want to delete %s? \n Age: %d \n[0] Yes \n[1] No \n", tmp->name, tmp->age);
    scanf("%d", &inputChoise);

    if (inputChoise == 0)
    {
        RemovePatient(inputName);
        tmp = SelectPatient(inputName);

        if (tmp == NULL)
        {
            printf("Patient deleted successfully\n");
        }
        else
        {
            printf("Something went wrong.. Try again.\n");
        }
    }
    else if (inputChoise == 1)
    {
        printf("Canceled deletion.\n");
    }
    else
    {
        printf("Input not recognised.\n");
    }
}

void handleSelectPatient()
{
    char inputName[MAX_NAME];

    userInputName(inputName);

    selected = SelectPatient(inputName);
    if (selected == NULL)
    {
        printf("Niks gevonden man");
    }
    else
    {
        printf("------------------------------------------------------ \n");
        printf("|                Patient selected! \n");
        printf("|\n");
        printf("|                Patient: %s \n", selected->name);
        printf("|                Leeftijd: %d \n", selected->age);
        printf("|                Dosage: %d \n", selected->doseage);
        
        if (selected->doseDate != NULL)
{
    printf("|                Dosage Date: %d-%02d-%02d \n", 
           selected->doseDate->year, 
           selected->doseDate->month, 
           selected->doseDate->day);
}
else
{
    printf("|                Dosage Date: geen datum \n");
}
        printf("------------------------------------------------------ \n");
    }
}

void handleSelectExam(CENTRAL_ACQUISITION_CONNECTION_STATE state)
{
   
    //inputs
    int inputExamType;
    int inputDose;
    
     if (selected == NULL)
    {
        printf("Geen patient geselecteerd! Selecteer eerst een patient.\n");
        return;
    }


    if (state == NOT_CONNECTED_WITH_CENTRAL_ACQUISITION)
    {
        printf("\nSelecteer onderzoekstype:\n");
        printf(" [0] Single Shot\n");
        printf(" [1] Series\n");
        printf(" [2] Series with Motion\n");
        printf(" [3] Fluoro\n");
        printf(" [4] None\n");
        printf("Keuze: ");

        scanf("%d", &inputExamType);

        //binnen de opties ofc
        if (inputExamType >= 0 && inputExamType <= 4)
        {
            printf("Input dose amount: ");
            scanf("%d", &inputDose);
            
            // get Date
            time_t now = time(NULL); //time_t zit in time.h library. time(NULL) is de tijd nu.
            struct tm *t = localtime(&now);     //data struct naar leesbare text

            Date date;          //variabele date aangemaakt. (net als entry in hashtable)

            date.year = t->tm_year + 1900;          //date dingen opslaan
            date.month = t->tm_mon + 1;
            date.day = t->tm_mday;

            AddPatientDose(selected->name, &date, inputDose);       //add patient met dose en dingen bij geselecteerde patient
            selectExaminationType((EXAMINATION_TYPES)inputExamType);    //select en stuur naar de central acquisition.
            printf("Onderzoekstype %d verzonden.\n", inputExamType);
        }

        
        else
        {
            printf("Invalid Choice.\n");
        }

        return;
    }

    printf("Central Acquisition not connected!!");
}

int handleQuit(CENTRAL_ACQUISITION_CONNECTION_STATE *state)
{
    if (*state == CONNECTED_WITH_CENTRAL_ACQUISITION)
    {
        disconnectFromCentralAcquisition();
    }

    *state = NOT_CONNECTED_WITH_CENTRAL_ACQUISITION;
    return 0;
}

void checkSelected()
{
    if (selected == NULL)
    {
        printf("\nCouldn't select John Doe..\n\n");
    }
    else
    {
        printf("\nHash Table created, John Doe selected!\n\n");
    }
}

int main(int argc, char *argv[])
{
    CreatePatientDoseAdmin();
    selected = SelectPatient("John Doe"); // selecteer een patient!
    checkSelected();

    CENTRAL_ACQUISITION_CONNECTION_STATE centralAcqConnectionState = initConnection();

    // char selectedPatient[MAX_PATIENTNAME_SIZE] = "JohnDoe";
    //(void)selectedPatient;

    displayMenu();

    while (true)
    {
        MenuOptions choice = getMenuChoice();

        if (choice == -1)
        {
            handleDoseReception(centralAcqConnectionState);
        }
        else
        {
            switch (choice)
            {
            case MO_ADD_PATIENT:
                handleAddPatient();
                break;

            case MO_DELETE_PATIENT:
                handleDeletePatient();
                break;

            case MO_SHOW_TABLE:
                PrintHashTable();
                break;

            case MO_SELECT_PATIENT:
                handleSelectPatient();
                break;

            case MO_SELECT_EXAMINATION_TYPE:

                handleSelectExam(centralAcqConnectionState);
                break;

            case MO_QUIT:
                return handleQuit(&centralAcqConnectionState);

            default:
                printf("Please, enter a valid number! %d\n", choice);
                break;
            }

            displayMenu();
        }
    }

    return 0;
}