#include <stdio.h>
#include "menu.h"
#include <fcntl.h>
#include "doseAdmin.h"
#include "CentralAcquisitionProxy.h"


//vars
#define MAX_NAME 256

#include <stdint.h>
void* PrintHashTable(void);

bool sendMessageToCentralAcquisition(const char* msg);



typedef enum {
	NOT_CONNECTED_WITH_CENTRAL_ACQUISITION, 
	CONNECTED_WITH_CENTRAL_ACQUISITION
} CENTRAL_ACQUISITION_CONNECTION_STATE;

/*---------------------------------------------------------------*/
int main(int argc, char* argv[])
{
	static CENTRAL_ACQUISITION_CONNECTION_STATE centralAcqConnectionState = NOT_CONNECTED_WITH_CENTRAL_ACQUISITION;
	if (connectWithCentralAcquisition()) {	
		centralAcqConnectionState = CONNECTED_WITH_CENTRAL_ACQUISITION;
	}
	else {
		printf("\n\nConnecting with CentralAcquisition Failed. No problem, you can continue with \n");
		printf("the functionality that does not depend on that connection!\n");
	}
	
	 
	char selectedPatient[MAX_PATIENTNAME_SIZE] = "JohnDoe";
	(void) selectedPatient; // remove this line when you are doing something with selectedPatient
	// add here the code that adds John Doe into the admin
	
	displayMenu();	
	while (true) {  
        MenuOptions choice = getMenuChoice();
		if (choice == -1) {
			if (centralAcqConnectionState == CONNECTED_WITH_CENTRAL_ACQUISITION) {
				uint32_t doseData;
				if (getDoseDataFromCentralAcquisition(&doseData)) {
					//printf("Received dose: %d\n", doseData); // instead of this print call here the function that handles the received dose datawqt
				}
			}
		}
		else {
			switch (choice)
			{
			case MO_ADD_PATIENT:{
			    char inputName[MAX_NAME];
				int inputAge;

				printf("Patient name: ");
				scanf("%s", inputName);
				printf("Patient age: ");
				scanf("%d", &inputAge);
				AddPatient(inputName, inputAge);
				    sendMessageToCentralAcquisition("led");
				break;
				}
			case MO_DELETE_PATIENT:
				// add here your delete patient code
				break;
			case MO_SHOW_TABLE:
				    PrintHashTable();
				break;
			case MO_SELECT_PATIENT:
			{
				char inputName[MAX_NAME];

				printf("Patient name: ");
				scanf("%s", inputName);
				Patient *tmp = SelectPatient(inputName);
					if(tmp == NULL){
						printf("Niks gevonden man");
					}
					else{
						printf("Patient: %s gevonden \n", tmp->name);
						printf("Leeftijd: %d", tmp->age);
					}
				
				break;
				}
			case MO_SELECT_EXAMINATION_TYPE:
			    if (centralAcqConnectionState == CONNECTED_WITH_CENTRAL_ACQUISITION) {	
					// add here your select examination code
				}
				else {
					printf("This option is only valid when connected with CentralAcquisition\n");
				}
				break;
			case MO_QUIT:
				if (centralAcqConnectionState == CONNECTED_WITH_CENTRAL_ACQUISITION) {
					disconnectFromCentralAcquisition();
				}
				centralAcqConnectionState = NOT_CONNECTED_WITH_CENTRAL_ACQUISITION;
				return 0;
				break;
			default:
				printf("Please, enter a valid number! %d\n", choice);
				break;
			}
			displayMenu();
		}
    }
	return 0;
}
