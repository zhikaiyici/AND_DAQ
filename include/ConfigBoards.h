#pragma once

#ifndef CONFIGBOARDS_h
#define CONFIGBOARDS_h 1

#include "OperateRegister.h"

typedef struct {

	//FILE* logfile;

	// Base Addresses
	uint32_t QTPBaseAddr;// = 0;
	uint32_t PLUBaseAddr;// = 0;
	uint32_t Discr1BaseAddr;// = 0;
	uint32_t Discr2BaseAddr;// = 0;

	// logic unit
	uint32_t gate_width;// = 50;
	uint32_t gate_delay;// = 50;
	uint32_t trigger_min;// = 1;
	uint32_t trigger_max;// = 1;

	// QDC
	int EnableSuppression;// = 1;		// Enable Zero and Overflow suppression if QTP boards
	uint16_t Iped;// = 255;			// pedestal of the QDC (or resolution of the TDC).
	uint16_t QTP_LLD[32];// = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
	uint32_t qtp_kill_chn; // = 0;

	// discriminator
	uint16_t Discr1ChMask;// = 0, 
	uint16_t Discr2ChMask;// = 0;		// Channel enable mask of the discriminator
	uint16_t DiscrOutputWidth;// = 10;	// Output wodth of the discriminator
	uint16_t DiscrThreshold[32];// = { 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5 };	// Thresholds of the discriminator
	
	// save files
	int EnableHistoFiles;// = 0;		// Enable periodic saving of histograms (once every second)
	int EnableListFile;// = 0;			// Enable saving of list file (sequence of events)
	int EnableRawDataFile;// = 0;		// Enable saving of raw data (memory dump)

	// measurement time
	double measure_period; // measurement period
	int measure_cycles; // measurement cycles

	int qtp_nch;

} BoardParam;

//extern BoardParam board_param;

typedef struct {
	double prompt_total_min;
	double prompt_total_max;
	double prompt_1st_min;
	double prompt_1st_max;
	double prompt_2nd_min;
	double prompt_2nd_max;

	double delay_total_min;
	double delay_total_max;
	double delay_1st_min;
	double delay_1st_max;
	double delay_2nd_min;
	double delay_2nd_max;
	double delay_3rd_min;
	double delay_3rd_max;
	double delay_4th_min;
	double delay_4th_max;

	double time_window_min;
	double time_window_max;

} SelectCriteria;

//extern SelectCriteria select_criteria;

int ConfigBoards(char* configFileName);
//int ConfigBoards(BoardParam board_param);

inline void SetConfigHandle(int handle) { config_info.handle = handle; };

//void DefaultConfig();
//int ReadConfigFile(char* config_file);
static int ReadConfigFile(char* config_file, BoardParam* board_param);

static int ConfigDiscr(uint16_t OutputWidth, uint16_t Threshold[16], uint16_t EnableMask, uint32_t DiscrBaseAddr);
static int ConfigQTP(int EnableSuppression, uint16_t Iped, uint16_t Threshold[32], uint32_t qtp_kill_chn, int* qtp_nch, uint32_t QTPBaseAddr);
static int ConfigPLU(uint32_t gate_delay, uint32_t gate_width, uint32_t trigger_min, uint32_t trigger_max, uint32_t PLUBaseAddr);

static void FindModelVersion(uint16_t model, uint16_t vers, char* modelVersion, int* ch);


#endif // !CONFIGBOARDS_h
