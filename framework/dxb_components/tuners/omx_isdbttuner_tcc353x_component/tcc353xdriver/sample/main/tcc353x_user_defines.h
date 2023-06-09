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
 #ifndef __TCC353X_USER_DEFINES_H__
#define __TCC353X_USER_DEFINES_H__

/* INIT PC Value for TCC3530, TCC3531, TCC3532 */
#define TCC353X_INIT_PC_H 		0xC0
#define TCC353X_INIT_PC_L 		0x00
/* INIT PC Value for TCC3530, TCC3531, TCC3535 */
#define TCC353X_INIT_PC_H_TCC3535 	0x00
#define TCC353X_INIT_PC_L_TCC3535	0x00

/* Remap Type and Buffer config */
#if 1	/* Normal mode */
#define TCC353X_REMAP_TYPE 	0x04 		/* normal mode */
#define TCC353X_BUFF_A_START 	0x00018000
#define TCC353X_BUFF_A_END 	0x00019f93
#define TCC353X_BUFF_B_START 	0x00018000
#define TCC353X_BUFF_B_END 	0x00019f93
#define TCC353X_BUFF_C_START 	0x00018000
#define TCC353X_BUFF_C_END 	0x00019f93
#define TCC353X_BUFF_D_START 	0x00018000
#define TCC353X_BUFF_D_END 	0x00019f93
#else	/* TMM CSPI only */
#define TCC353X_REMAP_TYPE 	0x03		/* tmm cspi only */
#define TCC353X_BUFF_A_START 	0x00010000
#define TCC353X_BUFF_A_END 	0x00027F57	/* inside driver fix */
#define TCC353X_BUFF_B_START 	0x00018000
#define TCC353X_BUFF_B_END 	0x00019f93
#define TCC353X_BUFF_C_START 	0x00018000
#define TCC353X_BUFF_C_END 	0x00019f93
#define TCC353X_BUFF_D_START 	0x00018000
#define TCC353X_BUFF_D_END 	0x00019f93
#endif

/* set stream threshold (using interrupt) */
#define TCC353X_STREAM_THRESHOLD_SPISLV	(188*250)
#define TCC353X_STREAM_THRESHOLD_SPISLV_WH	\
	(((TCC353X_STREAM_THRESHOLD_SPISLV>>2)>>8)&0xFF)
#define TCC353X_STREAM_THRESHOLD_SPISLV_WL	\
	((TCC353X_STREAM_THRESHOLD_SPISLV>>2)&0xFF)

/* use large frame buffer for cspi only */
/* 
#define TCC353X_USE_STREAM_BUFFERING
*/
#if defined (TCC353X_USE_STREAM_BUFFERING)
/* about 400k : 200ms at 16QAM MAX */
#define TCC353X_STREAM_BUFFER_PKT	(3840)
#define TCC353X_STREAM_BUFFER_SIZE	(188*TCC353X_STREAM_BUFFER_PKT)
#else
#define TCC353X_STREAM_BUFFER_SIZE	(TCC353X_STREAM_THRESHOLD_SPISLV)
#endif

/* set stream threshold (not use interrupt) */
#define TCC353X_STREAM_THRESHOLD	(188*8)
#define TCC353X_STREAM_THRESHOLD_WH	(((TCC353X_STREAM_THRESHOLD>>2)>>8)\
					&0xFF)
#define TCC353X_STREAM_THRESHOLD_WL	((TCC353X_STREAM_THRESHOLD>>2)&0xFF)

/* driving strength for sts (GPIO 04~07) */
/* 0x00 : min , 0xF0 : max */
#define TCC353X_DRV_STR_GPIO_0x13_07_00 	0xF0	/* 0xF0 */

/* STS(TSIF) Clk polarity */
#define STS_CLK_POS			0x00
#define STS_CLK_NEG			0x80
#define STS_SYNC_ACT_LOW		0x40
#define STS_SYNC_ACT_HIGH		0x00
#define STS_FRM_ACT_LOW			0x20
#define STS_FRM_ACT_HIGH		0x00

/*
#define STS_POLARITY	(STS_CLK_POS|STS_SYNC_ACT_LOW|STS_FRM_ACT_LOW)
*/
#define STS_POLARITY	(STS_CLK_POS|STS_SYNC_ACT_HIGH|STS_FRM_ACT_HIGH)

/* set stream speed (DLR) */
#define TCC353X_DLR                             1

#if 0	/* for LNA Control - not used */
#define _USE_LNA_CONTROL_

/* lna gain control threshold */
#define DEF_LNA_GAIN_HIGH_2_LOW_THR 	(-60)
#define DEF_LNA_GAIN_LOW_2_HIGH_THR 	(-70)
#define DEF_LNA_CONTROL_COUNT_THR	(1)
#endif

#endif
