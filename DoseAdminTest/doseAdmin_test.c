#include <string.h>
#include "doseAdmin.h"
#include "doseAdmin_internal.h"
#include "unity.h"
#include <stdlib.h>

#define MY_RUN_TEST(func) RUN_TEST(func, 0)

void setUp(void)
{
    // This is run before EACH test
}

void tearDown(void)
{
    // This is run after EACH test
}


// AddPatient


void addPatient_WhenNameTooLong_ThenReturnMinus3(void)
{
    CreatePatientDoseAdmin();
    TEST_ASSERT_EQUAL(-3, AddPatient("12345678901234567890123456789012345678901234567890123456789012345678901234567890x"));
}

void addPatient_WhenNameIsNull_ThenReturnMinus3(void)
{
    CreatePatientDoseAdmin();
    TEST_ASSERT_EQUAL(-3, AddPatient(NULL));
}

void addPatient_WhenPatientAlreadyPresent_ThenReturnMinus1(void)
{
    CreatePatientDoseAdmin();
    TEST_ASSERT_EQUAL(-1, AddPatient("John Doe")); // John Doe is added by CreatePatientDoseAdmin
}

void addPatient_WhenInputOk_ThenReturnZeroAndPatientIsAdded(void)
{
    CreatePatientDoseAdmin();
    TEST_ASSERT_EQUAL(0, AddPatient("flip"));

    int index = hashFunction("flip");
    Patient **temp = (Patient **)getHashTable();
    TEST_ASSERT_EQUAL_STRING("flip", temp[index]->name);
}

void addPatient_WhenMultiplePatientsAdded_ThenAllArePresent(void)
{
    CreatePatientDoseAdmin();
    TEST_ASSERT_EQUAL(0, AddPatient("Alice"));
    TEST_ASSERT_EQUAL(0, AddPatient("Bob"));
    TEST_ASSERT_EQUAL(1, IsPatientPresent("Alice"));
    TEST_ASSERT_EQUAL(1, IsPatientPresent("Bob"));
}


// IsPatientPresent


void isPatientPresent_WhenPatientExists_ThenReturnOne(void)
{
    CreatePatientDoseAdmin();
    TEST_ASSERT_EQUAL(1, IsPatientPresent("John Doe")); // added by CreatePatientDoseAdmin
}

void isPatientPresent_WhenPatientDoesNotExist_ThenReturnZero(void)
{
    CreatePatientDoseAdmin();
    TEST_ASSERT_EQUAL(0, IsPatientPresent("Nobody"));
}

void isPatientPresent_WhenTableIsEmpty_ThenReturnZero(void)
{
    RemoveAllDataFromPatientDoseAdmin();
    TEST_ASSERT_EQUAL(0, IsPatientPresent("John Doe"));
}


// RemovePatient


void removePatient_WhenPatientExists_ThenPatientIsGone(void)
{
    CreatePatientDoseAdmin();
    AddPatient("ToRemove");
    RemovePatient("ToRemove");
    TEST_ASSERT_EQUAL(0, IsPatientPresent("ToRemove"));
}

void removePatient_WhenPatientDoesNotExist_ThenReturnMinus1(void)
{
    CreatePatientDoseAdmin();
    TEST_ASSERT_EQUAL(-1, RemovePatient("Ghost"));
}

void removePatient_WhenPatientExists_ThenSlotIsNull(void)
{
    CreatePatientDoseAdmin();
    AddPatient("SlotTest");
    int index = hashFunction("SlotTest");
    RemovePatient("SlotTest");
    Patient **temp = (Patient **)getHashTable();
    TEST_ASSERT_NULL(temp[index]);
}


// RemoveAllDataFromPatientDoseAdmin


void removeAll_WhenCalled_ThenTableIsEmpty(void)
{
    CreatePatientDoseAdmin();
    AddPatient("Alice");
    AddPatient("Bob");
    RemoveAllDataFromPatientDoseAdmin();
    TEST_ASSERT_EQUAL(0, IsPatientPresent("Alice"));
    TEST_ASSERT_EQUAL(0, IsPatientPresent("Bob"));
    TEST_ASSERT_EQUAL(0, IsPatientPresent("John Doe"));
}


// SelectPatient


void selectPatient_WhenPatientExists_ThenReturnCorrectPatient(void)
{
    CreatePatientDoseAdmin();
    AddPatient("Karel");
    Patient *p = SelectPatient("Karel");
    TEST_ASSERT_NOT_NULL(p);
    TEST_ASSERT_EQUAL_STRING("Karel", p->name);
}

void selectPatient_WhenPatientDoesNotExist_ThenReturnNull(void)
{
    CreatePatientDoseAdmin();
    Patient *p = SelectPatient("Nobody");
    TEST_ASSERT_NULL(p);
}


// AddPatientDose


void addPatientDose_WhenPatientExistsAndDoseValid_ThenReturnZero(void)
{
    CreatePatientDoseAdmin();
    AddPatient("DoseGuy");
    Date d = {.day = 1, .month = 1, .year = 2024};
    TEST_ASSERT_EQUAL(0, AddPatientDose("DoseGuy", &d, 100));
}

void addPatientDose_WhenPatientExistsAndDoseValid_ThenDoseIsStored(void)
{
    CreatePatientDoseAdmin();
    AddPatient("DoseGuy2");
    Date d = {.day = 5, .month = 3, .year = 2024};
    AddPatientDose("DoseGuy2", &d, 250);

    Patient *p = SelectPatient("DoseGuy2");
    TEST_ASSERT_NOT_NULL(p);
    TEST_ASSERT_EQUAL(1, p->doseCount);
    TEST_ASSERT_EQUAL(250, p->dosages[0].dose);
    TEST_ASSERT_EQUAL(5,   p->dosages[0].doseDate.day);
    TEST_ASSERT_EQUAL(3,   p->dosages[0].doseDate.month);
    TEST_ASSERT_EQUAL(2024, p->dosages[0].doseDate.year);
}

void addPatientDose_WhenPatientDoesNotExist_ThenReturnMinus1(void)
{
    CreatePatientDoseAdmin();
    Date d = {.day = 1, .month = 1, .year = 2024};
    TEST_ASSERT_EQUAL(-1, AddPatientDose("Ghost", &d, 100));
}

void addPatientDose_WhenMultipleDosesAdded_ThenAllAreStored(void)
{
    CreatePatientDoseAdmin();
    AddPatient("MultiDose");
    Date d1 = {.day = 1, .month = 1, .year = 2024};
    Date d2 = {.day = 2, .month = 2, .year = 2024};
    AddPatientDose("MultiDose", &d1, 100);
    AddPatientDose("MultiDose", &d2, 200);

    Patient *p = SelectPatient("MultiDose");
    TEST_ASSERT_NOT_NULL(p);
    TEST_ASSERT_EQUAL(2, p->doseCount);
    TEST_ASSERT_EQUAL(100, p->dosages[0].dose);
    TEST_ASSERT_EQUAL(200, p->dosages[1].dose);
}


// CreatePatientDoseAdmin


void createPatientDoseAdmin_WhenCalled_ThenJohnDoeIsPresent(void)
{
    CreatePatientDoseAdmin();
    TEST_ASSERT_EQUAL(1, IsPatientPresent("John Doe"));
}

void createPatientDoseAdmin_WhenCalledTwice_ThenOnlyOneJohnDoe(void)
{
    CreatePatientDoseAdmin();
    CreatePatientDoseAdmin(); // should reset and re-add John Doe once
    TEST_ASSERT_EQUAL(1, IsPatientPresent("John Doe"));
}


// hashFunction


void hashFunction_WhenSameName_ThenSameHash(void)
{
    TEST_ASSERT_EQUAL(hashFunction("Alice"), hashFunction("Alice"));
}

void hashFunction_WhenDifferentNames_ThenResultInRange(void)
{
    // result must always be within table bounds
    TEST_ASSERT_TRUE(hashFunction("Alice") < HASHTABLE_SIZE);
    TEST_ASSERT_TRUE(hashFunction("Bob")   < HASHTABLE_SIZE);
    TEST_ASSERT_TRUE(hashFunction("XYZ123") < HASHTABLE_SIZE);
}


// main


int main()
{
    UnityBegin();

    // AddPatient
    MY_RUN_TEST(addPatient_WhenNameTooLong_ThenReturnMinus3);
    MY_RUN_TEST(addPatient_WhenNameIsNull_ThenReturnMinus3);
    MY_RUN_TEST(addPatient_WhenPatientAlreadyPresent_ThenReturnMinus1);
    MY_RUN_TEST(addPatient_WhenInputOk_ThenReturnZeroAndPatientIsAdded);
    MY_RUN_TEST(addPatient_WhenMultiplePatientsAdded_ThenAllArePresent);

    // IsPatientPresent
    MY_RUN_TEST(isPatientPresent_WhenPatientExists_ThenReturnOne);
    MY_RUN_TEST(isPatientPresent_WhenPatientDoesNotExist_ThenReturnZero);
    MY_RUN_TEST(isPatientPresent_WhenTableIsEmpty_ThenReturnZero);

    // RemovePatient
    MY_RUN_TEST(removePatient_WhenPatientExists_ThenPatientIsGone);
    MY_RUN_TEST(removePatient_WhenPatientDoesNotExist_ThenReturnMinus1);
    MY_RUN_TEST(removePatient_WhenPatientExists_ThenSlotIsNull);

    // RemoveAllDataFromPatientDoseAdmin
    MY_RUN_TEST(removeAll_WhenCalled_ThenTableIsEmpty);

    // SelectPatient
    MY_RUN_TEST(selectPatient_WhenPatientExists_ThenReturnCorrectPatient);
    MY_RUN_TEST(selectPatient_WhenPatientDoesNotExist_ThenReturnNull);

    // AddPatientDose
    MY_RUN_TEST(addPatientDose_WhenPatientExistsAndDoseValid_ThenReturnZero);
    MY_RUN_TEST(addPatientDose_WhenPatientExistsAndDoseValid_ThenDoseIsStored);
    MY_RUN_TEST(addPatientDose_WhenPatientDoesNotExist_ThenReturnMinus1);
    MY_RUN_TEST(addPatientDose_WhenMultipleDosesAdded_ThenAllAreStored);

    // CreatePatientDoseAdmin
    MY_RUN_TEST(createPatientDoseAdmin_WhenCalled_ThenJohnDoeIsPresent);
    MY_RUN_TEST(createPatientDoseAdmin_WhenCalledTwice_ThenOnlyOneJohnDoe);

    // hashFunction
    MY_RUN_TEST(hashFunction_WhenSameName_ThenSameHash);
    MY_RUN_TEST(hashFunction_WhenDifferentNames_ThenResultInRange);

    UnityEnd();
}