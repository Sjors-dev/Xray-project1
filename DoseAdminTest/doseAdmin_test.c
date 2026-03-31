#include <string.h>
#include "doseAdmin.h"
#include "doseAdmin_internal.h"
#include "unity.h"
#include <stdlib.h>

// I rather dislike keeping line numbers updated, so I made my own macro to ditch the line number
#define MY_RUN_TEST(func) RUN_TEST(func, 0)

void setUp(void)
{
    // This is run before EACH test
}

void tearDown(void)
{
    // This is run after EACH test
}



void addPatient_WhenNameTooLong_ThenReturnMinus3(void)
{
    CreatePatientDoseAdmin();
    TEST_ASSERT_EQUAL(-3, AddPatient("12345678901234567890123456789012345678901234567890123456789012345678901234567890x", 666));
}

void addPatient_WhenPatientAlreadyPresent_ThenReturnMinus1(void)
{
    CreatePatientDoseAdmin();
    TEST_ASSERT_EQUAL(-1, AddPatient("JohnDoe", 666));
}
void addPatient_WhenInputOk_ThenReturnZeroAndPatientIsAdded(void)
{
    CreatePatientDoseAdmin();
    TEST_ASSERT_EQUAL(0, AddPatient("flip", 666));

    int index = hashFunction("flip");
    Patient** temp = (Patient**) getHashTable();
    TEST_ASSERT_EQUAL_STRING("flip", temp[index]->name);
}

// add here all your dose admin testcases, and call them in main!! 
// Remove the given testcases, they were only added to check if everything is up and running

int main()
{
    UnityBegin();

    MY_RUN_TEST(addPatient_WhenNameTooLong_ThenReturnMinus3);
    MY_RUN_TEST(addPatient_WhenPatientAlreadyPresent_ThenReturnMinus1);
    MY_RUN_TEST(addPatient_WhenInputOk_ThenReturnZeroAndPatientIsAdded);

    UnityEnd();
}
