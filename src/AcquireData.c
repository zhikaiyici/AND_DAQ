
#include "CAEN_PLULib.h"
#include "ConfigBoards.h"
#include "OperateRegister.h"
#include "AcquireData.h"

#include <stdbool.h>
#include <math.h>

uint32_t histo[32][4096] = { 0 };
uint32_t module_histo[16][4096] = { 0 };
uint32_t chn_event[32] = { 0 };
uint32_t module_event[16] = { 0 };
uint32_t total_bytes = 0; // bytes in one statistic period, which is reset to 0 after one period
uint32_t num_event = 0; // event number in one statistic period, which is reset to 0 after one period
uint32_t total_event = 0; // total event number, which is accumulated until 'r' from keyboard is pressed

static uint32_t buffer[MAX_BLT_SIZE / sizeof(uint32_t)];
static uint32_t event_count = 0; // event number from QDC event counter
static uint32_t event_count_ov = 0; // event number overflow count
//static DataReadParam* data_read_param;

// ------------------------------------------------------------------------------------
// Acquisition loop
// ------------------------------------------------------------------------------------
int ReadQTPDataLoop(void* data_read_param_qtp_voidptr)
{
	DataReadParam* data_read_param = (DataReadParam*)data_read_param_qtp_voidptr;
	uint16_t ADCdata[32];
	buffer[0] = DATATYPE_FILLER;
	CVErrorCodes ret = 0;
	int buff_index = 0;  // word pointer
	int trans_int = 0; // num of lword read in the MBLT cycle
	int trans_bytes = 0;
	int DataType = DATATYPE_HEADER;
	int DataError = 0;
	int num_cnt = 0;
	int numCNT = 0;
	//int ch_index;

	// if needed, read a new block of data from the board 
	if ((buff_index == trans_int) || ((buffer[buff_index] & DATATYPE_MASK) == DATATYPE_FILLER)) {
		//read_qtp = ReadQTPBuffer(QTPBaseAddr, buffer, &trans_bytes);
		CVErrorCodes read_qtp = ReadQTPBuffer(data_read_param->board_param->QTPBaseAddr, buffer, &trans_bytes);
		if (read_qtp != cvSuccess && read_qtp != cvBusError) {
			printf("\nError during QTP reading.\n\n");
			return -1;
		}
		trans_int = trans_bytes / sizeof(uint32_t);
		total_bytes += trans_bytes;
		buff_index = 0;
	}

	while ((buff_index != trans_int) && ((buffer[buff_index] & DATATYPE_MASK) != DATATYPE_FILLER)) {

		//if (trans_int == 0) {  // no data available
		//	ControlFromKeyBoard();
		//	continue;
		//}

		/* header */
		switch (DataType) {
		case DATATYPE_HEADER:
			if ((buffer[buff_index] & DATATYPE_MASK) != DATATYPE_HEADER) {
				//printf("Header not found: %08X (buff_index=%d)\n", buffer[buff_index], buff_index);
				DataError = 1;
			}
			else {
				num_cnt = (buffer[buff_index] >> 8) & 0x3F;
				numCNT = num_cnt;
				memset(ADCdata, 0x0, 32 * sizeof(uint16_t));
				if (num_cnt > 0) {
					DataType = DATATYPE_CHDATA;
					num_event++;
					total_event++;
				}
				else
					DataType = DATATYPE_EOB;
			}
			break;

			/* Channel data */
		case DATATYPE_CHDATA:
			if ((buffer[buff_index] & DATATYPE_MASK) != DATATYPE_CHDATA) {
				//printf("Wrong Channel Data: %08X (buff_index=%d)\n", buffer[buff_index], buff_index);
				DataError = 1;
			}
			else {
				int chn = 0;
				if (data_read_param->board_param->qtp_nch == 32)
					chn = (int)((buffer[buff_index] >> 16) & 0x1F);  // for V792 (32 channels)
				else
					chn = (int)((buffer[buff_index] >> 17) & 0xF);  // for V792N (16 channels)
				ADCdata[chn] = buffer[buff_index] & 0xFFF;

				//if (((buffer[buff_index] >> 12) & 0x1) != 0) {
				//	printf("ADCdata[%d] = %d\n", chn, ADCdata[chn]);
				//	//getch();
				//}
				histo[chn][ADCdata[chn]]++;
				chn_event[chn]++;
				num_cnt--;
				if (num_cnt == 0)
					DataType = DATATYPE_EOB;
			}
			break;

			/* EOB */
		case DATATYPE_EOB:
			if ((buffer[buff_index] & DATATYPE_MASK) != DATATYPE_EOB) {
				//printf("EOB not found: %08X (buff_index=%d)\n", buffer[buff_index], buff_index);
				DataError = 1;
			}
			else {
				DataType = DATATYPE_HEADER;
				num_cnt--;
				ProcessData(ADCdata);

#if TEST_MODE
				RegulateHV(ADCdata);
#endif
				//if (ADC_file != NULL) {
				if (data_read_param->output_file != NULL) {
					int sum_ADC = 0;
					for (unsigned int i = 0; i < data_read_param->board_param->qtp_nch; i++)
						sum_ADC += ADCdata[i];
					if (sum_ADC != 0) {
						uint32_t old_event_count = event_count;
						event_count = buffer[buff_index] & 0xFFFFFF;
						// count over flow times
						if ((((old_event_count >> 23) & 0x1) ^ ((event_count >> 23) & 0x1)) != 0) {  //bit 24 changed
							if (((old_event_count >> 23) & 0x1) != 0) // bit 24 changed from 1 to 0
								event_count_ov++;
							// printf("event_count_ov: %d", event_count_ov);
						}
						//fprintf(data_read_param->output_file, "EventNum  %17d %6d %17d %6d %17d %6d %17d %6d\nChn00-07 ", event_count + (event_count_ov << 24),0,0,0,0,0,0,0);
						//for (unsigned int i = 0; i < data_read_param->board_param->qtp_nch; i++) {
						//	if ((i + 1) % 8 == 0 && i != (data_read_param->board_param->qtp_nch - 1))
						//		fprintf(data_read_param->output_file, " %6d\nChn%02d-%02d ", ADCdata[i], i + 1, i + 8);
						//	else if (i == (data_read_param->board_param->qtp_nch - 1))
						//		fprintf(data_read_param->output_file, " %6d\n", ADCdata[i]);
						//	else if ((i + 1) % 2 == 0 && (i + 1) % 8 != 0)
						//		fprintf(data_read_param->output_file, " %6d", ADCdata[i]);
						//	else
						//		fprintf(data_read_param->output_file, " %17d", ADCdata[i]);

						uint64_t event_num = (uint64_t)event_count + (uint64_t)(event_count_ov << 24);
						fprintf(data_read_param->output_file, "%llu %d %d %d %d %d %d %d\n", event_num, 0, 0, 0, 0, 0, 0, 0);
						for (unsigned int i = 0; i < data_read_param->board_param->qtp_nch; i++) {
							if ((i + 1) % 8 == 0)
								fprintf(data_read_param->output_file, "%d\n", ADCdata[i]);
							else
								fprintf(data_read_param->output_file, "%d ", ADCdata[i]);
						}
					}
				}
			}
			break;
		}
		buff_index++;

		if (DataError) {
			buff_index = trans_int;
			CVErrorCodes w_cleardata = WriteRegister(data_read_param->board_param->QTPBaseAddr + 0x1032, 0x4);// write_reg(0x1032, 0x4); // clear data
			CVErrorCodes w_reset_cleardata = WriteRegister(data_read_param->board_param->QTPBaseAddr + 0x1034, 0x4);// write_reg(0x1034, 0x4); // reset clear data
			if ((w_cleardata & w_reset_cleardata) != cvSuccess) {
				printf("\nError during QTP programming.\n\n");
				return -2;
			}
			DataType = DATATYPE_HEADER;
			DataError = 0;
		}
	}
	return 0;
}

//uint32_t bufferTimeStamp[4096] = { 0 };
//FILE* time_file = NULL;

int ReadPLUData(void* data_read_param_plu_voidptr)
{
	DataReadParam* data_read_param = (DataReadParam*)data_read_param_plu_voidptr;
	//if(time_file == NULL)
	//	time_file = fopen("time_file.txt", "w");

	//int list_addr = 0x103C; // Firmware (QDCtest) for test, no coincidence logic
	int list_addr = 0x106C; // Firmware for measurement

	uint32_t plu_buffer[4] = { 0 };
	uint32_t plubuff_index = 0;
	uint32_t pludata_type = GATE_COUNTS_MSB;
	uint64_t gcov = 0;
	uint64_t gcmsb = 0;
	uint64_t gclsb = 0;
	uint64_t tov = 0;
	uint64_t tmsb = 0;
	uint64_t tlsb = 0;
	int status = -1;
	//int status = ReadRegister(LogicBaseAddr + list_addr);
	CVErrorCodes r_status = ReadRegister(data_read_param->board_param->PLUBaseAddr + list_addr + 4, &status);
	if (r_status != cvSuccess)
		return r_status;
	//if ((status & 0x3) == 0x2) {
	//	printf("LIST IS FULL.\n");
	//	//getch();
	//}
	int DataError = 0;
	if ((status & 0x3) != 0x1) {// not empty
		for (unsigned int i = 0; i < 3 + 1; ++i) {
			//plu_buffer[i] = ReadRegister32(LogicBaseAddr + list_addr);
			CVErrorCodes r_plu_buffer = ReadRegister32(data_read_param->board_param->PLUBaseAddr + list_addr, &(plu_buffer[i]));
			if (r_plu_buffer != cvSuccess)
				return r_plu_buffer;
		}
		//status = ReadRegister(LogicBaseAddr + 0x1040);
		//if ((status & 0x3) == 0x2) {
		//	printf("LIST IS FULL.\n");
		//	//getch();
		//}

		for (unsigned int i = 0; i < 3 + 1; ++i) {
			switch (pludata_type) {
			case(GATE_COUNTS_MSB):
				if ((plu_buffer[i] & PLUDATA_MASK) != GATE_COUNTS_MSB)
					DataError = 1;
				else {
					gcmsb = plu_buffer[i] & 0xFFFFFFF;
					//gcov = (plu_buffer >> 4) & 0xFFFFFF;
					pludata_type = GATE_COUNTS_LSB;
				}
				break;
			case(GATE_COUNTS_LSB):
				if ((plu_buffer[i] & PLUDATA_MASK) != GATE_COUNTS_LSB)
					DataError = 1;
				else {
					gclsb = plu_buffer[i] & 0xFFFFFFF;
					pludata_type = CLK_COUNTS_MSB;
				}
				break;
			case(CLK_COUNTS_MSB):
				if ((plu_buffer[i] & PLUDATA_MASK) != CLK_COUNTS_MSB)
					DataError = 1;
				else {
					tmsb = plu_buffer[i] & 0xFFFFFFF;
					//tov = (plu_buffer >> 4) & 0xFFFFFF;
					pludata_type = CLK_COUNTS_LSB;
				}
				break;
			case(CLK_COUNTS_LSB):
				if ((plu_buffer[i] & PLUDATA_MASK) != CLK_COUNTS_LSB)
					DataError = 1;
				else {
					tlsb = plu_buffer[i] & 0xFFFFFFF;
					pludata_type = GATE_COUNTS_MSB;
					uint64_t gate_counts = gclsb + (gcmsb << 28);// +(gcov << 32);
					uint64_t clk_counts = tlsb + (tmsb << 28);// +(tov << 32);
					//fprintf(data_read_param->output_file, "EventNum %llu\n", gate_counts);
					////fprintf(data_read_param->output_file, "CLKNum %llu\n", clk_counts);
					//fprintf(data_read_param->output_file, "Timeus %.2lf\n", (double)clk_counts / 1e3 * 20.);

					fprintf(data_read_param->output_file, "%llu\n", gate_counts);
					//fprintf(data_read_param->output_file, "%llu\n", clk_counts);
					fprintf(data_read_param->output_file, "%.2lf\n", (double)clk_counts / 1e3 * 20.);

					//printf("Event Num. %llu\n", gate_counts);
					//printf("CLK Num. %llu\n", clk_counts);
					//printf("Time. %.2lf us\n\n", (double)clk_counts / 1e3 * 20.);
				}
				break;
			default:
				break;
			}
			if (DataError) {
				DataError = 0;
				printf("PLU data error.\n");
				break;
			}
		}

		//memset(bufferTimeStamp, 0, 1024 * 4);
		//CAEN_PLU_ERROR_CODE read_data = 0;
		//read_data = CAEN_PLU_ReadData32(config_info.handle, LogicBaseAddr + 0x1088, 4096, bufferTimeStamp, &size_);
		//read_data = ReadQTPBuffer(LogicBaseAddr, 1024, bufferTimeStamp, &size_);
		//for (unsigned int i = 0; i < size_ / sizeof(uint32_t); i++)
		//{
		//	printf("aa = %d\n", bufferTimeStamp[i]);
		//}

		/*int counts = 0;
		CVErrorCodes code = CAENVME_MBLTReadCycle(config_info.handle,
			LogicBaseAddr + 0x1040, bufferTimeStamp, 1024 * 4, cvA32_U_MBLT, &counts);
		if (code == cvSuccess)
		{
			if (counts >= 1023)
			{
				printf("counts >= 1023\n");
				break;
			}

			for (int i = 0; i < counts; i++)
			{
				printf("aa = %d\n", bufferTimeStamp[i]);
			}
		}*/

	}
	return 0;
}

static void ProcessData(uint16_t ADCdata[])
{
	double ADCdata_module[16] = { 0 }; // ADCdata_module[k] = sqrt(ADCdata[2k] * ADCdata[2k + 1]) 
	double ADC_module_org[16] = { 0 };
	double sum_ADC_module = 0.;
	for (unsigned int i = 0; i < 16; ++i) {
		ADCdata_module[i] = sqrt((double)ADCdata[2 * i] * (double)ADCdata[2 * i + 1]);
		ADC_module_org[i] = ADCdata_module[i];
		if ((int)ADCdata_module[i] != 0) {
			module_histo[i][(int)ADCdata_module[i]]++;
			module_event[i]++;
		}
		//sum_ADC_module += ADCdata_module[i];
	}
	//BubbleSort(ADCdata_module, sizeof(ADCdata_module) / sizeof(*ADCdata_module));

	//if (sum_ADC_module > select_criteria.delay_total_min && sum_ADC_module <= select_criteria.delay_total_max) {
	//	if (ADCdata_module[0] > select_criteria.delay_1st_min && ADCdata_module[0] <= select_criteria.delay_1st_max) {
	//		if (ADCdata_module[1] > select_criteria.delay_2nd_min && ADCdata_module[1] <= select_criteria.delay_2nd_max) {
	//			// 瞬时信号甄别条件是延迟信号甄别条件的子集
	//			if (sum_ADC_module > select_criteria.prompt_total_min && sum_ADC_module <= select_criteria.prompt_total_max) {
	//				if (ADCdata_module[0] > select_criteria.prompt_1st_min && ADCdata_module[0] <= select_criteria.prompt_1st_max) {
	//					if (ADCdata_module[1] < select_criteria.prompt_2nd_max) {
	//						printf("Prompt signal: \n");
	//						for (unsigned int i = 0; i < 16; ++i) {
	//							if ((i + 1) % 4 == 0)
	//								printf("%10.2lf\n", ADC_module_org[i]);
	//							else
	//								printf(" %10.2lf    ", ADC_module_org[i]);
	//						}
	//					}
	//				}
	//			}
	//			if (ADCdata_module[2] > select_criteria.delay_3rd_min && ADCdata_module[2] <= select_criteria.delay_3rd_max) {
	//				if (ADCdata_module[3] > select_criteria.delay_4th_min && ADCdata_module[3] <= select_criteria.delay_4th_max) {
	//					printf("Delayed signal: \n");
	//					for (unsigned int i = 0; i < 16; ++i) {
	//						if ((i + 1) % 4 == 0)
	//							printf("%10.2lf\n", ADC_module_org[i]);
	//						else
	//							printf(" %10.2lf    ", ADC_module_org[i]);
	//					}
	//				}
	//			}
	//		}
	//	}
	//}
}

static void BubbleSort(double arr[], unsigned int arr_len)
{
	double temp = 0.;
	bool is_swap = false;
	for (unsigned int j = arr_len - 1; j > 0; j--) {
		is_swap = false;
		for (unsigned int i = 0; i < j; ++i) {
			if (arr[i] < arr[i + 1]) {
				temp = arr[i];
				arr[i] = arr[i + 1];
				arr[i + 1] = temp;
				is_swap = true;
			}
		}
		if (!is_swap)
			break;
	}
}


