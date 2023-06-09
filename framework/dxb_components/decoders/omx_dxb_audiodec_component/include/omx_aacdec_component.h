/**

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


*   Description : omx_aacdec_component.h


*******************************************************************************/
#ifndef _OMX_AACDEC_COMPONENT_H_
#define _OMX_AACDEC_COMPONENT_H_

#include <omx_audiodec_component.h>

/* Component private entry points declaration */
OMX_ERRORTYPE dxb_omx_aacdec_component_Constructor(OMX_COMPONENTTYPE *openmaxStandComp,OMX_STRING cComponentName, int iAACType);
OMX_ERRORTYPE dxb_omx_audiodec_component_Change_AACdec(OMX_S16 nDevID, OMX_COMPONENTTYPE *openmaxStandComp, OMX_STRING cComponentName, int iAACType);

#endif /* _OMX_AACDEC_COMPONENT_H_ */

