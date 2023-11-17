#pragma once

#ifndef ACQUIREDATA_h
#define ACQUIREDATA_h 1

#include "TestFunctoin.h"
#include "DefineMacro.h"
#include "ConfigBoards.h"

#include <stdio.h>

typedef struct {
	BoardParam* board_param;
	//uint32_t BaseAddr;// = 0;
	FILE* output_file;
} DataReadParam;

//int ReadQTPDataLoop(uint32_t QTPBaseAddr, FILE* ADC_file);
//int ReadLogicUnitData(uint32_t LogicBaseAddr, FILE* time_file);
int ReadQTPDataLoop(void* data_read_param_qtp_voidptr);
int ReadPLUData(void* data_read_param_plu_voidptr);

static void ProcessData(uint16_t ADCdata[]);
static void BubbleSort(double arr[], unsigned int arr_len);


#endif // !ACQUIREDATA_h

