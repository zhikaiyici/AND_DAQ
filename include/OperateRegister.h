#pragma once

#ifndef OPERATEREGISTER_h
#define OPERATEREGISTER_h 1

#include "CAENVMElib.h"
#include "CAENVMEtypes.h"
#include "CAENComm.h"

typedef struct {
	int handle;
	CVErrorCodes VMEerror;
	char ErrorString[255];
	int plu_handle;
} ConfigInfo;

extern ConfigInfo config_info;

CVErrorCodes WriteRegister(uint32_t reg_addr, uint16_t data);
CVErrorCodes WriteRegister32(uint32_t reg_addr, uint32_t data);
CVErrorCodes ReadRegister(uint32_t reg_addr, uint16_t* data);
CVErrorCodes ReadRegister32(uint32_t reg_addr, uint32_t* data);
CVErrorCodes ReadQTPBuffer(uint32_t reg_addr, uint32_t* buffer, int* count);

static void PrintOperateResult(CVErrorCodes ret);

// CAENComm_Lib
CAENComm_ErrorCode WriteRegister_Comm(uint32_t reg_addr, uint16_t data);
CAENComm_ErrorCode WriteRegister32_Comm(uint32_t reg_addr, uint32_t data);
CAENComm_ErrorCode ReadRegister_Comm(uint32_t reg_addr, uint16_t* data);
CAENComm_ErrorCode ReadQTPBuffer_Comm(uint32_t reg_addr, uint32_t* buffer, int* count);

static void PrintOperateResult_Comm(CAENComm_ErrorCode ret);

#endif // !OPERATEREGISTER_h

