/******************************************************************************
 * # License
 * <b>Copyright 2023 Silicon Laboratories Inc. www.silabs.com</b>
 ******************************************************************************
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 *****************************************************************************/
#include <utility>
#include <vector>

#include "app-common/zap-generated/ids/Attributes.h"
#include "app-common/zap-generated/ids/Clusters.h"
/**
 * @defgroup mpc_volatile_attributes
 * @ingroup mpc_components
 * @brief Maitains the list of volatile attributes that MPC should
 *      monitor for each end node/device
 *
 * @{
 */

/// @brief minimum interval for MPC reportables
#define MPC_MIN_REPORT_INTERVAL (10 * 60) // 10 mins

/// @brief maximum interval for MPC reportables
#define MPC_MAX_REPORT_INTERVAL (60 * 60) // 60 mins

/// @brief retain previous MPC subscriptions
#define MPC_REPORT_KEEP_SUBS false

const std::vector<std::pair<chip::ClusterId, chip::AttributeId>> mpc_volatilte_attr_list{
    { chip::app::Clusters::OnOff::Id, chip::app::Clusters::OnOff::Attributes::OnOff::Id }
};

#ifndef MPC_VOLATILE_ATTRIBUTES_H
#define MPC_VOLATILE_ATTRIBUTES_H

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}
#endif

#endif // MPC_VOLATILE_ATTRIBUTES_H
/** @} end mpc_volatile_attributes */
