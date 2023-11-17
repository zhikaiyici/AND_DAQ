#pragma once

#ifndef DEFINEMACRO
#define DEFINEMACRO_h 1

/****************************************************/

#define MAX_BLT_SIZE		(256*1024)

#define DATATYPE_MASK		0x06000000
#define DATATYPE_HEADER		0x02000000
#define DATATYPE_CHDATA		0x00000000
#define DATATYPE_EOB		0x04000000
#define DATATYPE_FILLER		0x06000000

#define PLUDATA_MASK        0xF0000000
#define GATE_COUNTS_MSB     0xC0000000
#define GATE_COUNTS_LSB     0xE0000000
#define CLK_COUNTS_MSB      0x80000000
#define CLK_COUNTS_LSB      0xA0000000

#define LSB2PHY				100   // LSB (= ADC count) to Physical Quantity (time in ps, charge in fC, amplitude in mV)

#define LOGMEAS_NPTS		1000

#define ENABLE_LOG			0

#define FILES_IN_LOCAL_FOLDER	1

#define TEST_MODE           1

#define QUIT                1

/*******************************************************************/

#endif // !DEFINEMACRO

