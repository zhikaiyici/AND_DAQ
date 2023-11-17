#pragma once

#ifndef TESTFUNCTION_h
#define TESTFUNCTION_h 1

#include <stdio.h>
#include <stdint.h>

void RegulateHV(uint16_t ADCdata[]);
void PlotDiff(int ch_min, int ch_max);
void ClosePlot_();

#endif