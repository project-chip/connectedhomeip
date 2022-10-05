/******************************************************************************
 * # License
 * <b>Copyright 2022 Silicon Laboratories Inc. www.silabs.com</b>
 ******************************************************************************
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 *****************************************************************************/

#include "matter_cluster_interactor.hpp"
#include "matter.h"
#include "matter_device_translator.hpp"
#include "matter_endpoint_builder.hpp"
#include "sl_log.h"

constexpr const char * LOG_TAG = "cluster_interactor";
constexpr int kNodeLabelSize   = 32;
namespace {
/**
 * @brief these clusters are default clusters that are required by matter?
 * these clusters will be appended for each registered endpoint
 */
static void append_bridged_clusters(unify::matter_bridge::matter_endpoint_builder & endpoint_builder)
{
    constexpr int kDescriptorAttributeArraySize = 254;
    constexpr int kFixedLabelAttributeArraySize = 254;

    auto descriptor_cluster = endpoint_builder.register_cluster(ZCL_DESCRIPTOR_CLUSTER_ID);
    descriptor_cluster.attributes.push_back(EmberAfAttributeMetadata{ ZCL_DEVICE_LIST_ATTRIBUTE_ID, ZCL_ARRAY_ATTRIBUTE_TYPE,
                                                                      kDescriptorAttributeArraySize, 0, ZAP_EMPTY_DEFAULT() });
    descriptor_cluster.attributes.push_back(EmberAfAttributeMetadata{ ZCL_SERVER_LIST_ATTRIBUTE_ID, ZCL_ARRAY_ATTRIBUTE_TYPE,
                                                                      kDescriptorAttributeArraySize, 0, ZAP_EMPTY_DEFAULT() });
    descriptor_cluster.attributes.push_back(EmberAfAttributeMetadata{ ZCL_CLIENT_LIST_ATTRIBUTE_ID, ZCL_ARRAY_ATTRIBUTE_TYPE,
                                                                      kDescriptorAttributeArraySize, 0, ZAP_EMPTY_DEFAULT() });
    descriptor_cluster.attributes.push_back(EmberAfAttributeMetadata{ ZCL_PARTS_LIST_ATTRIBUTE_ID, ZCL_ARRAY_ATTRIBUTE_TYPE,
                                                                      kDescriptorAttributeArraySize, 0, ZAP_EMPTY_DEFAULT() });

    auto fixed_label_cluster = endpoint_builder.register_cluster(ZCL_FIXED_LABEL_CLUSTER_ID);
    fixed_label_cluster.attributes.push_back(EmberAfAttributeMetadata{ ZCL_LABEL_LIST_ATTRIBUTE_ID, ZCL_ARRAY_ATTRIBUTE_TYPE,
                                                                       kFixedLabelAttributeArraySize, 0, ZAP_EMPTY_DEFAULT() });
}

} // namespace

namespace unify::matter_bridge {
cluster_interactor::cluster_interactor(const device_translator & _translator, matter_endpoint_builder & _endpoint_builder) :
    translator(_translator), endpoint_builder(_endpoint_builder)
{}

void cluster_interactor::build_matter_cluster(const std::unordered_map<std::string, node_state_monitor::cluster> & clusters)
{

    bool basic_cluster_is_supported = false;
    for (const auto & [cluster_name, cluster] : clusters)
    {
        if (cluster_name == "NameAndLocation")
        {
            sl_log_info(LOG_TAG, " NameAndLocation cluster is already mapped to BasicBridgedDevice Info.");
            continue;
        }
        const auto cluster_id = translator.get_cluster_id(cluster_name);
        if (!cluster_id)
        {
            sl_log_info(LOG_TAG, "No cluster id known for %s", cluster_name.c_str());
            continue;
        }
        else
        {
            sl_log_info(LOG_TAG, "Mapping Custer %s", cluster_name.c_str());
        }

        auto cluster_builder = endpoint_builder.register_cluster(cluster_id.value());
        for (const auto & incoming : cluster.supported_commands)
        {
            auto command = translator.get_command_id(cluster_name, incoming);
            if (command)
            {
                cluster_builder.incoming_commands.push_back(command.value());
            }
        }

        for (const auto & outgoing : cluster.generated_commands)
        {
            auto command = translator.get_command_id(cluster_name, outgoing);
            if (command)
            {
                cluster_builder.outgoing_commands.push_back(command.value());
            }
        }

        for (const auto & attribute : cluster.attributes)
        {
            if (auto attribute_id = translator.get_attribute_id(cluster_name, attribute))
            {
                // TODO: required an API to get the attribute types from the cluster+ attribute name.
                // with its corresponding size
                cluster_builder.attributes.emplace_back(EmberAfAttributeMetadata{ attribute_id.value(), ZCL_BOOLEAN_ATTRIBUTE_TYPE,
                                                                                  1, CLUSTER_MASK_SERVER, ZAP_EMPTY_DEFAULT() });
            }
        }
        if (cluster_name == "Basic")
        {
            basic_cluster_is_supported = true;
            cluster_builder.attributes.emplace_back(EmberAfAttributeMetadata{
                ZCL_NODE_LABEL_ATTRIBUTE_ID, ZCL_CHAR_STRING_ATTRIBUTE_TYPE, kNodeLabelSize, 0, ZAP_EMPTY_DEFAULT() });
            cluster_builder.attributes.emplace_back(
                EmberAfAttributeMetadata{ ZCL_REACHABLE_ATTRIBUTE_ID, ZCL_BOOLEAN_ATTRIBUTE_TYPE, 1, 0, ZAP_EMPTY_DEFAULT() });
        }

        clusterlist.push_back(cluster_name.c_str());
    }
    if (!basic_cluster_is_supported)
    {
        auto bridged_cluster = endpoint_builder.register_cluster(ZCL_BRIDGED_DEVICE_BASIC_CLUSTER_ID);
        bridged_cluster.attributes.push_back(EmberAfAttributeMetadata{ ZCL_NODE_LABEL_ATTRIBUTE_ID, ZCL_CHAR_STRING_ATTRIBUTE_TYPE,
                                                                       kNodeLabelSize, 0, ZAP_EMPTY_DEFAULT() });
        bridged_cluster.attributes.push_back(
            EmberAfAttributeMetadata{ ZCL_REACHABLE_ATTRIBUTE_ID, ZCL_BOOLEAN_ATTRIBUTE_TYPE, 1, 0, ZAP_EMPTY_DEFAULT() });
    }
    append_bridged_clusters(endpoint_builder);
}

std::optional<uint16_t> cluster_interactor::get_matter_type() const
{
    std::vector<chip::DeviceTypeId> possible_device_types = translator.get_device_types(clusterlist);
    if (possible_device_types.empty())
    {
        return std::nullopt;
    }
    // ToDo: Here we are returning the first device type;
    // There should be a mechanism to find teh correct device type
    // for the possible list of Device types.
    return possible_device_types[0];
}

} // namespace unify::matter_bridge