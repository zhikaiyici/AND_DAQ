
#include <stdio.h>

#include "DefineMacro.h"
#include "ConfigBoards.h"
#include "OperateRegister.h"

ConfigInfo config_info = {
	-1, // handle
	0, // VMEerror
	0, // ErrorString[255];
	-1 // PLU handle 
};

/*******************************************************************************/
/*                               READ_REG                                      */
/*******************************************************************************/
CVErrorCodes ReadRegister(uint32_t reg_addr, uint16_t* data)
{
	CVErrorCodes ret;
	ret = CAENVME_ReadCycle(config_info.handle, reg_addr, data, cvA32_U_DATA, cvD16);
	if (ret != cvSuccess) {
		sprintf_s(config_info.ErrorString, 255, "Cannot read at address %08X. Error Code: %d. ", (uint32_t)(reg_addr), ret);
		config_info.VMEerror = ret;
		PrintOperateResult(ret);
	}
	//if (ENABLE_LOG)
	//	fprintf(logfile, " Reading register at address %08X; data=%04X; ret=%d\n", (uint32_t)(BaseAddress + reg_addr), data, (int)ret);
	return ret;
}

CVErrorCodes ReadRegister32(uint32_t reg_addr, uint32_t* data)
{
	CVErrorCodes ret;
	ret = CAENVME_ReadCycle(config_info.handle, reg_addr, data, cvA32_U_DATA, cvD32);
	if (ret != cvSuccess) {
		sprintf_s(config_info.ErrorString, 255, "Cannot read at address %08X. Error Code: %d. ", (uint32_t)(reg_addr), ret);
		config_info.VMEerror = ret;
		PrintOperateResult(ret);
	}
	//if (ENABLE_LOG)
	//	fprintf(logfile, " Reading register at address %08X; data=%04X; ret=%d\n", (uint32_t)(BaseAddress + reg_addr), data, (int)ret);
	return ret;
}

CVErrorCodes ReadQTPBuffer(uint32_t reg_addr, uint32_t* buffer, int* count)
{
	CVErrorCodes ret;
	ret = CAENVME_FIFOMBLTReadCycle(config_info.handle, reg_addr, (void*)buffer, MAX_BLT_SIZE, cvA32_U_MBLT, count);
	if (ret != cvSuccess && ret != cvBusError) {
		sprintf_s(config_info.ErrorString, 255, "Cannot read at address %08X. Error Code: %d. ", (uint32_t)(reg_addr), ret);
		config_info.VMEerror = ret;
		PrintOperateResult(ret);
	}
	return ret;
}

/*******************************************************************************/
/*                                WRITE_REG                                    */
/*******************************************************************************/
CVErrorCodes WriteRegister(uint32_t reg_addr, uint16_t data)
{
	CVErrorCodes ret;
	ret = CAENVME_WriteCycle(config_info.handle, reg_addr, &data, cvA32_U_DATA, cvD16);
	if (ret != cvSuccess) {
		sprintf_s(config_info.ErrorString, 255, "Cannot write at address %08X. Error Code: %d. ", (uint32_t)(reg_addr), ret);
		config_info.VMEerror = ret;
		PrintOperateResult(ret);
	}
	//if (ENABLE_LOG)
	//	fprintf(logfile, " Writing register at address %08X; data=%04X; ret=%d\n", (uint32_t)(BaseAddress + reg_addr), data, (int)ret);
	return ret;
}

CVErrorCodes WriteRegister32(uint32_t reg_addr, uint32_t data)
{
	CVErrorCodes ret;
	ret = CAENVME_WriteCycle(config_info.handle, reg_addr, &data, cvA32_U_DATA, cvD32);
	if (ret != cvSuccess) {
		sprintf_s(config_info.ErrorString, 255, "Cannot write at address %08X. Error Code: %d. ", (uint32_t)(reg_addr), ret);
		config_info.VMEerror = ret;
		PrintOperateResult(ret);
	}
	//if (ENABLE_LOG)
	//	fprintf(logfile, " Writing register at address %08X; data=%04X; ret=%d\n", (uint32_t)(BaseAddress + reg_addr), data, (int)ret);
	return ret;
}

static void PrintOperateResult(CVErrorCodes ret)
{
	char error_string[255];
	sprintf_s(error_string, 255, CAENVME_DecodeError(ret));
	printf("Board programming result: ");
	printf(config_info.ErrorString);
	printf(error_string);
	printf(".\n");
}


/*************************************************************************/
/*                          CAENComm_Lib                                 */  
/*************************************************************************/
CAENComm_ErrorCode WriteRegister_Comm(uint32_t reg_addr, uint16_t data)
{
	CAENComm_ErrorCode ret;
	ret = CAENComm_Write16(config_info.handle, reg_addr, data);
	if (ret != CAENComm_Success) {
		sprintf_s(config_info.ErrorString, 255, "Cannot write at address %08X. Error Code: %d. ", (uint32_t)(reg_addr), ret);
		config_info.VMEerror = ret;
		PrintOperateResult_Comm(ret);
	}
	return ret;
}

CAENComm_ErrorCode WriteRegister32_Comm(uint32_t reg_addr, uint32_t data)
{
	CAENComm_ErrorCode ret;
	ret = CAENComm_Write32(config_info.handle, reg_addr, data);
	if (ret != CAENComm_Success) {
		sprintf_s(config_info.ErrorString, 255, "Cannot write at address %08X. Error Code: %d. ", (uint32_t)(reg_addr), ret);
		config_info.VMEerror = ret;
		PrintOperateResult_Comm(ret);
	}
	return ret;
}

CAENComm_ErrorCode ReadRegister_Comm(uint32_t reg_addr, uint16_t* data)
{
	CAENComm_ErrorCode ret;
	ret = CAENComm_Read16(config_info.handle, reg_addr, data);
	if (ret != CAENComm_Success) {
		sprintf_s(config_info.ErrorString, 255, "Cannot write at address %08X. Error Code: %d. ", (uint32_t)(reg_addr), ret);
		config_info.VMEerror = ret;
		PrintOperateResult_Comm(ret);
	}
	return ret;
}

CAENComm_ErrorCode ReadQTPBuffer_Comm(uint32_t reg_addr, uint32_t* buffer, int* count)
{
	CAENComm_ErrorCode ret;
	ret = CAENComm_MBLTRead(config_info.handle, reg_addr, /*(char*)*/buffer, MAX_BLT_SIZE, count);
	if (ret != cvSuccess) {
		sprintf_s(config_info.ErrorString, 255, "Cannot read at address %08X. Error Code: %d. ", (uint32_t)(reg_addr), ret);
		config_info.VMEerror = ret;
		PrintOperateResult_Comm(ret);
	}
	return ret;
}

static void PrintOperateResult_Comm(CAENComm_ErrorCode ret)
{
	char error_string[255];
	CAENComm_DecodeError(ret, error_string);
	printf("Board programming result: ");
	printf(config_info.ErrorString);
	printf(error_string);
	printf(".\n");
}
