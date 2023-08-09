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
#include "mpc_attribute_cluster_parser.h"
#include "mpc_attribute_parser_fwk_internal.h"

#include "attribute_store_helper.h"
#include "sl_log.h"

#include <app-common/zap-generated/cluster-objects.h>

// Generic includes
#include <map>
#include <string>

#define LOG_TAG "MPC_ATTR_PARSER"

using namespace std;

static std::map<chip::ClusterId, AttributeParserFunction> mpcClusterParsers;

sl_status_t mpc_attribute_parser_invoke(const chip::app::ConcreteDataAttributePath & path, chip::TLV::TLVReader * data,
                                        attribute_store_node_t mNode)
{
    auto it = mpcClusterParsers.find(path.mClusterId);
    if (it == mpcClusterParsers.end())
    {
        sl_log_error(LOG_TAG, "Cluster ID [%u] not found in mpcClusterParsers", path.mClusterId);
        return SL_STATUS_FAIL;
    }

    return it->second(path, data, mNode);
}

sl_status_t mpc_attribute_parser_register(chip::ClusterId clusterID, AttributeParserFunction parserCallback)
{
    mpcClusterParsers.insert({ clusterID, parserCallback });
    return SL_STATUS_OK;
}

sl_status_t mpc_attribute_parser_init()
{
    sl_status_t status = 0;
    status |= mpc_attribute_parser_register(chip::app::Clusters::Descriptor::Id, &mpc_descriptor_cluster_parser);
    status |= mpc_attribute_parser_register(chip::app::Clusters::OnOff::Id, mpc_onoff_cluster_parser);
    return status;
}
