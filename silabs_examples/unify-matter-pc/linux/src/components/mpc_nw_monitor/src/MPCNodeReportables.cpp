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
#include "MPCNodeReportables.hpp"
#include "mpc_attribute_parser_fwk.h"
#include "mpc_attribute_store_defined_attribute_types.h"
#include "mpc_node_monitor.h"
#include "matter_pc_main.hpp"

#include "sl_log.h"

#define LOG_TAG "MPCNodeReportables"

void MPCNodeReportables::MPCNodeReportablesDelegate::OnDone(chip::app::ReadClient * client)
{
    chip::Platform::Delete<MPCNodeReportables>(mCtxt);
}
void MPCNodeReportables::MPCNodeReportablesDelegate::OnAttributeData(const ConcreteDataAttributePath & aPath,
                                                                     TLV::TLVReader * apData, const StatusIB & aStatus)
{
    CHIP_ERROR error = aStatus.ToChipError();
    if (CHIP_NO_ERROR != error)
    {
        sl_log_error(LOG_TAG, "OnAttributeData Failure : %s", chip::ErrorStr(error));
        return;
    }
    if (apData == nullptr)
    {
        sl_log_error(LOG_TAG, "empty report received by reportables delegate");
        return;
    }

    auto epNode = mNode.child_by_type_and_value<chip::EndpointId>(ATTRIBUTE_ENDPOINT_ID, aPath.mEndpointId);
    attribute_store_type_t reportedNodeType = ((aPath.mClusterId & 0xFFFF) << 16) | (aPath.mAttributeId & 0xFFFF);
    auto reportedNode                       = epNode.child_by_type(reportedNodeType);

    if (!reportedNode.is_valid())
    {
        sl_log_debug(LOG_TAG, "Unable to find reported attribute in attribute tree");
        return;
    }

    if (SL_STATUS_OK != mpc_attribute_parser_invoke(aPath, apData, reportedNode))
    {
        sl_log_warning(LOG_TAG, "failed to parse attribute from report");
        return;
    }
    sl_log_debug(LOG_TAG, "report parsed and updated successfully");
    mpc_schedule_contiki();
    
}

void MPCNodeReportables::MPCNodeReportablesDelegate::OnError(CHIP_ERROR aError)
{
    sl_log_error(LOG_TAG, "Reportables setup failed for node %u", mNode);
    mFailureReported = true;
    mpc_node_monitor_cancel_monitoring(mNode);
}

void MPCNodeReportables::MPCNodeReportablesDelegate::OnSubscriptionEstablished(SubscriptionId aSubscriptionId)
{
    sl_log_info(LOG_TAG, "Reportable successfully setup for %u", mNode);
}

sl_status_t MPCNodeReportables::Initiate()
{
    mSubscribe.SetCallbacks(&mDelegate);
    return mSubscribe.SendCommand();
}

sl_status_t MPCNodeReportables::Terminate()
{
    // TODO : Should we be able to manual terminate? Is a valid usecase? ReadClient does seems to
    //       expose any API for this
    return SL_STATUS_OK;
}
