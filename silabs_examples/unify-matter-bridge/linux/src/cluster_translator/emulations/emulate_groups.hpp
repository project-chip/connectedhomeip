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

#ifndef EMULATE_GROUPS_HPP
#define EMULATE_GROUPS_HPP

// Emulator interface
#include "emulator.hpp"

namespace unify::matter_bridge {

using namespace chip::app;
using namespace chip::app::Clusters;

class EmulateGroups : public EmulatorInterface
{
public:
    const char * emulated_cluster_name() const override { return "Groups"; }

    chip::ClusterId emulated_cluster() const override { return Groups::Id; };

    CHIP_ERROR
    emulate(const node_state_monitor::cluster & unify_cluster, matter_cluster_builder & cluster_builder) override
    {
        cluster_builder.attributes.clear();
        cluster_builder.incoming_commands.clear();
        cluster_builder.outgoing_commands.clear();

        // Add GroupsType attribute to the matter cluster
        cluster_builder.attributes.push_back({ZAP_SIMPLE_DEFAULT(128),
                                              Groups::Attributes::NameSupport::Id,
                                              1,
                                              ZAP_TYPE(BITMAP8),
                                              ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE)});
        cluster_builder.attributes.push_back({ZAP_SIMPLE_DEFAULT(4),
                                              Groups::Attributes::ClusterRevision::Id,
                                              2,
                                              ZAP_TYPE(INT16U),
                                              ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE)});
        cluster_builder.attributes.push_back({ZAP_SIMPLE_DEFAULT(1),
                                              Groups::Attributes::FeatureMap::Id,
                                              2,
                                              ZAP_TYPE(INT16U),
                                              ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE)});



        cluster_builder.incoming_commands.insert(cluster_builder.incoming_commands.end(),
                                                 { Groups::Commands::AddGroup::Id, Groups::Commands::ViewGroup::Id,
                                                   Groups::Commands::GetGroupMembership::Id, Groups::Commands::RemoveGroup::Id,
                                                   Groups::Commands::RemoveAllGroups::Id,
                                                   Groups::Commands::AddGroupIfIdentifying::Id });
        cluster_builder.outgoing_commands.insert(cluster_builder.outgoing_commands.end(),
                                                 {
                                                     Groups::Commands::AddGroupResponse::Id,
                                                     Groups::Commands::ViewGroupResponse::Id,
                                                     Groups::Commands::GetGroupMembershipResponse::Id,
                                                     Groups::Commands::RemoveGroupResponse::Id,
                                                 });
        
        return CHIP_NO_ERROR;
    }
};

} // namespace unify::matter_bridge

#endif // EMULATE_GROUPS_HPP
