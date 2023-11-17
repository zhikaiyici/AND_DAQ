
#include <stdio.h>

#include "PrintAndControl.h"
#include "ConfigBoards.h"
#include "CAEN_PLULib.h"

// default config
BoardParam board_param = {

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
	0xFFFF, // Discr2ChMask
	0, // DiscrOutputWidth
	5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, // DiscrThreshold

	0, 0, 0, // EnableHistoFiles, EnableListFile, EnableRawDataFile

	0.5, 7, // measurement period, measurement cycles

	32 // QTP channel number
};

SelectCriteria select_criteria = {
	
	0., 8.,
	0., 8,
	0., 8,

	0., 8.,
	0., 8.,
	0., 8.,
	0., 8.,
	0., 8.,

	0., 200.
};

/*void DefaultConfig()
{
	board_param.handle = -1;

	board_param.QTPBaseAddr = 0xCC000000;
	board_param.LogicBaseAddr = 0x32100000;
	board_param.Discr1BaseAddr = 0xFF000000;
	board_param.Discr2BaseAddr = 0xEE000000;

	board_param.gate_width = 10;
	board_param.trigger_num = 2;

	board_param.EnableSuppression = 1;
	board_param.Iped = 100;
	for (int i = 0; i < 32; ++i)
	{
		board_param.QTP_LLD[i] = 128;
	}

	board_param.Discr1ChMask = 0xFFFF;
	board_param.Discr2ChMask = 0x3;
	board_param.DiscrOutputWidth = 0;
	for (int i = 0; i < 32; ++i)
	{
		board_param.DiscrThreshold[i] = 5;
	}

	board_param.EnableHistoFiles = 0;
	board_param.EnableListFile = 0;
	board_param.EnableRawDataFile = 0;
}*/

//int ConfigBoards(BoardParam board_param)
int ConfigBoards(char* configFileName)
{
	//read_config = ReadConfigFile(ConfigFileName);
	int read_config = ReadConfigFile(configFileName, &board_param);
	if (read_config != 0)
		return read_config;

	int config_discr1 = ConfigDiscr(board_param.DiscrOutputWidth, board_param.DiscrThreshold, board_param.Discr1ChMask, board_param.Discr1BaseAddr);
	if (config_discr1 != 0)
		return config_discr1;

	int config_discr2 = ConfigDiscr(board_param.DiscrOutputWidth, board_param.DiscrThreshold + 16, board_param.Discr2ChMask, board_param.Discr2BaseAddr);
	if (config_discr2 != 0)
		return config_discr2;

	int config_plu = ConfigPLU(board_param.gate_delay, board_param.gate_width, board_param.trigger_min, board_param.trigger_max, board_param.PLUBaseAddr);
	if (config_plu != 0)
		return config_plu;

	int config_qtp = ConfigQTP(board_param.EnableSuppression, board_param.Iped, board_param.QTP_LLD, board_param.qtp_kill_chn, &(board_param.qtp_nch), board_param.QTPBaseAddr);
	if (config_qtp != 0)
		return config_qtp;

	PrintConfigInfo(/*board_param*/);

	return 0;
}

// ************************************************************************
// Read configuration file
// ************************************************************************
//int ReadConfigFile(char* config_file)
int ReadConfigFile(char* config_file, BoardParam* board_param)
{
	FILE* f_ini = NULL;// config file
	if ((f_ini = fopen(config_file, "r")) == NULL) {
		printf("Can't open Configuration File %s\n", config_file);
		return -1;
	}
	printf("Reading Configuration File %s\n", config_file);
	while (!feof(f_ini)) {
		char str[1000];
		unsigned int data;
		int i;

		str[0] = '#';
		fscanf(f_ini, "%s", str);
		if (str[0] == '#')
			fgets(str, 1000, f_ini);
		else {
			// Output Files
			if (strstr(str, "ENABLE_LIST_FILE") != NULL)
				//fscanf(f_ini, "%d", &(board_param.EnableListFile));
				fscanf(f_ini, "%d", &(board_param->EnableListFile));
			if (strstr(str, "ENABLE_HISTO_FILES") != NULL)
				//fscanf(f_ini, "%d", &(board_param.EnableHistoFiles));
				fscanf(f_ini, "%d", &(board_param->EnableHistoFiles));
			if (strstr(str, "ENABLE_RAW_DATA_FILE") != NULL)
				//fscanf(f_ini, "%d", &(board_param.EnableRawDataFile));
				fscanf(f_ini, "%d", &(board_param->EnableRawDataFile));

			// Base Addresses
			if (strstr(str, "QTP_BASE_ADDRESS") != NULL)
				//fscanf(f_ini, "%x", &(board_param.QTPBaseAddr));
				fscanf(f_ini, "%x", &(board_param->QTPBaseAddr));
			if (strstr(str, "DISCR1_BASE_ADDRESS") != NULL)
				//fscanf(f_ini, "%x", &(board_param.Discr1BaseAddr));
				fscanf(f_ini, "%x", &(board_param->Discr1BaseAddr));
			if (strstr(str, "DISCR2_BASE_ADDRESS") != NULL)
				//fscanf(f_ini, "%x", &(board_param.Discr2BaseAddr));
				fscanf(f_ini, "%x", &(board_param->Discr2BaseAddr));
			if (strstr(str, "PLU_BASE_ADDRESS") != NULL)
				//fscanf(f_ini, "%x", &(board_param.LogicBaseAddr));
				fscanf(f_ini, "%x", &(board_param->PLUBaseAddr));

			// I-pedestal
			if (strstr(str, "IPED") != NULL) {
				fscanf(f_ini, "%d", &data);
				//board_param.Iped = (uint16_t)data;
				board_param->Iped = (uint16_t)data;
			}

			//// Discr_ChannelMask
			//if (strstr(str, "DISCR1_CHANNEL_MASK") != NULL) {
			//	fscanf(f_ini, "%x", &data);
			//	board_param.Discr1ChMask = (uint16_t)data;
			//}
			//if (strstr(str, "DISCR2_CHANNEL_MASK") != NULL) {
			//	fscanf(f_ini, "%x", &data);
			//	board_param.Discr2ChMask = (uint16_t)data;
			//}

			// Discr_OutputWidth
			if (strstr(str, "DISCR_OUTPUT_WIDTH") != NULL) {
				fscanf(f_ini, "%d", &data);
				//board_param.DiscrOutputWidth = (uint16_t)data;
				board_param->DiscrOutputWidth = (uint16_t)data;
			}

			// Discr_Threshold
			if (strstr(str, "DISCR_THRESHOLD") != NULL) {
				int ch, thr;
				fscanf(f_ini, "%d", &ch);
				fscanf(f_ini, "%d", &thr);
				if (ch < 0) {
					for (i = 0; i < 32; i++)
						//board_param.DiscrThreshold[i] = thr;
						board_param->DiscrThreshold[i] = thr;
				}
				else if (ch < 32) {
					//board_param.DiscrThreshold[ch] = thr;
					board_param->DiscrThreshold[ch] = thr;
				}
			}

			// LLD for the QTP 
			if (strstr(str, "QTP_LLD") != NULL) {
				int ch, lld;
				fscanf(f_ini, "%d", &ch);
				fscanf(f_ini, "%d", &lld);
				if (ch < 0) {
					for (i = 0; i < 32; i++)
						//board_param.QTP_LLD[i] = lld;
						board_param->QTP_LLD[i] = lld;
				}
				else if (ch < 32) {
					//board_param.QTP_LLD[ch] = lld;
					board_param->QTP_LLD[ch] = lld;
				}
			}

			//qtp_kill_chn
			if (strstr(str, "QTP_KILL_CHANNEL") != NULL) {
				fscanf(f_ini, "%x", &data);
				//board_param.qtp_kill_chn = (uint32_t)data;
				board_param->qtp_kill_chn = (uint32_t)data;
			}


			// I-pedestal
			if (strstr(str, "ENABLE_SUPPRESSION") != NULL) {
				//fscanf(f_ini, "%d", &(board_param.EnableSuppression));
				fscanf(f_ini, "%d", &(board_param->EnableSuppression));
			}

			// gate width
			if (strstr(str, "GATE_WIDTH") != NULL) {
				//fscanf(f_ini, "%d", &(board_param.gate_width));
				fscanf(f_ini, "%d", &(board_param->gate_width));
			}

			// gate delay
			if (strstr(str, "GATE_DELAY") != NULL) {
				//fscanf(f_ini, "%d", &(board_param.gate_delay));
				fscanf(f_ini, "%d", &(board_param->gate_delay));
			}

			// trigger number
			if (strstr(str, "TRIGGER_NUMBER") != NULL) {
				//fscanf(f_ini, "%d %d", &(board_param.trigger_min), &(board_param.trigger_max));
				fscanf(f_ini, "%d %d", &(board_param->trigger_min), &(board_param->trigger_max));
			}

			// measurement period
			if (strstr(str, "MEASUREMENT_PERIOD") != NULL) {
				//fscanf(f_ini, "%lf", &(board_param.measure_period));
				fscanf(f_ini, "%lf", &(board_param->measure_period));
			}

			// measurement cycles
			if (strstr(str, "MEASUREMENT_CYCLES") != NULL) {
				//fscanf(f_ini, "%d", &(board_param.measure_cycles));
				fscanf(f_ini, "%d", &(board_param->measure_cycles));
			}

			if (strstr(str, "PROMPT_TOTAL") != NULL) {
				fscanf(f_ini, "%lf %lf", &(select_criteria.prompt_total_min), &(select_criteria.prompt_total_max));
				//(select_criteria.prompt_total_min) *= 1000.;
				//(select_criteria.prompt_total_max) *= 1000.;
			}

			if (strstr(str, "PROMPT_1ST") != NULL) {
				fscanf(f_ini, "%lf %lf", &(select_criteria.prompt_1st_min), &(select_criteria.prompt_1st_max));
				//(select_criteria.prompt_1st_min) *= 1000.;
				//(select_criteria.prompt_1st_max) *= 1000.;
			}

			if (strstr(str, "PROMPT_2ND") != NULL) {
				fscanf(f_ini, "%lf %lf", &(select_criteria.prompt_2nd_min), &(select_criteria.prompt_2nd_max));
				//(select_criteria.prompt_2nd_min) *= 1000.;
				//(select_criteria.prompt_2nd_max) *= 1000.;
			}

			if (strstr(str, "DELAYED_TOTAL") != NULL) {
				fscanf(f_ini, "%lf %lf", &(select_criteria.delay_total_min), &(select_criteria.delay_total_max));
				//(select_criteria.delay_total_min) *= 1000.;
				//(select_criteria.delay_total_max) *= 1000.;
			}

			if (strstr(str, "DELAYED_1ST") != NULL) {
				fscanf(f_ini, "%lf %lf", &(select_criteria.delay_1st_min), &(select_criteria.delay_1st_max));
				//(select_criteria.delay_1st_min) *= 1000.;
				//(select_criteria.delay_1st_max) *= 1000.;
			}

			if (strstr(str, "DELAYED_2ND") != NULL) {
				fscanf(f_ini, "%lf %lf", &(select_criteria.delay_2nd_min), &(select_criteria.delay_2nd_max));
				//(select_criteria.delay_2nd_min) *= 1000.;
				//(select_criteria.delay_2nd_max) *= 1000.;
			}

			if (strstr(str, "DELAYED_3RD") != NULL) {
				fscanf(f_ini, "%lf %lf", &(select_criteria.delay_3rd_min), &(select_criteria.delay_3rd_max));
				//(select_criteria.delay_3rd_min) *= 1000.;
				//(select_criteria.delay_3rd_max) *= 1000.;
			}

			if (strstr(str, "DELAYED_4TH") != NULL) {
				fscanf(f_ini, "%lf %lf", &(select_criteria.delay_4th_min), &(select_criteria.delay_4th_max));
				//(select_criteria.delay_4th_min) *= 1000.;
				//(select_criteria.delay_4th_max) *= 1000.;
			}

			if (strstr(str, "TIME_WINDOW") != NULL)
				fscanf(f_ini, "%lf %lf", &(select_criteria.time_window_min), &(select_criteria.time_window_max));

		}
	}
	fclose(f_ini);

	//board_param.Discr1ChMask = ~(board_param.qtp_kill_chn) & 0xFFFF;
	//board_param.Discr2ChMask = (~(board_param.qtp_kill_chn >> 16)) & 0xFFFF;
	board_param->Discr1ChMask = ~(board_param->qtp_kill_chn) & 0xFFFF;
	board_param->Discr2ChMask = (~(board_param->qtp_kill_chn >> 16)) & 0xFFFF;

	return 0;
}

// ************************************************************************
// Discriminitor settings
// ************************************************************************
int ConfigDiscr(uint16_t OutputWidth, uint16_t Threshold[16], uint16_t EnableMask, uint32_t DiscrBaseAddr)
{
	if (DiscrBaseAddr != 0) {
		printf("Discr1 Base Address = 0x%08X\n", DiscrBaseAddr);
		CVErrorCodes ret = 0;
		int write_mask = 0;
		int write_ow1 = 0;
		int write_ow2 = 0;
		int write_thr = 0;

		// Set channel mask
		write_mask = WriteRegister(DiscrBaseAddr + 0x004A, EnableMask);

		// set output width (same for all channels)
		write_ow1 = WriteRegister(DiscrBaseAddr + 0x0040, OutputWidth);
		write_ow2 = WriteRegister(DiscrBaseAddr + 0x0042, OutputWidth);

		// set CFD threshold
		for (int i = 0; i < 16; i++) {
			write_thr = WriteRegister(DiscrBaseAddr + i * 2, Threshold[i]);
		}

		if ((write_mask | write_ow1 | write_ow2 | write_thr) != cvSuccess) {
			printf("Error during discriminator programming.\n\n");
			return -1;
		}
		else {
			printf("Discriminator programmed successfully.\n");
			return 0;
		}
	}
	else {
		printf("Can't access to the discriminator at Base Address 0x%08X.\n", DiscrBaseAddr);
		return -1;
	}
}

// ************************************************************************
// QTP settings
// ************************************************************************
int ConfigQTP(int EnableSuppression, uint16_t Iped, uint16_t Threshold[32], uint32_t qtp_kill_chn, int* qtp_nch, uint32_t QTPBaseAddr)
{
	if (QTPBaseAddr != 0) {
		printf("QTP Base Address = 0x%08X\n", QTPBaseAddr);

		// Reset QTP board
		CVErrorCodes w_softrest = WriteRegister(QTPBaseAddr + 0x1006, 0x80);
		CVErrorCodes w_reset_softrest = WriteRegister(QTPBaseAddr + 0x1008, 0x80);
		//write_rssr = WriteRegister(QTPBaseAddr + 0x1016, 0);

		// Read FW revision
		uint16_t fwrev = 0;
		CVErrorCodes read_ferev = ReadRegister(QTPBaseAddr + 0x1000, &fwrev);

		uint16_t brd_id_lsb = 0;
		CVErrorCodes r_brd_id_lsb = ReadRegister(QTPBaseAddr + 0x803E, &brd_id_lsb);
		uint16_t brd_id_msb = 0;
		CVErrorCodes r_brd_id_msb = ReadRegister(QTPBaseAddr + 0x803A, &brd_id_msb);
		uint16_t model = (brd_id_lsb & 0xFF) + ((brd_id_msb & 0xFF) << 8);
		// read version (> 0xE0 = 16 channels)
		uint16_t vers = 0;
		CVErrorCodes r_vers = ReadRegister(QTPBaseAddr + 0x8032, &vers);
		vers = vers & 0xFF;

		// Read serial number
		uint16_t ser_lsb = 0; 
		CVErrorCodes r_serlsb = ReadRegister(QTPBaseAddr + 0x8F06, &ser_lsb);
		uint16_t ser_msb = 0;
		CVErrorCodes r_sermsb = ReadRegister(QTPBaseAddr + 0x8F02, &ser_msb);
		uint16_t sernum = (ser_lsb & 0xFF) + ((ser_msb & 0xFF) << 8);

		char modelVersion[3] = { 0 };
		FindModelVersion(model, vers, modelVersion, qtp_nch);
		printf("Model = V%d%s\n", model, modelVersion);
		printf("Serial Number = %d\n", sernum);
		printf("FW Revision = %d.%d\n", (fwrev >> 8) & 0xFF, fwrev & 0xFF);

		CVErrorCodes w_iped = WriteRegister(QTPBaseAddr + 0x1060, Iped);  // Set pedestal
		CVErrorCodes w_enable_berr = WriteRegister(QTPBaseAddr + 0x1010, 0x60);  // enable BERR to close BLT at and of block

		// Set LLD (low level threshold for ADC data)
		CVErrorCodes w_thr_step = WriteRegister(QTPBaseAddr + 0x1034, 0x100);  // set threshold step = 16
		CVErrorCodes w_thr = -1;
		int kill_chn = 0;
		for (int i = 0; i < *qtp_nch; i++) {
			kill_chn = ((qtp_kill_chn >> i) & 1) << 8;
			if (*qtp_nch == 16)
				w_thr = WriteRegister(QTPBaseAddr + 0x1080 + i * 4, kill_chn + Threshold[i] / 16);
			else
				w_thr = WriteRegister(QTPBaseAddr + 0x1080 + i * 2, kill_chn + Threshold[i] / 16);
		}

		//write_disos = WriteRegister(QTPBaseAddr + 0x1032, 0x0008);  // disable overrange suppression
		if (!EnableSuppression) {
			CVErrorCodes write_diszs = WriteRegister(QTPBaseAddr + 0x1032, 0x0010);  // disable zero suppression
			CVErrorCodes write_disos = WriteRegister(QTPBaseAddr + 0x1032, 0x0008);  // disable overrange suppression
			CVErrorCodes write_eee = WriteRegister(QTPBaseAddr + 0x1032, 0x1000);  // enable empty events
		}

		// clear Event Counter
		CVErrorCodes w_reset_eventcounter = WriteRegister(QTPBaseAddr + 0x1040, 0x0);// write_reg(0x1040, 0x0);
		// clear QTP
		CVErrorCodes w_cleardata = WriteRegister(QTPBaseAddr + 0x1032, 0x4);// write_reg(0x1032, 0x4); // clear data
		CVErrorCodes w_reset_cleardata = WriteRegister(QTPBaseAddr + 0x1034, 0x4);// write_reg(0x1034, 0x4); // reset clear data

		if (((w_softrest & w_reset_softrest & w_iped  & w_enable_berr & w_thr_step & w_thr| w_reset_eventcounter & w_cleardata & w_reset_cleardata)
			& (r_brd_id_msb & r_brd_id_lsb & read_ferev & r_vers & r_serlsb & r_sermsb)) != cvSuccess) {
			printf("Error during QTP programming.\n\n");
			return -1;
		}
		else {
			printf("QTP board programmed successfully.\n");
			return 0;
		}
	}
	else {
		printf("No Base Address setting found for the QTP board.\n");
		printf("Skipping QTP readout\n");
		return -1;
	}
}

// ************************************************************************
// Logic unit settings
// ************************************************************************
int ConfigPLU(uint32_t gate_delay, uint32_t gate_width, uint32_t trigger_min, uint32_t trigger_max, uint32_t PLUBaseAddr)
{
	if (PLUBaseAddr != 0) {
		printf("Logic unit Base Address = 0x%08X\n", PLUBaseAddr);

		//// Firmware (QDCtest) for test, no coincidence logic
		//int reset_addr = 0x1004;
		//int triggermin_addr = 0x104C;
		//int triggermax_addr = 0x1048;
		//int gatewidth_addr = 0x1000;
		//int gatedelay_addr = 0x1000;
		//int list_addr = 0x103C;

		// Firmware for measurement
		int reset_addr = 0x1064;
		int triggermin_addr = 0x1044;
		int triggermax_addr = 0x1048;
		int gatewidth_addr = 0x1054;
		int gatedelay_addr = 0x1058;
		int list_addr = 0x106C;

		CVErrorCodes w_reset = WriteRegister(PLUBaseAddr + reset_addr, 0xF); // global reset
		CVErrorCodes w_reset_reset = WriteRegister(PLUBaseAddr + reset_addr, 0x0);

		CVErrorCodes w_triggermax = WriteRegister32(PLUBaseAddr + triggermin_addr, trigger_min);
		CVErrorCodes w_triggermin = WriteRegister32(PLUBaseAddr + triggermax_addr, trigger_max);
		CVErrorCodes w_gatedelay = WriteRegister32(PLUBaseAddr + gatedelay_addr, gate_delay);
		CVErrorCodes w_gatewidth = WriteRegister32(PLUBaseAddr + gatewidth_addr, gate_width);
		// list config
		CVErrorCodes w_list_reset = WriteRegister(PLUBaseAddr + list_addr + 8, 0x2); // reset
		CVErrorCodes w_list_enablewrite = WriteRegister(PLUBaseAddr + list_addr + 8, 0x1); // enable writing

		//CAEN_PLU_ERROR_CODE ret_plu = 0;
		//int plu_handle;
		//int sn;
		//char str_base_addr[9];
		//sprintf(str_base_addr, "%x", LogicBaseAddr);
		//ret_plu = CAEN_PLU_OpenDevice2(CAEN_PLU_CONNECT_VME_V2718, &sn, 0, str_base_addr ,&(config_info.plu_handle));
		//ret_plu = CAEN_PLU_OpenDevice2(CAEN_PLU_CONNECT_DIRECT_USB, "10445", 0, 0, str_base_addr ,&(config_info.plu_handle));
		//ret_plu = CAEN_PLU_OpenDevice(CAEN_PLU_CONNECT_VME_V2718, str_base_addr,0 , 0 ,&(config_info.plu_handle));
		//if (ret_plu != CAEN_PLU_OK)
		//	printf("PLULib connection failed. Error code %d\n", ret_plu);
		//ret_plu = CAEN_PLU_InitGateAndDelayGenerators(plu_handle);
		//ret_plu = CAEN_PLU_SetGateAndDelayGenerators(plu_handle, 50, 0, 0);

		if ((w_reset & w_reset_reset & w_gatewidth & w_gatedelay & w_triggermax & w_triggermin & w_list_reset | w_list_enablewrite) != cvSuccess) {
			printf("Error during logic unit programming.\n\n");
			return -2;
		}
		else {
			printf("Logic unit programmed successfully.\n");
			return 0;
		}
	}
	else {
		printf("No Base Address setting found for the Logic board.\n");
		return -1;
	}
}

static void FindModelVersion(uint16_t model, uint16_t vers, char* modelVersion, int* ch)
{
	switch (model) {
	case 792:
		switch (vers) {
		case 0x11:
			strcpy(modelVersion, "AA");
			*ch = 32;
			return;
		case 0x13:
			strcpy(modelVersion, "AC");
			*ch = 32;
			return;
		case 0xE1:
			strcpy(modelVersion, "NA");
			*ch = 16;
			return;
		case 0xE3:
			strcpy(modelVersion, "NC");
			*ch = 16;
			return;
		default:
			strcpy(modelVersion, "-");
			*ch = 32;
			return;
		}
		break;
	case 965:
		switch (vers) {
		case 0x1E:
			strcpy(modelVersion, "A");
			*ch = 16;
			return;
		case 0xE3:
		case 0xE1:
			strcpy(modelVersion, " ");
			*ch = 32;
			return;
		default:
			strcpy(modelVersion, "-");
			*ch = 32;
			return;
		}
		break;
	case 775:
		switch (vers) {
		case 0x11:
			strcpy(modelVersion, "AA");
			*ch = 32;
			return;
		case 0x13:
			strcpy(modelVersion, "AC");
			*ch = 32;
			return;
		case 0xE1:
			strcpy(modelVersion, "NA");
			*ch = 16;
			return;
		case 0xE3:
			strcpy(modelVersion, "NC");
			*ch = 16;
			return;
		default:
			strcpy(modelVersion, "-");
			*ch = 32;
			return;
		}
		break;
	case 785:
		switch (vers) {
		case 0x11:
			strcpy(modelVersion, "AA");
			*ch = 32;
			return;
		case 0x12:
			strcpy(modelVersion, "Ab");
			*ch = 32;
			return;
		case 0x13:
			strcpy(modelVersion, "AC");
			*ch = 32;
			return;
		case 0x14:
			strcpy(modelVersion, "AD");
			*ch = 32;
			return;
		case 0x15:
			strcpy(modelVersion, "AE");
			*ch = 32;
			return;
		case 0x16:
			strcpy(modelVersion, "AF");
			*ch = 32;
			return;
		case 0x17:
			strcpy(modelVersion, "AG");
			*ch = 32;
			return;
		case 0x18:
			strcpy(modelVersion, "AH");
			*ch = 32;
			return;
		case 0x1B:
			strcpy(modelVersion, "AK");
			*ch = 32;
			return;
		case 0xE1:
			strcpy(modelVersion, "NA");
			*ch = 16;
			return;
		case 0xE2:
			strcpy(modelVersion, "NB");
			*ch = 16;
			return;
		case 0xE3:
			strcpy(modelVersion, "NC");
			*ch = 16;
			return;
		case 0xE4:
			strcpy(modelVersion, "ND");
			*ch = 16;
			return;
		default:
			strcpy(modelVersion, "-");
			*ch = 32;
			return;
		}
		break;
	case 862:
		switch (vers) {
		case 0x11:
			strcpy(modelVersion, "AA");
			*ch = 32;
			return;
		case 0x13:
			strcpy(modelVersion, "AC");
			*ch = 32;
			return;
		default:
			strcpy(modelVersion, "-");
			*ch = 32;
			return;
		}
		break;
	}
}
