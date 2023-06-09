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
#ifndef     _TCC_DXB_CONTROL_H_
#define     _TCC_DXB_CONTROL_H_
#include    <linux/types.h>
typedef enum
{
	BOARD_ISDBT_TCC353X,
	BOARD_MAX
}DXB_BOARD_TYPE;


#define DXB_RF_PATH_UHF 1
#define DXB_RF_PATH_VHF 2

typedef struct
{
	unsigned int uiI2C; //control channel of i2c
	unsigned int uiSPI; //control channel of spi
	unsigned int status;
}ST_CTRLINFO_ARG;

#define ISDBT_CTRL_DEV_FILE     "/dev/tcc_isdbt_ctrl"
#define ISDBT_CTRL_DEV_NAME     "tcc_isdbt_ctrl"
#define ISDBT_CTRL_DEV_MAJOR        250
#define ISDBT_CTRL_DEV_MINOR        0

#define IOCTL_DXB_CTRL_OFF          _IO(ISDBT_CTRL_DEV_MAJOR, 1)
#define IOCTL_DXB_CTRL_ON           _IO(ISDBT_CTRL_DEV_MAJOR, 2)
#define IOCTL_DXB_CTRL_RESET        _IO(ISDBT_CTRL_DEV_MAJOR, 3)
#define IOCTL_DXB_CTRL_SET_BOARD    _IO(ISDBT_CTRL_DEV_MAJOR, 4)
#define IOCTL_DXB_CTRL_GET_CTLINFO  _IO(ISDBT_CTRL_DEV_MAJOR, 5)
#define IOCTL_DXB_CTRL_RF_PATH      _IO(ISDBT_CTRL_DEV_MAJOR, 6)
#define IOCTL_DXB_CTRL_SET_CTRLMODE _IO(ISDBT_CTRL_DEV_MAJOR, 7)
#define IOCTL_DXB_CTRL_RESET_LOW    _IO(ISDBT_CTRL_DEV_MAJOR, 8)
#define IOCTL_DXB_CTRL_RESET_HIGH   _IO(ISDBT_CTRL_DEV_MAJOR, 9)
#define IOCTL_DXB_CTRL_PURE_ON      _IO(ISDBT_CTRL_DEV_MAJOR, 10)
#define IOCTL_DXB_CTRL_PURE_OFF     _IO(ISDBT_CTRL_DEV_MAJOR, 11)
/* add for HWDEMUX cipher */
#define IOCTL_DXB_CTRL_HWDEMUX_CIPHER_SET_ALGORITHM     _IO(ISDBT_CTRL_DEV_MAJOR, 12)
#define IOCTL_DXB_CTRL_HWDEMUX_CIPHER_SET_KEY           _IO(ISDBT_CTRL_DEV_MAJOR, 13)
#define IOCTL_DXB_CTRL_HWDEMUX_CIPHER_SET_VECTOR        _IO(ISDBT_CTRL_DEV_MAJOR, 14)
#define IOCTL_DXB_CTRL_HWDEMUX_CIPHER_SET_DATA      _IO(ISDBT_CTRL_DEV_MAJOR, 15)
#define IOCTL_DXB_CTRL_HWDEMUX_CIPHER_EXECUTE       _IO(ISDBT_CTRL_DEV_MAJOR, 16)
#define IOCTL_DXB_CTRL_UNLOCK                       _IO(ISDBT_CTRL_DEV_MAJOR, 17)
#define IOCTL_DXB_CTRL_LOCK                         _IO(ISDBT_CTRL_DEV_MAJOR, 18)
#endif
