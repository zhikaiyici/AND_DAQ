#include <string.h>
#include "TestFunctoin.h"

uint32_t histo_diff[16][4096 * 2] = { 0 };
static FILE* gnuplot_ = NULL;

void RegulateHV(uint16_t ADCdata[])
{
	int16_t ADCdiff[16] = { 0 };
	for (unsigned int i = 0; i < 16; i++) {
		if ((ADCdata[2 * i] & ADCdata[2 * i + 1]) != 0) {
			ADCdiff[i] = ADCdata[2 * i] - ADCdata[2 * i + 1];
			histo_diff[i][4096 + ADCdiff[i]]++;
		}
	}
}

void PlotDiff(int ch_min, int ch_max)
{
	FILE* fout_ = NULL;
	int i, j;
	for (j = ch_min / 2; j < ch_max / 2 + 1; j++) {
		char fname_[127];
		sprintf(fname_, "%s\\Histo_Diff_%d.txt", ".", j);
		fout_ = fopen(fname_, "w");
		if (fout_ == NULL) {
			printf("Can't open Histogram Diff File %s\n", fname_);
			return -1;
		}
		for (i = 0; i < 4096 * 2; i++) {
			fprintf(fout_, "%6d  ", i - 4096);
			fprintf(fout_, "  %6d\n", (int)histo_diff[j][i]);
		}
		fclose(fout_);
	}

	// 
	if (gnuplot_ == NULL) {
		gnuplot_ = _popen(".\\pgnuplot.exe", "w");
		fprintf(gnuplot_, "set grid\n");
		fprintf(gnuplot_, "set xrange [-500:500]\n");
	}
	char gnuplot_command_plot_[4095];
	char temp_[127];
	//int m_min = 15;
	sprintf(gnuplot_command_plot_, "plot '.\\Histo_Diff_%d.txt' with step title 'Modu. %d'", ch_min / 2, ch_min / 2);
	for (int i = ch_min / 2; i < ch_max / 2; ++i) {
		sprintf(temp_, ", '.\\Histo_diff_%d.txt' with step title 'Modu. %d'", i + 1, i + 1);
		strcat(gnuplot_command_plot_, temp_);
	}

	fprintf(gnuplot_, "%s\n", gnuplot_command_plot_);
	fflush(gnuplot_);
}

void ClosePlot_()
{
	if (gnuplot_ != NULL) {
		_pclose(gnuplot_);
		gnuplot_ = NULL;
	}
}
