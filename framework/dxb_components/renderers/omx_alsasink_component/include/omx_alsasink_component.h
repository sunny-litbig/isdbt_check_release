/**
  OpenMAX ALSA sink component. This component is an audio sink that uses ALSA library.

  Copyright (C) 2007-2008  STMicroelectronics
  Copyright (C) 2007-2008 Nokia Corporation and/or its subsidiary(-ies).

  This library is free software; you can redistribute it and/or modify it under
  the terms of the GNU Lesser General Public License as published by the Free
  Software Foundation; either version 2.1 of the License, or (at your option)
  any later version.

  This library is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
  FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
  details.

  You should have received a copy of the GNU Lesser General Public License
  along with this library; if not, write to the Free Software Foundation, Inc.,
  51 Franklin St, Fifth Floor, Boston, MA
  02110-1301  USA

*/
/*******************************************************************************


*   Modified by Telechips Inc.


*   Modified date : 2020.05.26


*   Description : omx_alsasink_component.h


*******************************************************************************/ 
#ifndef _OMX_ALSASINK_COMPONENT_H_
#define _OMX_ALSASINK_COMPONENT_H_

#include <OMX_Types.h>
#include <OMX_Component.h>
#include <OMX_Core.h>
#include <OMX_Audio.h>
#include <pthread.h>
#include <omx_base_sink.h>

#include "TCCFile.h"
#include "TCCMemory.h"
#include <tccaudio.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Alsasinkport component private structure.
 * see the define above
 * @param sPCMModeParam Audio PCM specific OpenMAX parameter  
 * @param AudioPCMConfigured boolean flag to check if the audio has been configured 
 * @param playback_handle ALSA specif handle for audio player
 * @param xScale the scale of the media clock
 * @param eState the state of the media clock
 * @param hw_params ALSA specif hardware parameters 
 */
DERIVEDCLASS(omx_alsasink_component_PrivateType, omx_base_sink_PrivateType)
#define omx_alsasink_component_PrivateType_FIELDS omx_base_sink_PrivateType_FIELDS \
  OMX_AUDIO_PARAM_PCMMODETYPE  	sPCMModeParam[TOTAL_AUDIO_TRACK]; \
  char                         	AudioPCMConfigured;  \
  OMX_S32                      	xScale; \
  OMX_TIME_CLOCKSTATE          	eState; \
  int						   	alsa_configure_request; \
  int							iPatternToCheckPTSnSTC;		\
  int							iAudioSyncWaitTimeGap;		\
  int							iAudioSyncDropTimeGap;		\
  int							iAudioSyncJumpTimeGap;		\
  int							enable_output_audio_ch_index[TOTAL_AUDIO_TRACK]; 		\
  int							setVolumeValue[TOTAL_AUDIO_TRACK];		  \
  int							iAudioMuteConfig;		\
  OMX_BOOL						bAudioOutStartSyncWithVideo; \
  OMX_BOOL						frameDropFlag;		\
  OMX_BOOL						balsasinkbypass;	\
  int							balsasinkStopRequested[TOTAL_AUDIO_TRACK];	\
  int							audioskipOutput;	\
  int							iAlsaCloseFlag;	\
  OMX_U32						nOutputMode[TOTAL_AUDIO_TRACK];		\
  OMX_U32                       nPVRFlags; \
  OMX_PTR                       pPrivateData;
ENDCLASS(omx_alsasink_component_PrivateType)


/* Component private entry points declaration */
OMX_ERRORTYPE dxb_omx_alsasink_component_Destructor(OMX_COMPONENTTYPE *openmaxStandComp);
OMX_ERRORTYPE dxb_omx_alsasink_component_MessageHandler(OMX_COMPONENTTYPE* openmaxStandComp, internalRequestMessageType *message);
OMX_ERRORTYPE dxb_omx_alsasink_component_Deinit(OMX_COMPONENTTYPE *openmaxStandComp);
OMX_ERRORTYPE dxb_omx_alsasink_no_clockcomp_component_Init(OMX_COMPONENTTYPE *openmaxStandComp);
OMX_ERRORTYPE dxb_omx_alsasink_set_skip_output(int Off);


void dxb_omx_alsasink_component_BufferMgmtCallback(
  OMX_COMPONENTTYPE *openmaxStandComp,
  OMX_BUFFERHEADERTYPE* inputbuffer);

OMX_ERRORTYPE dxb_omx_alsasink_component_port_SendBufferFunction(
  omx_base_PortType *openmaxStandPort, 
  OMX_BUFFERHEADERTYPE* pBuffer);

OMX_ERRORTYPE dxb_omx_alsasink_component_GetParameter(
  OMX_IN  OMX_HANDLETYPE hComponent,
  OMX_IN  OMX_INDEXTYPE nParamIndex,
  OMX_INOUT OMX_PTR ComponentParameterStructure);

OMX_ERRORTYPE dxb_omx_alsasink_component_SetParameter(
  OMX_IN  OMX_HANDLETYPE hComponent,
  OMX_IN  OMX_INDEXTYPE nParamIndex,
  OMX_IN  OMX_PTR ComponentParameterStructure);

OMX_ERRORTYPE dxb_omx_alsasink_component_GetConfig(
  OMX_IN  OMX_HANDLETYPE hComponent,
  OMX_IN  OMX_INDEXTYPE nIndex,
  OMX_INOUT OMX_PTR pComponentConfigStructure) ;

  OMX_ERRORTYPE dxb_omx_alsasink_component_SetConfig(
  OMX_IN  OMX_HANDLETYPE hComponent,
  OMX_IN  OMX_INDEXTYPE nIndex,
  OMX_INOUT OMX_PTR pComponentConfigStructure) ;

#ifdef __cplusplus
}
#endif

#endif
