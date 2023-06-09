/**

  OpenMAX Base Filter component. This component does not perform any multimedia
  processing. It derives from base component and contains two ports. It can be used 
  as a base class for codec and filter components.

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


*   Description : omx_base_filter.c


*******************************************************************************/ 
#define LOG_TAG	"omx_bse_filter"
#include <utils/Log.h>
#include <omxcore.h>

#include "omx_base_filter.h"


OMX_ERRORTYPE dxb_omx_base_filter_Constructor(OMX_COMPONENTTYPE *openmaxStandComp,OMX_STRING cComponentName) {
  OMX_ERRORTYPE err = OMX_ErrorNone;  
  omx_base_filter_PrivateType* omx_base_filter_Private;

  if (openmaxStandComp->pComponentPrivate) {
    omx_base_filter_Private = (omx_base_filter_PrivateType*)openmaxStandComp->pComponentPrivate;
  } else {
    omx_base_filter_Private = TCC_fo_calloc (__func__, __LINE__,1, sizeof(omx_base_filter_PrivateType));
    if (!omx_base_filter_Private) {
      return OMX_ErrorInsufficientResources;
    }
    openmaxStandComp->pComponentPrivate=omx_base_filter_Private;
  }

  /* Call the base class constructory */
  err = dxb_omx_base_component_Constructor(openmaxStandComp,cComponentName);

  /* here we can override whatever defaults the base_component constructor set
  * e.g. we can override the function pointers in the private struct */
  omx_base_filter_Private = openmaxStandComp->pComponentPrivate;

  omx_base_filter_Private->BufferMgmtFunction = dxb_omx_base_filter_BufferMgmtFunction;

  return err;
}

OMX_ERRORTYPE dxb_omx_base_filter_Destructor(OMX_COMPONENTTYPE *openmaxStandComp) {
  
  return dxb_omx_base_component_Destructor(openmaxStandComp);
}

/** This is the central function for component processing. It
  * is executed in a separate thread, is synchronized with 
  * semaphores at each port, those are released each time a new buffer
  * is available on the given port.
  */
void* dxb_omx_base_filter_BufferMgmtFunction (void* param) {
  OMX_COMPONENTTYPE* openmaxStandComp = (OMX_COMPONENTTYPE*)param;
  omx_base_component_PrivateType* omx_base_component_Private=(omx_base_component_PrivateType*)openmaxStandComp->pComponentPrivate;
  omx_base_filter_PrivateType* omx_base_filter_Private = (omx_base_filter_PrivateType*)omx_base_component_Private;
  omx_base_PortType *pInPort=(omx_base_PortType *)omx_base_filter_Private->ports[OMX_BASE_FILTER_INPUTPORT_INDEX];
  omx_base_PortType *pOutPort=(omx_base_PortType *)omx_base_filter_Private->ports[OMX_BASE_FILTER_OUTPUTPORT_INDEX];
  tsem_t* pInputSem = pInPort->pBufferSem;
  tsem_t* pOutputSem = pOutPort->pBufferSem;
  queue_t* pInputQueue = pInPort->pBufferQueue;
  queue_t* pOutputQueue = pOutPort->pBufferQueue;
  OMX_BUFFERHEADERTYPE* pOutputBuffer=NULL;
  OMX_BUFFERHEADERTYPE* pInputBuffer=NULL;
  OMX_BOOL isInputBufferNeeded=OMX_TRUE,isOutputBufferNeeded=OMX_TRUE;
  int inBufExchanged=0,outBufExchanged=0;

  DEBUG(DEB_LEV_FUNCTION_NAME, "In %s\n", __func__);
  while(omx_base_filter_Private->state == OMX_StateIdle || omx_base_filter_Private->state == OMX_StateExecuting ||  omx_base_filter_Private->state == OMX_StatePause || 
    omx_base_filter_Private->transientState == OMX_TransStateLoadedToIdle){

    /*Wait till the ports are being flushed*/
    pthread_mutex_lock(&omx_base_filter_Private->flush_mutex);
    while( PORT_IS_BEING_FLUSHED(pInPort) || 
           PORT_IS_BEING_FLUSHED(pOutPort)) {
      pthread_mutex_unlock(&omx_base_filter_Private->flush_mutex);
      
      DEBUG(DEB_LEV_FULL_SEQ, "In %s 1 signalling flush all cond iE=%d,iF=%d,oE=%d,oF=%d iSemVal=%d,oSemval=%d\n", 
        __func__,inBufExchanged,isInputBufferNeeded,outBufExchanged,isOutputBufferNeeded,pInputSem->semval,pOutputSem->semval);

      if(isOutputBufferNeeded==OMX_FALSE && PORT_IS_BEING_FLUSHED(pOutPort)) {
        pOutPort->ReturnBufferFunction(pOutPort,pOutputBuffer);
        outBufExchanged--;
        pOutputBuffer=NULL;
        isOutputBufferNeeded=OMX_TRUE;
        DEBUG(DEB_LEV_FULL_SEQ, "Ports are flushing,so returning output buffer\n");
      }

      if(isInputBufferNeeded==OMX_FALSE && PORT_IS_BEING_FLUSHED(pInPort)) {
        pInPort->ReturnBufferFunction(pInPort,pInputBuffer);
        inBufExchanged--;
        pInputBuffer=NULL;
        isInputBufferNeeded=OMX_TRUE;
        DEBUG(DEB_LEV_FULL_SEQ, "Ports are flushing,so returning input buffer\n");
      }

      DEBUG(DEB_LEV_FULL_SEQ, "In %s 2 signalling flush all cond iE=%d,iF=%d,oE=%d,oF=%d iSemVal=%d,oSemval=%d\n", 
        __func__,inBufExchanged,isInputBufferNeeded,outBufExchanged,isOutputBufferNeeded,pInputSem->semval,pOutputSem->semval);
  
      tsem_up(omx_base_filter_Private->flush_all_condition);
      tsem_down(omx_base_filter_Private->flush_condition);
      pthread_mutex_lock(&omx_base_filter_Private->flush_mutex);
    }
    pthread_mutex_unlock(&omx_base_filter_Private->flush_mutex);

    /*No buffer to process. So wait here*/
    if((isInputBufferNeeded==OMX_TRUE && pInputSem->semval==0) && 
      (omx_base_filter_Private->state != OMX_StateLoaded && omx_base_filter_Private->state != OMX_StateInvalid)) {
      //Signalled from EmptyThisBuffer or FillThisBuffer or some thing else
      DEBUG(DEB_LEV_FULL_SEQ, "Waiting for next input/output buffer\n");
      tsem_down(omx_base_filter_Private->bMgmtSem);
      
    }
    if(omx_base_filter_Private->state == OMX_StateLoaded || omx_base_filter_Private->state == OMX_StateInvalid) {
      DEBUG(DEB_LEV_SIMPLE_SEQ, "In %s Buffer Management Thread is exiting\n",__func__);
      break;
    }
    if((isOutputBufferNeeded==OMX_TRUE && pOutputSem->semval==0) && 
      (omx_base_filter_Private->state != OMX_StateLoaded && omx_base_filter_Private->state != OMX_StateInvalid) &&
       !(PORT_IS_BEING_FLUSHED(pInPort) || PORT_IS_BEING_FLUSHED(pOutPort))) {
      //Signalled from EmptyThisBuffer or FillThisBuffer or some thing else
      DEBUG(DEB_LEV_FULL_SEQ, "Waiting for next input/output buffer\n");
      tsem_down(omx_base_filter_Private->bMgmtSem);
      
    }
    if(omx_base_filter_Private->state == OMX_StateLoaded || omx_base_filter_Private->state == OMX_StateInvalid) {
      DEBUG(DEB_LEV_SIMPLE_SEQ, "In %s Buffer Management Thread is exiting\n",__func__);
      break;
    }
 
    DEBUG(DEB_LEV_SIMPLE_SEQ, "Waiting for input buffer semval=%d \n",pInputSem->semval);
    if(pInputSem->semval>0 && isInputBufferNeeded==OMX_TRUE ) {
      tsem_down(pInputSem);
      if(pInputQueue->nelem>0){
        inBufExchanged++;
        isInputBufferNeeded=OMX_FALSE;
        pInputBuffer = dxb_dequeue(pInputQueue);
        if(pInputBuffer == NULL){
          DEBUG(DEB_LEV_ERR, "Had NULL input buffer!!\n");
          break;
        }
	 if(checkHeader(pInputBuffer, sizeof(OMX_BUFFERHEADERTYPE)))
	 {
          DEBUG(DEB_LEV_ERR, "In %s, crashed input buffer!!\n", __func__);
	   isInputBufferNeeded = OMX_TRUE;
	   inBufExchanged--;
   	   pInputBuffer = NULL;
	 }
      }
    }
    /*When we have input buffer to process then get one output buffer*/
    if(pOutputSem->semval>0 && isOutputBufferNeeded==OMX_TRUE) {
      tsem_down(pOutputSem);
      if(pOutputQueue->nelem>0){
        outBufExchanged++;
        isOutputBufferNeeded=OMX_FALSE;
        pOutputBuffer = dxb_dequeue(pOutputQueue);
        if(pOutputBuffer == NULL){
          DEBUG(DEB_LEV_ERR, "Had NULL output buffer!! op is=%d,iq=%d\n",pOutputSem->semval,pOutputQueue->nelem);
          break;
        }
	 if(checkHeader(pOutputBuffer, sizeof(OMX_BUFFERHEADERTYPE)))
	 {
          DEBUG(DEB_LEV_ERR, "crashed output buffer!!\n");
	   isOutputBufferNeeded=OMX_TRUE;		  
   	   outBufExchanged--;
	   pOutputBuffer = NULL;
	 }		
      }
    }

    if(isInputBufferNeeded==OMX_FALSE) {
      if(pInputBuffer->hMarkTargetComponent != NULL){
        if((OMX_COMPONENTTYPE*)pInputBuffer->hMarkTargetComponent ==(OMX_COMPONENTTYPE *)openmaxStandComp) {
        /*Clear the mark and generate an event*/
        (*(omx_base_filter_Private->callbacks->EventHandler))
          (openmaxStandComp,
          omx_base_filter_Private->callbackData,
          OMX_EventMark, /* The command was completed */
          1, /* The commands was a OMX_CommandStateSet */
          0, /* The state has been changed in message->messageParam2 */
          pInputBuffer->pMarkData);
        } else {
          /*If this is not the target component then pass the mark*/
          omx_base_filter_Private->pMark.hMarkTargetComponent = pInputBuffer->hMarkTargetComponent;
          omx_base_filter_Private->pMark.pMarkData            = pInputBuffer->pMarkData;
        }
        pInputBuffer->hMarkTargetComponent = NULL;
      }
    }

    if(isInputBufferNeeded==OMX_FALSE && isOutputBufferNeeded==OMX_FALSE) {

      if(omx_base_filter_Private->pMark.hMarkTargetComponent != NULL){
        pOutputBuffer->hMarkTargetComponent = omx_base_filter_Private->pMark.hMarkTargetComponent;
        pOutputBuffer->pMarkData            = omx_base_filter_Private->pMark.pMarkData;
        omx_base_filter_Private->pMark.hMarkTargetComponent = NULL;
        omx_base_filter_Private->pMark.pMarkData            = NULL;
      }

      pOutputBuffer->nTimeStamp = pInputBuffer->nTimeStamp;

#if 0
      if(pInputBuffer->nFlags == OMX_BUFFERFLAG_SYNCFRAME) {    
         pOutputBuffer->nFlags = pInputBuffer->nFlags;
         pInputBuffer->nFlags = 0;
      }
#endif
      if (omx_base_filter_Private->BufferMgmtCallback && pInputBuffer->nFilledLen > 0) {
		  	if(omx_base_filter_Private->state == OMX_StateExecuting) {			  	
			    (*(omx_base_filter_Private->BufferMgmtCallback))(openmaxStandComp, pInputBuffer, pOutputBuffer);				
		  	} else {
		  		usleep(1000);
		  	}
			if(pInputBuffer->nFlags == OMX_BUFFERFLAG_SEEKONLY) {
		        	pOutputBuffer->nFlags = pInputBuffer->nFlags;
				pInputBuffer->nFlags = 0;
		  	}

		    if(pInputBuffer->nFlags == OMX_BUFFERFLAG_SYNCFRAME) {    
		        pOutputBuffer->nFlags = pInputBuffer->nFlags;
		        pInputBuffer->nFlags = 0;
		    }
      } else {
        /*It no buffer management call back the explicitly consume input buffer*/
        pInputBuffer->nFilledLen = 0;
        usleep(1000);
      }

      if(pInputBuffer->nFlags == OMX_BUFFERFLAG_STARTTIME) {    
         DEBUG(DEB_LEV_FULL_SEQ,"Detected  START TIME flag in the input buffer filled len=%d\n", (int)pInputBuffer->nFilledLen);
         pOutputBuffer->nFlags = pInputBuffer->nFlags;
         pInputBuffer->nFlags = 0;
      }
      
      if(((pInputBuffer->nFlags & OMX_BUFFERFLAG_EOS) == 1) && pInputBuffer->nFilledLen==0 ) {
        pOutputBuffer->nFlags=pInputBuffer->nFlags;
        pInputBuffer->nFlags=0;
        (*(omx_base_filter_Private->callbacks->EventHandler))
          (openmaxStandComp,
          omx_base_filter_Private->callbackData,
          OMX_EventBufferFlag, /* The command was completed */
          1, /* The commands was a OMX_CommandStateSet */
          pOutputBuffer->nFlags, /* The state has been changed in message->messageParam2 */
          NULL);
        omx_base_filter_Private->bIsEOSReached = OMX_TRUE;
      }
			
			if(PORT_IS_BEING_CHANGED(pOutPort))
			{
				ALOGD("PORT_IS_BEING_CHANGED before !! ");
				tsem_wait(omx_base_component_Private->bPortChangeSem);
				ALOGD("PORT_IS_BEING_CHANGED after !!  ");
			}
			
      if(omx_base_filter_Private->state==OMX_StatePause && !(PORT_IS_BEING_FLUSHED(pInPort) || PORT_IS_BEING_FLUSHED(pOutPort))) {
        /*Waiting at paused state*/
        tsem_wait(omx_base_component_Private->bStateSem);
      }
      /*If EOS and Input buffer Filled Len Zero then Return output buffer immediately*/
      if((pOutputBuffer->nFilledLen != 0) || ((pOutputBuffer->nFlags & OMX_BUFFERFLAG_EOS) == 1) || (omx_base_filter_Private->bIsEOSReached == OMX_TRUE)) {
        pOutPort->ReturnBufferFunction(pOutPort,pOutputBuffer);
        outBufExchanged--;
        pOutputBuffer=NULL;
        isOutputBufferNeeded=OMX_TRUE;

        
      }
//    } // removed by daniel

    if(omx_base_filter_Private->state==OMX_StatePause && !(PORT_IS_BEING_FLUSHED(pInPort) || PORT_IS_BEING_FLUSHED(pOutPort))) {
      /*Waiting at paused state*/
      tsem_wait(omx_base_component_Private->bStateSem);
    }

    /*Input Buffer has been completely consumed. So, return input buffer*/
    if((isInputBufferNeeded == OMX_FALSE) && (pInputBuffer->nFilledLen==0)) {
      pInPort->ReturnBufferFunction(pInPort,pInputBuffer);
      inBufExchanged--;
      pInputBuffer=NULL;
      isInputBufferNeeded=OMX_TRUE;
    }
    }	// added by daniel
  }
  DEBUG(DEB_LEV_SIMPLE_SEQ,"Exiting Buffer Management Thread\n");
  return NULL;
}

void* dxb_omx_twoport_filter_component_BufferMgmtFunction (void* param)
{
  OMX_COMPONENTTYPE* openmaxStandComp = (OMX_COMPONENTTYPE*)param;
  omx_base_component_PrivateType* omx_base_component_Private=(omx_base_component_PrivateType*)openmaxStandComp->pComponentPrivate;
  omx_base_filter_PrivateType* omx_base_filter_Private = (omx_base_filter_PrivateType*)omx_base_component_Private;
  omx_base_PortType *pInPort[2];
  omx_base_PortType *pOutPort=(omx_base_PortType *)omx_base_filter_Private->ports[2];
  tsem_t* pInputSem[2];
  tsem_t* pOutputSem = pOutPort->pBufferSem;
  queue_t* pInputQueue[2];
  queue_t* pOutputQueue = pOutPort->pBufferQueue;
  OMX_BUFFERHEADERTYPE* pOutputBuffer=NULL;
  OMX_BUFFERHEADERTYPE* pInputBuffer[2];
  OMX_BOOL isInputBufferNeeded[2],isOutputBufferNeeded=OMX_TRUE;
  int inBufExchanged[2],outBufExchanged=0;
  int i;

  DEBUG(DEB_LEV_FULL_SEQ, "In %s\n", __func__);

  for (i=0; i < 2; i++) {
  	pInPort[i] = (omx_base_PortType *)omx_base_filter_Private->ports[i];
	pInputSem[i] = pInPort[i]->pBufferSem;
	pInputQueue[i]= pInPort[i]->pBufferQueue;
	isInputBufferNeeded[i] = OMX_TRUE;
	inBufExchanged[i] = 0;
	pInputBuffer[i] = NULL;
  }
  while(omx_base_filter_Private->state == OMX_StateIdle || omx_base_filter_Private->state == OMX_StateExecuting ||  omx_base_filter_Private->state == OMX_StatePause || 
    omx_base_filter_Private->transientState == OMX_TransStateLoadedToIdle) {

    /*Wait till the ports are being flushed*/
    pthread_mutex_lock(&omx_base_filter_Private->flush_mutex);
    while( PORT_IS_BEING_FLUSHED(pInPort[0]) || PORT_IS_BEING_FLUSHED(pInPort[1]) || PORT_IS_BEING_FLUSHED(pOutPort)) {
      pthread_mutex_unlock(&omx_base_filter_Private->flush_mutex);
      
      DEBUG(DEB_LEV_FULL_SEQ, "In %s 1 signalling flush all cond iE=%d,%d,iF=%d,%d,oE=%d,oF=%d iSemVal=%d,%d,oSemval=%d\n",
        __func__,inBufExchanged[0],inBufExchanged[1],isInputBufferNeeded[0],isInputBufferNeeded[1],outBufExchanged,
        isOutputBufferNeeded,pInputSem[0]->semval,pInputSem[1]->semval,pOutputSem->semval);

      if(isOutputBufferNeeded==OMX_FALSE && PORT_IS_BEING_FLUSHED(pOutPort)) {
        pOutPort->ReturnBufferFunction(pOutPort,pOutputBuffer);
        outBufExchanged--;
        pOutputBuffer=NULL;
        isOutputBufferNeeded=OMX_TRUE;
        DEBUG(DEB_LEV_FULL_SEQ, "OutPorts are flushing,so returning output buffer\n");
      }

      for (i=0; i < 2; i++) {
        if(isInputBufferNeeded[i]==OMX_FALSE && PORT_IS_BEING_FLUSHED(pInPort[i])) {
          pInPort[i]->ReturnBufferFunction(pInPort[i],pInputBuffer[i]);
          inBufExchanged[i]--;
          pInputBuffer[i]=NULL;
          isInputBufferNeeded[i]=OMX_TRUE;
          DEBUG(DEB_LEV_FULL_SEQ, "InPorts[%d] are flushing,so returning input buffer\n", i);
        }
      }

      DEBUG(DEB_LEV_FULL_SEQ, "In %s 2 signalling flush all cond iE=%d,%d,iF=%d,%d,oE=%d,oF=%d iSemVal=%d,%d,oSemval=%d\n", 
        __func__,inBufExchanged[0],inBufExchanged[1],isInputBufferNeeded[0],isInputBufferNeeded[1],outBufExchanged,isOutputBufferNeeded,pInputSem[0]->semval,pInputSem[1]->semval,pOutputSem->semval);
  
      tsem_up(omx_base_filter_Private->flush_all_condition);
      tsem_down(omx_base_filter_Private->flush_condition);
      pthread_mutex_lock(&omx_base_filter_Private->flush_mutex);
    }
    pthread_mutex_unlock(&omx_base_filter_Private->flush_mutex);

    /*No buffer to process. So wait here*/
    if((isInputBufferNeeded[0]==OMX_TRUE && pInputSem[0]->semval==0) && (isInputBufferNeeded[1]==OMX_TRUE && pInputSem[1]->semval==0) &&
      (omx_base_filter_Private->state != OMX_StateLoaded && omx_base_filter_Private->state != OMX_StateInvalid)) {
      //Signalled from EmptyThisBuffer or FillThisBuffer or some thing else
      DEBUG(DEB_LEV_FULL_SEQ, "Waiting for next input/output buffer\n");
      tsem_down(omx_base_filter_Private->bMgmtSem);
      
    }
    if(omx_base_filter_Private->state == OMX_StateLoaded || omx_base_filter_Private->state == OMX_StateInvalid) {
      DEBUG(DEB_LEV_FULL_SEQ, "In %s Buffer Management Thread is exiting\n",__func__);
      break;
    }
    if((isOutputBufferNeeded==OMX_TRUE && pOutputSem->semval==0) && 
      (omx_base_filter_Private->state != OMX_StateLoaded && omx_base_filter_Private->state != OMX_StateInvalid) &&
       !(PORT_IS_BEING_FLUSHED(pInPort[0]) || PORT_IS_BEING_FLUSHED(pInPort[1]) || PORT_IS_BEING_FLUSHED(pOutPort))) {
      //Signalled from EmptyThisBuffer or FillThisBuffer or some thing else
      DEBUG(DEB_LEV_FULL_SEQ, "Waiting for next input/output buffer\n");
      tsem_down(omx_base_filter_Private->bMgmtSem);
    }
    if(omx_base_filter_Private->state == OMX_StateLoaded || omx_base_filter_Private->state == OMX_StateInvalid) {
      DEBUG(DEB_LEV_FULL_SEQ, "In %s Buffer Management Thread is exiting\n",__func__);
      break;
    }
 
    DEBUG(DEB_LEV_FULL_SEQ, "Waiting for input buffer semval=%d,%d \n",pInputSem[0]->semval,pInputSem[1]->semval);
	for (i=0; i < 2;i++) {
      if(pInputSem[i]->semval>0 && isInputBufferNeeded[i]==OMX_TRUE ) {
        tsem_down(pInputSem[i]);
        if(pInputQueue[i]->nelem>0){
          inBufExchanged[i]++;
          isInputBufferNeeded[i]=OMX_FALSE;
          pInputBuffer[i] = dxb_dequeue(pInputQueue[i]);
          if(pInputBuffer[i] == NULL){
            DEBUG(DEB_LEV_FULL_SEQ, "Had NULL input buffer!!\n");
            break;
          }
          if(checkHeader(pInputBuffer[i], sizeof(OMX_BUFFERHEADERTYPE)))
          {
            DEBUG(DEB_LEV_FULL_SEQ, "In %s, crashed input buffer!!\n", __func__);
            isInputBufferNeeded[i] = OMX_TRUE;
            inBufExchanged[i]--;
            pInputBuffer[i] = NULL;
          }
        }
      }
    }
    /*When we have input buffer to process then get one output buffer*/
    if(pOutputSem->semval>0 && isOutputBufferNeeded==OMX_TRUE) {
      tsem_down(pOutputSem);
      if(pOutputQueue->nelem>0){
        outBufExchanged++;
        isOutputBufferNeeded=OMX_FALSE;
        pOutputBuffer = dxb_dequeue(pOutputQueue);
        if(pOutputBuffer == NULL){
          DEBUG(DEB_LEV_FULL_SEQ, "Had NULL output buffer!! op is=%d,iq=%d\n",pOutputSem->semval,pOutputQueue->nelem);
          break;
        }
        if(checkHeader(pOutputBuffer, sizeof(OMX_BUFFERHEADERTYPE)))
        {
          DEBUG(DEB_LEV_FULL_SEQ, "crashed output buffer!!\n");
          isOutputBufferNeeded=OMX_TRUE;		  
          outBufExchanged--;
          pOutputBuffer = NULL;
        }
      }
    }

    for (i=0;i < 2; i++) {
      if(isInputBufferNeeded[i]==OMX_FALSE) {
        if(pInputBuffer[i] != NULL && pInputBuffer[i]->hMarkTargetComponent != NULL){
          if((OMX_COMPONENTTYPE*)pInputBuffer[i]->hMarkTargetComponent ==(OMX_COMPONENTTYPE *)openmaxStandComp) {
            /*Clear the mark and generate an event*/
            (*(omx_base_filter_Private->callbacks->EventHandler))
              (openmaxStandComp,
              omx_base_filter_Private->callbackData,
              OMX_EventMark, /* The command was completed */
              1, /* The commands was a OMX_CommandStateSet */
              0, /* The state has been changed in message->messageParam2 */
              pInputBuffer[i]->pMarkData);
          } else {
            /*If this is not the target component then pass the mark*/
            omx_base_filter_Private->pMark.hMarkTargetComponent = pInputBuffer[i]->hMarkTargetComponent;
            omx_base_filter_Private->pMark.pMarkData            = pInputBuffer[i]->pMarkData;
          }
          pInputBuffer[i]->hMarkTargetComponent = NULL;
        } 
      }

      if(isInputBufferNeeded[i]==OMX_FALSE && isOutputBufferNeeded==OMX_FALSE && pInputBuffer[i] != NULL) {
        if(omx_base_filter_Private->pMark.hMarkTargetComponent != NULL){
          pOutputBuffer->hMarkTargetComponent = omx_base_filter_Private->pMark.hMarkTargetComponent;
          pOutputBuffer->pMarkData            = omx_base_filter_Private->pMark.pMarkData;
          omx_base_filter_Private->pMark.hMarkTargetComponent = NULL;
          omx_base_filter_Private->pMark.pMarkData            = NULL;
        }

        pOutputBuffer->nTimeStamp = pInputBuffer[i]->nTimeStamp;

        if (omx_base_filter_Private->BufferMgmtCallback && pInputBuffer[i]->nFilledLen > 0) {
          if(omx_base_filter_Private->state == OMX_StateExecuting) {			  	
            (*(omx_base_filter_Private->BufferMgmtCallback))(openmaxStandComp, pInputBuffer[i], pOutputBuffer);				
          }
          if(pInputBuffer[i]->nFlags == OMX_BUFFERFLAG_SEEKONLY) {
            pOutputBuffer->nFlags = pInputBuffer[i]->nFlags;
            pInputBuffer[i]->nFlags = 0;
          }

          if(pInputBuffer[i]->nFlags == OMX_BUFFERFLAG_SYNCFRAME) {    
            pOutputBuffer->nFlags = pInputBuffer[i]->nFlags;
            pInputBuffer[i]->nFlags = 0;
          }
        } else {
          /*It no buffer management call back the explicitly consume input buffer*/
          pInputBuffer[i]->nFilledLen = 0;
        }

        if(pInputBuffer[i]->nFlags == OMX_BUFFERFLAG_STARTTIME) {    
           DEBUG(DEB_LEV_FULL_SEQ, "Detected  START TIME flag in the input[%d] buffer filled len=%d\n", i, (int)pInputBuffer[i]->nFilledLen);
           pOutputBuffer->nFlags = pInputBuffer[i]->nFlags;
           pInputBuffer[i]->nFlags = 0;
        }
    
        if(((pInputBuffer[i]->nFlags & OMX_BUFFERFLAG_EOS) == 1) && pInputBuffer[i]->nFilledLen==0 ) {
          pOutputBuffer->nFlags=pInputBuffer[i]->nFlags;
          pInputBuffer[i]->nFlags=0;
          (*(omx_base_filter_Private->callbacks->EventHandler))
            (openmaxStandComp,
            omx_base_filter_Private->callbackData,
            OMX_EventBufferFlag, /* The command was completed */
            1, /* The commands was a OMX_CommandStateSet */
            pOutputBuffer->nFlags, /* The state has been changed in message->messageParam2 */
            NULL);
          omx_base_filter_Private->bIsEOSReached = OMX_TRUE;
        }

        if(PORT_IS_BEING_CHANGED(pOutPort))
        {
          ALOGD("PORT_IS_BEING_CHANGED before !! ");
          tsem_wait(omx_base_component_Private->bPortChangeSem);
          ALOGD("PORT_IS_BEING_CHANGED after !!  ");
        }
			
        if(omx_base_filter_Private->state==OMX_StatePause && !(PORT_IS_BEING_FLUSHED(pInPort[0]) || PORT_IS_BEING_FLUSHED(pInPort[1]) || PORT_IS_BEING_FLUSHED(pOutPort))) {
          /*Waiting at paused state*/
          ALOGE("In %s, component state is paused\n", __func__);
          tsem_wait(omx_base_component_Private->bStateSem);
        }
        /*If EOS and Input buffer Filled Len Zero then Return output buffer immediately*/
        if((pOutputBuffer->nFilledLen != 0) || ((pOutputBuffer->nFlags & OMX_BUFFERFLAG_EOS) == 1) || (omx_base_filter_Private->bIsEOSReached == OMX_TRUE)) {
          pOutPort->ReturnBufferFunction(pOutPort,pOutputBuffer);
          outBufExchanged--;
          pOutputBuffer=NULL;
          isOutputBufferNeeded=OMX_TRUE;
        }

        if(omx_base_filter_Private->state==OMX_StatePause && !(PORT_IS_BEING_FLUSHED(pInPort[0]) || PORT_IS_BEING_FLUSHED(pInPort[1]) || PORT_IS_BEING_FLUSHED(pOutPort))) {
          /*Waiting at paused state*/
          ALOGE("In %s, component state is paused\n", __func__);
          tsem_wait(omx_base_component_Private->bStateSem);
        }

        /*Input Buffer has been completely consumed. So, return input buffer*/
        if((isInputBufferNeeded[i]== OMX_FALSE) && (pInputBuffer[i]->nFilledLen==0)) {
          pInPort[i]->ReturnBufferFunction(pInPort[i],pInputBuffer[i]);
          inBufExchanged[i]--;
          pInputBuffer[i]=NULL;
          isInputBufferNeeded[i]=OMX_TRUE;
        }
      }
    }	// added by daniel
  }
  DEBUG(DEB_LEV_SIMPLE_SEQ,"Exiting Buffer Management Thread\n");
  return NULL;
}

