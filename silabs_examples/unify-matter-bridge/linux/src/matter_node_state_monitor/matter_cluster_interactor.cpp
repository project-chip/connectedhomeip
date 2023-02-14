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
#include "attribute_type_size.hpp"
#include "matter.h"
#include "matter_device_translator.hpp"
#include "matter_endpoint_builder.hpp"
#include "sl_log.h"

constexpr const char * LOG_TAG              = "cluster_interactor";
constexpr int kNodeLabelSize                = 32;
constexpr int kDescriptorAttributeArraySize = 254;
constexpr int kFixedLabelAttributeArraySize = 254;



namespace {
/**
 * @brief these clusters are default clusters that are required by matter?
 * these clusters will be appended for each registered endpoint
 */
static void append_bridged_clusters(unify::matter_bridge::matter_endpoint_builder & endpoint_builder)
{

    auto descriptor_cluster = endpoint_builder.register_cluster(chip::app::Clusters::Descriptor::Id);
    descriptor_cluster.attributes.emplace_back(EmberAfAttributeMetadata{ZAP_EMPTY_DEFAULT(),
                                                                        chip::app::Clusters::Descriptor::Attributes::DeviceTypeList::Id,
                                                                        kDescriptorAttributeArraySize,
                                                                        ZCL_ARRAY_ATTRIBUTE_TYPE,
                                                                        0});
    descriptor_cluster.attributes.emplace_back(EmberAfAttributeMetadata{ZAP_EMPTY_DEFAULT(),
                                                                        chip::app::Clusters::Descriptor::Attributes::ServerList::Id,
                                                                        kDescriptorAttributeArraySize,
                                                                        ZCL_ARRAY_ATTRIBUTE_TYPE,
                                                                        0});
    descriptor_cluster.attributes.emplace_back(EmberAfAttributeMetadata{ZAP_EMPTY_DEFAULT(),
                                                                        chip::app::Clusters::Descriptor::Attributes::ClientList::Id,
                                                                        kDescriptorAttributeArraySize,
                                                                        ZCL_ARRAY_ATTRIBUTE_TYPE,
                                                                        0});
    descriptor_cluster.attributes.emplace_back(EmberAfAttributeMetadata{ZAP_EMPTY_DEFAULT(),
                                                                        chip::app::Clusters::Descriptor::Attributes::PartsList::Id,
                                                                        kDescriptorAttributeArraySize,
                                                                        ZCL_ARRAY_ATTRIBUTE_TYPE,
                                                                        0});
    descriptor_cluster.attributes.emplace_back(EmberAfAttributeMetadata{ZAP_EMPTY_DEFAULT(),
                                                                        chip::app::Clusters::Globals::Attributes::ClusterRevision::Id,
                                                                        2,
                                                                        ZCL_INT16U_ATTRIBUTE_TYPE,
                                                                        0});

    auto fixed_label_cluster = endpoint_builder.register_cluster(chip::app::Clusters::FixedLabel::Id);
    fixed_label_cluster.attributes.emplace_back(EmberAfAttributeMetadata{ZAP_EMPTY_DEFAULT(),
                                                                         chip::app::Clusters::FixedLabel::Attributes::LabelList::Id,
                                                                         kFixedLabelAttributeArraySize,
                                                                         ZCL_ARRAY_ATTRIBUTE_TYPE,
                                                                         0});
    fixed_label_cluster.attributes.emplace_back(EmberAfAttributeMetadata{ZAP_EMPTY_DEFAULT(),
                                                                         chip::app::Clusters::Globals::Attributes::ClusterRevision::Id,
                                                                         2,
                                                                         ZCL_INT16U_ATTRIBUTE_TYPE,
                                                                         0});
}

} // namespace

namespace unify::matter_bridge {
cluster_interactor::cluster_interactor(ClusterEmulator& _emulator,const device_translator & _translator, matter_endpoint_builder & _endpoint_builder) :
     endpoint_builder(_endpoint_builder),emulator(_emulator),translator(_translator)
{}

void cluster_interactor::build_matter_cluster(const std::unordered_map<std::string, node_state_monitor::cluster> & clusters)
{
    bool basic_cluster_is_supported = false;
    for (const auto & [cluster_name, cluster] : clusters)
    {
        // Skip clusters that doesn't have attributes
        if (cluster.attributes.empty()) {
            continue;
        }
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
            sl_log_info(LOG_TAG, "Mapping cluster %s to cluster_id: %d", cluster_name.c_str(), cluster_id.value());
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
        cluster_builder.incoming_commands.push_back(chip::kInvalidCommandId);
        for (const auto & outgoing : cluster.generated_commands)
        {
            auto command = translator.get_command_id(cluster_name, outgoing);
            if (command)
            {
                cluster_builder.outgoing_commands.push_back(command.value());
            }
        }
        cluster_builder.outgoing_commands.push_back(chip::kInvalidCommandId);
        for (const auto & attribute : cluster.attributes)
        {
            if (auto attribute_id = translator.get_attribute_id(cluster_name, attribute))
            {
                attr_type_size type_size = get_attribute_type_size(cluster_id.value(), attribute_id.value());
                //TODO we should have the unify node state monitor detect if attributes are writable 
                cluster_builder.attributes.emplace_back(EmberAfAttributeMetadata{ZAP_EMPTY_DEFAULT(),
                                                                                 attribute_id.value(),
                                                                                 type_size.attrSize,
                                                                                 type_size.attrType,
                                                                                 ATTRIBUTE_MASK_EXTERNAL_STORAGE | ATTRIBUTE_MASK_WRITABLE});
            }
        }
        if (cluster_name == "Basic")
        {
            basic_cluster_is_supported = true;
            cluster_builder.attributes.emplace_back(EmberAfAttributeMetadata{ZAP_EMPTY_DEFAULT(),
                                                                             chip::app::Clusters::BasicInformation::Attributes::NodeLabel::Id,
                                                                             kNodeLabelSize,
                                                                             ZCL_CHAR_STRING_ATTRIBUTE_TYPE,
                                                                             0});
            cluster_builder.attributes.emplace_back(EmberAfAttributeMetadata{ZAP_EMPTY_DEFAULT(),
                                                                             chip::app::Clusters::BasicInformation::Attributes::Reachable::Id,
                                                                             1,
                                                                             ZCL_BOOLEAN_ATTRIBUTE_TYPE,
                                                                             0});
        }

        emulator.add_emulated_commands_and_attributes(cluster, cluster_builder);
    }
    if (!basic_cluster_is_supported)
    {
        auto bridged_cluster = endpoint_builder.register_cluster(chip::app::Clusters::BridgedDeviceBasicInformation::Id);
        bridged_cluster.attributes.emplace_back(EmberAfAttributeMetadata{ZAP_EMPTY_DEFAULT(),
                                                                         chip::app::Clusters::BridgedDeviceBasicInformation::Attributes::NodeLabel::Id,
                                                                         kNodeLabelSize,
                                                                         ZCL_CHAR_STRING_ATTRIBUTE_TYPE,
                                                                         0});
        bridged_cluster.attributes.emplace_back(EmberAfAttributeMetadata{ZAP_EMPTY_DEFAULT(),
                                                                         chip::app::Clusters::BridgedDeviceBasicInformation::Attributes::Reachable::Id,
                                                                         1,
                                                                         ZCL_BOOLEAN_ATTRIBUTE_TYPE,
                                                                         0});
        bridged_cluster.attributes.emplace_back(EmberAfAttributeMetadata{ZAP_EMPTY_DEFAULT(),
                                                                         chip::app::Clusters::BridgedDeviceBasicInformation::Attributes::FeatureMap::Id,
                                                                         4,
                                                                         ZCL_BITMAP32_ATTRIBUTE_TYPE,
                                                                         0});
        bridged_cluster.attributes.emplace_back(EmberAfAttributeMetadata{ZAP_EMPTY_DEFAULT(),
                                                                         chip::app::Clusters::BridgedDeviceBasicInformation::Attributes::ClusterRevision::Id,
                                                                         2,
                                                                         ZCL_INT16U_ATTRIBUTE_TYPE,
                                                                         0});
    }
    append_bridged_clusters(endpoint_builder);
}

std::optional<uint16_t> cluster_interactor::get_matter_type() const
{
    std::vector<chip::DeviceTypeId> possible_device_types = translator.get_device_types(endpoint_builder.clusters);
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