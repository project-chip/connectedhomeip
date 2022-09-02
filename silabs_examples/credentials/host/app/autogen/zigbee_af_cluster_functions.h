/*******************************************************************************
 * @file zigbee_af_cluster_functions.h
 * @brief Cluster action callback definitions
 *******************************************************************************
 * # License
 * <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 ******************************************************************************/
//
// *** Generated file. Do not edit! ***
//

#include "af.h"



// Array of cluster function (aka cluster action callbacks) structures.
// Last entry is a dummy, otherwise an empty array would fail IAR builds.
#define GENERATED_FUNCTION_STRUCTURES_ARRAY  { \
  { 0x8000u,\
    0x00u,\
    (EmberAfGenericClusterFunction)((void *)0)\
  }\
}


// The following structure is not used anywhere in the code, its purpose is 
// compile-time detection of duplicate cluster action callbacks.
// Only a single instance of a given callback type (e.g. default_response_function)
// can exist for a given cluster and side (client/server).
// A compilation error in this structure indicates a duplicate "cluster_functions"
// template contribution.

