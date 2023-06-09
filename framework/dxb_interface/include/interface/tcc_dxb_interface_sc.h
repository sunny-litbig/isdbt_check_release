/*******************************************************************************

*   Copyright (c) Telechips Inc.


*   TCC Version 1.0

This source code contains confidential information of Telechips.

Any unauthorized use without a written permission of Telechips including not
limited to re-distribution in source or binary form is strictly prohibited.

This source code is provided "AS IS" and nothing contained in this source code
shall constitute any express or implied warranty of any kind, including without
limitation, any warranty of merchantability, fitness for a particular purpose
or non-infringement of any patent, copyright or other third party intellectual
property right.
No warranty is made, express or implied, regarding the information's accuracy,
completeness, or performance.

In no event shall Telechips be liable for any claim, damages or other
liability arising from, out of or in connection with this source code or
the use in the source code.

This source code is provided subject to the terms of a Mutual Non-Disclosure
Agreement between Telechips and Company.
*
*******************************************************************************/
#ifndef _TCC_DXB_INTERFACE_SC_H_
#define _TCC_DXB_INTERFACE_SC_H_
#include "tcc_dxb_interface_type.h"

typedef enum 
{
	DxB_SC_STD_ISO,			//ISO7816 -> T0 or T1
	DxB_SC_STD_CONAX,
	DxB_SC_STD_IRDETO,		// T14
	DxB_SC_STD_NDS, 		// NDS
	DxB_SC_STD_EMV2000
} DxB_SC_STD;

typedef enum 
{
	DxB_T_0 = 0,	//T0 default is IR
	DxB_T_1,		//T1 default is NA

	DxB_T_0_IR,
	DxB_T_0_CX,
	DxB_T_1_NA,
	DxB_T_1_ARIB,

	DxB_T_14 = 14
} DxB_SC_PROTOCOL;

typedef enum 
{
	DxB_SC_NOT_PRESENT = 0,
	DxB_SC_PRESENT
} DxB_SC_STATUS;

typedef enum 
{
	DxB_SC_REMOVED = 0, 
	DxB_SC_INSERTED,
	DxB_SC_EVENT_MAX
} DxB_SC_EVENT;

typedef enum DxB_SC_VccLevel {
	DxB_SC_VccLevel_e5V = 0,   /* 5v is default value */
	DxB_SC_VccLevel_e3V = 1    /* 3v  */  
} DxB_SC_VccLevel;

typedef void (*pfnDxB_SC_EVT_CALLBACK)(UINT32 unDeviceId, DxB_SC_STATUS nStatus);


/********************************************************************************************/
/********************************************************************************************
						FOR MW LAYER FUNCTION
********************************************************************************************/
/********************************************************************************************/

DxB_ERR_CODE TCC_DxB_SC_GetCapability(UINT32*pNumDevice);
DxB_ERR_CODE TCC_DxB_SC_NegotiatePTS(UINT32 unDeviceId, unsigned char *pucWriteBuf, int iNumToWrite, unsigned char *pucReadBuf,unsigned int *piNumRead);
DxB_ERR_CODE TCC_DxB_SC_SetParams(UINT32 unDeviceId, DxB_SC_PROTOCOL nProtocol, unsigned long ulMinClock, unsigned long ulSrcBaudrate, unsigned char ucFI, unsigned char ucDI, unsigned char ucN, unsigned char ucCWI, unsigned char ucBWI);
DxB_ERR_CODE TCC_DxB_SC_GetParams(UINT32 unDeviceId, DxB_SC_PROTOCOL *pnProtocol, unsigned long *pulClock, unsigned long *pulBaudrate, unsigned char *pucFI, unsigned char *pucDI, unsigned char *pucN, unsigned char *pucCWI, unsigned char *pucBWI);
DxB_ERR_CODE TCC_DxB_SC_GetCardStatus(UINT32 unDeviceId, DxB_SC_STATUS *pnStatus);
DxB_ERR_CODE TCC_DxB_SC_Reset (UINT32 unDeviceId, UINT8 *pucAtr, UINT32 *pucAtrlen);
DxB_ERR_CODE TCC_DxB_SC_TransferData(UINT32 unDeviceId, unsigned char *pucWriteBuf, int iNumToWrite, unsigned char *pucReadBuf, unsigned int *piNumRead);
DxB_ERR_CODE TCC_DxB_SC_ReadData(UINT32 unDeviceId, unsigned char *pucWriteBuf, int iNumToWrite, unsigned char *pucReadBuf, unsigned int *piNumRead);
DxB_ERR_CODE TCC_DxB_SC_WriteData(UINT32 unDeviceId, unsigned char *pucWriteBuf, int iNumToWrite, unsigned char *pucReadBuf, unsigned int *piNumRead);
DxB_ERR_CODE TCC_DxB_SC_RegisterCallback(UINT32 unDeviceId, pfnDxB_SC_EVT_CALLBACK pfnSCCallback);
DxB_ERR_CODE TCC_DxB_SC_SetVccLevel(UINT32 unDeviceId, DxB_SC_VccLevel in_vccLevel);
DxB_ERR_CODE TCC_DxB_SC_DownUpVCC(UINT32 unDeviceId, unsigned int unDownTime /*ms*/);
DxB_ERR_CODE TCC_DxB_SC_Activate(UINT32 unDeviceId);
DxB_ERR_CODE TCC_DxB_SC_Deactivate(UINT32 unDeviceId);
DxB_ERR_CODE TCC_DxB_SC_CardDetect(void);
DxB_ERR_CODE TCC_DxB_SC_Open(void);
DxB_ERR_CODE TCC_DxB_SC_Close(void);

#endif

