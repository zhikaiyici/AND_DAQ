#pragma once

#ifndef PRINTANDCONTROL_h
#define PRINTANDCONTROL_h 1

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include "ConfigBoards.h"

//extern uint64_t begin_time;
//extern uint64_t seconds;

void PrintConfigInfo(/*BoardParam board_param*/);
void ClosePlot();
//int StatisticAndPlot(BoardParam board_param);
void StatisticAndPlot();
int SaveHistograms(int ch_min, int ch_max, char* path, bool print_save);

/*static*/ int ControlFromKeyBoard(char* configFileName/*, BoardParam board_param*/);

#endif // !PRINTANDCONTROL_h

