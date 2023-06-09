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
 #include "tcc353x_common.h"
#include "tcpal_os.h"

extern I32S Tcc353xAdaptSpiReadWrite(int moduleidx, I08U * pBufIn,
				     I08U * pBufOut, I32U Length,
				     I08U ReservedOption);
static I32S Tcc353xTccspiSingleRW(I32S _moduleIndex, I32S _chipAddress,
				  I08U _registerAddr, I08U * _data,
				  I08U _writeFlag);
static I32S Tcc353xTccspiMultiRW(I32S _moduleIndex, I32S _chipAddress,
				 I08U _registerAddr, I08U * _data,
				 I32S _size, I08U _writeFlag);
static I08U Tcc353xCheckCrc7(I08U * _data, I32U _len);
static I32S Tcc353xTccspiReset(I32S _moduleIndex, I32S _chipAddress);

#define DMA_MAX_SIZE	(2048)

#define SPICMD_BUFF_LEN     8
#define SPICMD_ACK          0x47

#define _WRITE_FLAG_        1
#define _READ_FLAG_         0

const I08U SdioCrc7Table[256] = {
	0x00, 0x09, 0x12, 0x1b, 0x24, 0x2d, 0x36, 0x3f,
	0x48, 0x41, 0x5a, 0x53, 0x6c, 0x65, 0x7e, 0x77,
	0x19, 0x10, 0x0b, 0x02, 0x3d, 0x34, 0x2f, 0x26,
	0x51, 0x58, 0x43, 0x4a, 0x75, 0x7c, 0x67, 0x6e,
	0x32, 0x3b, 0x20, 0x29, 0x16, 0x1f, 0x04, 0x0d,
	0x7a, 0x73, 0x68, 0x61, 0x5e, 0x57, 0x4c, 0x45,
	0x2b, 0x22, 0x39, 0x30, 0x0f, 0x06, 0x1d, 0x14,
	0x63, 0x6a, 0x71, 0x78, 0x47, 0x4e, 0x55, 0x5c,
	0x64, 0x6d, 0x76, 0x7f, 0x40, 0x49, 0x52, 0x5b,
	0x2c, 0x25, 0x3e, 0x37, 0x08, 0x01, 0x1a, 0x13,
	0x7d, 0x74, 0x6f, 0x66, 0x59, 0x50, 0x4b, 0x42,
	0x35, 0x3c, 0x27, 0x2e, 0x11, 0x18, 0x03, 0x0a,
	0x56, 0x5f, 0x44, 0x4d, 0x72, 0x7b, 0x60, 0x69,
	0x1e, 0x17, 0x0c, 0x05, 0x3a, 0x33, 0x28, 0x21,
	0x4f, 0x46, 0x5d, 0x54, 0x6b, 0x62, 0x79, 0x70,
	0x07, 0x0e, 0x15, 0x1c, 0x23, 0x2a, 0x31, 0x38,
	0x41, 0x48, 0x53, 0x5a, 0x65, 0x6c, 0x77, 0x7e,
	0x09, 0x00, 0x1b, 0x12, 0x2d, 0x24, 0x3f, 0x36,
	0x58, 0x51, 0x4a, 0x43, 0x7c, 0x75, 0x6e, 0x67,
	0x10, 0x19, 0x02, 0x0b, 0x34, 0x3d, 0x26, 0x2f,
	0x73, 0x7a, 0x61, 0x68, 0x57, 0x5e, 0x45, 0x4c,
	0x3b, 0x32, 0x29, 0x20, 0x1f, 0x16, 0x0d, 0x04,
	0x6a, 0x63, 0x78, 0x71, 0x4e, 0x47, 0x5c, 0x55,
	0x22, 0x2b, 0x30, 0x39, 0x06, 0x0f, 0x14, 0x1d,
	0x25, 0x2c, 0x37, 0x3e, 0x01, 0x08, 0x13, 0x1a,
	0x6d, 0x64, 0x7f, 0x76, 0x49, 0x40, 0x5b, 0x52,
	0x3c, 0x35, 0x2e, 0x27, 0x18, 0x11, 0x0a, 0x03,
	0x74, 0x7d, 0x66, 0x6f, 0x50, 0x59, 0x42, 0x4b,
	0x17, 0x1e, 0x05, 0x0c, 0x33, 0x3a, 0x21, 0x28,
	0x5f, 0x56, 0x4d, 0x44, 0x7b, 0x72, 0x69, 0x60,
	0x0e, 0x07, 0x1c, 0x15, 0x2a, 0x23, 0x38, 0x31,
	0x46, 0x4f, 0x54, 0x5d, 0x62, 0x6b, 0x70, 0x79
};

I08U NullDatas[DMA_MAX_SIZE];
I08U DummyDatas[DMA_MAX_SIZE];
I08U BurstDatas[DMA_MAX_SIZE];

void Tcc353xTccspiInit(void)
{
	TcpalMemset(&DummyDatas[0], 0x00, DMA_MAX_SIZE);
	TcpalMemset(&NullDatas[0], 0x00, DMA_MAX_SIZE);
}

I32S Tcc353xTccspiRead(I32S _moduleIndex, I32S _chipAddress,
		       I08U _registerAddr, I08U * _outData, I32S _size)
{
	I32S ret = TCC353X_RETURN_FAIL;
	I32S i;

	if (_size == 1) {
		ret =
		    Tcc353xTccspiSingleRW(_moduleIndex, _chipAddress,
					  _registerAddr, _outData,
					  _READ_FLAG_);
	} else if ((_size % 4) != 0) {
		for (i = 0; i < _size; i++) {
			if (_registerAddr & 0x80)
				ret =
				    Tcc353xTccspiSingleRW(_moduleIndex,
							  _chipAddress,
							  _registerAddr,
							  &_outData[i],
							  _READ_FLAG_);
			else
				ret =
				    Tcc353xTccspiSingleRW(_moduleIndex,
							  _chipAddress,
							  (I08U)
							  (_registerAddr +
							   i),
							  &_outData[i],
							  _READ_FLAG_);
		}
	} else {
		I32S result;
		I32S cmax;
		I32S cremain;
		I32S i;

		cmax = (_size / DMA_MAX_SIZE);
		cremain = (_size % DMA_MAX_SIZE);

		for (i = 0; i < cmax; i++) {
			result =
			    Tcc353xTccspiMultiRW(_moduleIndex,
						 _chipAddress,
						 _registerAddr,
						 &_outData[i *
							   DMA_MAX_SIZE],
						 DMA_MAX_SIZE - 1,
						 _READ_FLAG_);
			if (result != TCC353X_RETURN_SUCCESS) {
				Tcc353xTccspiReset(_moduleIndex,
						   _chipAddress);
				return TCC353X_RETURN_FAIL;
			}
		}

		if (cremain != 0) {
			result =
			    Tcc353xTccspiMultiRW(_moduleIndex,
						 _chipAddress,
						 _registerAddr,
						 &_outData[i *
							   DMA_MAX_SIZE],
						 cremain - 1, _READ_FLAG_);
			if (result != TCC353X_RETURN_SUCCESS) {
				Tcc353xTccspiReset(_moduleIndex,
						   _chipAddress);
				return TCC353X_RETURN_FAIL;
			}
		}
		ret = TCC353X_RETURN_SUCCESS;
	}

	if (ret != TCC353X_RETURN_SUCCESS) {
		Tcc353xTccspiReset(_moduleIndex, _chipAddress);
		return TCC353X_RETURN_FAIL;
	}

	return ret;
}

I32S Tcc353xTccspiWrite(I32S _moduleIndex, I32S _chipAddress,
			I08U _registerAddr, I08U * _inputData, I32S _size)
{
	I32S ret = TCC353X_RETURN_FAIL;
	I32S i;

	if (_size == 1) {
		ret =
		    Tcc353xTccspiSingleRW(_moduleIndex, _chipAddress,
					  _registerAddr, _inputData,
					  _WRITE_FLAG_);
	} else if ((_size % 4) != 0) {
		for (i = 0; i < _size; i++) {
			if (_registerAddr & 0x80)
				ret =
				    Tcc353xTccspiSingleRW(_moduleIndex,
							  _chipAddress,
							  _registerAddr,
							  &_inputData[i],
							  _WRITE_FLAG_);
			else
				ret =
				    Tcc353xTccspiSingleRW(_moduleIndex,
							  _chipAddress,
							  (I08U)
							  (_registerAddr +
							   i),
							  &_inputData[i],
							  _WRITE_FLAG_);
		}
	} else {
		I32S result;
		I32S cmax;
		I32S cremain;
		I32S i;

		cmax = (_size / DMA_MAX_SIZE);
		cremain = (_size % DMA_MAX_SIZE);

		for (i = 0; i < cmax; i++) {
			result =
			    Tcc353xTccspiMultiRW(_moduleIndex,
						 _chipAddress,
						 _registerAddr,
						 &_inputData[i *
							     DMA_MAX_SIZE],
						 DMA_MAX_SIZE - 1,
						 _WRITE_FLAG_);
			if (result != TCC353X_RETURN_SUCCESS) {
				Tcc353xTccspiReset(_moduleIndex,
						   _chipAddress);
				return TCC353X_RETURN_FAIL;
			}
		}

		if (cremain != 0) {
			result =
			    Tcc353xTccspiMultiRW(_moduleIndex,
						 _chipAddress,
						 _registerAddr,
						 &_inputData[i *
							     DMA_MAX_SIZE],
						 cremain - 1,
						 _WRITE_FLAG_);
			if (result != TCC353X_RETURN_SUCCESS) {
				Tcc353xTccspiReset(_moduleIndex,
						   _chipAddress);
				return TCC353X_RETURN_FAIL;
			}
		}
		ret = TCC353X_RETURN_SUCCESS;
	}

	if (ret != TCC353X_RETURN_SUCCESS) {
		Tcc353xTccspiReset(_moduleIndex, _chipAddress);
		return TCC353X_RETURN_FAIL;
	}

	return ret;
}

static I08U Tcc353xCheckCrc7(I08U * _data, I32U _len)
{
#ifndef CRC_CHECK_BY_TABLE
	I16U masking, carry;
	I16U crc;
	I32U i, loop, remain;

	crc = 0x0000;
	loop = _len / 8;
	remain = _len - loop * 8;

	for (i = 0; i < loop; i++) {
		masking = 1 << 8;
		while ((masking >>= 1)) {
			carry = crc & 0x40;
			crc <<= 1;
			if ((!carry) ^ (!(*_data & masking)))
				crc ^= 0x9;
			crc &= 0x7f;
		}
		_data++;
	}

	masking = 1 << 8;
	while (remain) {
		carry = crc & 0x40;
		crc <<= 1;
		masking >>= 1;
		if ((!carry) ^ (!(*_data & masking)))
			crc ^= 0x9;
		crc &= 0x7f;
		remain--;
	}

	return (I08U) crc;
#else
	I08U crc7_accum = 0;
	I32U i;

	for (i = 0; i < _len; i++) {
		crc7_accum = SdioCrc7Table[(crc7_accum << 1) ^ _data[i]];
	}
	return crc7_accum;
#endif
}

I32S Tcc353xSpiRW(I32S _moduleIndex, I08U * _bufferIn, I08U * _bufferOut,
		  I32S _size, I08U _reservedOption)
{
	/* for avoid const data confliction */
	TcpalMemcpy(&BurstDatas[0], _bufferIn, _size);
	Tcc353xAdaptSpiReadWrite(_moduleIndex, &BurstDatas[0], _bufferOut,
				 _size, _reservedOption);
	return TCC353X_RETURN_SUCCESS;
}

static I32S Tcc353xTccspiReset(I32S _moduleIndex, I32S _chipAddress)
{
	I08U FFData[SPICMD_BUFF_LEN];
	I08U buffout[SPICMD_BUFF_LEN];

	TcpalMemset(&FFData[0], 0xFF, SPICMD_BUFF_LEN);
	Tcc353xSpiRW(_moduleIndex, &FFData[0], buffout, SPICMD_BUFF_LEN,
		     0);
	return TCC353X_RETURN_SUCCESS;
}

static I32S Tcc353xTccspiSingleRW(I32S _moduleIndex, I32S _chipAddress,
				  I08U _registerAddr, I08U * _data,
				  I08U _writeFlag)
{
	I08U buffer[SPICMD_BUFF_LEN];
	I08U buffout[SPICMD_BUFF_LEN];
	I08U crc;

	buffer[0] = (I08U) (_chipAddress);	/* start bit(1) + chip_id(7) */

	/* mode(1) + rw(1) + fix(1) + addr(5) */
	buffer[1] =
	    0 << 7 | _writeFlag << 6 | 1 << 5 | ((_registerAddr & 0x7c0) >>
						 6);

	/* addr(6bit) + NULL(2bit) */
	buffer[2] = (_registerAddr & 0x03f) << 2 | 0x0;

	if (_writeFlag)		/* write */
		buffer[3] = _data[0];
	else
		buffer[3] = 0x0;	/* null(8) */

	buffer[4] = 0x00;

	crc = Tcc353xCheckCrc7(buffer, 36);
	buffer[4] = 0x00 | ((crc & 0x7f) >> 3);	/* null(4) + crc(4) */
	buffer[5] = ((crc & 0x07) << 5) | 0x1f;	/* crc(3) + end bit(5) */
	buffer[6] = 0xff;
	buffer[7] = 0xff;

	Tcc353xSpiRW(_moduleIndex, buffer, buffout, SPICMD_BUFF_LEN, 1);

	if (buffout[7] != SPICMD_ACK) {
		/* ack */
		TcpalPrintErr((I08S *) "[TCC353X] Single %s ACK error",
			      _writeFlag ? "Write" : "Read");
		TcpalPrintErr((I08S *)
			      "[TCC353X] [%02x][%02x][%02x][%02x] [%02x][%02x][%02x][%02x]//[%02x]\n",
			      buffer[0], buffer[1], buffer[2], buffer[3],
			      buffer[4], buffer[5], buffer[6], buffer[7],
			      crc);
		return TCC353X_RETURN_FAIL;
	}

	if (_writeFlag == 0) {
		_data[0] = buffout[6];
	}

	return TCC353X_RETURN_SUCCESS;
}

static I32S Tcc353xTccspiMultiRW(I32S _moduleIndex, I32S _chipAddress,
				 I08U _registerAddr, I08U * _data,
				 I32S _size, I08U _writeFlag)
{
	I08U crc;
	I08U buffer[SPICMD_BUFF_LEN];
	I08U buffout[SPICMD_BUFF_LEN];
	I08U fixedMode = 0;

	if (_registerAddr & 0x80)
		fixedMode = 1;
	else
		fixedMode = 0;

	_registerAddr = (_registerAddr & 0x7F);

	if (_size > DMA_MAX_SIZE)
		return (-1);

	/* MAX 16KB (Output buffer max size 7KB) (LENGTH + 1 Byte) */
	/* start bit(1) + chip_id(7) */
	buffer[0] = (I08U) (_chipAddress);	/* start bit(1) + chip_id(7) */
	/* mode(1) + rw(1) + fix(1) + addr(5) */
	buffer[1] =
	    1 << 7 | _writeFlag << 6 | fixedMode << 5 |
	    ((_registerAddr & 0x7c0) >> 6);
	/* addr(6bit) + length(2bit) */
	buffer[2] =
	    (I08U) ((_registerAddr & 0x03f) << 2 |
		    ((_size & 0x3000) >> 12));
	/* length(8bit) */
	buffer[3] = (I08U) ((_size & 0xff0) >> 4);
	buffer[4] = (I08U) ((_size & 0xf) << 4);
	crc = Tcc353xCheckCrc7(buffer, 36);
	/* length(4) + crc(4) */
	buffer[4] = (I08U) (((_size & 0xf) << 4) | ((crc & 0x7f) >> 3));
	/* crc(3) + end bit(5) */
	buffer[5] = ((crc & 0x07) << 5) | 0x1f;
	buffer[6] = 0xff;
	buffer[7] = 0xff;

	Tcc353xSpiRW(_moduleIndex, buffer, buffout, SPICMD_BUFF_LEN, 1);

	if (buffout[7] != SPICMD_ACK) {	/* ack */
		TcpalPrintErr((I08S *) "[TCC353X] Burst %s ACK error\n",
			      _writeFlag ? "Write" : "Read");
		TcpalPrintErr((I08S *)
			      "[TCC353X] [%x][%x][%x][%x] [%x][%x][%x][%x]//[%x]\n",
			      buffout[0], buffout[1], buffout[2],
			      buffout[3], buffout[4], buffout[5],
			      buffout[6], buffout[7], crc);
		return TCC353X_RETURN_FAIL;
	}

	if (_writeFlag == 0) {
		I08U FFData[SPICMD_BUFF_LEN];
		/* Receive Data */
		Tcc353xSpiRW(_moduleIndex, &NullDatas[0], _data, _size + 1,
			     0);

		/* initialize tcc353x spi cmd interface and 
		 * read crc for pData 
		 */
		TcpalMemset(&FFData[0], 0xFF, SPICMD_BUFF_LEN);
		Tcc353xSpiRW(_moduleIndex, &FFData[0], &buffout[0],
			     SPICMD_BUFF_LEN, 0);
		/*TODO check crc */
	} else {
		I08U FFData[SPICMD_BUFF_LEN];
		/* Send Data */
		Tcc353xSpiRW(_moduleIndex, _data, &DummyDatas[0],
			     _size + 1, 0);

		/* initialize tcc353x spi cmd interface */
		TcpalMemset(&FFData[0], 0xFF, SPICMD_BUFF_LEN);
		Tcc353xSpiRW(_moduleIndex, &FFData[0], &buffout[0],
			     SPICMD_BUFF_LEN, 0);
	}

	return TCC353X_RETURN_SUCCESS;
}
