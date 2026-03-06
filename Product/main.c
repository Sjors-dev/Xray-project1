#include <stdio.h>
#include "menu.h"
#include <fcntl.h>
#include "doseAdmin.h"
#include "CentralAcquisitionProxy.h"
#include <stdint.h>

#define MAX_NAME 256

void *PrintHashTable(void);

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

    printf("Patient name: ");
    scanf("%s", inputName);

    printf("Patient age: ");
    scanf("%d", &inputAge);

    AddPatient(inputName, inputAge);
}



void handleDeletePatient()
{
    char inputName[MAX_NAME];
    int inputChoise;

    printf("Patient name to delete:");
    scanf("%s", inputName);

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

    printf("Patient name: ");
    scanf("%s", inputName);

    Patient *tmp = SelectPatient(inputName);

    if (tmp == NULL)
    {
        printf("Niks gevonden man");
    }
    else
    {
        printf("Patient: %s gevonden \n", tmp->name);
        printf("Leeftijd: %d", tmp->age);
    }
}



void handleSelectExam(CENTRAL_ACQUISITION_CONNECTION_STATE state)
{
    int inputExamType;

    if (state == CONNECTED_WITH_CENTRAL_ACQUISITION)
    {
        printf("\nSelecteer onderzoekstype:\n");
        printf(" [0] Single Shot\n");
        printf(" [1] Series\n");
        printf(" [2] Series with Motion\n");
        printf(" [3] Fluoro\n");
        printf(" [4] None\n");
        printf("Keuze: ");

        scanf("%d", &inputExamType);

        if (inputExamType >= 0 && inputExamType <= 4)
        {
            selectExaminationType((EXAMINATION_TYPES)inputExamType);
            printf("Onderzoekstype %d verzonden.\n", inputExamType);
        }
        else
        {
            printf("Invalid Choice.\n");
        }
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



int main(int argc, char *argv[])
{
    CENTRAL_ACQUISITION_CONNECTION_STATE centralAcqConnectionState = initConnection();

    char selectedPatient[MAX_PATIENTNAME_SIZE] = "JohnDoe";
    (void)selectedPatient;

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