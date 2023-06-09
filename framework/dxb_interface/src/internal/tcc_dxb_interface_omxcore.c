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
#define LOG_NDEBUG 0
#define LOG_TAG	"OMX_CORE"
#include <utils/Log.h>


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <dirent.h>
#include <strings.h>
#include <errno.h>

#include <OMX_Types.h>
#include <OMX_Core.h>
#include <OMX_Component.h>


#include "omxcore.h"


#if 0
#undef  DEBUG
#define DEBUG(n, args...) ALOGE(args)
#define printf(args...) ALOGE(args)
#endif


/** The following include file contains the st staic component loader st_static_loader
 */

/** The static field initialized is equal to OMX_FALSE if the core is not initialized. 
 * It is equal to OMX_TRUE whern the OMX_Init has been called
 */
static OMX_BOOL initialized;

/** The macro NUM_LOADERS contains the number of loaders available in the system.
 */
#define NUM_LOADERS 1

/** The pointer to the loaders list. This list contains the all the different component loaders 
 * present in the system or added by the IL Client with the BOSA_AddComponentLoader function.
 * The component loader is a implementation specific way to handle a set of components. The implementation
 * of the IL core accesses to the loaders in a standard way, but the different loaders can handle
 * different types of components, or handle in different ways the same components. It can be used also
 * to create a multi-OS support
 */
static BOSA_COMPONENTLOADER *loadersList[NUM_LOADERS];

/** @brief The OMX_Init standard function
 * 
 * This function calls the init function of each componente loader added. If there 
 * is no component loaders present, the ST default component loader (static libraries)
 * is loaded as default component loader.
 * 
 * @return OMX_ErrorNone
 */

#if 0

extern void st_static_InitComponentLoader();

OMX_ERRORTYPE OMX_Init() 
{
  	printf("OMX_Init\n");

	  DEBUG(DEB_LEV_FUNCTION_NAME, "In %s \n", __func__);
	  initialized = 0;
	  if(!initialized) 
	  {
		    int i;
		    OMX_ERRORTYPE err;

		    st_static_InitComponentLoader();
		    loadersList[0] = &st_static_loader;

		    for (i = 0; i<NUM_LOADERS; i++) 
		    {
			      err = loadersList[i]->BOSA_InitComponentLoader(loadersList[i]);
			      if (err != OMX_ErrorNone) 
			      {
				        DEBUG(DEB_LEV_ERR, "A Component loader constructor fails. Exiting\n");
				        return err;
			      }
		    }
		    initialized = OMX_TRUE;
	  }
	  DEBUG(DEB_LEV_FUNCTION_NAME, "Out of %s\n", __func__);
	  return OMX_ErrorNone;
}
#endif

/** @brief The OMX_Deinit standard function
 * 
 * In this function the Deinit function for each component loader is performed
 */
OMX_ERRORTYPE OMX_Deinit() 
{
	  DEBUG(DEB_LEV_FUNCTION_NAME, "In %s\n", __func__);
	  if(initialized)
	  {
		    int i;
		    for (i = 0; i<NUM_LOADERS; i++) 
		    {
		     	 loadersList[i]->BOSA_DeInitComponentLoader(loadersList[i]);
		    }
		    initialized = OMX_FALSE;
	  }
	  DEBUG(DEB_LEV_FUNCTION_NAME, "Out of %s\n", __func__);
	  return OMX_ErrorNone;
}

/** @brief the OMX_GetHandle standard function
 * 
 * This function will scan inside any component loader to search for
 * the requested component. If there are more components with the same name 
 * the first component is returned. The existence of multiple components with
 * the same name is not contemplated in OpenMAX specification. The assumption is
 * that this behavior is NOT allowed
 * 
 * @return OMX_ErrorNone if a component has been found
 *         OMX_ErrorComponentNotFound if the requested component has not been found 
 *                                    in any loader
 */
OMX_ERRORTYPE OMX_GetHandle(OMX_OUT OMX_HANDLETYPE* pHandle,
  OMX_IN  OMX_STRING cComponentName,
  OMX_IN  OMX_PTR pAppData,
  OMX_IN  OMX_CALLBACKTYPE* pCallBacks)
  {

	  int i;
	  OMX_ERRORTYPE err;
	  DEBUG(DEB_LEV_FUNCTION_NAME, "In %s\n", __func__);

	  for (i = 0; i<NUM_LOADERS; i++) 
	  {
		    err = loadersList[i]->BOSA_CreateComponent(
		            loadersList[i],
		            pHandle,
		            cComponentName,
		            pAppData,
		            pCallBacks);
		    if (err == OMX_ErrorNone) 
		    {
			      // the component has been found
			      return OMX_ErrorNone;
		    }
		    else if (err == OMX_ErrorInsufficientResources) 
		    {
		    	  return OMX_ErrorInsufficientResources;
		    }
	  }
	  DEBUG(DEB_LEV_FUNCTION_NAME, "Out of %s\n", __func__);
	  return OMX_ErrorComponentNotFound;
}

/** @brief the OMX_FreeHandle standard function
 * 
 * This function calls the component loader destroy function.
 */
OMX_ERRORTYPE OMX_FreeHandle(OMX_IN OMX_HANDLETYPE hComponent)
{
	  int i;
	  OMX_ERRORTYPE err;
	  for (i = 0; i < NUM_LOADERS; i++) 
	  {
		    err = loadersList[i]->BOSA_DestroyComponent(loadersList[i], hComponent);
		    if (err == OMX_ErrorNone) 
		    {
			      // the component has been found and destroyed
			      return OMX_ErrorNone;
		    }
	  }
	  return OMX_ErrorComponentNotFound;
}

/** @brief the OMX_ComponentNameEnum standard function
 * 
 * This function build a complete list of names from all the loaders. 
 * FOr each loaader the index is from 0 to max, but this function must provide a single 
 * list, with a common index. This implementation orders the loaders and the 
 * related list of components
 */
OMX_ERRORTYPE OMX_ComponentNameEnum(OMX_OUT OMX_STRING cComponentName,
  OMX_IN OMX_U32 nNameLength,
  OMX_IN OMX_U32 nIndex)
  {
	  OMX_ERRORTYPE err = OMX_ErrorNone;
	  int i, offset = 0;
	  int end_index, index = 0;

	  DEBUG(DEB_LEV_FUNCTION_NAME, "In %s\n", __func__);
	  for (i = 0; i < NUM_LOADERS; i++) 
	  {
		    err = loadersList[i]->BOSA_ComponentNameEnum(
		            loadersList[i],
		            cComponentName,
		            nNameLength,
		            nIndex - offset);
		    if (err != OMX_ErrorNone) 
		    {
			      /** The component has been not found with the current loader.
			      * the first step is to find the curent number of component for the
			      * current loader, and use it as offset for the next loader
			      */
			      end_index = 0;
			      index = 0;
			      while (!end_index) 
			      {
				        err = loadersList[i]->BOSA_ComponentNameEnum(
				        loadersList[i],
				        cComponentName,
				        nNameLength,
				        index);
				        if (err == OMX_ErrorNone) 
				        {
				        	  index++;
				        }
				        else 
				        {
					          end_index = 1;
					          offset+=index;
				        }
			      }
		    } 
		    else 
		    {
			      DEBUG(DEB_LEV_FUNCTION_NAME, "Out of %s with OMX_ErrorNone\n", __func__);
			      return OMX_ErrorNone;
		    }
	  }
	  cComponentName = NULL;
	  DEBUG(DEB_LEV_FUNCTION_NAME, "Out of %s with OMX_ErrorNoMore\n", __func__);
	  return OMX_ErrorNoMore;
}

/** @brief the OMX_SetupTunnel standard function
 * 
 * The implementation of this function is described in the OpenMAX spec
 */
OMX_ERRORTYPE OMX_SetupTunnel(
  OMX_IN  OMX_HANDLETYPE hOutput,
  OMX_IN  OMX_U32 nPortOutput,
  OMX_IN  OMX_HANDLETYPE hInput,
  OMX_IN  OMX_U32 nPortInput) 
{

	  OMX_ERRORTYPE err;
	  OMX_COMPONENTTYPE* component;
	  OMX_TUNNELSETUPTYPE tunnelSetup;

	  DEBUG(DEB_LEV_FUNCTION_NAME, "In %s\n", __func__);

	  if (hOutput == NULL || hInput == NULL) 
	  {
	    return OMX_ErrorBadParameter;
	  }

	  component = (OMX_COMPONENTTYPE*)hOutput;
	  tunnelSetup.nTunnelFlags = 0;
	  tunnelSetup.eSupplier = 0;

/*
Noah, To avoid CodeSonar warning, Redundant Condition
"if (hOutput)" is always true because NULL check has been already checked above.
	  if (hOutput)
*/
	  {
		    err = component->ComponentTunnelRequest(hOutput, nPortOutput, hInput, nPortInput, &tunnelSetup);
		    if (err != OMX_ErrorNone) 
		    {
			      DEBUG(DEB_LEV_ERR, "Tunneling failed: output port rejects it - err = %i\n", err);
			      return err;
		    }
	  }
	  DEBUG(DEB_LEV_PARAMS, "First stage of tunneling acheived:\n");
	  DEBUG(DEB_LEV_PARAMS, "       - supplier proposed = %u\n",  tunnelSetup.eSupplier);
	  DEBUG(DEB_LEV_PARAMS, "       - flags             = %lu\n", tunnelSetup.nTunnelFlags);

	  component = (OMX_COMPONENTTYPE*)hInput;
/*
Noah, To avoid CodeSonar warning, Redundant Condition
"if (hInput)" is always true because NULL check has been already checked above.
	  if (hInput)    
*/
	  {
		    err = component->ComponentTunnelRequest(hInput, nPortInput, hOutput, nPortOutput, &tunnelSetup);
		    if (err != OMX_ErrorNone)
		    {
			      DEBUG(DEB_LEV_ERR, "Tunneling failed: input port rejects it - err = %08x\n", err);
			      // the second stage fails. the tunnel on poutput port has to be removed
			      component = (OMX_COMPONENTTYPE*)hOutput;
			      err = component->ComponentTunnelRequest(hOutput, nPortOutput, NULL, 0, &tunnelSetup);
			      if (err != OMX_ErrorNone)
			      {
				        // This error should never happen. It is critical, and not recoverable
				        DEBUG(DEB_LEV_FUNCTION_NAME, "Out of %s with OMX_ErrorUndefined\n", __func__);
				        return OMX_ErrorUndefined;
			      }
			      DEBUG(DEB_LEV_FUNCTION_NAME, "Out of %s with OMX_ErrorPortsNotCompatible\n", __func__);
			      return OMX_ErrorPortsNotCompatible;
		    }
	  }
	  DEBUG(DEB_LEV_PARAMS, "Second stage of tunneling acheived:\n");
	  DEBUG(DEB_LEV_PARAMS, "       - supplier proposed = %u\n",  tunnelSetup.eSupplier);
	  DEBUG(DEB_LEV_PARAMS, "       - flags             = %lu\n", tunnelSetup.nTunnelFlags);
	  DEBUG(DEB_LEV_FUNCTION_NAME, "Out of %s\n", __func__);
	  return OMX_ErrorNone;
}

/** @brief the OMX_GetRolesOfComponent standard function
 */ 
OMX_ERRORTYPE OMX_GetRolesOfComponent ( 
  OMX_IN      OMX_STRING CompName, 
  OMX_INOUT   OMX_U32 *pNumRoles,
  OMX_OUT     OMX_U8 **roles) 
  {
	  OMX_ERRORTYPE err;
	  int i;

	  DEBUG(DEB_LEV_FUNCTION_NAME, "In %s\n", __func__);
	  for (i = 0; i<NUM_LOADERS; i++)
	  {
		    err = loadersList[i]->BOSA_GetRolesOfComponent(
		            loadersList[i],
		            CompName,
		            pNumRoles,
		            roles);
		    if (err == OMX_ErrorNone) 
		    {
		     	 return OMX_ErrorNone;
		    }
	  }
	  DEBUG(DEB_LEV_FUNCTION_NAME, "Out of %s\n", __func__);
	  return OMX_ErrorComponentNotFound;
}

/** @brief the OMX_GetComponentsOfRole standard function
 * 
 * This function searches in all the component loaders any component
 * supporting the requested role
 */
OMX_ERRORTYPE OMX_GetComponentsOfRole ( 
  OMX_IN      OMX_STRING role,
  OMX_INOUT   OMX_U32 *pNumComps,
  OMX_INOUT   OMX_U8  **compNames) 
  {
	  OMX_ERRORTYPE err;
	  int i;
	  int only_number_requested, full_number=0;
	  OMX_U32 temp_num_comp;

	  DEBUG(DEB_LEV_FUNCTION_NAME, "In %s\n", __func__);
	  if (compNames == NULL) 
	  {
	   	 only_number_requested = 1;
	  } 
	  else 
	  {
	   	 only_number_requested = 0;
	  }
	  for (i = 0; i<NUM_LOADERS; i++) 
	  {
		    temp_num_comp = *pNumComps;
		    err = loadersList[i]->BOSA_GetComponentsOfRole(
		            loadersList[i],
		            role,
		            &temp_num_comp,
		            NULL);
		    if (err != OMX_ErrorNone) 
		    {
			      DEBUG(DEB_LEV_FUNCTION_NAME, "Out of %s\n", __func__);
			      return OMX_ErrorComponentNotFound;
		    }
		    if (only_number_requested == 0)
		    {
			      err = loadersList[i]->BOSA_GetComponentsOfRole(
			              loadersList[i],
			              role,
			              pNumComps,
			              compNames);
			      if (err != OMX_ErrorNone)
			      {
				        DEBUG(DEB_LEV_FUNCTION_NAME, "Out of %s\n", __func__);
				        return OMX_ErrorComponentNotFound;
			      }
		    }
		    full_number += temp_num_comp;
	  }
	  *pNumComps = full_number;
	  DEBUG(DEB_LEV_FUNCTION_NAME, "Out of %s\n", __func__);
	  return OMX_ErrorNone;
}
