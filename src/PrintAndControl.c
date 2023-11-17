
#include "Console.h"
#include "ConfigBoards.h"
#include "AcquireData.h"
#include "TestFunctoin.h"
#include "PrintAndControl.h"

#include <errno.h>

uint64_t begin_time = 0;
uint64_t seconds = 0;

extern uint32_t histo[32][4096];
extern uint32_t module_histo[16][4096];
extern uint32_t chn_event[32];
extern uint32_t module_event[16];
extern uint32_t total_event;

static FILE* gnuplot = NULL;	// gnuplot (will be opened in a pipe)
static uint64_t PrevPlotTime = 0;
static int ch_min = 0;
static int ch_max = 1;

void ClosePlot()
{
	if (gnuplot != NULL) {
		_pclose(gnuplot);
		gnuplot = NULL;
	}
#if TEST_MODE
	ClosePlot_();
#endif
}

void PrintConfigInfo(/*BoardParam board_param*/)
{
	extern BoardParam board_param;
	printf("\n");
	printf(" +===============================================================================================+\n");
	printf(" |                                   Discriminator Parameters                                    |\n");
	printf(" +-----------------------------------------------+-----------------------------------------------+\n");
	printf(" | DISCR1_CHANNEL_MASK: %4X                     |", board_param.Discr1ChMask); 
	printf(" DISCR2_CHANNEL_MASK: %4X                     |\n", board_param.Discr2ChMask);
	printf(" +-----------------------------------------------+-----------------------------------------------+\n");
	printf(" | DISCR_OUTPUT_WIDTH: %3d                       |                                               |\n", board_param.DiscrOutputWidth);
	printf(" +-----------------------------------------------+-----------------------------------------------+\n");
	printf(" | DISCR_THRESHOLD:                                                                              |\n");
	for (int i = 0; i < 32; i++) {
		if (((~board_param.qtp_kill_chn >> i) & 0x1) == 0) {
			if ((i + 1) % 4 == 0)
				printf(" |   Channel %2d: MASKED  |\n", i);
			else
				printf(" |   Channel %2d: MASKED ", i);
		}
		else {
			if ((i + 1) % 4 == 0)
				printf(" |   Channel %2d: %2d mV   |\n", i, board_param.DiscrThreshold[i]);
			else
				printf(" |   Channel %2d: %2d mV  ", i, board_param.DiscrThreshold[i]);
		}
	}
		
	printf(" +===============================================+===============================================+\n");
	printf(" |                                         QDC Parameters                                        |\n");
	printf(" +-----------------------------------------------+-----------------------------------------------+\n");
	printf(" | QTP_KILL_CHANNEL: %8X                    |", board_param.qtp_kill_chn);
	printf(" QTP_IPED: %3d                                 |\n", board_param.Iped);
	printf(" +-----------------------------------------------+-----------------------------------------------+\n");
	printf(" | ENABLE_SUPPRESSION: %d                         |                                               |\n", board_param.EnableSuppression);
	printf(" +-----------------------------------------------+-----------------------------------------------+\n");
	printf(" | QTP_LLD:                                                                                      |\n");
	for (int i = 0; i < 32; i++) {
		if (((board_param.qtp_kill_chn >> i) & 0x1) == 1) {
			if ((i + 1) % 4 == 0)
				printf(" |   Channel %2d: KILLED  |\n", i);
			else
				printf(" |   Channel %2d: KILLED ", i);
		}
		else {
			if ((i + 1) % 4 == 0)
				printf(" |   Channel %2d: %3d     |\n", i, board_param.QTP_LLD[i]);
			else
				printf(" |   Channel %2d: %3d    ", i, board_param.QTP_LLD[i]);
		}
	}
		
	printf(" +===============================================+===============================================+\n");
	printf(" |                                     Logic Unit Parameters                                     |\n");
	printf(" +-----------------------------------------------+-----------------------------------------------+\n");
	printf(" | GATE_WIDTH: %3d                               |", board_param.gate_width);
	printf("  GATE_DELAY: %3d                              |\n", board_param.gate_delay);
	printf(" +-----------------------------------------------+-----------------------------------------------+\n");
	printf(" | %2d <= TRIGGER_NUMBER <= %2d                    |                                               |\n",
		board_param.trigger_min, board_param.trigger_max);
	printf(" +===============================================+===============================================+\n");
	printf(" |                                    Measurement Parameters                                     |\n");
	printf(" +-----------------------------------------------+-----------------------------------------------+\n");
	printf(" | MEASUREMENT_PERIOD: %2.2f h                    |", board_param.measure_period);
	printf(" MEASUREMENT_CYCLES: %3d                       |\n", board_param.measure_cycles);

	printf(" +===============================================+===============================================+\n");
	//printf(" |                                Selection Criteria Parameters                                  |\n");
	//printf(" +-----------------------------------------------+-----------------------------------------------+\n");
	//printf(" | Parameters of prompt signal:                                                                  |\n");
	//printf(" |   %.2lf MeV < Edep_total <= %.2lf MeV           |                                               |\n", 
	//	select_criteria.prompt_total_min, select_criteria.prompt_total_max);
	//printf(" |   %.2lf MeV < Edep_1st <= %.2lf MeV             |   %.2lf MeV < Edep_2nd < %.2lf MeV              |\n", 
	//	select_criteria.prompt_1st_min, select_criteria.prompt_1st_max, select_criteria.prompt_2nd_min, select_criteria.prompt_2nd_max);
	//printf(" +-----------------------------------------------+-----------------------------------------------+\n");
	//printf(" | Parameters of delayed signal:                                                                 |\n");
	//printf(" |   %.2lf MeV < Edep_total <= %.2lf MeV           |                                               |\n",
	//	select_criteria.delay_total_min, select_criteria.delay_total_max);
	//printf(" |   %.2lf MeV < Edep_1st <= %.2lf MeV             |   %.2lf MeV < Edep_2nd <= %.2lf MeV             |\n",
	//	select_criteria.delay_1st_min, select_criteria.delay_1st_max, select_criteria.delay_2nd_min, select_criteria.delay_2nd_max);
	//printf(" |   %.2lf MeV < Edep_3rd <= %.2lf MeV             |   %.2lf MeV < Edep_4th <= %.2lf MeV             |\n",
	//	select_criteria.delay_3rd_min, select_criteria.delay_3rd_max, select_criteria.delay_4th_min, select_criteria.delay_4th_max);
	//printf(" +-----------------------------------------------+-----------------------------------------------+\n");
	//printf(" | %.0lf us < TIME_WINDOW < %.0lf us                   |                                               |\n",
	//	select_criteria.time_window_min,select_criteria.time_window_max);
	//printf(" +===============================================+===============================================+\n");

	printf("\nPress any key to start.\n");
	getch();
}

//int StatisticAndPlot(BoardParam board_param)
void StatisticAndPlot()
{
	extern uint32_t total_bytes;
	extern uint32_t num_event;
	//int ret = ControlFromKeyBoard(board_param);

	// Log statistics on the screen and plot histograms
	uint64_t CurrentTime = GetTime(); // Time in milliseconds
	uint64_t ElapsedTime = CurrentTime - PrevPlotTime;
	int refresh_time = 1000;
	if (ElapsedTime > refresh_time) {
		ClearScreen();
		uint64_t PassedTime = CurrentTime - begin_time;
		seconds = (double)PassedTime / 1000.;
		int sec_passed = (int)seconds % 60;
		uint64_t minutes = (seconds - sec_passed) / 60;
		int min_passed = minutes % 60;
		uint64_t hours = (minutes - min_passed) / 60;
		int hour_passed = (int)hours % 24;
		int day_passed = (hours - hour_passed) / 24;
		float rate = 0.0;
		float bytes_rate = 0.0;
		int total_counts = 0;
		int total_module_counts = 0;
		//char c = 0;
		//char histoFileName[255];
		char str_time_passed[63];
		char gnuplot_command_plot[4095];

		//char gnuplot_command_title[127];
		//FILE* fh;	// plotting data file 
		//for (size_t i = 0; i < board_param.qtp_nch; i++)
		//	total_counts += chn_event[i];
		//for (size_t i = 0; i < sizeof(module_event) / sizeof(*module_event); i++)
		//	total_module_counts += module_event[i];

		sprintf(str_time_passed, "%d day(s) % d hour(s) % d minute(s) % d second(s)", day_passed, hour_passed, min_passed, sec_passed);
		printf("Acquired %d events in %s\n", total_event, str_time_passed);

		rate = (float)num_event / (float)ElapsedTime;
		if (rate > 1.)
			printf("Trigger Rate = %.3f kHz\n", rate);
		else
			printf("Trigger Rate = %.3f Hz\n", rate * 1000.);
		bytes_rate = ((float)total_bytes / 1024. / 1024.) / ((float)ElapsedTime / 1000.);
		if (bytes_rate > 1.)
			printf("Readout Rate = %.3f MB/s\n", bytes_rate);
		else
			printf("Readout Rate = %.3f kB/s\n", bytes_rate * 1000.);
		printf("\n");

		for (int i = ch_min; i < ch_max + 1; i++)
			printf("Acquired %d events on channel %d\n", chn_event[i], i);
		printf("\n");

		for (int i = ch_min / 2; i < ch_max / 2 + 1; i++)
			printf("Acquired %d events on module %d\n", module_event[i], i);
		printf("\n\n");

		printf("[q] quit  [r] reset statistics  [c] change channel  [p] plot channel  [e] exit plotting\n\n");

		if (gnuplot != NULL) {
			//sprintf_s(histoFileName, 255, ".\\histo_forplot.txt");
			//errno_t err = fopen_s(&fh, histoFileName, "w");
			//if (err != 0) {
			//	printf("Can't open plot histogram file %s\n", histoFileName);
			//	return -1;
			//}
			//for (int i = 0; i < 4096; i++) {
			//	fprintf(fh, "%d\n", (int)histo[ch][i]);
			//}
			//fclose(fh);
			//fprintf(gnuplot, "set title 'Ch. %d (Rate = %.3f kHz, counts = %d)'\n", ch, rate, chn_event[ch]);
			//fprintf(gnuplot, "plot '%s' with step\n", histoFileName);
			SaveHistograms(ch_min, ch_max, ".\\histo", false);
			char temp[127];
			sprintf(gnuplot_command_plot, "plot '.\\histo\\Histo_%d.txt' with step title 'Ch. %d (counts = %d)'", ch_min, ch_min, chn_event[ch_min]);
			for (int i = ch_min; i < ch_max; ++i) {
				sprintf(temp, ", '.\\histo\\Histo_%d.txt' with step title 'Ch. %d (counts = %d)'", i + 1, i + 1, chn_event[i + 1]);
				strcat(gnuplot_command_plot, temp);
			}
			sprintf(temp, "; plot '.\\histo\\Histo_Module_%d.txt' with step title 'Modu. %d (counts = %d)'", ch_min / 2, ch_min / 2, module_event[ch_min / 2]);
			strcat(gnuplot_command_plot, temp);
			for (int i = ch_min / 2; i < ch_max / 2; ++i) {
				sprintf(temp, ", '.\\histo\\Histo_Module_%d.txt' with step title 'Modu. %d (counts = %d)'", i + 1, i + 1, module_event[i + 1]);
				strcat(gnuplot_command_plot, temp);
			}
			if (num_event > 1000)
				sprintf(temp , "'Trigger rate = %.3f kHz, total events = %d (in %s)'", rate, total_event, str_time_passed);
			else
				sprintf(temp, "'Trigger rate = %.3f Hz, total events = %d (in %s)'", rate, total_event, str_time_passed);
			fprintf(gnuplot, "set multiplot layout 2, 1 title %s\n", temp);
			fprintf(gnuplot, "%s\n", gnuplot_command_plot);
			fflush(gnuplot);

#if TEST_MODE
			PlotDiff(ch_min, ch_max);
#endif
		}

		num_event = 0;
		total_bytes = 0;
		PrevPlotTime = CurrentTime;
	}
	//return ret;
}

int ControlFromKeyBoard(char* configFileName/*, BoardParam board_param*/)
{
	char keyboard_command = 0;
	if (kbhit()) {
		keyboard_command = getch();
		switch (keyboard_command) {
		case 'r':
		{
			//int read_config = ReadConfigFile(configFileName, &board_param);
			//if (read_config != 0)
			//	return read_config;

			int config_boards = ConfigBoards(configFileName);
			if (config_boards != 0)
				return config_boards;

			//PrintConfigInfo(board_param);

			for (int i = 0; i < 32; i++) {
				chn_event[i] = 0;
				memset(histo[i], 0, sizeof(uint32_t) * 4096);
				if (i % 2 == 0) {
					module_event[i / 2] = 0;
					memset(module_histo[i / 2], 0, sizeof(uint32_t) * 4096);
#if TEST_MODE
					extern uint32_t histo_diff[16][4096 * 2];
					memset(histo_diff[i / 2], 0, sizeof(uint32_t) * 4096 * 2);
#endif
				}
			}
			begin_time = GetTime();
			total_event = 0;
			break;
		}
		case 'q':
			return 1;
		case 'c':
			do {
				extern BoardParam board_param;
				printf("Enter new channel n and m (Ch. n to Ch. m, 0 <= n <= m < 32) : "/*, board_param.qtp_nch*/);
				scanf("%d %d", &ch_min, &ch_max);
				//if (ch_min == ch_max && ch_min >= 0 && ch_max < board_param.qtp_nch) break;
			} while (ch_min < 0 || ch_max >= 32/*board_param.qtp_nch*/ /*|| ch_min %2 != 0 || ch_max % 2 == 0*/);
			break;
		case 'p':
			// Open gnuplot (as a pipe)
			if (gnuplot == NULL) {
#ifdef LINUX
				gnuplot = popen("/usr/bin/gnuplot", "w");
#else
				char tmp[255];
				sprintf(tmp, ".\\pgnuplot.exe");
				//sprintf(tmp, ".\\gnuplot\\binary\\pgnuplot.exe");
				gnuplot = _popen(tmp, "w");
#endif
				FILE* temp = NULL;
				//if (gnuplot == NULL) {
				if ((temp = fopen(tmp, "r")) == NULL) {
					printf("Can't open gnuplot\n\n");
					gnuplot = NULL;
					return -1;
					//exit(0);
				}
				fprintf(gnuplot, "set ylabel 'Counts'\n");
				fprintf(gnuplot, "set xlabel 'ADC channels'\n");
				fprintf(gnuplot, "set yrange [0:]\n");
				fprintf(gnuplot, "set grid\n");
				fprintf(gnuplot, "set key samplen 0.5\n");
				fprintf(gnuplot, "set key spacing 0.4\n");
				fprintf(gnuplot, "set key width 0\n");
				fprintf(gnuplot, "set xrange [200:1000]\n");
			}
			break;
		case 'e':
			ClosePlot();
			break;
		default:
			break;
		}
		return 0;
	}
}

// ************************************************************************
// Save Histograms to files
// ************************************************************************
int SaveHistograms(int ch_min, int ch_max, char* path, bool print_save)
{
	FILE* fout = NULL;
	FILE* fout_cps = NULL;
	char fname[127];
	char fname_cps[127];
	int i, j;
	for (j = ch_min; j < ch_max + 1; j++) {
		sprintf(fname, "%s\\Histo_%d.txt", path, j);
		fout = fopen(fname, "w");
		if (fout == NULL) {
			printf("Can't open Histogram File %s\n", fname);
			return -1;
		}
		sprintf(fname_cps, "%s\\CPS_Histo_%d.txt", path, j);
		fout_cps = fopen(fname_cps, "w");
		if (fout_cps == NULL) {
			printf("Can't open Histogram File %s\n", fname_cps);
			return -1;
		}
		for (i = 0; i < 4096; i++) {
			fprintf(fout, "%d\n", (int)histo[j][i]);
			fprintf(fout_cps, "%.2lf\n", (double)histo[j][i] / seconds);
		}
		fclose(fout);
		fclose(fout_cps);
		if (print_save) printf("Saved histograms to output files %s and %s.\n", fname, fname_cps);
	}
	for (j = ch_min / 2; j < ch_max / 2 + 1; j++) {
		sprintf(fname, "%s\\Histo_Module_%d.txt", path, j);
		fout = fopen(fname, "w");
		if (fout == NULL) {
			printf("Can't open Module Histogram File %s\n", fname);
			return -1;
		}
		sprintf(fname_cps, "%s\\CPS_Histo_Module_%d.txt", path, j);
		fout_cps = fopen(fname_cps, "w");
		if (fout_cps == NULL) {
			printf("Can't open Module Histogram File %s\n", fname_cps);
			return -1;
		}
		for (i = 0; i < 4096; i++) {
			fprintf(fout, "%d\n", (int)module_histo[j][i]);
			fprintf(fout_cps, "%.2lf\n", (double)module_histo[j][i] / seconds);
		}
		fclose(fout);
		fclose(fout_cps);
		if (print_save) printf("Saved module histograms to output files %s and %s.\n", fname, fname_cps);
	}
	return 0;
}

