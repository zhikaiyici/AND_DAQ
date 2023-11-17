#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

#ifdef _WIN32
#include <sys/timeb.h>
#include <direct.h>
#include <windows.h>
#include <conio.h>
#include <process.h>
#define kbhit _kbhit
#define getch _getch
#else
#include <unistd.h>
#include <sys/time.h>
#define Sleep(x) usleep((x)*1000)
#endif

#include "DefineMacro.h"
#include "ConfigBoards.h"
#include "PrintAndControl.h"
#include "Console.h"
#include "AcquireData.h"

//static uint32_t histo[32][4096] = { 0 };

struct tm* RunningTime();

/******************************************************************************/
/*                                   MAIN                                     */
/******************************************************************************/
int main(int argc, char* argv[])
{
	//uint16_t test[32] = { 100, 101, 185, 190, 1800, 1795, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	short conet_node = 0;
	int link_num = 0;
	int32_t handle = -1;
	int command = 0;
	int read_config = 0;
	int read_qtp = 0;
	int read_plu = 0;
	int config_discr1 = 0;
	int config_discr2 = 0;
	int config_qtp = 0;
	int config_plu = 0;
	int config_boards = 0;
	double time_passed = 0.;
	FILE* ADC_file = NULL;
	FILE* time_file = NULL;
	DataReadParam qtp_read_param;
	DataReadParam plu_read_param;
	// default config
	/*BoardParam board_param = {

		0xCC0000000, // QTPBaseAddr
		0x32100000, // 	LogicBaseAddr
		0xFF000000, // Discr1BaseAddr
		0xEE000000, // Discr2BaseAddr

		50, // gate_width
		0, // gate_daley
		1, // trigger_min
		1, // trigger_max

		1, // EnableSuppression
		255, // Iped
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // QTP_LLD
		0,//0xFFFC0000, //qtp_kill_chn

		0xFFFF, // Discr1ChMask
		0x3, // Discr2ChMask
		0, // DiscrOutputWidth
		5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, // DiscrThreshold

		0, 0, 0, // EnableHistoFiles, EnableListFile, EnableRawDataFile

		0.5, 7, // measurement period, measurement cycles

		32 // QTP channel number
	};*/

	printf("\n");
	printf("***************************************************************************************************\n");
	printf("*                                                                                                 *\n");
	printf("*                           AntiNeutrino Detector DAQ        (VERSION 1.0)                        *\n");
	printf("*                                                                                                 *\n");
	printf("***************************************************************************************************\n");

	// open VME bridge (V1718 or V2718)
	CVErrorCodes ret = 0;
	ret = CAENVME_Init2(cvV1718, &link_num, conet_node, &handle);
	if (ret != cvSuccess) {
		ret = CAENVME_Init2(cvV2718, &link_num, conet_node, &handle);
		if (ret != cvSuccess) {
			ret = CAENVME_Init2(cvUSB_A4818_V2718, &link_num, conet_node, &handle);
			if (ret !=cvSuccess) {
				printf("\nCan't open VME controller.\n");
				char error_string[255];
				char* temp = error_string;
				temp = CAENVME_DecodeError(ret);
				CAENComm_DecodeError(ret, error_string);
				printf("Error code: %d. ", ret);
				//printf(error_string);
				printf(temp);
				printf(".\n");
				Sleep(1000);
				//goto QUIT_PROGRAM;
				printf("\nPress any key to exit.\n");
				getch();
				return -1;
			}
		}
	}
	SetConfigHandle(handle);

	//// CAENComm
	//if (CAENComm_OpenDevice(CAENComm_OpticalLink, link, bdnum, board_param.Discr1BaseAddr, &(config_info.handle)) != CAENComm_Success) {
	//	printf("Can't open VME controller\n");
	//	Sleep(1000);
	//	return -1;
	//}

	char ConfigFileName[255] = "config.txt";	// configuration file name
	if (argc > 1)
		strcpy(ConfigFileName, argv[1]);
	
	////read_config = ReadConfigFile(ConfigFileName);
	//read_config = ReadConfigFile(ConfigFileName, &board_param);
	//if (read_config != 0)
	//	return -1;

	config_boards = ConfigBoards(ConfigFileName);
	if (config_boards != 0)
		goto QUIT_PROGRAM;

	////PrintConfigInfo();
	////
	////ProcessData(test);

	//config_discr1 = ConfigDiscr(board_param.DiscrOutputWidth, board_param.DiscrThreshold, board_param.Discr1ChMask, board_param.Discr1BaseAddr);
	//if (config_discr1 != 0)
	//	goto QUIT_PROGRAM;

	//config_discr2 = ConfigDiscr(board_param.DiscrOutputWidth, board_param.DiscrThreshold + 16, board_param.Discr2ChMask, board_param.Discr2BaseAddr);
	//if (config_discr2 != 0)
	//	goto QUIT_PROGRAM;

	//config_plu = ConfigPLU(board_param.gate_delay, board_param.gate_width, board_param.trigger_min, board_param.trigger_max, board_param.PLUBaseAddr);
	//if (config_plu != 0)
	//	goto QUIT_PROGRAM;

	//config_qtp = ConfigQTP(board_param.EnableSuppression, board_param.Iped, board_param.QTP_LLD, board_param.qtp_kill_chn, &(board_param.qtp_nch), board_param.QTPBaseAddr);
	//if (config_qtp != 0)
	//	goto QUIT_PROGRAM;

	//PrintConfigInfo(board_param);

	extern uint64_t begin_time;
	extern uint64_t seconds;
	extern BoardParam board_param;

	begin_time = GetTime();
	double mp = board_param.measure_period;
	int mc = board_param.measure_cycles;
	int i = -1;
	qtp_read_param.board_param = &board_param;
	plu_read_param.board_param = &board_param;
	while ((command != QUIT) && (seconds != (int)(mp * 3600 * mc)))
	{
		command = ControlFromKeyBoard(ConfigFileName);
		mp = board_param.measure_period;
		mc = board_param.measure_cycles;
		qtp_read_param.board_param = &board_param;
		plu_read_param.board_param = &board_param;
		StatisticAndPlot();
		if (command == -1) {
			printf("\nPress any key to continue.\n\n");
			getch();
		}

		time_passed = (double)seconds;
		time_passed /= 3600.0;
		if ((int)(time_passed / mp) != i) {
			i = (int)(time_passed / mp);
			if (ADC_file != NULL) {
				fclose(ADC_file);
				ADC_file = NULL;
			}
			if (time_file != NULL) {
				fclose(time_file);
				time_file = NULL;
			}
			char tmp[255];
			struct tm* running_time = RunningTime();
			sprintf(tmp, ".\\output\\List%d_%4d%02d%02d%02d%02d.txt", i, 
				running_time->tm_year + 1900, running_time->tm_mon + 1, running_time->tm_mday, running_time->tm_hour, running_time->tm_min);
			ADC_file = fopen(tmp, "w");
			
			if ((ADC_file = fopen(tmp, "w")) != NULL) 
				qtp_read_param.output_file = ADC_file;
			else {
				printf("Can't open ADC File %s\n", tmp);
				goto QUIT_PROGRAM;
			}
			sprintf(tmp, ".\\output\\Time%d_%4d%02d%02d%02d%02d.txt", i,
				running_time->tm_year + 1900, running_time->tm_mon + 1, running_time->tm_mday, running_time->tm_hour, running_time->tm_min);
			if ((time_file = fopen(tmp, "w")) != NULL)
				plu_read_param.output_file = time_file;
			else {
				printf("Can't open Time File %s\n", tmp);
				goto QUIT_PROGRAM;
			}
		}

		read_qtp = ReadQTPDataLoop((void*)&qtp_read_param);
		if (read_qtp != 0)
			goto QUIT_PROGRAM;

		//_beginthread(ReadQTPDataLoop, 0, (void*)&qtp_read_param);

		read_plu = ReadPLUData((void*)&plu_read_param);
		if (read_plu != 0)
			goto QUIT_PROGRAM;

		//_beginthread(ReadLogicUnitData, 0, (void*)&plu_read_param);

	}

//**************************************************************************************

QUIT_PROGRAM:
	if (ADC_file != NULL) fclose(ADC_file);
	if (time_file != NULL) fclose(time_file);
	ClosePlot();
	if (handle >= 0) CAENVME_End(handle);

	SaveHistograms(0, board_param.qtp_nch - 1, ".\\histo", true);
	printf("\nPress any key to exit.\n");
	getch();
	return 0;
}

struct tm* RunningTime()
{
	time_t seconds = time(NULL); // 格林威治时间
	struct tm* info = localtime(&seconds);
	//printf("date: %s", asctime(info));

	return info;
}
