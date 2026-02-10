#ifndef PROTOCOL_PATIENTADMIN_CENTRALACQ_H
#define PROTOCOL_PATIENTADMIN_CENTRALACQ_H


typedef enum {
	MSG_START_SYMBOL = '$', 
	MSG_END_SYMBOL = '#',
	MSG_ARGUMENT_SEPARATOR = ':'
} SYMBOLS;

typedef enum {
	EXAM_TYPE_SINGLE_SHOT,
	EXAM_TYPE_SERIES,
	EXAM_TYPE_SERIES_WITH_MOTION, 
	EXAM_TYPE_FLUORO,
	EXAM_TYPE_NONE
} EXAMINATION_TYPES;

static const char CONNECT_MSG[] = "CONNECT";
static const char DISCONNECT_MSG[] = "DISCONNECT";
static const char DOSE_MSG[] = "DOSE";
static const char EXAM_MSG[] = "EXAM";  
       // Remark that the exam msg will have an argument.
       // For instance the actual msg could be EXAM:0 
	   // The 0 indicates a single shot exam
	   
	   
#define MAX_MSG_SIZE 	(100)



#endif