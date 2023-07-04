#include <tlv/meta/clusters_meta.h>

namespace chip {
namespace TLVMeta {
namespace {

using namespace chip::FlatTree;
using namespace chip::TLV;

const Entry<ItemInfo> _Identify[] = {
    { { AttributeTag(0), "identifyTime", ItemType::kDefault }, kInvalidNodeIndex },          // Identify::int16u
    { { AttributeTag(1), "identifyType", ItemType::kEnum }, kInvalidNodeIndex },             // Identify::IdentifyTypeEnum
    { { AttributeTag(65528), "generatedCommandList", ItemType::kList }, kInvalidNodeIndex }, // Identify::command_id[]
    { { AttributeTag(65529), "acceptedCommandList", ItemType::kList }, kInvalidNodeIndex },  // Identify::command_id[]
    { { AttributeTag(65530), "eventList", ItemType::kList }, kInvalidNodeIndex },            // Identify::event_id[]
    { { AttributeTag(65531), "attributeList", ItemType::kList }, kInvalidNodeIndex },        // Identify::attrib_id[]
    { { AttributeTag(65532), "featureMap", ItemType::kDefault }, kInvalidNodeIndex },        // Identify::bitmap32
    { { AttributeTag(65533), "clusterRevision", ItemType::kDefault }, kInvalidNodeIndex },   // Identify::int16u
    { { CommandTag(0), "IdentifyRequest", ItemType::kDefault }, 2 },                         // Identify::Identify::IdentifyRequest
    { { CommandTag(64), "TriggerEffectRequest", ItemType::kDefault }, 3 }, // Identify::TriggerEffect::TriggerEffectRequest
};

const Entry<ItemInfo> _Identify_IdentifyRequest[] = {
    { { ContextTag(0), "identifyTime", ItemType::kDefault }, kInvalidNodeIndex }, // Identify::INT16U
};

const Entry<ItemInfo> _Identify_TriggerEffectRequest[] = {
    { { ContextTag(0), "effectIdentifier", ItemType::kEnum }, kInvalidNodeIndex }, // Identify::EffectIdentifierEnum
    { { ContextTag(1), "effectVariant", ItemType::kEnum }, kInvalidNodeIndex },    // Identify::EffectVariantEnum
};

const Entry<ItemInfo> _Groups[] = {
    { { AttributeTag(0), "nameSupport", ItemType::kBitmap }, kInvalidNodeIndex },            // Groups::NameSupportBitmap
    { { AttributeTag(65528), "generatedCommandList", ItemType::kList }, kInvalidNodeIndex }, // Groups::command_id[]
    { { AttributeTag(65529), "acceptedCommandList", ItemType::kList }, kInvalidNodeIndex },  // Groups::command_id[]
    { { AttributeTag(65530), "eventList", ItemType::kList }, kInvalidNodeIndex },            // Groups::event_id[]
    { { AttributeTag(65531), "attributeList", ItemType::kList }, kInvalidNodeIndex },        // Groups::attrib_id[]
    { { AttributeTag(65532), "featureMap", ItemType::kDefault }, kInvalidNodeIndex },        // Groups::bitmap32
    { { AttributeTag(65533), "clusterRevision", ItemType::kDefault }, kInvalidNodeIndex },   // Groups::int16u
    { { CommandTag(0), "AddGroupRequest", ItemType::kDefault }, 5 },                         // Groups::AddGroup::AddGroupRequest
    { { CommandTag(1), "ViewGroupRequest", ItemType::kDefault }, 7 },                        // Groups::ViewGroup::ViewGroupRequest
    { { CommandTag(2), "GetGroupMembershipRequest", ItemType::kDefault },
      9 },                                                               // Groups::GetGroupMembership::GetGroupMembershipRequest
    { { CommandTag(3), "RemoveGroupRequest", ItemType::kDefault }, 11 }, // Groups::RemoveGroup::RemoveGroupRequest
    { { CommandTag(5), "AddGroupIfIdentifyingRequest", ItemType::kDefault },
      13 },                                                           // Groups::AddGroupIfIdentifying::AddGroupIfIdentifyingRequest
    { { CommandTag(0), "AddGroupResponse", ItemType::kDefault }, 6 }, // Groups::AddGroupResponse
    { { CommandTag(1), "ViewGroupResponse", ItemType::kDefault }, 8 },           // Groups::ViewGroupResponse
    { { CommandTag(2), "GetGroupMembershipResponse", ItemType::kDefault }, 10 }, // Groups::GetGroupMembershipResponse
    { { CommandTag(3), "RemoveGroupResponse", ItemType::kDefault }, 12 },        // Groups::RemoveGroupResponse
};

const Entry<ItemInfo> _Groups_AddGroupRequest[] = {
    { { ContextTag(0), "groupID", ItemType::kDefault }, kInvalidNodeIndex },   // Groups::group_id
    { { ContextTag(1), "groupName", ItemType::kDefault }, kInvalidNodeIndex }, // Groups::CHAR_STRING
};

const Entry<ItemInfo> _Groups_AddGroupResponse[] = {
    { { ContextTag(0), "status", ItemType::kDefault }, kInvalidNodeIndex },  // Groups::ENUM8
    { { ContextTag(1), "groupID", ItemType::kDefault }, kInvalidNodeIndex }, // Groups::group_id
};

const Entry<ItemInfo> _Groups_ViewGroupRequest[] = {
    { { ContextTag(0), "groupID", ItemType::kDefault }, kInvalidNodeIndex }, // Groups::group_id
};

const Entry<ItemInfo> _Groups_ViewGroupResponse[] = {
    { { ContextTag(0), "status", ItemType::kDefault }, kInvalidNodeIndex },    // Groups::ENUM8
    { { ContextTag(1), "groupID", ItemType::kDefault }, kInvalidNodeIndex },   // Groups::group_id
    { { ContextTag(2), "groupName", ItemType::kDefault }, kInvalidNodeIndex }, // Groups::CHAR_STRING
};

const Entry<ItemInfo> _Groups_GetGroupMembershipRequest[] = {
    { { ContextTag(0), "groupList", ItemType::kList }, kInvalidNodeIndex }, // Groups::group_id[]
};

const Entry<ItemInfo> _Groups_GetGroupMembershipResponse[] = {
    { { ContextTag(0), "capacity", ItemType::kDefault }, kInvalidNodeIndex }, // Groups::INT8U
    { { ContextTag(1), "groupList", ItemType::kList }, kInvalidNodeIndex },   // Groups::group_id[]
};

const Entry<ItemInfo> _Groups_RemoveGroupRequest[] = {
    { { ContextTag(0), "groupID", ItemType::kDefault }, kInvalidNodeIndex }, // Groups::group_id
};

const Entry<ItemInfo> _Groups_RemoveGroupResponse[] = {
    { { ContextTag(0), "status", ItemType::kDefault }, kInvalidNodeIndex },  // Groups::ENUM8
    { { ContextTag(1), "groupID", ItemType::kDefault }, kInvalidNodeIndex }, // Groups::group_id
};

const Entry<ItemInfo> _Groups_AddGroupIfIdentifyingRequest[] = {
    { { ContextTag(0), "groupID", ItemType::kDefault }, kInvalidNodeIndex },   // Groups::group_id
    { { ContextTag(1), "groupName", ItemType::kDefault }, kInvalidNodeIndex }, // Groups::CHAR_STRING
};

const Entry<ItemInfo> _Scenes[] = {
    { { AttributeTag(0), "sceneCount", ItemType::kDefault }, kInvalidNodeIndex },            // Scenes::int8u
    { { AttributeTag(1), "currentScene", ItemType::kDefault }, kInvalidNodeIndex },          // Scenes::int8u
    { { AttributeTag(2), "currentGroup", ItemType::kDefault }, kInvalidNodeIndex },          // Scenes::group_id
    { { AttributeTag(3), "sceneValid", ItemType::kDefault }, kInvalidNodeIndex },            // Scenes::boolean
    { { AttributeTag(4), "nameSupport", ItemType::kDefault }, kInvalidNodeIndex },           // Scenes::bitmap8
    { { AttributeTag(5), "lastConfiguredBy", ItemType::kDefault }, kInvalidNodeIndex },      // Scenes::node_id
    { { AttributeTag(6), "sceneTableSize", ItemType::kDefault }, kInvalidNodeIndex },        // Scenes::int16u
    { { AttributeTag(7), "remainingCapacity", ItemType::kDefault }, kInvalidNodeIndex },     // Scenes::int8u
    { { AttributeTag(65528), "generatedCommandList", ItemType::kList }, kInvalidNodeIndex }, // Scenes::command_id[]
    { { AttributeTag(65529), "acceptedCommandList", ItemType::kList }, kInvalidNodeIndex },  // Scenes::command_id[]
    { { AttributeTag(65530), "eventList", ItemType::kList }, kInvalidNodeIndex },            // Scenes::event_id[]
    { { AttributeTag(65531), "attributeList", ItemType::kList }, kInvalidNodeIndex },        // Scenes::attrib_id[]
    { { AttributeTag(65532), "featureMap", ItemType::kDefault }, kInvalidNodeIndex },        // Scenes::bitmap32
    { { AttributeTag(65533), "clusterRevision", ItemType::kDefault }, kInvalidNodeIndex },   // Scenes::int16u
    { { CommandTag(0), "AddSceneRequest", ItemType::kDefault }, 17 },                        // Scenes::AddScene::AddSceneRequest
    { { CommandTag(1), "ViewSceneRequest", ItemType::kDefault }, 19 },                       // Scenes::ViewScene::ViewSceneRequest
    { { CommandTag(2), "RemoveSceneRequest", ItemType::kDefault }, 21 },     // Scenes::RemoveScene::RemoveSceneRequest
    { { CommandTag(3), "RemoveAllScenesRequest", ItemType::kDefault }, 23 }, // Scenes::RemoveAllScenes::RemoveAllScenesRequest
    { { CommandTag(4), "StoreSceneRequest", ItemType::kDefault }, 25 },      // Scenes::StoreScene::StoreSceneRequest
    { { CommandTag(5), "RecallSceneRequest", ItemType::kDefault }, 27 },     // Scenes::RecallScene::RecallSceneRequest
    { { CommandTag(6), "GetSceneMembershipRequest", ItemType::kDefault },
      28 }, // Scenes::GetSceneMembership::GetSceneMembershipRequest
    { { CommandTag(64), "EnhancedAddSceneRequest", ItemType::kDefault }, 30 }, // Scenes::EnhancedAddScene::EnhancedAddSceneRequest
    { { CommandTag(65), "EnhancedViewSceneRequest", ItemType::kDefault },
      32 },                                                                   // Scenes::EnhancedViewScene::EnhancedViewSceneRequest
    { { CommandTag(66), "CopySceneRequest", ItemType::kDefault }, 34 },       // Scenes::CopyScene::CopySceneRequest
    { { CommandTag(0), "AddSceneResponse", ItemType::kDefault }, 18 },        // Scenes::AddSceneResponse
    { { CommandTag(1), "ViewSceneResponse", ItemType::kDefault }, 20 },       // Scenes::ViewSceneResponse
    { { CommandTag(2), "RemoveSceneResponse", ItemType::kDefault }, 22 },     // Scenes::RemoveSceneResponse
    { { CommandTag(3), "RemoveAllScenesResponse", ItemType::kDefault }, 24 }, // Scenes::RemoveAllScenesResponse
    { { CommandTag(4), "StoreSceneResponse", ItemType::kDefault }, 26 },      // Scenes::StoreSceneResponse
    { { CommandTag(6), "GetSceneMembershipResponse", ItemType::kDefault }, 29 }, // Scenes::GetSceneMembershipResponse
    { { CommandTag(64), "EnhancedAddSceneResponse", ItemType::kDefault }, 31 },  // Scenes::EnhancedAddSceneResponse
    { { CommandTag(65), "EnhancedViewSceneResponse", ItemType::kDefault }, 33 }, // Scenes::EnhancedViewSceneResponse
    { { CommandTag(66), "CopySceneResponse", ItemType::kDefault }, 35 },         // Scenes::CopySceneResponse
};

const Entry<ItemInfo> _Scenes_AttributeValuePair[] = {
    { { ContextTag(0), "attributeID", ItemType::kDefault }, kInvalidNodeIndex },    // Scenes::attrib_id
    { { ContextTag(1), "attributeValue", ItemType::kDefault }, kInvalidNodeIndex }, // Scenes::int32u
};

const Entry<ItemInfo> _Scenes_ExtensionFieldSet[] = {
    { { ContextTag(0), "clusterID", ItemType::kDefault }, kInvalidNodeIndex }, // Scenes::cluster_id
    { { ContextTag(1), "attributeValueList", ItemType::kList }, 37 },          // Scenes::AttributeValuePair[]
};

const Entry<ItemInfo> _Scenes_AddSceneRequest[] = {
    { { ContextTag(0), "groupID", ItemType::kDefault }, kInvalidNodeIndex },        // Scenes::group_id
    { { ContextTag(1), "sceneID", ItemType::kDefault }, kInvalidNodeIndex },        // Scenes::INT8U
    { { ContextTag(2), "transitionTime", ItemType::kDefault }, kInvalidNodeIndex }, // Scenes::INT16U
    { { ContextTag(3), "sceneName", ItemType::kDefault }, kInvalidNodeIndex },      // Scenes::CHAR_STRING
    { { ContextTag(4), "extensionFieldSets", ItemType::kList }, 36 },               // Scenes::ExtensionFieldSet[]
};

const Entry<ItemInfo> _Scenes_AddSceneResponse[] = {
    { { ContextTag(0), "status", ItemType::kDefault }, kInvalidNodeIndex },  // Scenes::status
    { { ContextTag(1), "groupID", ItemType::kDefault }, kInvalidNodeIndex }, // Scenes::group_id
    { { ContextTag(2), "sceneID", ItemType::kDefault }, kInvalidNodeIndex }, // Scenes::INT8U
};

const Entry<ItemInfo> _Scenes_ViewSceneRequest[] = {
    { { ContextTag(0), "groupID", ItemType::kDefault }, kInvalidNodeIndex }, // Scenes::group_id
    { { ContextTag(1), "sceneID", ItemType::kDefault }, kInvalidNodeIndex }, // Scenes::INT8U
};

const Entry<ItemInfo> _Scenes_ViewSceneResponse[] = {
    { { ContextTag(0), "status", ItemType::kDefault }, kInvalidNodeIndex },         // Scenes::status
    { { ContextTag(1), "groupID", ItemType::kDefault }, kInvalidNodeIndex },        // Scenes::group_id
    { { ContextTag(2), "sceneID", ItemType::kDefault }, kInvalidNodeIndex },        // Scenes::INT8U
    { { ContextTag(3), "transitionTime", ItemType::kDefault }, kInvalidNodeIndex }, // Scenes::INT16U
    { { ContextTag(4), "sceneName", ItemType::kDefault }, kInvalidNodeIndex },      // Scenes::CHAR_STRING
    { { ContextTag(5), "extensionFieldSets", ItemType::kList }, 36 },               // Scenes::ExtensionFieldSet[]
};

const Entry<ItemInfo> _Scenes_RemoveSceneRequest[] = {
    { { ContextTag(0), "groupID", ItemType::kDefault }, kInvalidNodeIndex }, // Scenes::group_id
    { { ContextTag(1), "sceneID", ItemType::kDefault }, kInvalidNodeIndex }, // Scenes::INT8U
};

const Entry<ItemInfo> _Scenes_RemoveSceneResponse[] = {
    { { ContextTag(0), "status", ItemType::kDefault }, kInvalidNodeIndex },  // Scenes::status
    { { ContextTag(1), "groupID", ItemType::kDefault }, kInvalidNodeIndex }, // Scenes::group_id
    { { ContextTag(2), "sceneID", ItemType::kDefault }, kInvalidNodeIndex }, // Scenes::INT8U
};

const Entry<ItemInfo> _Scenes_RemoveAllScenesRequest[] = {
    { { ContextTag(0), "groupID", ItemType::kDefault }, kInvalidNodeIndex }, // Scenes::group_id
};

const Entry<ItemInfo> _Scenes_RemoveAllScenesResponse[] = {
    { { ContextTag(0), "status", ItemType::kDefault }, kInvalidNodeIndex },  // Scenes::status
    { { ContextTag(1), "groupID", ItemType::kDefault }, kInvalidNodeIndex }, // Scenes::group_id
};

const Entry<ItemInfo> _Scenes_StoreSceneRequest[] = {
    { { ContextTag(0), "groupID", ItemType::kDefault }, kInvalidNodeIndex }, // Scenes::group_id
    { { ContextTag(1), "sceneID", ItemType::kDefault }, kInvalidNodeIndex }, // Scenes::INT8U
};

const Entry<ItemInfo> _Scenes_StoreSceneResponse[] = {
    { { ContextTag(0), "status", ItemType::kDefault }, kInvalidNodeIndex },  // Scenes::status
    { { ContextTag(1), "groupID", ItemType::kDefault }, kInvalidNodeIndex }, // Scenes::group_id
    { { ContextTag(2), "sceneID", ItemType::kDefault }, kInvalidNodeIndex }, // Scenes::INT8U
};

const Entry<ItemInfo> _Scenes_RecallSceneRequest[] = {
    { { ContextTag(0), "groupID", ItemType::kDefault }, kInvalidNodeIndex },        // Scenes::group_id
    { { ContextTag(1), "sceneID", ItemType::kDefault }, kInvalidNodeIndex },        // Scenes::INT8U
    { { ContextTag(2), "transitionTime", ItemType::kDefault }, kInvalidNodeIndex }, // Scenes::INT16U
};

const Entry<ItemInfo> _Scenes_GetSceneMembershipRequest[] = {
    { { ContextTag(0), "groupID", ItemType::kDefault }, kInvalidNodeIndex }, // Scenes::group_id
};

const Entry<ItemInfo> _Scenes_GetSceneMembershipResponse[] = {
    { { ContextTag(0), "status", ItemType::kDefault }, kInvalidNodeIndex },   // Scenes::status
    { { ContextTag(1), "capacity", ItemType::kDefault }, kInvalidNodeIndex }, // Scenes::INT8U
    { { ContextTag(2), "groupID", ItemType::kDefault }, kInvalidNodeIndex },  // Scenes::group_id
    { { ContextTag(3), "sceneList", ItemType::kList }, kInvalidNodeIndex },   // Scenes::INT8U[]
};

const Entry<ItemInfo> _Scenes_EnhancedAddSceneRequest[] = {
    { { ContextTag(0), "groupID", ItemType::kDefault }, kInvalidNodeIndex },        // Scenes::group_id
    { { ContextTag(1), "sceneID", ItemType::kDefault }, kInvalidNodeIndex },        // Scenes::INT8U
    { { ContextTag(2), "transitionTime", ItemType::kDefault }, kInvalidNodeIndex }, // Scenes::INT16U
    { { ContextTag(3), "sceneName", ItemType::kDefault }, kInvalidNodeIndex },      // Scenes::CHAR_STRING
    { { ContextTag(4), "extensionFieldSets", ItemType::kList }, 36 },               // Scenes::ExtensionFieldSet[]
};

const Entry<ItemInfo> _Scenes_EnhancedAddSceneResponse[] = {
    { { ContextTag(0), "status", ItemType::kDefault }, kInvalidNodeIndex },  // Scenes::status
    { { ContextTag(1), "groupID", ItemType::kDefault }, kInvalidNodeIndex }, // Scenes::group_id
    { { ContextTag(2), "sceneID", ItemType::kDefault }, kInvalidNodeIndex }, // Scenes::INT8U
};

const Entry<ItemInfo> _Scenes_EnhancedViewSceneRequest[] = {
    { { ContextTag(0), "groupID", ItemType::kDefault }, kInvalidNodeIndex }, // Scenes::group_id
    { { ContextTag(1), "sceneID", ItemType::kDefault }, kInvalidNodeIndex }, // Scenes::INT8U
};

const Entry<ItemInfo> _Scenes_EnhancedViewSceneResponse[] = {
    { { ContextTag(0), "status", ItemType::kDefault }, kInvalidNodeIndex },         // Scenes::status
    { { ContextTag(1), "groupID", ItemType::kDefault }, kInvalidNodeIndex },        // Scenes::group_Id
    { { ContextTag(2), "sceneID", ItemType::kDefault }, kInvalidNodeIndex },        // Scenes::INT8U
    { { ContextTag(3), "transitionTime", ItemType::kDefault }, kInvalidNodeIndex }, // Scenes::INT16U
    { { ContextTag(4), "sceneName", ItemType::kDefault }, kInvalidNodeIndex },      // Scenes::CHAR_STRING
    { { ContextTag(5), "extensionFieldSets", ItemType::kList }, 36 },               // Scenes::ExtensionFieldSet[]
};

const Entry<ItemInfo> _Scenes_CopySceneRequest[] = {
    { { ContextTag(0), "mode", ItemType::kBitmap }, kInvalidNodeIndex },                 // Scenes::ScenesCopyMode
    { { ContextTag(1), "groupIdentifierFrom", ItemType::kDefault }, kInvalidNodeIndex }, // Scenes::group_id
    { { ContextTag(2), "sceneIdentifierFrom", ItemType::kDefault }, kInvalidNodeIndex }, // Scenes::INT8U
    { { ContextTag(3), "groupIdentifierTo", ItemType::kDefault }, kInvalidNodeIndex },   // Scenes::group_id
    { { ContextTag(4), "sceneIdentifierTo", ItemType::kDefault }, kInvalidNodeIndex },   // Scenes::INT8U
};

const Entry<ItemInfo> _Scenes_CopySceneResponse[] = {
    { { ContextTag(0), "status", ItemType::kDefault }, kInvalidNodeIndex },              // Scenes::status
    { { ContextTag(1), "groupIdentifierFrom", ItemType::kDefault }, kInvalidNodeIndex }, // Scenes::group_Id
    { { ContextTag(2), "sceneIdentifierFrom", ItemType::kDefault }, kInvalidNodeIndex }, // Scenes::INT8U
};

const Entry<ItemInfo> _Scenes_ExtensionFieldSet_list_[] = {
    { { AnonymousTag(), "[]", ItemType::kDefault }, 16 }, // Scenes_ExtensionFieldSet[]
};

const Entry<ItemInfo> _Scenes_AttributeValuePair_list_[] = {
    { { AnonymousTag(), "[]", ItemType::kDefault }, 15 }, // Scenes_AttributeValuePair[]
};

const Entry<ItemInfo> _OnOff[] = {
    { { AttributeTag(0), "onOff", ItemType::kDefault }, kInvalidNodeIndex },                  // OnOff::boolean
    { { AttributeTag(16384), "globalSceneControl", ItemType::kDefault }, kInvalidNodeIndex }, // OnOff::boolean
    { { AttributeTag(16385), "onTime", ItemType::kDefault }, kInvalidNodeIndex },             // OnOff::int16u
    { { AttributeTag(16386), "offWaitTime", ItemType::kDefault }, kInvalidNodeIndex },        // OnOff::int16u
    { { AttributeTag(16387), "startUpOnOff", ItemType::kEnum }, kInvalidNodeIndex },          // OnOff::OnOffStartUpOnOff
    { { AttributeTag(65528), "generatedCommandList", ItemType::kList }, kInvalidNodeIndex },  // OnOff::command_id[]
    { { AttributeTag(65529), "acceptedCommandList", ItemType::kList }, kInvalidNodeIndex },   // OnOff::command_id[]
    { { AttributeTag(65530), "eventList", ItemType::kList }, kInvalidNodeIndex },             // OnOff::event_id[]
    { { AttributeTag(65531), "attributeList", ItemType::kList }, kInvalidNodeIndex },         // OnOff::attrib_id[]
    { { AttributeTag(65532), "featureMap", ItemType::kDefault }, kInvalidNodeIndex },         // OnOff::bitmap32
    { { AttributeTag(65533), "clusterRevision", ItemType::kDefault }, kInvalidNodeIndex },    // OnOff::int16u
    { { CommandTag(64), "OffWithEffectRequest", ItemType::kDefault }, 39 },  // OnOff::OffWithEffect::OffWithEffectRequest
    { { CommandTag(66), "OnWithTimedOffRequest", ItemType::kDefault }, 40 }, // OnOff::OnWithTimedOff::OnWithTimedOffRequest
};

const Entry<ItemInfo> _OnOff_OffWithEffectRequest[] = {
    { { ContextTag(0), "effectIdentifier", ItemType::kEnum }, kInvalidNodeIndex }, // OnOff::OnOffEffectIdentifier
    { { ContextTag(1), "effectVariant", ItemType::kDefault }, kInvalidNodeIndex }, // OnOff::int8u
};

const Entry<ItemInfo> _OnOff_OnWithTimedOffRequest[] = {
    { { ContextTag(0), "onOffControl", ItemType::kBitmap }, kInvalidNodeIndex }, // OnOff::OnOffControl
    { { ContextTag(1), "onTime", ItemType::kDefault }, kInvalidNodeIndex },      // OnOff::int16u
    { { ContextTag(2), "offWaitTime", ItemType::kDefault }, kInvalidNodeIndex }, // OnOff::int16u
};

const Entry<ItemInfo> _OnOffSwitchConfiguration[] = {
    { { AttributeTag(0), "switchType", ItemType::kDefault }, kInvalidNodeIndex },     // OnOffSwitchConfiguration::enum8
    { { AttributeTag(16), "switchActions", ItemType::kDefault }, kInvalidNodeIndex }, // OnOffSwitchConfiguration::enum8
    { { AttributeTag(65528), "generatedCommandList", ItemType::kList },
      kInvalidNodeIndex }, // OnOffSwitchConfiguration::command_id[]
    { { AttributeTag(65529), "acceptedCommandList", ItemType::kList },
      kInvalidNodeIndex },                                                                 // OnOffSwitchConfiguration::command_id[]
    { { AttributeTag(65530), "eventList", ItemType::kList }, kInvalidNodeIndex },          // OnOffSwitchConfiguration::event_id[]
    { { AttributeTag(65531), "attributeList", ItemType::kList }, kInvalidNodeIndex },      // OnOffSwitchConfiguration::attrib_id[]
    { { AttributeTag(65532), "featureMap", ItemType::kDefault }, kInvalidNodeIndex },      // OnOffSwitchConfiguration::bitmap32
    { { AttributeTag(65533), "clusterRevision", ItemType::kDefault }, kInvalidNodeIndex }, // OnOffSwitchConfiguration::int16u
};

const Entry<ItemInfo> _LevelControl[] = {
    { { AttributeTag(0), "currentLevel", ItemType::kDefault }, kInvalidNodeIndex },            // LevelControl::int8u
    { { AttributeTag(1), "remainingTime", ItemType::kDefault }, kInvalidNodeIndex },           // LevelControl::int16u
    { { AttributeTag(2), "minLevel", ItemType::kDefault }, kInvalidNodeIndex },                // LevelControl::int8u
    { { AttributeTag(3), "maxLevel", ItemType::kDefault }, kInvalidNodeIndex },                // LevelControl::int8u
    { { AttributeTag(4), "currentFrequency", ItemType::kDefault }, kInvalidNodeIndex },        // LevelControl::int16u
    { { AttributeTag(5), "minFrequency", ItemType::kDefault }, kInvalidNodeIndex },            // LevelControl::int16u
    { { AttributeTag(6), "maxFrequency", ItemType::kDefault }, kInvalidNodeIndex },            // LevelControl::int16u
    { { AttributeTag(15), "options", ItemType::kBitmap }, kInvalidNodeIndex },                 // LevelControl::LevelControlOptions
    { { AttributeTag(16), "onOffTransitionTime", ItemType::kDefault }, kInvalidNodeIndex },    // LevelControl::int16u
    { { AttributeTag(17), "onLevel", ItemType::kDefault }, kInvalidNodeIndex },                // LevelControl::int8u
    { { AttributeTag(18), "onTransitionTime", ItemType::kDefault }, kInvalidNodeIndex },       // LevelControl::int16u
    { { AttributeTag(19), "offTransitionTime", ItemType::kDefault }, kInvalidNodeIndex },      // LevelControl::int16u
    { { AttributeTag(20), "defaultMoveRate", ItemType::kDefault }, kInvalidNodeIndex },        // LevelControl::int8u
    { { AttributeTag(16384), "startUpCurrentLevel", ItemType::kDefault }, kInvalidNodeIndex }, // LevelControl::int8u
    { { AttributeTag(65528), "generatedCommandList", ItemType::kList }, kInvalidNodeIndex },   // LevelControl::command_id[]
    { { AttributeTag(65529), "acceptedCommandList", ItemType::kList }, kInvalidNodeIndex },    // LevelControl::command_id[]
    { { AttributeTag(65530), "eventList", ItemType::kList }, kInvalidNodeIndex },              // LevelControl::event_id[]
    { { AttributeTag(65531), "attributeList", ItemType::kList }, kInvalidNodeIndex },          // LevelControl::attrib_id[]
    { { AttributeTag(65532), "featureMap", ItemType::kDefault }, kInvalidNodeIndex },          // LevelControl::bitmap32
    { { AttributeTag(65533), "clusterRevision", ItemType::kDefault }, kInvalidNodeIndex },     // LevelControl::int16u
    { { CommandTag(0), "MoveToLevelRequest", ItemType::kDefault }, 43 }, // LevelControl::MoveToLevel::MoveToLevelRequest
    { { CommandTag(1), "MoveRequest", ItemType::kDefault }, 44 },        // LevelControl::Move::MoveRequest
    { { CommandTag(2), "StepRequest", ItemType::kDefault }, 45 },        // LevelControl::Step::StepRequest
    { { CommandTag(3), "StopRequest", ItemType::kDefault }, 46 },        // LevelControl::Stop::StopRequest
    { { CommandTag(4), "MoveToLevelWithOnOffRequest", ItemType::kDefault },
      47 }, // LevelControl::MoveToLevelWithOnOff::MoveToLevelWithOnOffRequest
    { { CommandTag(5), "MoveWithOnOffRequest", ItemType::kDefault }, 48 }, // LevelControl::MoveWithOnOff::MoveWithOnOffRequest
    { { CommandTag(6), "StepWithOnOffRequest", ItemType::kDefault }, 49 }, // LevelControl::StepWithOnOff::StepWithOnOffRequest
    { { CommandTag(7), "StopWithOnOffRequest", ItemType::kDefault }, 50 }, // LevelControl::StopWithOnOff::StopWithOnOffRequest
    { { CommandTag(8), "MoveToClosestFrequencyRequest", ItemType::kDefault },
      51 }, // LevelControl::MoveToClosestFrequency::MoveToClosestFrequencyRequest
};

const Entry<ItemInfo> _LevelControl_MoveToLevelRequest[] = {
    { { ContextTag(0), "level", ItemType::kDefault }, kInvalidNodeIndex },          // LevelControl::INT8U
    { { ContextTag(1), "transitionTime", ItemType::kDefault }, kInvalidNodeIndex }, // LevelControl::INT16U
    { { ContextTag(2), "optionsMask", ItemType::kBitmap }, kInvalidNodeIndex },     // LevelControl::LevelControlOptions
    { { ContextTag(3), "optionsOverride", ItemType::kBitmap }, kInvalidNodeIndex }, // LevelControl::LevelControlOptions
};

const Entry<ItemInfo> _LevelControl_MoveRequest[] = {
    { { ContextTag(0), "moveMode", ItemType::kEnum }, kInvalidNodeIndex },          // LevelControl::MoveMode
    { { ContextTag(1), "rate", ItemType::kDefault }, kInvalidNodeIndex },           // LevelControl::INT8U
    { { ContextTag(2), "optionsMask", ItemType::kBitmap }, kInvalidNodeIndex },     // LevelControl::LevelControlOptions
    { { ContextTag(3), "optionsOverride", ItemType::kBitmap }, kInvalidNodeIndex }, // LevelControl::LevelControlOptions
};

const Entry<ItemInfo> _LevelControl_StepRequest[] = {
    { { ContextTag(0), "stepMode", ItemType::kEnum }, kInvalidNodeIndex },          // LevelControl::StepMode
    { { ContextTag(1), "stepSize", ItemType::kDefault }, kInvalidNodeIndex },       // LevelControl::INT8U
    { { ContextTag(2), "transitionTime", ItemType::kDefault }, kInvalidNodeIndex }, // LevelControl::INT16U
    { { ContextTag(3), "optionsMask", ItemType::kBitmap }, kInvalidNodeIndex },     // LevelControl::LevelControlOptions
    { { ContextTag(4), "optionsOverride", ItemType::kBitmap }, kInvalidNodeIndex }, // LevelControl::LevelControlOptions
};

const Entry<ItemInfo> _LevelControl_StopRequest[] = {
    { { ContextTag(0), "optionsMask", ItemType::kBitmap }, kInvalidNodeIndex },     // LevelControl::LevelControlOptions
    { { ContextTag(1), "optionsOverride", ItemType::kBitmap }, kInvalidNodeIndex }, // LevelControl::LevelControlOptions
};

const Entry<ItemInfo> _LevelControl_MoveToLevelWithOnOffRequest[] = {
    { { ContextTag(0), "level", ItemType::kDefault }, kInvalidNodeIndex },          // LevelControl::INT8U
    { { ContextTag(1), "transitionTime", ItemType::kDefault }, kInvalidNodeIndex }, // LevelControl::INT16U
    { { ContextTag(2), "optionsMask", ItemType::kBitmap }, kInvalidNodeIndex },     // LevelControl::LevelControlOptions
    { { ContextTag(3), "optionsOverride", ItemType::kBitmap }, kInvalidNodeIndex }, // LevelControl::LevelControlOptions
};

const Entry<ItemInfo> _LevelControl_MoveWithOnOffRequest[] = {
    { { ContextTag(0), "moveMode", ItemType::kEnum }, kInvalidNodeIndex },          // LevelControl::MoveMode
    { { ContextTag(1), "rate", ItemType::kDefault }, kInvalidNodeIndex },           // LevelControl::INT8U
    { { ContextTag(2), "optionsMask", ItemType::kBitmap }, kInvalidNodeIndex },     // LevelControl::LevelControlOptions
    { { ContextTag(3), "optionsOverride", ItemType::kBitmap }, kInvalidNodeIndex }, // LevelControl::LevelControlOptions
};

const Entry<ItemInfo> _LevelControl_StepWithOnOffRequest[] = {
    { { ContextTag(0), "stepMode", ItemType::kEnum }, kInvalidNodeIndex },          // LevelControl::StepMode
    { { ContextTag(1), "stepSize", ItemType::kDefault }, kInvalidNodeIndex },       // LevelControl::INT8U
    { { ContextTag(2), "transitionTime", ItemType::kDefault }, kInvalidNodeIndex }, // LevelControl::INT16U
    { { ContextTag(3), "optionsMask", ItemType::kBitmap }, kInvalidNodeIndex },     // LevelControl::LevelControlOptions
    { { ContextTag(4), "optionsOverride", ItemType::kBitmap }, kInvalidNodeIndex }, // LevelControl::LevelControlOptions
};

const Entry<ItemInfo> _LevelControl_StopWithOnOffRequest[] = {
    { { ContextTag(0), "optionsMask", ItemType::kBitmap }, kInvalidNodeIndex },     // LevelControl::LevelControlOptions
    { { ContextTag(1), "optionsOverride", ItemType::kBitmap }, kInvalidNodeIndex }, // LevelControl::LevelControlOptions
};

const Entry<ItemInfo> _LevelControl_MoveToClosestFrequencyRequest[] = {
    { { ContextTag(0), "frequency", ItemType::kDefault }, kInvalidNodeIndex }, // LevelControl::INT16U
};

const Entry<ItemInfo> _BinaryInputBasic[] = {
    { { AttributeTag(4), "activeText", ItemType::kDefault }, kInvalidNodeIndex },            // BinaryInputBasic::char_string
    { { AttributeTag(28), "description", ItemType::kDefault }, kInvalidNodeIndex },          // BinaryInputBasic::char_string
    { { AttributeTag(46), "inactiveText", ItemType::kDefault }, kInvalidNodeIndex },         // BinaryInputBasic::char_string
    { { AttributeTag(81), "outOfService", ItemType::kDefault }, kInvalidNodeIndex },         // BinaryInputBasic::boolean
    { { AttributeTag(84), "polarity", ItemType::kDefault }, kInvalidNodeIndex },             // BinaryInputBasic::enum8
    { { AttributeTag(85), "presentValue", ItemType::kDefault }, kInvalidNodeIndex },         // BinaryInputBasic::boolean
    { { AttributeTag(103), "reliability", ItemType::kDefault }, kInvalidNodeIndex },         // BinaryInputBasic::enum8
    { { AttributeTag(111), "statusFlags", ItemType::kDefault }, kInvalidNodeIndex },         // BinaryInputBasic::bitmap8
    { { AttributeTag(256), "applicationType", ItemType::kDefault }, kInvalidNodeIndex },     // BinaryInputBasic::int32u
    { { AttributeTag(65528), "generatedCommandList", ItemType::kList }, kInvalidNodeIndex }, // BinaryInputBasic::command_id[]
    { { AttributeTag(65529), "acceptedCommandList", ItemType::kList }, kInvalidNodeIndex },  // BinaryInputBasic::command_id[]
    { { AttributeTag(65530), "eventList", ItemType::kList }, kInvalidNodeIndex },            // BinaryInputBasic::event_id[]
    { { AttributeTag(65531), "attributeList", ItemType::kList }, kInvalidNodeIndex },        // BinaryInputBasic::attrib_id[]
    { { AttributeTag(65532), "featureMap", ItemType::kDefault }, kInvalidNodeIndex },        // BinaryInputBasic::bitmap32
    { { AttributeTag(65533), "clusterRevision", ItemType::kDefault }, kInvalidNodeIndex },   // BinaryInputBasic::int16u
};

const Entry<ItemInfo> _PulseWidthModulation[] = {
    { { AttributeTag(65528), "generatedCommandList", ItemType::kList }, kInvalidNodeIndex }, // PulseWidthModulation::command_id[]
    { { AttributeTag(65529), "acceptedCommandList", ItemType::kList }, kInvalidNodeIndex },  // PulseWidthModulation::command_id[]
    { { AttributeTag(65530), "eventList", ItemType::kList }, kInvalidNodeIndex },            // PulseWidthModulation::event_id[]
    { { AttributeTag(65531), "attributeList", ItemType::kList }, kInvalidNodeIndex },        // PulseWidthModulation::attrib_id[]
    { { AttributeTag(65532), "featureMap", ItemType::kDefault }, kInvalidNodeIndex },        // PulseWidthModulation::bitmap32
    { { AttributeTag(65533), "clusterRevision", ItemType::kDefault }, kInvalidNodeIndex },   // PulseWidthModulation::int16u
};

const Entry<ItemInfo> _Descriptor[] = {
    { { AttributeTag(0), "deviceTypeList", ItemType::kList }, 56 },                          // Descriptor::DeviceTypeStruct[]
    { { AttributeTag(1), "serverList", ItemType::kList }, kInvalidNodeIndex },               // Descriptor::CLUSTER_ID[]
    { { AttributeTag(2), "clientList", ItemType::kList }, kInvalidNodeIndex },               // Descriptor::CLUSTER_ID[]
    { { AttributeTag(3), "partsList", ItemType::kList }, kInvalidNodeIndex },                // Descriptor::ENDPOINT_NO[]
    { { AttributeTag(65528), "generatedCommandList", ItemType::kList }, kInvalidNodeIndex }, // Descriptor::command_id[]
    { { AttributeTag(65529), "acceptedCommandList", ItemType::kList }, kInvalidNodeIndex },  // Descriptor::command_id[]
    { { AttributeTag(65530), "eventList", ItemType::kList }, kInvalidNodeIndex },            // Descriptor::event_id[]
    { { AttributeTag(65531), "attributeList", ItemType::kList }, kInvalidNodeIndex },        // Descriptor::attrib_id[]
    { { AttributeTag(65532), "featureMap", ItemType::kDefault }, kInvalidNodeIndex },        // Descriptor::bitmap32
    { { AttributeTag(65533), "clusterRevision", ItemType::kDefault }, kInvalidNodeIndex },   // Descriptor::int16u
};

const Entry<ItemInfo> _Descriptor_DeviceTypeStruct[] = {
    { { ContextTag(0), "deviceType", ItemType::kDefault }, kInvalidNodeIndex }, // Descriptor::devtype_id
    { { ContextTag(1), "revision", ItemType::kDefault }, kInvalidNodeIndex },   // Descriptor::int16u
};

const Entry<ItemInfo> _Descriptor_DeviceTypeStruct_list_[] = {
    { { AnonymousTag(), "[]", ItemType::kDefault }, 55 }, // Descriptor_DeviceTypeStruct[]
};

const Entry<ItemInfo> _Binding[] = {
    { { AttributeTag(0), "binding", ItemType::kList }, 59 },                                 // Binding::TargetStruct[]
    { { AttributeTag(65528), "generatedCommandList", ItemType::kList }, kInvalidNodeIndex }, // Binding::command_id[]
    { { AttributeTag(65529), "acceptedCommandList", ItemType::kList }, kInvalidNodeIndex },  // Binding::command_id[]
    { { AttributeTag(65530), "eventList", ItemType::kList }, kInvalidNodeIndex },            // Binding::event_id[]
    { { AttributeTag(65531), "attributeList", ItemType::kList }, kInvalidNodeIndex },        // Binding::attrib_id[]
    { { AttributeTag(65532), "featureMap", ItemType::kDefault }, kInvalidNodeIndex },        // Binding::bitmap32
    { { AttributeTag(65533), "clusterRevision", ItemType::kDefault }, kInvalidNodeIndex },   // Binding::int16u
};

const Entry<ItemInfo> _Binding_TargetStruct[] = {
    { { ContextTag(1), "node", ItemType::kDefault }, kInvalidNodeIndex },          // Binding::node_id
    { { ContextTag(2), "group", ItemType::kDefault }, kInvalidNodeIndex },         // Binding::group_id
    { { ContextTag(3), "endpoint", ItemType::kDefault }, kInvalidNodeIndex },      // Binding::endpoint_no
    { { ContextTag(4), "cluster", ItemType::kDefault }, kInvalidNodeIndex },       // Binding::cluster_id
    { { ContextTag(254), "fabricIndex", ItemType::kDefault }, kInvalidNodeIndex }, // Binding::fabric_idx
};

const Entry<ItemInfo> _Binding_TargetStruct_list_[] = {
    { { AnonymousTag(), "[]", ItemType::kDefault }, 58 }, // Binding_TargetStruct[]
};

const Entry<ItemInfo> _AccessControl[] = {
    { { AttributeTag(0), "acl", ItemType::kList }, 68 },       // AccessControl::AccessControlEntryStruct[]
    { { AttributeTag(1), "extension", ItemType::kList }, 66 }, // AccessControl::AccessControlExtensionStruct[]
    { { AttributeTag(2), "subjectsPerAccessControlEntry", ItemType::kDefault }, kInvalidNodeIndex }, // AccessControl::int16u
    { { AttributeTag(3), "targetsPerAccessControlEntry", ItemType::kDefault }, kInvalidNodeIndex },  // AccessControl::int16u
    { { AttributeTag(4), "accessControlEntriesPerFabric", ItemType::kDefault }, kInvalidNodeIndex }, // AccessControl::int16u
    { { AttributeTag(65528), "generatedCommandList", ItemType::kList }, kInvalidNodeIndex },         // AccessControl::command_id[]
    { { AttributeTag(65529), "acceptedCommandList", ItemType::kList }, kInvalidNodeIndex },          // AccessControl::command_id[]
    { { AttributeTag(65530), "eventList", ItemType::kList }, kInvalidNodeIndex },                    // AccessControl::event_id[]
    { { AttributeTag(65531), "attributeList", ItemType::kList }, kInvalidNodeIndex },                // AccessControl::attrib_id[]
    { { AttributeTag(65532), "featureMap", ItemType::kDefault }, kInvalidNodeIndex },                // AccessControl::bitmap32
    { { AttributeTag(65533), "clusterRevision", ItemType::kDefault }, kInvalidNodeIndex },           // AccessControl::int16u
    { { EventTag(0), "AccessControlEntryChanged", ItemType::kDefault }, 64 },     // AccessControl::AccessControlEntryChanged
    { { EventTag(1), "AccessControlExtensionChanged", ItemType::kDefault }, 65 }, // AccessControl::AccessControlExtensionChanged
};

const Entry<ItemInfo> _AccessControl_AccessControlTargetStruct[] = {
    { { ContextTag(0), "cluster", ItemType::kDefault }, kInvalidNodeIndex },    // AccessControl::cluster_id
    { { ContextTag(1), "endpoint", ItemType::kDefault }, kInvalidNodeIndex },   // AccessControl::endpoint_no
    { { ContextTag(2), "deviceType", ItemType::kDefault }, kInvalidNodeIndex }, // AccessControl::devtype_id
};

const Entry<ItemInfo> _AccessControl_AccessControlEntryStruct[] = {
    { { ContextTag(1), "privilege", ItemType::kEnum }, kInvalidNodeIndex },        // AccessControl::AccessControlEntryPrivilegeEnum
    { { ContextTag(2), "authMode", ItemType::kEnum }, kInvalidNodeIndex },         // AccessControl::AccessControlEntryAuthModeEnum
    { { ContextTag(3), "subjects", ItemType::kList }, kInvalidNodeIndex },         // AccessControl::int64u[]
    { { ContextTag(4), "targets", ItemType::kList }, 67 },                         // AccessControl::AccessControlTargetStruct[]
    { { ContextTag(254), "fabricIndex", ItemType::kDefault }, kInvalidNodeIndex }, // AccessControl::fabric_idx
};

const Entry<ItemInfo> _AccessControl_AccessControlExtensionStruct[] = {
    { { ContextTag(1), "data", ItemType::kDefault }, kInvalidNodeIndex },          // AccessControl::octet_string
    { { ContextTag(254), "fabricIndex", ItemType::kDefault }, kInvalidNodeIndex }, // AccessControl::fabric_idx
};

const Entry<ItemInfo> _AccessControl_AccessControlEntryChanged[] = {
    { { ContextTag(1), "adminNodeID", ItemType::kDefault }, kInvalidNodeIndex },     // AccessControl::node_id
    { { ContextTag(2), "adminPasscodeID", ItemType::kDefault }, kInvalidNodeIndex }, // AccessControl::INT16U
    { { ContextTag(3), "changeType", ItemType::kEnum }, kInvalidNodeIndex },         // AccessControl::ChangeTypeEnum
    { { ContextTag(4), "latestValue", ItemType::kDefault }, 62 },                    // AccessControl::AccessControlEntryStruct
    { { ContextTag(254), "fabricIndex", ItemType::kDefault }, kInvalidNodeIndex },   // AccessControl::fabric_idx
};

const Entry<ItemInfo> _AccessControl_AccessControlExtensionChanged[] = {
    { { ContextTag(1), "adminNodeID", ItemType::kDefault }, kInvalidNodeIndex },     // AccessControl::node_id
    { { ContextTag(2), "adminPasscodeID", ItemType::kDefault }, kInvalidNodeIndex }, // AccessControl::INT16U
    { { ContextTag(3), "changeType", ItemType::kEnum }, kInvalidNodeIndex },         // AccessControl::ChangeTypeEnum
    { { ContextTag(4), "latestValue", ItemType::kDefault }, 63 },                    // AccessControl::AccessControlExtensionStruct
    { { ContextTag(254), "fabricIndex", ItemType::kDefault }, kInvalidNodeIndex },   // AccessControl::fabric_idx
};

const Entry<ItemInfo> _AccessControl_AccessControlExtensionStruct_list_[] = {
    { { AnonymousTag(), "[]", ItemType::kDefault }, 63 }, // AccessControl_AccessControlExtensionStruct[]
};

const Entry<ItemInfo> _AccessControl_AccessControlTargetStruct_list_[] = {
    { { AnonymousTag(), "[]", ItemType::kDefault }, 61 }, // AccessControl_AccessControlTargetStruct[]
};

const Entry<ItemInfo> _AccessControl_AccessControlEntryStruct_list_[] = {
    { { AnonymousTag(), "[]", ItemType::kDefault }, 62 }, // AccessControl_AccessControlEntryStruct[]
};

const Entry<ItemInfo> _Actions[] = {
    { { AttributeTag(0), "actionList", ItemType::kList }, 86 },                              // Actions::ActionStruct[]
    { { AttributeTag(1), "endpointLists", ItemType::kList }, 87 },                           // Actions::EndpointListStruct[]
    { { AttributeTag(2), "setupURL", ItemType::kDefault }, kInvalidNodeIndex },              // Actions::long_char_string
    { { AttributeTag(65528), "generatedCommandList", ItemType::kList }, kInvalidNodeIndex }, // Actions::command_id[]
    { { AttributeTag(65529), "acceptedCommandList", ItemType::kList }, kInvalidNodeIndex },  // Actions::command_id[]
    { { AttributeTag(65530), "eventList", ItemType::kList }, kInvalidNodeIndex },            // Actions::event_id[]
    { { AttributeTag(65531), "attributeList", ItemType::kList }, kInvalidNodeIndex },        // Actions::attrib_id[]
    { { AttributeTag(65532), "featureMap", ItemType::kDefault }, kInvalidNodeIndex },        // Actions::bitmap32
    { { AttributeTag(65533), "clusterRevision", ItemType::kDefault }, kInvalidNodeIndex },   // Actions::int16u
    { { EventTag(0), "StateChanged", ItemType::kDefault }, 84 },                             // Actions::StateChanged
    { { EventTag(1), "ActionFailed", ItemType::kDefault }, 85 },                             // Actions::ActionFailed
    { { CommandTag(0), "InstantActionRequest", ItemType::kDefault }, 72 }, // Actions::InstantAction::InstantActionRequest
    { { CommandTag(1), "InstantActionWithTransitionRequest", ItemType::kDefault },
      73 }, // Actions::InstantActionWithTransition::InstantActionWithTransitionRequest
    { { CommandTag(2), "StartActionRequest", ItemType::kDefault }, 74 }, // Actions::StartAction::StartActionRequest
    { { CommandTag(3), "StartActionWithDurationRequest", ItemType::kDefault },
      75 }, // Actions::StartActionWithDuration::StartActionWithDurationRequest
    { { CommandTag(4), "StopActionRequest", ItemType::kDefault }, 76 },  // Actions::StopAction::StopActionRequest
    { { CommandTag(5), "PauseActionRequest", ItemType::kDefault }, 77 }, // Actions::PauseAction::PauseActionRequest
    { { CommandTag(6), "PauseActionWithDurationRequest", ItemType::kDefault },
      78 }, // Actions::PauseActionWithDuration::PauseActionWithDurationRequest
    { { CommandTag(7), "ResumeActionRequest", ItemType::kDefault }, 79 }, // Actions::ResumeAction::ResumeActionRequest
    { { CommandTag(8), "EnableActionRequest", ItemType::kDefault }, 80 }, // Actions::EnableAction::EnableActionRequest
    { { CommandTag(9), "EnableActionWithDurationRequest", ItemType::kDefault },
      81 }, // Actions::EnableActionWithDuration::EnableActionWithDurationRequest
    { { CommandTag(10), "DisableActionRequest", ItemType::kDefault }, 82 }, // Actions::DisableAction::DisableActionRequest
    { { CommandTag(11), "DisableActionWithDurationRequest", ItemType::kDefault },
      83 }, // Actions::DisableActionWithDuration::DisableActionWithDurationRequest
};

const Entry<ItemInfo> _Actions_ActionStruct[] = {
    { { ContextTag(0), "actionID", ItemType::kDefault }, kInvalidNodeIndex },         // Actions::int16u
    { { ContextTag(1), "name", ItemType::kDefault }, kInvalidNodeIndex },             // Actions::char_string
    { { ContextTag(2), "type", ItemType::kEnum }, kInvalidNodeIndex },                // Actions::ActionTypeEnum
    { { ContextTag(3), "endpointListID", ItemType::kDefault }, kInvalidNodeIndex },   // Actions::int16u
    { { ContextTag(4), "supportedCommands", ItemType::kBitmap }, kInvalidNodeIndex }, // Actions::CommandBits
    { { ContextTag(5), "state", ItemType::kEnum }, kInvalidNodeIndex },               // Actions::ActionStateEnum
};

const Entry<ItemInfo> _Actions_EndpointListStruct[] = {
    { { ContextTag(0), "endpointListID", ItemType::kDefault }, kInvalidNodeIndex }, // Actions::int16u
    { { ContextTag(1), "name", ItemType::kDefault }, kInvalidNodeIndex },           // Actions::char_string
    { { ContextTag(2), "type", ItemType::kEnum }, kInvalidNodeIndex },              // Actions::EndpointListTypeEnum
    { { ContextTag(3), "endpoints", ItemType::kList }, kInvalidNodeIndex },         // Actions::endpoint_no[]
};

const Entry<ItemInfo> _Actions_InstantActionRequest[] = {
    { { ContextTag(0), "actionID", ItemType::kDefault }, kInvalidNodeIndex }, // Actions::INT16U
    { { ContextTag(1), "invokeID", ItemType::kDefault }, kInvalidNodeIndex }, // Actions::INT32U
};

const Entry<ItemInfo> _Actions_InstantActionWithTransitionRequest[] = {
    { { ContextTag(0), "actionID", ItemType::kDefault }, kInvalidNodeIndex },       // Actions::INT16U
    { { ContextTag(1), "invokeID", ItemType::kDefault }, kInvalidNodeIndex },       // Actions::INT32U
    { { ContextTag(2), "transitionTime", ItemType::kDefault }, kInvalidNodeIndex }, // Actions::INT16U
};

const Entry<ItemInfo> _Actions_StartActionRequest[] = {
    { { ContextTag(0), "actionID", ItemType::kDefault }, kInvalidNodeIndex }, // Actions::INT16U
    { { ContextTag(1), "invokeID", ItemType::kDefault }, kInvalidNodeIndex }, // Actions::INT32U
};

const Entry<ItemInfo> _Actions_StartActionWithDurationRequest[] = {
    { { ContextTag(0), "actionID", ItemType::kDefault }, kInvalidNodeIndex }, // Actions::INT16U
    { { ContextTag(1), "invokeID", ItemType::kDefault }, kInvalidNodeIndex }, // Actions::INT32U
    { { ContextTag(2), "duration", ItemType::kDefault }, kInvalidNodeIndex }, // Actions::INT32U
};

const Entry<ItemInfo> _Actions_StopActionRequest[] = {
    { { ContextTag(0), "actionID", ItemType::kDefault }, kInvalidNodeIndex }, // Actions::INT16U
    { { ContextTag(1), "invokeID", ItemType::kDefault }, kInvalidNodeIndex }, // Actions::INT32U
};

const Entry<ItemInfo> _Actions_PauseActionRequest[] = {
    { { ContextTag(0), "actionID", ItemType::kDefault }, kInvalidNodeIndex }, // Actions::INT16U
    { { ContextTag(1), "invokeID", ItemType::kDefault }, kInvalidNodeIndex }, // Actions::INT32U
};

const Entry<ItemInfo> _Actions_PauseActionWithDurationRequest[] = {
    { { ContextTag(0), "actionID", ItemType::kDefault }, kInvalidNodeIndex }, // Actions::INT16U
    { { ContextTag(1), "invokeID", ItemType::kDefault }, kInvalidNodeIndex }, // Actions::INT32U
    { { ContextTag(2), "duration", ItemType::kDefault }, kInvalidNodeIndex }, // Actions::INT32U
};

const Entry<ItemInfo> _Actions_ResumeActionRequest[] = {
    { { ContextTag(0), "actionID", ItemType::kDefault }, kInvalidNodeIndex }, // Actions::INT16U
    { { ContextTag(1), "invokeID", ItemType::kDefault }, kInvalidNodeIndex }, // Actions::INT32U
};

const Entry<ItemInfo> _Actions_EnableActionRequest[] = {
    { { ContextTag(0), "actionID", ItemType::kDefault }, kInvalidNodeIndex }, // Actions::INT16U
    { { ContextTag(1), "invokeID", ItemType::kDefault }, kInvalidNodeIndex }, // Actions::INT32U
};

const Entry<ItemInfo> _Actions_EnableActionWithDurationRequest[] = {
    { { ContextTag(0), "actionID", ItemType::kDefault }, kInvalidNodeIndex }, // Actions::INT16U
    { { ContextTag(1), "invokeID", ItemType::kDefault }, kInvalidNodeIndex }, // Actions::INT32U
    { { ContextTag(2), "duration", ItemType::kDefault }, kInvalidNodeIndex }, // Actions::INT32U
};

const Entry<ItemInfo> _Actions_DisableActionRequest[] = {
    { { ContextTag(0), "actionID", ItemType::kDefault }, kInvalidNodeIndex }, // Actions::INT16U
    { { ContextTag(1), "invokeID", ItemType::kDefault }, kInvalidNodeIndex }, // Actions::INT32U
};

const Entry<ItemInfo> _Actions_DisableActionWithDurationRequest[] = {
    { { ContextTag(0), "actionID", ItemType::kDefault }, kInvalidNodeIndex }, // Actions::INT16U
    { { ContextTag(1), "invokeID", ItemType::kDefault }, kInvalidNodeIndex }, // Actions::INT32U
    { { ContextTag(2), "duration", ItemType::kDefault }, kInvalidNodeIndex }, // Actions::INT32U
};

const Entry<ItemInfo> _Actions_StateChanged[] = {
    { { ContextTag(0), "actionID", ItemType::kDefault }, kInvalidNodeIndex }, // Actions::INT16U
    { { ContextTag(1), "invokeID", ItemType::kDefault }, kInvalidNodeIndex }, // Actions::INT32U
    { { ContextTag(2), "newState", ItemType::kEnum }, kInvalidNodeIndex },    // Actions::ActionStateEnum
};

const Entry<ItemInfo> _Actions_ActionFailed[] = {
    { { ContextTag(0), "actionID", ItemType::kDefault }, kInvalidNodeIndex }, // Actions::INT16U
    { { ContextTag(1), "invokeID", ItemType::kDefault }, kInvalidNodeIndex }, // Actions::INT32U
    { { ContextTag(2), "newState", ItemType::kEnum }, kInvalidNodeIndex },    // Actions::ActionStateEnum
    { { ContextTag(3), "error", ItemType::kEnum }, kInvalidNodeIndex },       // Actions::ActionErrorEnum
};

const Entry<ItemInfo> _Actions_ActionStruct_list_[] = {
    { { AnonymousTag(), "[]", ItemType::kDefault }, 70 }, // Actions_ActionStruct[]
};

const Entry<ItemInfo> _Actions_EndpointListStruct_list_[] = {
    { { AnonymousTag(), "[]", ItemType::kDefault }, 71 }, // Actions_EndpointListStruct[]
};

const Entry<ItemInfo> _BasicInformation[] = {
    { { AttributeTag(0), "dataModelRevision", ItemType::kDefault }, kInvalidNodeIndex },      // BasicInformation::int16u
    { { AttributeTag(1), "vendorName", ItemType::kDefault }, kInvalidNodeIndex },             // BasicInformation::char_string
    { { AttributeTag(2), "vendorID", ItemType::kDefault }, kInvalidNodeIndex },               // BasicInformation::vendor_id
    { { AttributeTag(3), "productName", ItemType::kDefault }, kInvalidNodeIndex },            // BasicInformation::char_string
    { { AttributeTag(4), "productID", ItemType::kDefault }, kInvalidNodeIndex },              // BasicInformation::int16u
    { { AttributeTag(5), "nodeLabel", ItemType::kDefault }, kInvalidNodeIndex },              // BasicInformation::char_string
    { { AttributeTag(6), "location", ItemType::kDefault }, kInvalidNodeIndex },               // BasicInformation::char_string
    { { AttributeTag(7), "hardwareVersion", ItemType::kDefault }, kInvalidNodeIndex },        // BasicInformation::int16u
    { { AttributeTag(8), "hardwareVersionString", ItemType::kDefault }, kInvalidNodeIndex },  // BasicInformation::char_string
    { { AttributeTag(9), "softwareVersion", ItemType::kDefault }, kInvalidNodeIndex },        // BasicInformation::int32u
    { { AttributeTag(10), "softwareVersionString", ItemType::kDefault }, kInvalidNodeIndex }, // BasicInformation::char_string
    { { AttributeTag(11), "manufacturingDate", ItemType::kDefault }, kInvalidNodeIndex },     // BasicInformation::char_string
    { { AttributeTag(12), "partNumber", ItemType::kDefault }, kInvalidNodeIndex },            // BasicInformation::char_string
    { { AttributeTag(13), "productURL", ItemType::kDefault }, kInvalidNodeIndex },            // BasicInformation::long_char_string
    { { AttributeTag(14), "productLabel", ItemType::kDefault }, kInvalidNodeIndex },          // BasicInformation::char_string
    { { AttributeTag(15), "serialNumber", ItemType::kDefault }, kInvalidNodeIndex },          // BasicInformation::char_string
    { { AttributeTag(16), "localConfigDisabled", ItemType::kDefault }, kInvalidNodeIndex },   // BasicInformation::boolean
    { { AttributeTag(17), "reachable", ItemType::kDefault }, kInvalidNodeIndex },             // BasicInformation::boolean
    { { AttributeTag(18), "uniqueID", ItemType::kDefault }, kInvalidNodeIndex },              // BasicInformation::char_string
    { { AttributeTag(19), "capabilityMinima", ItemType::kDefault }, 89 },  // BasicInformation::CapabilityMinimaStruct
    { { AttributeTag(20), "productAppearance", ItemType::kDefault }, 90 }, // BasicInformation::ProductAppearanceStruct
    { { AttributeTag(65528), "generatedCommandList", ItemType::kList }, kInvalidNodeIndex }, // BasicInformation::command_id[]
    { { AttributeTag(65529), "acceptedCommandList", ItemType::kList }, kInvalidNodeIndex },  // BasicInformation::command_id[]
    { { AttributeTag(65530), "eventList", ItemType::kList }, kInvalidNodeIndex },            // BasicInformation::event_id[]
    { { AttributeTag(65531), "attributeList", ItemType::kList }, kInvalidNodeIndex },        // BasicInformation::attrib_id[]
    { { AttributeTag(65532), "featureMap", ItemType::kDefault }, kInvalidNodeIndex },        // BasicInformation::bitmap32
    { { AttributeTag(65533), "clusterRevision", ItemType::kDefault }, kInvalidNodeIndex },   // BasicInformation::int16u
    { { EventTag(0), "StartUp", ItemType::kDefault }, 91 },                                  // BasicInformation::StartUp
    { { EventTag(1), "ShutDown", ItemType::kDefault }, 92 },                                 // BasicInformation::ShutDown
    { { EventTag(2), "Leave", ItemType::kDefault }, 93 },                                    // BasicInformation::Leave
    { { EventTag(3), "ReachableChanged", ItemType::kDefault }, 94 },                         // BasicInformation::ReachableChanged
};

const Entry<ItemInfo> _BasicInformation_CapabilityMinimaStruct[] = {
    { { ContextTag(0), "caseSessionsPerFabric", ItemType::kDefault }, kInvalidNodeIndex },  // BasicInformation::int16u
    { { ContextTag(1), "subscriptionsPerFabric", ItemType::kDefault }, kInvalidNodeIndex }, // BasicInformation::int16u
};

const Entry<ItemInfo> _BasicInformation_ProductAppearanceStruct[] = {
    { { ContextTag(0), "finish", ItemType::kEnum }, kInvalidNodeIndex },       // BasicInformation::ProductFinishEnum
    { { ContextTag(1), "primaryColor", ItemType::kEnum }, kInvalidNodeIndex }, // BasicInformation::ColorEnum
};

const Entry<ItemInfo> _BasicInformation_StartUp[] = {
    { { ContextTag(0), "softwareVersion", ItemType::kDefault }, kInvalidNodeIndex }, // BasicInformation::INT32U
};

const Entry<ItemInfo> _BasicInformation_ShutDown[] = {};

const Entry<ItemInfo> _BasicInformation_Leave[] = {
    { { ContextTag(0), "fabricIndex", ItemType::kDefault }, kInvalidNodeIndex }, // BasicInformation::fabric_idx
};

const Entry<ItemInfo> _BasicInformation_ReachableChanged[] = {
    { { ContextTag(0), "reachableNewValue", ItemType::kDefault }, kInvalidNodeIndex }, // BasicInformation::boolean
};

const Entry<ItemInfo> _OtaSoftwareUpdateProvider[] = {
    { { AttributeTag(65528), "generatedCommandList", ItemType::kList },
      kInvalidNodeIndex }, // OtaSoftwareUpdateProvider::command_id[]
    { { AttributeTag(65529), "acceptedCommandList", ItemType::kList },
      kInvalidNodeIndex },                                                            // OtaSoftwareUpdateProvider::command_id[]
    { { AttributeTag(65530), "eventList", ItemType::kList }, kInvalidNodeIndex },     // OtaSoftwareUpdateProvider::event_id[]
    { { AttributeTag(65531), "attributeList", ItemType::kList }, kInvalidNodeIndex }, // OtaSoftwareUpdateProvider::attrib_id[]
    { { AttributeTag(65532), "featureMap", ItemType::kDefault }, kInvalidNodeIndex }, // OtaSoftwareUpdateProvider::bitmap32
    { { AttributeTag(65533), "clusterRevision", ItemType::kDefault }, kInvalidNodeIndex }, // OtaSoftwareUpdateProvider::int16u
    { { CommandTag(0), "QueryImageRequest", ItemType::kDefault }, 96 }, // OtaSoftwareUpdateProvider::QueryImage::QueryImageRequest
    { { CommandTag(2), "ApplyUpdateRequestRequest", ItemType::kDefault },
      98 }, // OtaSoftwareUpdateProvider::ApplyUpdateRequest::ApplyUpdateRequestRequest
    { { CommandTag(4), "NotifyUpdateAppliedRequest", ItemType::kDefault },
      100 }, // OtaSoftwareUpdateProvider::NotifyUpdateApplied::NotifyUpdateAppliedRequest
    { { CommandTag(1), "QueryImageResponse", ItemType::kDefault }, 97 },  // OtaSoftwareUpdateProvider::QueryImageResponse
    { { CommandTag(3), "ApplyUpdateResponse", ItemType::kDefault }, 99 }, // OtaSoftwareUpdateProvider::ApplyUpdateResponse
};

const Entry<ItemInfo> _OtaSoftwareUpdateProvider_QueryImageRequest[] = {
    { { ContextTag(0), "vendorID", ItemType::kDefault }, kInvalidNodeIndex },        // OtaSoftwareUpdateProvider::vendor_id
    { { ContextTag(1), "productID", ItemType::kDefault }, kInvalidNodeIndex },       // OtaSoftwareUpdateProvider::INT16U
    { { ContextTag(2), "softwareVersion", ItemType::kDefault }, kInvalidNodeIndex }, // OtaSoftwareUpdateProvider::INT32U
    { { ContextTag(3), "protocolsSupported", ItemType::kList },
      kInvalidNodeIndex }, // OtaSoftwareUpdateProvider::OTADownloadProtocol[]
    { { ContextTag(4), "hardwareVersion", ItemType::kDefault }, kInvalidNodeIndex },     // OtaSoftwareUpdateProvider::INT16U
    { { ContextTag(5), "location", ItemType::kDefault }, kInvalidNodeIndex },            // OtaSoftwareUpdateProvider::CHAR_STRING
    { { ContextTag(6), "requestorCanConsent", ItemType::kDefault }, kInvalidNodeIndex }, // OtaSoftwareUpdateProvider::BOOLEAN
    { { ContextTag(7), "metadataForProvider", ItemType::kDefault }, kInvalidNodeIndex }, // OtaSoftwareUpdateProvider::OCTET_STRING
};

const Entry<ItemInfo> _OtaSoftwareUpdateProvider_QueryImageResponse[] = {
    { { ContextTag(0), "status", ItemType::kEnum }, kInvalidNodeIndex },               // OtaSoftwareUpdateProvider::OTAQueryStatus
    { { ContextTag(1), "delayedActionTime", ItemType::kDefault }, kInvalidNodeIndex }, // OtaSoftwareUpdateProvider::INT32U
    { { ContextTag(2), "imageURI", ItemType::kDefault }, kInvalidNodeIndex },          // OtaSoftwareUpdateProvider::CHAR_STRING
    { { ContextTag(3), "softwareVersion", ItemType::kDefault }, kInvalidNodeIndex },   // OtaSoftwareUpdateProvider::INT32U
    { { ContextTag(4), "softwareVersionString", ItemType::kDefault }, kInvalidNodeIndex }, // OtaSoftwareUpdateProvider::CHAR_STRING
    { { ContextTag(5), "updateToken", ItemType::kDefault }, kInvalidNodeIndex },          // OtaSoftwareUpdateProvider::OCTET_STRING
    { { ContextTag(6), "userConsentNeeded", ItemType::kDefault }, kInvalidNodeIndex },    // OtaSoftwareUpdateProvider::BOOLEAN
    { { ContextTag(7), "metadataForRequestor", ItemType::kDefault }, kInvalidNodeIndex }, // OtaSoftwareUpdateProvider::OCTET_STRING
};

const Entry<ItemInfo> _OtaSoftwareUpdateProvider_ApplyUpdateRequestRequest[] = {
    { { ContextTag(0), "updateToken", ItemType::kDefault }, kInvalidNodeIndex }, // OtaSoftwareUpdateProvider::OCTET_STRING
    { { ContextTag(1), "newVersion", ItemType::kDefault }, kInvalidNodeIndex },  // OtaSoftwareUpdateProvider::INT32U
};

const Entry<ItemInfo> _OtaSoftwareUpdateProvider_ApplyUpdateResponse[] = {
    { { ContextTag(0), "action", ItemType::kEnum }, kInvalidNodeIndex }, // OtaSoftwareUpdateProvider::OTAApplyUpdateAction
    { { ContextTag(1), "delayedActionTime", ItemType::kDefault }, kInvalidNodeIndex }, // OtaSoftwareUpdateProvider::INT32U
};

const Entry<ItemInfo> _OtaSoftwareUpdateProvider_NotifyUpdateAppliedRequest[] = {
    { { ContextTag(0), "updateToken", ItemType::kDefault }, kInvalidNodeIndex },     // OtaSoftwareUpdateProvider::OCTET_STRING
    { { ContextTag(1), "softwareVersion", ItemType::kDefault }, kInvalidNodeIndex }, // OtaSoftwareUpdateProvider::INT32U
};

const Entry<ItemInfo> _OtaSoftwareUpdateRequestor[] = {
    { { AttributeTag(0), "defaultOTAProviders", ItemType::kList }, 107 }, // OtaSoftwareUpdateRequestor::ProviderLocation[]
    { { AttributeTag(1), "updatePossible", ItemType::kDefault }, kInvalidNodeIndex }, // OtaSoftwareUpdateRequestor::boolean
    { { AttributeTag(2), "updateState", ItemType::kEnum }, kInvalidNodeIndex }, // OtaSoftwareUpdateRequestor::OTAUpdateStateEnum
    { { AttributeTag(3), "updateStateProgress", ItemType::kDefault }, kInvalidNodeIndex }, // OtaSoftwareUpdateRequestor::int8u
    { { AttributeTag(65528), "generatedCommandList", ItemType::kList },
      kInvalidNodeIndex }, // OtaSoftwareUpdateRequestor::command_id[]
    { { AttributeTag(65529), "acceptedCommandList", ItemType::kList },
      kInvalidNodeIndex },                                                            // OtaSoftwareUpdateRequestor::command_id[]
    { { AttributeTag(65530), "eventList", ItemType::kList }, kInvalidNodeIndex },     // OtaSoftwareUpdateRequestor::event_id[]
    { { AttributeTag(65531), "attributeList", ItemType::kList }, kInvalidNodeIndex }, // OtaSoftwareUpdateRequestor::attrib_id[]
    { { AttributeTag(65532), "featureMap", ItemType::kDefault }, kInvalidNodeIndex }, // OtaSoftwareUpdateRequestor::bitmap32
    { { AttributeTag(65533), "clusterRevision", ItemType::kDefault }, kInvalidNodeIndex }, // OtaSoftwareUpdateRequestor::int16u
    { { EventTag(0), "StateTransition", ItemType::kDefault }, 104 }, // OtaSoftwareUpdateRequestor::StateTransition
    { { EventTag(1), "VersionApplied", ItemType::kDefault }, 105 },  // OtaSoftwareUpdateRequestor::VersionApplied
    { { EventTag(2), "DownloadError", ItemType::kDefault }, 106 },   // OtaSoftwareUpdateRequestor::DownloadError
    { { CommandTag(0), "AnnounceOTAProviderRequest", ItemType::kDefault },
      103 }, // OtaSoftwareUpdateRequestor::AnnounceOTAProvider::AnnounceOTAProviderRequest
};

const Entry<ItemInfo> _OtaSoftwareUpdateRequestor_ProviderLocation[] = {
    { { ContextTag(1), "providerNodeID", ItemType::kDefault }, kInvalidNodeIndex }, // OtaSoftwareUpdateRequestor::node_id
    { { ContextTag(2), "endpoint", ItemType::kDefault }, kInvalidNodeIndex },       // OtaSoftwareUpdateRequestor::endpoint_no
    { { ContextTag(254), "fabricIndex", ItemType::kDefault }, kInvalidNodeIndex },  // OtaSoftwareUpdateRequestor::fabric_idx
};

const Entry<ItemInfo> _OtaSoftwareUpdateRequestor_AnnounceOTAProviderRequest[] = {
    { { ContextTag(0), "providerNodeID", ItemType::kDefault }, kInvalidNodeIndex }, // OtaSoftwareUpdateRequestor::node_id
    { { ContextTag(1), "vendorID", ItemType::kDefault }, kInvalidNodeIndex },       // OtaSoftwareUpdateRequestor::vendor_id
    { { ContextTag(2), "announcementReason", ItemType::kEnum },
      kInvalidNodeIndex }, // OtaSoftwareUpdateRequestor::OTAAnnouncementReason
    { { ContextTag(3), "metadataForNode", ItemType::kDefault }, kInvalidNodeIndex }, // OtaSoftwareUpdateRequestor::OCTET_STRING
    { { ContextTag(4), "endpoint", ItemType::kDefault }, kInvalidNodeIndex },        // OtaSoftwareUpdateRequestor::endpoint_no
};

const Entry<ItemInfo> _OtaSoftwareUpdateRequestor_StateTransition[] = {
    { { ContextTag(0), "previousState", ItemType::kEnum }, kInvalidNodeIndex }, // OtaSoftwareUpdateRequestor::OTAUpdateStateEnum
    { { ContextTag(1), "newState", ItemType::kEnum }, kInvalidNodeIndex },      // OtaSoftwareUpdateRequestor::OTAUpdateStateEnum
    { { ContextTag(2), "reason", ItemType::kEnum }, kInvalidNodeIndex },        // OtaSoftwareUpdateRequestor::OTAChangeReasonEnum
    { { ContextTag(3), "targetSoftwareVersion", ItemType::kDefault }, kInvalidNodeIndex }, // OtaSoftwareUpdateRequestor::INT32U
};

const Entry<ItemInfo> _OtaSoftwareUpdateRequestor_VersionApplied[] = {
    { { ContextTag(0), "softwareVersion", ItemType::kDefault }, kInvalidNodeIndex }, // OtaSoftwareUpdateRequestor::INT32U
    { { ContextTag(1), "productID", ItemType::kDefault }, kInvalidNodeIndex },       // OtaSoftwareUpdateRequestor::INT16U
};

const Entry<ItemInfo> _OtaSoftwareUpdateRequestor_DownloadError[] = {
    { { ContextTag(0), "softwareVersion", ItemType::kDefault }, kInvalidNodeIndex }, // OtaSoftwareUpdateRequestor::INT32U
    { { ContextTag(1), "bytesDownloaded", ItemType::kDefault }, kInvalidNodeIndex }, // OtaSoftwareUpdateRequestor::INT64U
    { { ContextTag(2), "progressPercent", ItemType::kDefault }, kInvalidNodeIndex }, // OtaSoftwareUpdateRequestor::INT8U
    { { ContextTag(3), "platformCode", ItemType::kDefault }, kInvalidNodeIndex },    // OtaSoftwareUpdateRequestor::INT64S
};

const Entry<ItemInfo> _OtaSoftwareUpdateRequestor_ProviderLocation_list_[] = {
    { { AnonymousTag(), "[]", ItemType::kDefault }, 102 }, // OtaSoftwareUpdateRequestor_ProviderLocation[]
};

const Entry<ItemInfo> _LocalizationConfiguration[] = {
    { { AttributeTag(0), "activeLocale", ItemType::kDefault }, kInvalidNodeIndex },  // LocalizationConfiguration::char_string
    { { AttributeTag(1), "supportedLocales", ItemType::kList }, kInvalidNodeIndex }, // LocalizationConfiguration::CHAR_STRING[]
    { { AttributeTag(65528), "generatedCommandList", ItemType::kList },
      kInvalidNodeIndex }, // LocalizationConfiguration::command_id[]
    { { AttributeTag(65529), "acceptedCommandList", ItemType::kList },
      kInvalidNodeIndex },                                                            // LocalizationConfiguration::command_id[]
    { { AttributeTag(65530), "eventList", ItemType::kList }, kInvalidNodeIndex },     // LocalizationConfiguration::event_id[]
    { { AttributeTag(65531), "attributeList", ItemType::kList }, kInvalidNodeIndex }, // LocalizationConfiguration::attrib_id[]
    { { AttributeTag(65532), "featureMap", ItemType::kDefault }, kInvalidNodeIndex }, // LocalizationConfiguration::bitmap32
    { { AttributeTag(65533), "clusterRevision", ItemType::kDefault }, kInvalidNodeIndex }, // LocalizationConfiguration::int16u
};

const Entry<ItemInfo> _TimeFormatLocalization[] = {
    { { AttributeTag(0), "hourFormat", ItemType::kEnum }, kInvalidNodeIndex },         // TimeFormatLocalization::HourFormatEnum
    { { AttributeTag(1), "activeCalendarType", ItemType::kEnum }, kInvalidNodeIndex }, // TimeFormatLocalization::CalendarTypeEnum
    { { AttributeTag(2), "supportedCalendarTypes", ItemType::kList },
      kInvalidNodeIndex }, // TimeFormatLocalization::CalendarTypeEnum[]
    { { AttributeTag(65528), "generatedCommandList", ItemType::kList }, kInvalidNodeIndex }, // TimeFormatLocalization::command_id[]
    { { AttributeTag(65529), "acceptedCommandList", ItemType::kList }, kInvalidNodeIndex },  // TimeFormatLocalization::command_id[]
    { { AttributeTag(65530), "eventList", ItemType::kList }, kInvalidNodeIndex },            // TimeFormatLocalization::event_id[]
    { { AttributeTag(65531), "attributeList", ItemType::kList }, kInvalidNodeIndex },        // TimeFormatLocalization::attrib_id[]
    { { AttributeTag(65532), "featureMap", ItemType::kDefault }, kInvalidNodeIndex },        // TimeFormatLocalization::bitmap32
    { { AttributeTag(65533), "clusterRevision", ItemType::kDefault }, kInvalidNodeIndex },   // TimeFormatLocalization::int16u
};

const Entry<ItemInfo> _UnitLocalization[] = {
    { { AttributeTag(0), "temperatureUnit", ItemType::kEnum }, kInvalidNodeIndex },          // UnitLocalization::TempUnitEnum
    { { AttributeTag(65528), "generatedCommandList", ItemType::kList }, kInvalidNodeIndex }, // UnitLocalization::command_id[]
    { { AttributeTag(65529), "acceptedCommandList", ItemType::kList }, kInvalidNodeIndex },  // UnitLocalization::command_id[]
    { { AttributeTag(65530), "eventList", ItemType::kList }, kInvalidNodeIndex },            // UnitLocalization::event_id[]
    { { AttributeTag(65531), "attributeList", ItemType::kList }, kInvalidNodeIndex },        // UnitLocalization::attrib_id[]
    { { AttributeTag(65532), "featureMap", ItemType::kDefault }, kInvalidNodeIndex },        // UnitLocalization::bitmap32
    { { AttributeTag(65533), "clusterRevision", ItemType::kDefault }, kInvalidNodeIndex },   // UnitLocalization::int16u
};

const Entry<ItemInfo> _PowerSourceConfiguration[] = {
    { { AttributeTag(0), "sources", ItemType::kList }, kInvalidNodeIndex }, // PowerSourceConfiguration::INT8U[]
    { { AttributeTag(65528), "generatedCommandList", ItemType::kList },
      kInvalidNodeIndex }, // PowerSourceConfiguration::command_id[]
    { { AttributeTag(65529), "acceptedCommandList", ItemType::kList },
      kInvalidNodeIndex },                                                                 // PowerSourceConfiguration::command_id[]
    { { AttributeTag(65530), "eventList", ItemType::kList }, kInvalidNodeIndex },          // PowerSourceConfiguration::event_id[]
    { { AttributeTag(65531), "attributeList", ItemType::kList }, kInvalidNodeIndex },      // PowerSourceConfiguration::attrib_id[]
    { { AttributeTag(65532), "featureMap", ItemType::kDefault }, kInvalidNodeIndex },      // PowerSourceConfiguration::bitmap32
    { { AttributeTag(65533), "clusterRevision", ItemType::kDefault }, kInvalidNodeIndex }, // PowerSourceConfiguration::int16u
};

const Entry<ItemInfo> _PowerSource[] = {
    { { AttributeTag(0), "status", ItemType::kEnum }, kInvalidNodeIndex },         // PowerSource::PowerSourceStatusEnum
    { { AttributeTag(1), "order", ItemType::kDefault }, kInvalidNodeIndex },       // PowerSource::int8u
    { { AttributeTag(2), "description", ItemType::kDefault }, kInvalidNodeIndex }, // PowerSource::char_string
    { { AttributeTag(3), "wiredAssessedInputVoltage", ItemType::kDefault }, kInvalidNodeIndex },   // PowerSource::int32u
    { { AttributeTag(4), "wiredAssessedInputFrequency", ItemType::kDefault }, kInvalidNodeIndex }, // PowerSource::int16u
    { { AttributeTag(5), "wiredCurrentType", ItemType::kEnum }, kInvalidNodeIndex },         // PowerSource::WiredCurrentTypeEnum
    { { AttributeTag(6), "wiredAssessedCurrent", ItemType::kDefault }, kInvalidNodeIndex },  // PowerSource::int32u
    { { AttributeTag(7), "wiredNominalVoltage", ItemType::kDefault }, kInvalidNodeIndex },   // PowerSource::int32u
    { { AttributeTag(8), "wiredMaximumCurrent", ItemType::kDefault }, kInvalidNodeIndex },   // PowerSource::int32u
    { { AttributeTag(9), "wiredPresent", ItemType::kDefault }, kInvalidNodeIndex },          // PowerSource::boolean
    { { AttributeTag(10), "activeWiredFaults", ItemType::kList }, kInvalidNodeIndex },       // PowerSource::WiredFaultEnum[]
    { { AttributeTag(11), "batVoltage", ItemType::kDefault }, kInvalidNodeIndex },           // PowerSource::int32u
    { { AttributeTag(12), "batPercentRemaining", ItemType::kDefault }, kInvalidNodeIndex },  // PowerSource::int8u
    { { AttributeTag(13), "batTimeRemaining", ItemType::kDefault }, kInvalidNodeIndex },     // PowerSource::int32u
    { { AttributeTag(14), "batChargeLevel", ItemType::kEnum }, kInvalidNodeIndex },          // PowerSource::BatChargeLevelEnum
    { { AttributeTag(15), "batReplacementNeeded", ItemType::kDefault }, kInvalidNodeIndex }, // PowerSource::boolean
    { { AttributeTag(16), "batReplaceability", ItemType::kEnum }, kInvalidNodeIndex },       // PowerSource::BatReplaceabilityEnum
    { { AttributeTag(17), "batPresent", ItemType::kDefault }, kInvalidNodeIndex },           // PowerSource::boolean
    { { AttributeTag(18), "activeBatFaults", ItemType::kList }, kInvalidNodeIndex },         // PowerSource::BatFaultEnum[]
    { { AttributeTag(19), "batReplacementDescription", ItemType::kDefault }, kInvalidNodeIndex }, // PowerSource::char_string
    { { AttributeTag(20), "batCommonDesignation", ItemType::kEnum }, kInvalidNodeIndex },   // PowerSource::BatCommonDesignationEnum
    { { AttributeTag(21), "batANSIDesignation", ItemType::kDefault }, kInvalidNodeIndex },  // PowerSource::char_string
    { { AttributeTag(22), "batIECDesignation", ItemType::kDefault }, kInvalidNodeIndex },   // PowerSource::char_string
    { { AttributeTag(23), "batApprovedChemistry", ItemType::kEnum }, kInvalidNodeIndex },   // PowerSource::BatApprovedChemistryEnum
    { { AttributeTag(24), "batCapacity", ItemType::kDefault }, kInvalidNodeIndex },         // PowerSource::int32u
    { { AttributeTag(25), "batQuantity", ItemType::kDefault }, kInvalidNodeIndex },         // PowerSource::int8u
    { { AttributeTag(26), "batChargeState", ItemType::kEnum }, kInvalidNodeIndex },         // PowerSource::BatChargeStateEnum
    { { AttributeTag(27), "batTimeToFullCharge", ItemType::kDefault }, kInvalidNodeIndex }, // PowerSource::int32u
    { { AttributeTag(28), "batFunctionalWhileCharging", ItemType::kDefault }, kInvalidNodeIndex }, // PowerSource::boolean
    { { AttributeTag(29), "batChargingCurrent", ItemType::kDefault }, kInvalidNodeIndex },         // PowerSource::int32u
    { { AttributeTag(30), "activeBatChargeFaults", ItemType::kList }, kInvalidNodeIndex },   // PowerSource::BatChargeFaultEnum[]
    { { AttributeTag(65528), "generatedCommandList", ItemType::kList }, kInvalidNodeIndex }, // PowerSource::command_id[]
    { { AttributeTag(65529), "acceptedCommandList", ItemType::kList }, kInvalidNodeIndex },  // PowerSource::command_id[]
    { { AttributeTag(65530), "eventList", ItemType::kList }, kInvalidNodeIndex },            // PowerSource::event_id[]
    { { AttributeTag(65531), "attributeList", ItemType::kList }, kInvalidNodeIndex },        // PowerSource::attrib_id[]
    { { AttributeTag(65532), "featureMap", ItemType::kDefault }, kInvalidNodeIndex },        // PowerSource::bitmap32
    { { AttributeTag(65533), "clusterRevision", ItemType::kDefault }, kInvalidNodeIndex },   // PowerSource::int16u
    { { EventTag(0), "WiredFaultChange", ItemType::kDefault }, 116 },                        // PowerSource::WiredFaultChange
    { { EventTag(1), "BatFaultChange", ItemType::kDefault }, 117 },                          // PowerSource::BatFaultChange
    { { EventTag(2), "BatChargeFaultChange", ItemType::kDefault }, 118 },                    // PowerSource::BatChargeFaultChange
};

const Entry<ItemInfo> _PowerSource_BatChargeFaultChangeType[] = {
    { { ContextTag(0), "current", ItemType::kList }, kInvalidNodeIndex },  // PowerSource::BatChargeFaultEnum[]
    { { ContextTag(1), "previous", ItemType::kList }, kInvalidNodeIndex }, // PowerSource::BatChargeFaultEnum[]
};

const Entry<ItemInfo> _PowerSource_BatFaultChangeType[] = {
    { { ContextTag(0), "current", ItemType::kList }, kInvalidNodeIndex },  // PowerSource::BatFaultEnum[]
    { { ContextTag(1), "previous", ItemType::kList }, kInvalidNodeIndex }, // PowerSource::BatFaultEnum[]
};

const Entry<ItemInfo> _PowerSource_WiredFaultChangeType[] = {
    { { ContextTag(0), "current", ItemType::kList }, kInvalidNodeIndex },  // PowerSource::WiredFaultEnum[]
    { { ContextTag(1), "previous", ItemType::kList }, kInvalidNodeIndex }, // PowerSource::WiredFaultEnum[]
};

const Entry<ItemInfo> _PowerSource_WiredFaultChange[] = {
    { { ContextTag(0), "current", ItemType::kList }, kInvalidNodeIndex },  // PowerSource::WiredFaultEnum[]
    { { ContextTag(1), "previous", ItemType::kList }, kInvalidNodeIndex }, // PowerSource::WiredFaultEnum[]
};

const Entry<ItemInfo> _PowerSource_BatFaultChange[] = {
    { { ContextTag(0), "current", ItemType::kList }, kInvalidNodeIndex },  // PowerSource::BatFaultEnum[]
    { { ContextTag(1), "previous", ItemType::kList }, kInvalidNodeIndex }, // PowerSource::BatFaultEnum[]
};

const Entry<ItemInfo> _PowerSource_BatChargeFaultChange[] = {
    { { ContextTag(0), "current", ItemType::kList }, kInvalidNodeIndex },  // PowerSource::BatChargeFaultEnum[]
    { { ContextTag(1), "previous", ItemType::kList }, kInvalidNodeIndex }, // PowerSource::BatChargeFaultEnum[]
};

const Entry<ItemInfo> _GeneralCommissioning[] = {
    { { AttributeTag(0), "breadcrumb", ItemType::kDefault }, kInvalidNodeIndex }, // GeneralCommissioning::int64u
    { { AttributeTag(1), "basicCommissioningInfo", ItemType::kDefault }, 120 },   // GeneralCommissioning::BasicCommissioningInfo
    { { AttributeTag(2), "regulatoryConfig", ItemType::kEnum },
      kInvalidNodeIndex }, // GeneralCommissioning::RegulatoryLocationTypeEnum
    { { AttributeTag(3), "locationCapability", ItemType::kEnum },
      kInvalidNodeIndex }, // GeneralCommissioning::RegulatoryLocationTypeEnum
    { { AttributeTag(4), "supportsConcurrentConnection", ItemType::kDefault }, kInvalidNodeIndex }, // GeneralCommissioning::boolean
    { { AttributeTag(65528), "generatedCommandList", ItemType::kList }, kInvalidNodeIndex }, // GeneralCommissioning::command_id[]
    { { AttributeTag(65529), "acceptedCommandList", ItemType::kList }, kInvalidNodeIndex },  // GeneralCommissioning::command_id[]
    { { AttributeTag(65530), "eventList", ItemType::kList }, kInvalidNodeIndex },            // GeneralCommissioning::event_id[]
    { { AttributeTag(65531), "attributeList", ItemType::kList }, kInvalidNodeIndex },        // GeneralCommissioning::attrib_id[]
    { { AttributeTag(65532), "featureMap", ItemType::kDefault }, kInvalidNodeIndex },        // GeneralCommissioning::bitmap32
    { { AttributeTag(65533), "clusterRevision", ItemType::kDefault }, kInvalidNodeIndex },   // GeneralCommissioning::int16u
    { { CommandTag(0), "ArmFailSafeRequest", ItemType::kDefault }, 121 }, // GeneralCommissioning::ArmFailSafe::ArmFailSafeRequest
    { { CommandTag(2), "SetRegulatoryConfigRequest", ItemType::kDefault },
      123 }, // GeneralCommissioning::SetRegulatoryConfig::SetRegulatoryConfigRequest
    { { CommandTag(1), "ArmFailSafeResponse", ItemType::kDefault }, 122 }, // GeneralCommissioning::ArmFailSafeResponse
    { { CommandTag(3), "SetRegulatoryConfigResponse", ItemType::kDefault },
      124 }, // GeneralCommissioning::SetRegulatoryConfigResponse
    { { CommandTag(5), "CommissioningCompleteResponse", ItemType::kDefault },
      125 }, // GeneralCommissioning::CommissioningCompleteResponse
};

const Entry<ItemInfo> _GeneralCommissioning_BasicCommissioningInfo[] = {
    { { ContextTag(0), "failSafeExpiryLengthSeconds", ItemType::kDefault }, kInvalidNodeIndex },  // GeneralCommissioning::int16u
    { { ContextTag(1), "maxCumulativeFailsafeSeconds", ItemType::kDefault }, kInvalidNodeIndex }, // GeneralCommissioning::int16u
};

const Entry<ItemInfo> _GeneralCommissioning_ArmFailSafeRequest[] = {
    { { ContextTag(0), "expiryLengthSeconds", ItemType::kDefault }, kInvalidNodeIndex }, // GeneralCommissioning::INT16U
    { { ContextTag(1), "breadcrumb", ItemType::kDefault }, kInvalidNodeIndex },          // GeneralCommissioning::INT64U
};

const Entry<ItemInfo> _GeneralCommissioning_ArmFailSafeResponse[] = {
    { { ContextTag(0), "errorCode", ItemType::kEnum }, kInvalidNodeIndex },    // GeneralCommissioning::CommissioningErrorEnum
    { { ContextTag(1), "debugText", ItemType::kDefault }, kInvalidNodeIndex }, // GeneralCommissioning::CHAR_STRING
};

const Entry<ItemInfo> _GeneralCommissioning_SetRegulatoryConfigRequest[] = {
    { { ContextTag(0), "newRegulatoryConfig", ItemType::kEnum },
      kInvalidNodeIndex },                                                       // GeneralCommissioning::RegulatoryLocationTypeEnum
    { { ContextTag(1), "countryCode", ItemType::kDefault }, kInvalidNodeIndex }, // GeneralCommissioning::CHAR_STRING
    { { ContextTag(2), "breadcrumb", ItemType::kDefault }, kInvalidNodeIndex },  // GeneralCommissioning::INT64U
};

const Entry<ItemInfo> _GeneralCommissioning_SetRegulatoryConfigResponse[] = {
    { { ContextTag(0), "errorCode", ItemType::kEnum }, kInvalidNodeIndex },    // GeneralCommissioning::CommissioningErrorEnum
    { { ContextTag(1), "debugText", ItemType::kDefault }, kInvalidNodeIndex }, // GeneralCommissioning::CHAR_STRING
};

const Entry<ItemInfo> _GeneralCommissioning_CommissioningCompleteResponse[] = {
    { { ContextTag(0), "errorCode", ItemType::kEnum }, kInvalidNodeIndex },    // GeneralCommissioning::CommissioningErrorEnum
    { { ContextTag(1), "debugText", ItemType::kDefault }, kInvalidNodeIndex }, // GeneralCommissioning::CHAR_STRING
};

const Entry<ItemInfo> _NetworkCommissioning[] = {
    { { AttributeTag(0), "maxNetworks", ItemType::kDefault }, kInvalidNodeIndex }, // NetworkCommissioning::int8u
    { { AttributeTag(1), "networks", ItemType::kList }, 140 },                     // NetworkCommissioning::NetworkInfoStruct[]
    { { AttributeTag(2), "scanMaxTimeSeconds", ItemType::kDefault }, kInvalidNodeIndex },    // NetworkCommissioning::int8u
    { { AttributeTag(3), "connectMaxTimeSeconds", ItemType::kDefault }, kInvalidNodeIndex }, // NetworkCommissioning::int8u
    { { AttributeTag(4), "interfaceEnabled", ItemType::kDefault }, kInvalidNodeIndex },      // NetworkCommissioning::boolean
    { { AttributeTag(5), "lastNetworkingStatus", ItemType::kEnum },
      kInvalidNodeIndex }, // NetworkCommissioning::NetworkCommissioningStatusEnum
    { { AttributeTag(6), "lastNetworkID", ItemType::kDefault }, kInvalidNodeIndex },         // NetworkCommissioning::octet_string
    { { AttributeTag(7), "lastConnectErrorValue", ItemType::kDefault }, kInvalidNodeIndex }, // NetworkCommissioning::int32s
    { { AttributeTag(65528), "generatedCommandList", ItemType::kList }, kInvalidNodeIndex }, // NetworkCommissioning::command_id[]
    { { AttributeTag(65529), "acceptedCommandList", ItemType::kList }, kInvalidNodeIndex },  // NetworkCommissioning::command_id[]
    { { AttributeTag(65530), "eventList", ItemType::kList }, kInvalidNodeIndex },            // NetworkCommissioning::event_id[]
    { { AttributeTag(65531), "attributeList", ItemType::kList }, kInvalidNodeIndex },        // NetworkCommissioning::attrib_id[]
    { { AttributeTag(65532), "featureMap", ItemType::kDefault }, kInvalidNodeIndex },        // NetworkCommissioning::bitmap32
    { { AttributeTag(65533), "clusterRevision", ItemType::kDefault }, kInvalidNodeIndex },   // NetworkCommissioning::int16u
    { { CommandTag(0), "ScanNetworksRequest", ItemType::kDefault },
      130 }, // NetworkCommissioning::ScanNetworks::ScanNetworksRequest
    { { CommandTag(2), "AddOrUpdateWiFiNetworkRequest", ItemType::kDefault },
      132 }, // NetworkCommissioning::AddOrUpdateWiFiNetwork::AddOrUpdateWiFiNetworkRequest
    { { CommandTag(3), "AddOrUpdateThreadNetworkRequest", ItemType::kDefault },
      133 }, // NetworkCommissioning::AddOrUpdateThreadNetwork::AddOrUpdateThreadNetworkRequest
    { { CommandTag(4), "RemoveNetworkRequest", ItemType::kDefault },
      134 }, // NetworkCommissioning::RemoveNetwork::RemoveNetworkRequest
    { { CommandTag(6), "ConnectNetworkRequest", ItemType::kDefault },
      136 }, // NetworkCommissioning::ConnectNetwork::ConnectNetworkRequest
    { { CommandTag(8), "ReorderNetworkRequest", ItemType::kDefault },
      138 }, // NetworkCommissioning::ReorderNetwork::ReorderNetworkRequest
    { { CommandTag(1), "ScanNetworksResponse", ItemType::kDefault }, 131 },   // NetworkCommissioning::ScanNetworksResponse
    { { CommandTag(5), "NetworkConfigResponse", ItemType::kDefault }, 135 },  // NetworkCommissioning::NetworkConfigResponse
    { { CommandTag(7), "ConnectNetworkResponse", ItemType::kDefault }, 137 }, // NetworkCommissioning::ConnectNetworkResponse
};

const Entry<ItemInfo> _NetworkCommissioning_NetworkInfoStruct[] = {
    { { ContextTag(0), "networkID", ItemType::kDefault }, kInvalidNodeIndex }, // NetworkCommissioning::octet_string
    { { ContextTag(1), "connected", ItemType::kDefault }, kInvalidNodeIndex }, // NetworkCommissioning::boolean
};

const Entry<ItemInfo> _NetworkCommissioning_ThreadInterfaceScanResultStruct[] = {
    { { ContextTag(0), "panId", ItemType::kDefault }, kInvalidNodeIndex },           // NetworkCommissioning::int16u
    { { ContextTag(1), "extendedPanId", ItemType::kDefault }, kInvalidNodeIndex },   // NetworkCommissioning::int64u
    { { ContextTag(2), "networkName", ItemType::kDefault }, kInvalidNodeIndex },     // NetworkCommissioning::char_string
    { { ContextTag(3), "channel", ItemType::kDefault }, kInvalidNodeIndex },         // NetworkCommissioning::int16u
    { { ContextTag(4), "version", ItemType::kDefault }, kInvalidNodeIndex },         // NetworkCommissioning::int8u
    { { ContextTag(5), "extendedAddress", ItemType::kDefault }, kInvalidNodeIndex }, // NetworkCommissioning::octet_string
    { { ContextTag(6), "rssi", ItemType::kDefault }, kInvalidNodeIndex },            // NetworkCommissioning::int8s
    { { ContextTag(7), "lqi", ItemType::kDefault }, kInvalidNodeIndex },             // NetworkCommissioning::int8u
};

const Entry<ItemInfo> _NetworkCommissioning_WiFiInterfaceScanResultStruct[] = {
    { { ContextTag(0), "security", ItemType::kBitmap }, kInvalidNodeIndex }, // NetworkCommissioning::WiFiSecurityBitmap
    { { ContextTag(1), "ssid", ItemType::kDefault }, kInvalidNodeIndex },    // NetworkCommissioning::octet_string
    { { ContextTag(2), "bssid", ItemType::kDefault }, kInvalidNodeIndex },   // NetworkCommissioning::octet_string
    { { ContextTag(3), "channel", ItemType::kDefault }, kInvalidNodeIndex }, // NetworkCommissioning::int16u
    { { ContextTag(4), "wiFiBand", ItemType::kEnum }, kInvalidNodeIndex },   // NetworkCommissioning::WiFiBandEnum
    { { ContextTag(5), "rssi", ItemType::kDefault }, kInvalidNodeIndex },    // NetworkCommissioning::int8s
};

const Entry<ItemInfo> _NetworkCommissioning_ScanNetworksRequest[] = {
    { { ContextTag(0), "ssid", ItemType::kDefault }, kInvalidNodeIndex },       // NetworkCommissioning::OCTET_STRING
    { { ContextTag(1), "breadcrumb", ItemType::kDefault }, kInvalidNodeIndex }, // NetworkCommissioning::INT64U
};

const Entry<ItemInfo> _NetworkCommissioning_ScanNetworksResponse[] = {
    { { ContextTag(0), "networkingStatus", ItemType::kEnum },
      kInvalidNodeIndex }, // NetworkCommissioning::NetworkCommissioningStatusEnum
    { { ContextTag(1), "debugText", ItemType::kDefault }, kInvalidNodeIndex }, // NetworkCommissioning::CHAR_STRING
    { { ContextTag(2), "wiFiScanResults", ItemType::kList }, 139 },   // NetworkCommissioning::WiFiInterfaceScanResultStruct[]
    { { ContextTag(3), "threadScanResults", ItemType::kList }, 141 }, // NetworkCommissioning::ThreadInterfaceScanResultStruct[]
};

const Entry<ItemInfo> _NetworkCommissioning_AddOrUpdateWiFiNetworkRequest[] = {
    { { ContextTag(0), "ssid", ItemType::kDefault }, kInvalidNodeIndex },        // NetworkCommissioning::OCTET_STRING
    { { ContextTag(1), "credentials", ItemType::kDefault }, kInvalidNodeIndex }, // NetworkCommissioning::OCTET_STRING
    { { ContextTag(2), "breadcrumb", ItemType::kDefault }, kInvalidNodeIndex },  // NetworkCommissioning::INT64U
};

const Entry<ItemInfo> _NetworkCommissioning_AddOrUpdateThreadNetworkRequest[] = {
    { { ContextTag(0), "operationalDataset", ItemType::kDefault }, kInvalidNodeIndex }, // NetworkCommissioning::OCTET_STRING
    { { ContextTag(1), "breadcrumb", ItemType::kDefault }, kInvalidNodeIndex },         // NetworkCommissioning::INT64U
};

const Entry<ItemInfo> _NetworkCommissioning_RemoveNetworkRequest[] = {
    { { ContextTag(0), "networkID", ItemType::kDefault }, kInvalidNodeIndex },  // NetworkCommissioning::OCTET_STRING
    { { ContextTag(1), "breadcrumb", ItemType::kDefault }, kInvalidNodeIndex }, // NetworkCommissioning::INT64U
};

const Entry<ItemInfo> _NetworkCommissioning_NetworkConfigResponse[] = {
    { { ContextTag(0), "networkingStatus", ItemType::kEnum },
      kInvalidNodeIndex }, // NetworkCommissioning::NetworkCommissioningStatusEnum
    { { ContextTag(1), "debugText", ItemType::kDefault }, kInvalidNodeIndex },    // NetworkCommissioning::CHAR_STRING
    { { ContextTag(2), "networkIndex", ItemType::kDefault }, kInvalidNodeIndex }, // NetworkCommissioning::INT8U
};

const Entry<ItemInfo> _NetworkCommissioning_ConnectNetworkRequest[] = {
    { { ContextTag(0), "networkID", ItemType::kDefault }, kInvalidNodeIndex },  // NetworkCommissioning::OCTET_STRING
    { { ContextTag(1), "breadcrumb", ItemType::kDefault }, kInvalidNodeIndex }, // NetworkCommissioning::INT64U
};

const Entry<ItemInfo> _NetworkCommissioning_ConnectNetworkResponse[] = {
    { { ContextTag(0), "networkingStatus", ItemType::kEnum },
      kInvalidNodeIndex }, // NetworkCommissioning::NetworkCommissioningStatusEnum
    { { ContextTag(1), "debugText", ItemType::kDefault }, kInvalidNodeIndex },  // NetworkCommissioning::CHAR_STRING
    { { ContextTag(2), "errorValue", ItemType::kDefault }, kInvalidNodeIndex }, // NetworkCommissioning::INT32S
};

const Entry<ItemInfo> _NetworkCommissioning_ReorderNetworkRequest[] = {
    { { ContextTag(0), "networkID", ItemType::kDefault }, kInvalidNodeIndex },    // NetworkCommissioning::OCTET_STRING
    { { ContextTag(1), "networkIndex", ItemType::kDefault }, kInvalidNodeIndex }, // NetworkCommissioning::INT8U
    { { ContextTag(2), "breadcrumb", ItemType::kDefault }, kInvalidNodeIndex },   // NetworkCommissioning::INT64U
};

const Entry<ItemInfo> _NetworkCommissioning_WiFiInterfaceScanResultStruct_list_[] = {
    { { AnonymousTag(), "[]", ItemType::kDefault }, 129 }, // NetworkCommissioning_WiFiInterfaceScanResultStruct[]
};

const Entry<ItemInfo> _NetworkCommissioning_NetworkInfoStruct_list_[] = {
    { { AnonymousTag(), "[]", ItemType::kDefault }, 127 }, // NetworkCommissioning_NetworkInfoStruct[]
};

const Entry<ItemInfo> _NetworkCommissioning_ThreadInterfaceScanResultStruct_list_[] = {
    { { AnonymousTag(), "[]", ItemType::kDefault }, 128 }, // NetworkCommissioning_ThreadInterfaceScanResultStruct[]
};

const Entry<ItemInfo> _DiagnosticLogs[] = {
    { { AttributeTag(65528), "generatedCommandList", ItemType::kList }, kInvalidNodeIndex }, // DiagnosticLogs::command_id[]
    { { AttributeTag(65529), "acceptedCommandList", ItemType::kList }, kInvalidNodeIndex },  // DiagnosticLogs::command_id[]
    { { AttributeTag(65530), "eventList", ItemType::kList }, kInvalidNodeIndex },            // DiagnosticLogs::event_id[]
    { { AttributeTag(65531), "attributeList", ItemType::kList }, kInvalidNodeIndex },        // DiagnosticLogs::attrib_id[]
    { { AttributeTag(65532), "featureMap", ItemType::kDefault }, kInvalidNodeIndex },        // DiagnosticLogs::bitmap32
    { { AttributeTag(65533), "clusterRevision", ItemType::kDefault }, kInvalidNodeIndex },   // DiagnosticLogs::int16u
    { { CommandTag(0), "RetrieveLogsRequestRequest", ItemType::kDefault },
      143 }, // DiagnosticLogs::RetrieveLogsRequest::RetrieveLogsRequestRequest
    { { CommandTag(1), "RetrieveLogsResponse", ItemType::kDefault }, 144 }, // DiagnosticLogs::RetrieveLogsResponse
};

const Entry<ItemInfo> _DiagnosticLogs_RetrieveLogsRequestRequest[] = {
    { { ContextTag(0), "intent", ItemType::kEnum }, kInvalidNodeIndex },                    // DiagnosticLogs::IntentEnum
    { { ContextTag(1), "requestedProtocol", ItemType::kEnum }, kInvalidNodeIndex },         // DiagnosticLogs::TransferProtocolEnum
    { { ContextTag(2), "transferFileDesignator", ItemType::kDefault }, kInvalidNodeIndex }, // DiagnosticLogs::CHAR_STRING
};

const Entry<ItemInfo> _DiagnosticLogs_RetrieveLogsResponse[] = {
    { { ContextTag(0), "status", ItemType::kEnum }, kInvalidNodeIndex },           // DiagnosticLogs::StatusEnum
    { { ContextTag(1), "logContent", ItemType::kDefault }, kInvalidNodeIndex },    // DiagnosticLogs::LONG_OCTET_STRING
    { { ContextTag(2), "UTCTimeStamp", ItemType::kDefault }, kInvalidNodeIndex },  // DiagnosticLogs::epoch_us
    { { ContextTag(3), "timeSinceBoot", ItemType::kDefault }, kInvalidNodeIndex }, // DiagnosticLogs::systime_us
};

const Entry<ItemInfo> _GeneralDiagnostics[] = {
    { { AttributeTag(0), "networkInterfaces", ItemType::kList }, 152 },            // GeneralDiagnostics::NetworkInterface[]
    { { AttributeTag(1), "rebootCount", ItemType::kDefault }, kInvalidNodeIndex }, // GeneralDiagnostics::int16u
    { { AttributeTag(2), "upTime", ItemType::kDefault }, kInvalidNodeIndex },      // GeneralDiagnostics::int64u
    { { AttributeTag(3), "totalOperationalHours", ItemType::kDefault }, kInvalidNodeIndex }, // GeneralDiagnostics::int32u
    { { AttributeTag(4), "bootReason", ItemType::kEnum }, kInvalidNodeIndex },               // GeneralDiagnostics::BootReasonEnum
    { { AttributeTag(5), "activeHardwareFaults", ItemType::kList }, kInvalidNodeIndex }, // GeneralDiagnostics::HardwareFaultEnum[]
    { { AttributeTag(6), "activeRadioFaults", ItemType::kList }, kInvalidNodeIndex },    // GeneralDiagnostics::RadioFaultEnum[]
    { { AttributeTag(7), "activeNetworkFaults", ItemType::kList }, kInvalidNodeIndex },  // GeneralDiagnostics::NetworkFaultEnum[]
    { { AttributeTag(8), "testEventTriggersEnabled", ItemType::kDefault }, kInvalidNodeIndex }, // GeneralDiagnostics::boolean
    { { AttributeTag(65528), "generatedCommandList", ItemType::kList }, kInvalidNodeIndex },    // GeneralDiagnostics::command_id[]
    { { AttributeTag(65529), "acceptedCommandList", ItemType::kList }, kInvalidNodeIndex },     // GeneralDiagnostics::command_id[]
    { { AttributeTag(65530), "eventList", ItemType::kList }, kInvalidNodeIndex },               // GeneralDiagnostics::event_id[]
    { { AttributeTag(65531), "attributeList", ItemType::kList }, kInvalidNodeIndex },           // GeneralDiagnostics::attrib_id[]
    { { AttributeTag(65532), "featureMap", ItemType::kDefault }, kInvalidNodeIndex },           // GeneralDiagnostics::bitmap32
    { { AttributeTag(65533), "clusterRevision", ItemType::kDefault }, kInvalidNodeIndex },      // GeneralDiagnostics::int16u
    { { EventTag(0), "HardwareFaultChange", ItemType::kDefault }, 148 }, // GeneralDiagnostics::HardwareFaultChange
    { { EventTag(1), "RadioFaultChange", ItemType::kDefault }, 149 },    // GeneralDiagnostics::RadioFaultChange
    { { EventTag(2), "NetworkFaultChange", ItemType::kDefault }, 150 },  // GeneralDiagnostics::NetworkFaultChange
    { { EventTag(3), "BootReason", ItemType::kDefault }, 151 },          // GeneralDiagnostics::BootReason
    { { CommandTag(0), "TestEventTriggerRequest", ItemType::kDefault },
      147 }, // GeneralDiagnostics::TestEventTrigger::TestEventTriggerRequest
};

const Entry<ItemInfo> _GeneralDiagnostics_NetworkInterface[] = {
    { { ContextTag(0), "name", ItemType::kDefault }, kInvalidNodeIndex },          // GeneralDiagnostics::char_string
    { { ContextTag(1), "isOperational", ItemType::kDefault }, kInvalidNodeIndex }, // GeneralDiagnostics::boolean
    { { ContextTag(2), "offPremiseServicesReachableIPv4", ItemType::kDefault }, kInvalidNodeIndex }, // GeneralDiagnostics::boolean
    { { ContextTag(3), "offPremiseServicesReachableIPv6", ItemType::kDefault }, kInvalidNodeIndex }, // GeneralDiagnostics::boolean
    { { ContextTag(4), "hardwareAddress", ItemType::kDefault }, kInvalidNodeIndex }, // GeneralDiagnostics::octet_string
    { { ContextTag(5), "IPv4Addresses", ItemType::kList }, kInvalidNodeIndex },      // GeneralDiagnostics::octet_string[]
    { { ContextTag(6), "IPv6Addresses", ItemType::kList }, kInvalidNodeIndex },      // GeneralDiagnostics::octet_string[]
    { { ContextTag(7), "type", ItemType::kEnum }, kInvalidNodeIndex },               // GeneralDiagnostics::InterfaceTypeEnum
};

const Entry<ItemInfo> _GeneralDiagnostics_TestEventTriggerRequest[] = {
    { { ContextTag(0), "enableKey", ItemType::kDefault }, kInvalidNodeIndex },    // GeneralDiagnostics::OCTET_STRING
    { { ContextTag(1), "eventTrigger", ItemType::kDefault }, kInvalidNodeIndex }, // GeneralDiagnostics::INT64U
};

const Entry<ItemInfo> _GeneralDiagnostics_HardwareFaultChange[] = {
    { { ContextTag(0), "current", ItemType::kList }, kInvalidNodeIndex },  // GeneralDiagnostics::HardwareFaultEnum[]
    { { ContextTag(1), "previous", ItemType::kList }, kInvalidNodeIndex }, // GeneralDiagnostics::HardwareFaultEnum[]
};

const Entry<ItemInfo> _GeneralDiagnostics_RadioFaultChange[] = {
    { { ContextTag(0), "current", ItemType::kList }, kInvalidNodeIndex },  // GeneralDiagnostics::RadioFaultEnum[]
    { { ContextTag(1), "previous", ItemType::kList }, kInvalidNodeIndex }, // GeneralDiagnostics::RadioFaultEnum[]
};

const Entry<ItemInfo> _GeneralDiagnostics_NetworkFaultChange[] = {
    { { ContextTag(0), "current", ItemType::kList }, kInvalidNodeIndex },  // GeneralDiagnostics::NetworkFaultEnum[]
    { { ContextTag(1), "previous", ItemType::kList }, kInvalidNodeIndex }, // GeneralDiagnostics::NetworkFaultEnum[]
};

const Entry<ItemInfo> _GeneralDiagnostics_BootReason[] = {
    { { ContextTag(0), "bootReason", ItemType::kEnum }, kInvalidNodeIndex }, // GeneralDiagnostics::BootReasonEnum
};

const Entry<ItemInfo> _GeneralDiagnostics_NetworkInterface_list_[] = {
    { { AnonymousTag(), "[]", ItemType::kDefault }, 146 }, // GeneralDiagnostics_NetworkInterface[]
};

const Entry<ItemInfo> _SoftwareDiagnostics[] = {
    { { AttributeTag(0), "threadMetrics", ItemType::kList }, 156 },                    // SoftwareDiagnostics::ThreadMetricsStruct[]
    { { AttributeTag(1), "currentHeapFree", ItemType::kDefault }, kInvalidNodeIndex }, // SoftwareDiagnostics::int64u
    { { AttributeTag(2), "currentHeapUsed", ItemType::kDefault }, kInvalidNodeIndex }, // SoftwareDiagnostics::int64u
    { { AttributeTag(3), "currentHeapHighWatermark", ItemType::kDefault }, kInvalidNodeIndex }, // SoftwareDiagnostics::int64u
    { { AttributeTag(65528), "generatedCommandList", ItemType::kList }, kInvalidNodeIndex },    // SoftwareDiagnostics::command_id[]
    { { AttributeTag(65529), "acceptedCommandList", ItemType::kList }, kInvalidNodeIndex },     // SoftwareDiagnostics::command_id[]
    { { AttributeTag(65530), "eventList", ItemType::kList }, kInvalidNodeIndex },               // SoftwareDiagnostics::event_id[]
    { { AttributeTag(65531), "attributeList", ItemType::kList }, kInvalidNodeIndex },           // SoftwareDiagnostics::attrib_id[]
    { { AttributeTag(65532), "featureMap", ItemType::kDefault }, kInvalidNodeIndex },           // SoftwareDiagnostics::bitmap32
    { { AttributeTag(65533), "clusterRevision", ItemType::kDefault }, kInvalidNodeIndex },      // SoftwareDiagnostics::int16u
    { { EventTag(0), "SoftwareFault", ItemType::kDefault }, 155 }, // SoftwareDiagnostics::SoftwareFault
};

const Entry<ItemInfo> _SoftwareDiagnostics_ThreadMetricsStruct[] = {
    { { ContextTag(0), "id", ItemType::kDefault }, kInvalidNodeIndex },               // SoftwareDiagnostics::int64u
    { { ContextTag(1), "name", ItemType::kDefault }, kInvalidNodeIndex },             // SoftwareDiagnostics::char_string
    { { ContextTag(2), "stackFreeCurrent", ItemType::kDefault }, kInvalidNodeIndex }, // SoftwareDiagnostics::int32u
    { { ContextTag(3), "stackFreeMinimum", ItemType::kDefault }, kInvalidNodeIndex }, // SoftwareDiagnostics::int32u
    { { ContextTag(4), "stackSize", ItemType::kDefault }, kInvalidNodeIndex },        // SoftwareDiagnostics::int32u
};

const Entry<ItemInfo> _SoftwareDiagnostics_SoftwareFault[] = {
    { { ContextTag(0), "id", ItemType::kDefault }, kInvalidNodeIndex },             // SoftwareDiagnostics::INT64U
    { { ContextTag(1), "name", ItemType::kDefault }, kInvalidNodeIndex },           // SoftwareDiagnostics::CHAR_STRING
    { { ContextTag(2), "faultRecording", ItemType::kDefault }, kInvalidNodeIndex }, // SoftwareDiagnostics::OCTET_STRING
};

const Entry<ItemInfo> _SoftwareDiagnostics_ThreadMetricsStruct_list_[] = {
    { { AnonymousTag(), "[]", ItemType::kDefault }, 154 }, // SoftwareDiagnostics_ThreadMetricsStruct[]
};

const Entry<ItemInfo> _ThreadNetworkDiagnostics[] = {
    { { AttributeTag(0), "channel", ItemType::kDefault }, kInvalidNodeIndex },         // ThreadNetworkDiagnostics::int16u
    { { AttributeTag(1), "routingRole", ItemType::kEnum }, kInvalidNodeIndex },        // ThreadNetworkDiagnostics::RoutingRoleEnum
    { { AttributeTag(2), "networkName", ItemType::kDefault }, kInvalidNodeIndex },     // ThreadNetworkDiagnostics::char_string
    { { AttributeTag(3), "panId", ItemType::kDefault }, kInvalidNodeIndex },           // ThreadNetworkDiagnostics::int16u
    { { AttributeTag(4), "extendedPanId", ItemType::kDefault }, kInvalidNodeIndex },   // ThreadNetworkDiagnostics::int64u
    { { AttributeTag(5), "meshLocalPrefix", ItemType::kDefault }, kInvalidNodeIndex }, // ThreadNetworkDiagnostics::octet_string
    { { AttributeTag(6), "overrunCount", ItemType::kDefault }, kInvalidNodeIndex },    // ThreadNetworkDiagnostics::int64u
    { { AttributeTag(7), "neighborTable", ItemType::kList }, 164 }, // ThreadNetworkDiagnostics::NeighborTableStruct[]
    { { AttributeTag(8), "routeTable", ItemType::kList }, 165 },    // ThreadNetworkDiagnostics::RouteTableStruct[]
    { { AttributeTag(9), "partitionId", ItemType::kDefault }, kInvalidNodeIndex },             // ThreadNetworkDiagnostics::int32u
    { { AttributeTag(10), "weighting", ItemType::kDefault }, kInvalidNodeIndex },              // ThreadNetworkDiagnostics::int8u
    { { AttributeTag(11), "dataVersion", ItemType::kDefault }, kInvalidNodeIndex },            // ThreadNetworkDiagnostics::int8u
    { { AttributeTag(12), "stableDataVersion", ItemType::kDefault }, kInvalidNodeIndex },      // ThreadNetworkDiagnostics::int8u
    { { AttributeTag(13), "leaderRouterId", ItemType::kDefault }, kInvalidNodeIndex },         // ThreadNetworkDiagnostics::int8u
    { { AttributeTag(14), "detachedRoleCount", ItemType::kDefault }, kInvalidNodeIndex },      // ThreadNetworkDiagnostics::int16u
    { { AttributeTag(15), "childRoleCount", ItemType::kDefault }, kInvalidNodeIndex },         // ThreadNetworkDiagnostics::int16u
    { { AttributeTag(16), "routerRoleCount", ItemType::kDefault }, kInvalidNodeIndex },        // ThreadNetworkDiagnostics::int16u
    { { AttributeTag(17), "leaderRoleCount", ItemType::kDefault }, kInvalidNodeIndex },        // ThreadNetworkDiagnostics::int16u
    { { AttributeTag(18), "attachAttemptCount", ItemType::kDefault }, kInvalidNodeIndex },     // ThreadNetworkDiagnostics::int16u
    { { AttributeTag(19), "partitionIdChangeCount", ItemType::kDefault }, kInvalidNodeIndex }, // ThreadNetworkDiagnostics::int16u
    { { AttributeTag(20), "betterPartitionAttachAttemptCount", ItemType::kDefault },
      kInvalidNodeIndex },                                                                    // ThreadNetworkDiagnostics::int16u
    { { AttributeTag(21), "parentChangeCount", ItemType::kDefault }, kInvalidNodeIndex },     // ThreadNetworkDiagnostics::int16u
    { { AttributeTag(22), "txTotalCount", ItemType::kDefault }, kInvalidNodeIndex },          // ThreadNetworkDiagnostics::int32u
    { { AttributeTag(23), "txUnicastCount", ItemType::kDefault }, kInvalidNodeIndex },        // ThreadNetworkDiagnostics::int32u
    { { AttributeTag(24), "txBroadcastCount", ItemType::kDefault }, kInvalidNodeIndex },      // ThreadNetworkDiagnostics::int32u
    { { AttributeTag(25), "txAckRequestedCount", ItemType::kDefault }, kInvalidNodeIndex },   // ThreadNetworkDiagnostics::int32u
    { { AttributeTag(26), "txAckedCount", ItemType::kDefault }, kInvalidNodeIndex },          // ThreadNetworkDiagnostics::int32u
    { { AttributeTag(27), "txNoAckRequestedCount", ItemType::kDefault }, kInvalidNodeIndex }, // ThreadNetworkDiagnostics::int32u
    { { AttributeTag(28), "txDataCount", ItemType::kDefault }, kInvalidNodeIndex },           // ThreadNetworkDiagnostics::int32u
    { { AttributeTag(29), "txDataPollCount", ItemType::kDefault }, kInvalidNodeIndex },       // ThreadNetworkDiagnostics::int32u
    { { AttributeTag(30), "txBeaconCount", ItemType::kDefault }, kInvalidNodeIndex },         // ThreadNetworkDiagnostics::int32u
    { { AttributeTag(31), "txBeaconRequestCount", ItemType::kDefault }, kInvalidNodeIndex },  // ThreadNetworkDiagnostics::int32u
    { { AttributeTag(32), "txOtherCount", ItemType::kDefault }, kInvalidNodeIndex },          // ThreadNetworkDiagnostics::int32u
    { { AttributeTag(33), "txRetryCount", ItemType::kDefault }, kInvalidNodeIndex },          // ThreadNetworkDiagnostics::int32u
    { { AttributeTag(34), "txDirectMaxRetryExpiryCount", ItemType::kDefault },
      kInvalidNodeIndex }, // ThreadNetworkDiagnostics::int32u
    { { AttributeTag(35), "txIndirectMaxRetryExpiryCount", ItemType::kDefault },
      kInvalidNodeIndex },                                                                      // ThreadNetworkDiagnostics::int32u
    { { AttributeTag(36), "txErrCcaCount", ItemType::kDefault }, kInvalidNodeIndex },           // ThreadNetworkDiagnostics::int32u
    { { AttributeTag(37), "txErrAbortCount", ItemType::kDefault }, kInvalidNodeIndex },         // ThreadNetworkDiagnostics::int32u
    { { AttributeTag(38), "txErrBusyChannelCount", ItemType::kDefault }, kInvalidNodeIndex },   // ThreadNetworkDiagnostics::int32u
    { { AttributeTag(39), "rxTotalCount", ItemType::kDefault }, kInvalidNodeIndex },            // ThreadNetworkDiagnostics::int32u
    { { AttributeTag(40), "rxUnicastCount", ItemType::kDefault }, kInvalidNodeIndex },          // ThreadNetworkDiagnostics::int32u
    { { AttributeTag(41), "rxBroadcastCount", ItemType::kDefault }, kInvalidNodeIndex },        // ThreadNetworkDiagnostics::int32u
    { { AttributeTag(42), "rxDataCount", ItemType::kDefault }, kInvalidNodeIndex },             // ThreadNetworkDiagnostics::int32u
    { { AttributeTag(43), "rxDataPollCount", ItemType::kDefault }, kInvalidNodeIndex },         // ThreadNetworkDiagnostics::int32u
    { { AttributeTag(44), "rxBeaconCount", ItemType::kDefault }, kInvalidNodeIndex },           // ThreadNetworkDiagnostics::int32u
    { { AttributeTag(45), "rxBeaconRequestCount", ItemType::kDefault }, kInvalidNodeIndex },    // ThreadNetworkDiagnostics::int32u
    { { AttributeTag(46), "rxOtherCount", ItemType::kDefault }, kInvalidNodeIndex },            // ThreadNetworkDiagnostics::int32u
    { { AttributeTag(47), "rxAddressFilteredCount", ItemType::kDefault }, kInvalidNodeIndex },  // ThreadNetworkDiagnostics::int32u
    { { AttributeTag(48), "rxDestAddrFilteredCount", ItemType::kDefault }, kInvalidNodeIndex }, // ThreadNetworkDiagnostics::int32u
    { { AttributeTag(49), "rxDuplicatedCount", ItemType::kDefault }, kInvalidNodeIndex },       // ThreadNetworkDiagnostics::int32u
    { { AttributeTag(50), "rxErrNoFrameCount", ItemType::kDefault }, kInvalidNodeIndex },       // ThreadNetworkDiagnostics::int32u
    { { AttributeTag(51), "rxErrUnknownNeighborCount", ItemType::kDefault },
      kInvalidNodeIndex },                                                                       // ThreadNetworkDiagnostics::int32u
    { { AttributeTag(52), "rxErrInvalidSrcAddrCount", ItemType::kDefault }, kInvalidNodeIndex }, // ThreadNetworkDiagnostics::int32u
    { { AttributeTag(53), "rxErrSecCount", ItemType::kDefault }, kInvalidNodeIndex },            // ThreadNetworkDiagnostics::int32u
    { { AttributeTag(54), "rxErrFcsCount", ItemType::kDefault }, kInvalidNodeIndex },            // ThreadNetworkDiagnostics::int32u
    { { AttributeTag(55), "rxErrOtherCount", ItemType::kDefault }, kInvalidNodeIndex },          // ThreadNetworkDiagnostics::int32u
    { { AttributeTag(56), "activeTimestamp", ItemType::kDefault }, kInvalidNodeIndex },          // ThreadNetworkDiagnostics::int64u
    { { AttributeTag(57), "pendingTimestamp", ItemType::kDefault }, kInvalidNodeIndex },         // ThreadNetworkDiagnostics::int64u
    { { AttributeTag(58), "delay", ItemType::kDefault }, kInvalidNodeIndex },                    // ThreadNetworkDiagnostics::int32u
    { { AttributeTag(59), "securityPolicy", ItemType::kDefault }, 161 },                 // ThreadNetworkDiagnostics::SecurityPolicy
    { { AttributeTag(60), "channelPage0Mask", ItemType::kDefault }, kInvalidNodeIndex }, // ThreadNetworkDiagnostics::octet_string
    { { AttributeTag(61), "operationalDatasetComponents", ItemType::kDefault },
      159 }, // ThreadNetworkDiagnostics::OperationalDatasetComponents
    { { AttributeTag(62), "activeNetworkFaultsList", ItemType::kList },
      kInvalidNodeIndex }, // ThreadNetworkDiagnostics::NetworkFaultEnum[]
    { { AttributeTag(65528), "generatedCommandList", ItemType::kList },
      kInvalidNodeIndex }, // ThreadNetworkDiagnostics::command_id[]
    { { AttributeTag(65529), "acceptedCommandList", ItemType::kList },
      kInvalidNodeIndex },                                                                 // ThreadNetworkDiagnostics::command_id[]
    { { AttributeTag(65530), "eventList", ItemType::kList }, kInvalidNodeIndex },          // ThreadNetworkDiagnostics::event_id[]
    { { AttributeTag(65531), "attributeList", ItemType::kList }, kInvalidNodeIndex },      // ThreadNetworkDiagnostics::attrib_id[]
    { { AttributeTag(65532), "featureMap", ItemType::kDefault }, kInvalidNodeIndex },      // ThreadNetworkDiagnostics::bitmap32
    { { AttributeTag(65533), "clusterRevision", ItemType::kDefault }, kInvalidNodeIndex }, // ThreadNetworkDiagnostics::int16u
    { { EventTag(0), "ConnectionStatus", ItemType::kDefault }, 162 },   // ThreadNetworkDiagnostics::ConnectionStatus
    { { EventTag(1), "NetworkFaultChange", ItemType::kDefault }, 163 }, // ThreadNetworkDiagnostics::NetworkFaultChange
};

const Entry<ItemInfo> _ThreadNetworkDiagnostics_NeighborTableStruct[] = {
    { { ContextTag(0), "extAddress", ItemType::kDefault }, kInvalidNodeIndex },        // ThreadNetworkDiagnostics::int64u
    { { ContextTag(1), "age", ItemType::kDefault }, kInvalidNodeIndex },               // ThreadNetworkDiagnostics::int32u
    { { ContextTag(2), "rloc16", ItemType::kDefault }, kInvalidNodeIndex },            // ThreadNetworkDiagnostics::int16u
    { { ContextTag(3), "linkFrameCounter", ItemType::kDefault }, kInvalidNodeIndex },  // ThreadNetworkDiagnostics::int32u
    { { ContextTag(4), "mleFrameCounter", ItemType::kDefault }, kInvalidNodeIndex },   // ThreadNetworkDiagnostics::int32u
    { { ContextTag(5), "lqi", ItemType::kDefault }, kInvalidNodeIndex },               // ThreadNetworkDiagnostics::int8u
    { { ContextTag(6), "averageRssi", ItemType::kDefault }, kInvalidNodeIndex },       // ThreadNetworkDiagnostics::int8s
    { { ContextTag(7), "lastRssi", ItemType::kDefault }, kInvalidNodeIndex },          // ThreadNetworkDiagnostics::int8s
    { { ContextTag(8), "frameErrorRate", ItemType::kDefault }, kInvalidNodeIndex },    // ThreadNetworkDiagnostics::int8u
    { { ContextTag(9), "messageErrorRate", ItemType::kDefault }, kInvalidNodeIndex },  // ThreadNetworkDiagnostics::int8u
    { { ContextTag(10), "rxOnWhenIdle", ItemType::kDefault }, kInvalidNodeIndex },     // ThreadNetworkDiagnostics::boolean
    { { ContextTag(11), "fullThreadDevice", ItemType::kDefault }, kInvalidNodeIndex }, // ThreadNetworkDiagnostics::boolean
    { { ContextTag(12), "fullNetworkData", ItemType::kDefault }, kInvalidNodeIndex },  // ThreadNetworkDiagnostics::boolean
    { { ContextTag(13), "isChild", ItemType::kDefault }, kInvalidNodeIndex },          // ThreadNetworkDiagnostics::boolean
};

const Entry<ItemInfo> _ThreadNetworkDiagnostics_OperationalDatasetComponents[] = {
    { { ContextTag(0), "activeTimestampPresent", ItemType::kDefault }, kInvalidNodeIndex },  // ThreadNetworkDiagnostics::boolean
    { { ContextTag(1), "pendingTimestampPresent", ItemType::kDefault }, kInvalidNodeIndex }, // ThreadNetworkDiagnostics::boolean
    { { ContextTag(2), "masterKeyPresent", ItemType::kDefault }, kInvalidNodeIndex },        // ThreadNetworkDiagnostics::boolean
    { { ContextTag(3), "networkNamePresent", ItemType::kDefault }, kInvalidNodeIndex },      // ThreadNetworkDiagnostics::boolean
    { { ContextTag(4), "extendedPanIdPresent", ItemType::kDefault }, kInvalidNodeIndex },    // ThreadNetworkDiagnostics::boolean
    { { ContextTag(5), "meshLocalPrefixPresent", ItemType::kDefault }, kInvalidNodeIndex },  // ThreadNetworkDiagnostics::boolean
    { { ContextTag(6), "delayPresent", ItemType::kDefault }, kInvalidNodeIndex },            // ThreadNetworkDiagnostics::boolean
    { { ContextTag(7), "panIdPresent", ItemType::kDefault }, kInvalidNodeIndex },            // ThreadNetworkDiagnostics::boolean
    { { ContextTag(8), "channelPresent", ItemType::kDefault }, kInvalidNodeIndex },          // ThreadNetworkDiagnostics::boolean
    { { ContextTag(9), "pskcPresent", ItemType::kDefault }, kInvalidNodeIndex },             // ThreadNetworkDiagnostics::boolean
    { { ContextTag(10), "securityPolicyPresent", ItemType::kDefault }, kInvalidNodeIndex },  // ThreadNetworkDiagnostics::boolean
    { { ContextTag(11), "channelMaskPresent", ItemType::kDefault }, kInvalidNodeIndex },     // ThreadNetworkDiagnostics::boolean
};

const Entry<ItemInfo> _ThreadNetworkDiagnostics_RouteTableStruct[] = {
    { { ContextTag(0), "extAddress", ItemType::kDefault }, kInvalidNodeIndex },      // ThreadNetworkDiagnostics::int64u
    { { ContextTag(1), "rloc16", ItemType::kDefault }, kInvalidNodeIndex },          // ThreadNetworkDiagnostics::int16u
    { { ContextTag(2), "routerId", ItemType::kDefault }, kInvalidNodeIndex },        // ThreadNetworkDiagnostics::int8u
    { { ContextTag(3), "nextHop", ItemType::kDefault }, kInvalidNodeIndex },         // ThreadNetworkDiagnostics::int8u
    { { ContextTag(4), "pathCost", ItemType::kDefault }, kInvalidNodeIndex },        // ThreadNetworkDiagnostics::int8u
    { { ContextTag(5), "LQIIn", ItemType::kDefault }, kInvalidNodeIndex },           // ThreadNetworkDiagnostics::int8u
    { { ContextTag(6), "LQIOut", ItemType::kDefault }, kInvalidNodeIndex },          // ThreadNetworkDiagnostics::int8u
    { { ContextTag(7), "age", ItemType::kDefault }, kInvalidNodeIndex },             // ThreadNetworkDiagnostics::int8u
    { { ContextTag(8), "allocated", ItemType::kDefault }, kInvalidNodeIndex },       // ThreadNetworkDiagnostics::boolean
    { { ContextTag(9), "linkEstablished", ItemType::kDefault }, kInvalidNodeIndex }, // ThreadNetworkDiagnostics::boolean
};

const Entry<ItemInfo> _ThreadNetworkDiagnostics_SecurityPolicy[] = {
    { { ContextTag(0), "rotationTime", ItemType::kDefault }, kInvalidNodeIndex }, // ThreadNetworkDiagnostics::int16u
    { { ContextTag(1), "flags", ItemType::kDefault }, kInvalidNodeIndex },        // ThreadNetworkDiagnostics::int16u
};

const Entry<ItemInfo> _ThreadNetworkDiagnostics_ConnectionStatus[] = {
    { { ContextTag(0), "connectionStatus", ItemType::kEnum }, kInvalidNodeIndex }, // ThreadNetworkDiagnostics::ConnectionStatusEnum
};

const Entry<ItemInfo> _ThreadNetworkDiagnostics_NetworkFaultChange[] = {
    { { ContextTag(0), "current", ItemType::kList }, kInvalidNodeIndex },  // ThreadNetworkDiagnostics::NetworkFaultEnum[]
    { { ContextTag(1), "previous", ItemType::kList }, kInvalidNodeIndex }, // ThreadNetworkDiagnostics::NetworkFaultEnum[]
};

const Entry<ItemInfo> _ThreadNetworkDiagnostics_NeighborTableStruct_list_[] = {
    { { AnonymousTag(), "[]", ItemType::kDefault }, 158 }, // ThreadNetworkDiagnostics_NeighborTableStruct[]
};

const Entry<ItemInfo> _ThreadNetworkDiagnostics_RouteTableStruct_list_[] = {
    { { AnonymousTag(), "[]", ItemType::kDefault }, 160 }, // ThreadNetworkDiagnostics_RouteTableStruct[]
};

const Entry<ItemInfo> _WiFiNetworkDiagnostics[] = {
    { { AttributeTag(0), "bssid", ItemType::kDefault }, kInvalidNodeIndex },           // WiFiNetworkDiagnostics::octet_string
    { { AttributeTag(1), "securityType", ItemType::kEnum }, kInvalidNodeIndex },       // WiFiNetworkDiagnostics::SecurityTypeEnum
    { { AttributeTag(2), "wiFiVersion", ItemType::kEnum }, kInvalidNodeIndex },        // WiFiNetworkDiagnostics::WiFiVersionEnum
    { { AttributeTag(3), "channelNumber", ItemType::kDefault }, kInvalidNodeIndex },   // WiFiNetworkDiagnostics::int16u
    { { AttributeTag(4), "rssi", ItemType::kDefault }, kInvalidNodeIndex },            // WiFiNetworkDiagnostics::int8s
    { { AttributeTag(5), "beaconLostCount", ItemType::kDefault }, kInvalidNodeIndex }, // WiFiNetworkDiagnostics::int32u
    { { AttributeTag(6), "beaconRxCount", ItemType::kDefault }, kInvalidNodeIndex },   // WiFiNetworkDiagnostics::int32u
    { { AttributeTag(7), "packetMulticastRxCount", ItemType::kDefault }, kInvalidNodeIndex }, // WiFiNetworkDiagnostics::int32u
    { { AttributeTag(8), "packetMulticastTxCount", ItemType::kDefault }, kInvalidNodeIndex }, // WiFiNetworkDiagnostics::int32u
    { { AttributeTag(9), "packetUnicastRxCount", ItemType::kDefault }, kInvalidNodeIndex },   // WiFiNetworkDiagnostics::int32u
    { { AttributeTag(10), "packetUnicastTxCount", ItemType::kDefault }, kInvalidNodeIndex },  // WiFiNetworkDiagnostics::int32u
    { { AttributeTag(11), "currentMaxRate", ItemType::kDefault }, kInvalidNodeIndex },        // WiFiNetworkDiagnostics::int64u
    { { AttributeTag(12), "overrunCount", ItemType::kDefault }, kInvalidNodeIndex },          // WiFiNetworkDiagnostics::int64u
    { { AttributeTag(65528), "generatedCommandList", ItemType::kList }, kInvalidNodeIndex }, // WiFiNetworkDiagnostics::command_id[]
    { { AttributeTag(65529), "acceptedCommandList", ItemType::kList }, kInvalidNodeIndex },  // WiFiNetworkDiagnostics::command_id[]
    { { AttributeTag(65530), "eventList", ItemType::kList }, kInvalidNodeIndex },            // WiFiNetworkDiagnostics::event_id[]
    { { AttributeTag(65531), "attributeList", ItemType::kList }, kInvalidNodeIndex },        // WiFiNetworkDiagnostics::attrib_id[]
    { { AttributeTag(65532), "featureMap", ItemType::kDefault }, kInvalidNodeIndex },        // WiFiNetworkDiagnostics::bitmap32
    { { AttributeTag(65533), "clusterRevision", ItemType::kDefault }, kInvalidNodeIndex },   // WiFiNetworkDiagnostics::int16u
    { { EventTag(0), "Disconnection", ItemType::kDefault }, 167 },      // WiFiNetworkDiagnostics::Disconnection
    { { EventTag(1), "AssociationFailure", ItemType::kDefault }, 168 }, // WiFiNetworkDiagnostics::AssociationFailure
    { { EventTag(2), "ConnectionStatus", ItemType::kDefault }, 169 },   // WiFiNetworkDiagnostics::ConnectionStatus
};

const Entry<ItemInfo> _WiFiNetworkDiagnostics_Disconnection[] = {
    { { ContextTag(0), "reasonCode", ItemType::kDefault }, kInvalidNodeIndex }, // WiFiNetworkDiagnostics::INT16U
};

const Entry<ItemInfo> _WiFiNetworkDiagnostics_AssociationFailure[] = {
    { { ContextTag(0), "associationFailure", ItemType::kEnum },
      kInvalidNodeIndex },                                                  // WiFiNetworkDiagnostics::AssociationFailureCauseEnum
    { { ContextTag(1), "status", ItemType::kDefault }, kInvalidNodeIndex }, // WiFiNetworkDiagnostics::INT16U
};

const Entry<ItemInfo> _WiFiNetworkDiagnostics_ConnectionStatus[] = {
    { { ContextTag(0), "connectionStatus", ItemType::kEnum }, kInvalidNodeIndex }, // WiFiNetworkDiagnostics::ConnectionStatusEnum
};

const Entry<ItemInfo> _EthernetNetworkDiagnostics[] = {
    { { AttributeTag(0), "PHYRate", ItemType::kEnum }, kInvalidNodeIndex },           // EthernetNetworkDiagnostics::PHYRateEnum
    { { AttributeTag(1), "fullDuplex", ItemType::kDefault }, kInvalidNodeIndex },     // EthernetNetworkDiagnostics::boolean
    { { AttributeTag(2), "packetRxCount", ItemType::kDefault }, kInvalidNodeIndex },  // EthernetNetworkDiagnostics::int64u
    { { AttributeTag(3), "packetTxCount", ItemType::kDefault }, kInvalidNodeIndex },  // EthernetNetworkDiagnostics::int64u
    { { AttributeTag(4), "txErrCount", ItemType::kDefault }, kInvalidNodeIndex },     // EthernetNetworkDiagnostics::int64u
    { { AttributeTag(5), "collisionCount", ItemType::kDefault }, kInvalidNodeIndex }, // EthernetNetworkDiagnostics::int64u
    { { AttributeTag(6), "overrunCount", ItemType::kDefault }, kInvalidNodeIndex },   // EthernetNetworkDiagnostics::int64u
    { { AttributeTag(7), "carrierDetect", ItemType::kDefault }, kInvalidNodeIndex },  // EthernetNetworkDiagnostics::boolean
    { { AttributeTag(8), "timeSinceReset", ItemType::kDefault }, kInvalidNodeIndex }, // EthernetNetworkDiagnostics::int64u
    { { AttributeTag(65528), "generatedCommandList", ItemType::kList },
      kInvalidNodeIndex }, // EthernetNetworkDiagnostics::command_id[]
    { { AttributeTag(65529), "acceptedCommandList", ItemType::kList },
      kInvalidNodeIndex },                                                            // EthernetNetworkDiagnostics::command_id[]
    { { AttributeTag(65530), "eventList", ItemType::kList }, kInvalidNodeIndex },     // EthernetNetworkDiagnostics::event_id[]
    { { AttributeTag(65531), "attributeList", ItemType::kList }, kInvalidNodeIndex }, // EthernetNetworkDiagnostics::attrib_id[]
    { { AttributeTag(65532), "featureMap", ItemType::kDefault }, kInvalidNodeIndex }, // EthernetNetworkDiagnostics::bitmap32
    { { AttributeTag(65533), "clusterRevision", ItemType::kDefault }, kInvalidNodeIndex }, // EthernetNetworkDiagnostics::int16u
};

const Entry<ItemInfo> _TimeSynchronization[] = {
    { { AttributeTag(0), "UTCTime", ItemType::kDefault }, kInvalidNodeIndex },       // TimeSynchronization::epoch_us
    { { AttributeTag(1), "granularity", ItemType::kEnum }, kInvalidNodeIndex },      // TimeSynchronization::GranularityEnum
    { { AttributeTag(2), "timeSource", ItemType::kEnum }, kInvalidNodeIndex },       // TimeSynchronization::TimeSourceEnum
    { { AttributeTag(3), "trustedTimeSource", ItemType::kDefault }, 175 },           // TimeSynchronization::TrustedTimeSourceStruct
    { { AttributeTag(4), "defaultNTP", ItemType::kDefault }, kInvalidNodeIndex },    // TimeSynchronization::char_string
    { { AttributeTag(5), "timeZone", ItemType::kList }, 188 },                       // TimeSynchronization::TimeZoneStruct[]
    { { AttributeTag(6), "DSTOffset", ItemType::kList }, 187 },                      // TimeSynchronization::DSTOffsetStruct[]
    { { AttributeTag(7), "localTime", ItemType::kDefault }, kInvalidNodeIndex },     // TimeSynchronization::epoch_us
    { { AttributeTag(8), "timeZoneDatabase", ItemType::kEnum }, kInvalidNodeIndex }, // TimeSynchronization::TimeZoneDatabaseEnum
    { { AttributeTag(9), "NTPServerAvailable", ItemType::kDefault }, kInvalidNodeIndex },    // TimeSynchronization::boolean
    { { AttributeTag(10), "timeZoneListMaxSize", ItemType::kDefault }, kInvalidNodeIndex },  // TimeSynchronization::int8u
    { { AttributeTag(11), "DSTOffsetListMaxSize", ItemType::kDefault }, kInvalidNodeIndex }, // TimeSynchronization::int8u
    { { AttributeTag(12), "supportsDNSResolve", ItemType::kDefault }, kInvalidNodeIndex },   // TimeSynchronization::boolean
    { { AttributeTag(65528), "generatedCommandList", ItemType::kList }, kInvalidNodeIndex }, // TimeSynchronization::command_id[]
    { { AttributeTag(65529), "acceptedCommandList", ItemType::kList }, kInvalidNodeIndex },  // TimeSynchronization::command_id[]
    { { AttributeTag(65530), "eventList", ItemType::kList }, kInvalidNodeIndex },            // TimeSynchronization::event_id[]
    { { AttributeTag(65531), "attributeList", ItemType::kList }, kInvalidNodeIndex },        // TimeSynchronization::attrib_id[]
    { { AttributeTag(65532), "featureMap", ItemType::kDefault }, kInvalidNodeIndex },        // TimeSynchronization::bitmap32
    { { AttributeTag(65533), "clusterRevision", ItemType::kDefault }, kInvalidNodeIndex },   // TimeSynchronization::int16u
    { { EventTag(0), "DSTTableEmpty", ItemType::kDefault }, 182 },                           // TimeSynchronization::DSTTableEmpty
    { { EventTag(1), "DSTStatus", ItemType::kDefault }, 183 },                               // TimeSynchronization::DSTStatus
    { { EventTag(2), "TimeZoneStatus", ItemType::kDefault }, 184 },                          // TimeSynchronization::TimeZoneStatus
    { { EventTag(3), "TimeFailure", ItemType::kDefault }, 185 },                             // TimeSynchronization::TimeFailure
    { { EventTag(4), "MissingTrustedTimeSource", ItemType::kDefault }, 186 }, // TimeSynchronization::MissingTrustedTimeSource
    { { CommandTag(0), "SetUTCTimeRequest", ItemType::kDefault }, 176 },      // TimeSynchronization::SetUTCTime::SetUTCTimeRequest
    { { CommandTag(1), "SetTrustedTimeSourceRequest", ItemType::kDefault },
      177 }, // TimeSynchronization::SetTrustedTimeSource::SetTrustedTimeSourceRequest
    { { CommandTag(2), "SetTimeZoneRequest", ItemType::kDefault }, 178 },  // TimeSynchronization::SetTimeZone::SetTimeZoneRequest
    { { CommandTag(4), "SetDSTOffsetRequest", ItemType::kDefault }, 180 }, // TimeSynchronization::SetDSTOffset::SetDSTOffsetRequest
    { { CommandTag(5), "SetDefaultNTPRequest", ItemType::kDefault },
      181 }, // TimeSynchronization::SetDefaultNTP::SetDefaultNTPRequest
    { { CommandTag(3), "SetTimeZoneResponse", ItemType::kDefault }, 179 }, // TimeSynchronization::SetTimeZoneResponse
};

const Entry<ItemInfo> _TimeSynchronization_DSTOffsetStruct[] = {
    { { ContextTag(0), "offset", ItemType::kDefault }, kInvalidNodeIndex },        // TimeSynchronization::int32s
    { { ContextTag(1), "validStarting", ItemType::kDefault }, kInvalidNodeIndex }, // TimeSynchronization::epoch_us
    { { ContextTag(2), "validUntil", ItemType::kDefault }, kInvalidNodeIndex },    // TimeSynchronization::epoch_us
};

const Entry<ItemInfo> _TimeSynchronization_FabricScopedTrustedTimeSourceStruct[] = {
    { { ContextTag(0), "nodeID", ItemType::kDefault }, kInvalidNodeIndex },   // TimeSynchronization::node_id
    { { ContextTag(1), "endpoint", ItemType::kDefault }, kInvalidNodeIndex }, // TimeSynchronization::endpoint_no
};

const Entry<ItemInfo> _TimeSynchronization_TimeZoneStruct[] = {
    { { ContextTag(0), "offset", ItemType::kDefault }, kInvalidNodeIndex },  // TimeSynchronization::int32s
    { { ContextTag(1), "validAt", ItemType::kDefault }, kInvalidNodeIndex }, // TimeSynchronization::epoch_us
    { { ContextTag(2), "name", ItemType::kDefault }, kInvalidNodeIndex },    // TimeSynchronization::char_string
};

const Entry<ItemInfo> _TimeSynchronization_TrustedTimeSourceStruct[] = {
    { { ContextTag(0), "fabricIndex", ItemType::kDefault }, kInvalidNodeIndex }, // TimeSynchronization::fabric_idx
    { { ContextTag(1), "nodeID", ItemType::kDefault }, kInvalidNodeIndex },      // TimeSynchronization::node_id
    { { ContextTag(2), "endpoint", ItemType::kDefault }, kInvalidNodeIndex },    // TimeSynchronization::endpoint_no
};

const Entry<ItemInfo> _TimeSynchronization_SetUTCTimeRequest[] = {
    { { ContextTag(0), "UTCTime", ItemType::kDefault }, kInvalidNodeIndex },  // TimeSynchronization::epoch_us
    { { ContextTag(1), "granularity", ItemType::kEnum }, kInvalidNodeIndex }, // TimeSynchronization::GranularityEnum
    { { ContextTag(2), "timeSource", ItemType::kEnum }, kInvalidNodeIndex },  // TimeSynchronization::TimeSourceEnum
};

const Entry<ItemInfo> _TimeSynchronization_SetTrustedTimeSourceRequest[] = {
    { { ContextTag(0), "trustedTimeSource", ItemType::kDefault }, 173 }, // TimeSynchronization::FabricScopedTrustedTimeSourceStruct
};

const Entry<ItemInfo> _TimeSynchronization_SetTimeZoneRequest[] = {
    { { ContextTag(0), "timeZone", ItemType::kList }, 188 }, // TimeSynchronization::TimeZoneStruct[]
};

const Entry<ItemInfo> _TimeSynchronization_SetTimeZoneResponse[] = {
    { { ContextTag(0), "DSTOffsetRequired", ItemType::kDefault }, kInvalidNodeIndex }, // TimeSynchronization::boolean
};

const Entry<ItemInfo> _TimeSynchronization_SetDSTOffsetRequest[] = {
    { { ContextTag(0), "DSTOffset", ItemType::kList }, 187 }, // TimeSynchronization::DSTOffsetStruct[]
};

const Entry<ItemInfo> _TimeSynchronization_SetDefaultNTPRequest[] = {
    { { ContextTag(0), "defaultNTP", ItemType::kDefault }, kInvalidNodeIndex }, // TimeSynchronization::CHAR_STRING
};

const Entry<ItemInfo> _TimeSynchronization_DSTTableEmpty[] = {};

const Entry<ItemInfo> _TimeSynchronization_DSTStatus[] = {
    { { ContextTag(0), "DSTOffsetActive", ItemType::kDefault }, kInvalidNodeIndex }, // TimeSynchronization::boolean
};

const Entry<ItemInfo> _TimeSynchronization_TimeZoneStatus[] = {
    { { ContextTag(0), "offset", ItemType::kDefault }, kInvalidNodeIndex }, // TimeSynchronization::INT32S
    { { ContextTag(1), "name", ItemType::kDefault }, kInvalidNodeIndex },   // TimeSynchronization::CHAR_STRING
};

const Entry<ItemInfo> _TimeSynchronization_TimeFailure[] = {};

const Entry<ItemInfo> _TimeSynchronization_MissingTrustedTimeSource[] = {};

const Entry<ItemInfo> _TimeSynchronization_DSTOffsetStruct_list_[] = {
    { { AnonymousTag(), "[]", ItemType::kDefault }, 172 }, // TimeSynchronization_DSTOffsetStruct[]
};

const Entry<ItemInfo> _TimeSynchronization_TimeZoneStruct_list_[] = {
    { { AnonymousTag(), "[]", ItemType::kDefault }, 174 }, // TimeSynchronization_TimeZoneStruct[]
};

const Entry<ItemInfo> _BridgedDeviceBasicInformation[] = {
    { { AttributeTag(1), "vendorName", ItemType::kDefault }, kInvalidNodeIndex },      // BridgedDeviceBasicInformation::char_string
    { { AttributeTag(2), "vendorID", ItemType::kDefault }, kInvalidNodeIndex },        // BridgedDeviceBasicInformation::vendor_id
    { { AttributeTag(3), "productName", ItemType::kDefault }, kInvalidNodeIndex },     // BridgedDeviceBasicInformation::char_string
    { { AttributeTag(5), "nodeLabel", ItemType::kDefault }, kInvalidNodeIndex },       // BridgedDeviceBasicInformation::char_string
    { { AttributeTag(7), "hardwareVersion", ItemType::kDefault }, kInvalidNodeIndex }, // BridgedDeviceBasicInformation::int16u
    { { AttributeTag(8), "hardwareVersionString", ItemType::kDefault },
      kInvalidNodeIndex },                                                             // BridgedDeviceBasicInformation::char_string
    { { AttributeTag(9), "softwareVersion", ItemType::kDefault }, kInvalidNodeIndex }, // BridgedDeviceBasicInformation::int32u
    { { AttributeTag(10), "softwareVersionString", ItemType::kDefault },
      kInvalidNodeIndex }, // BridgedDeviceBasicInformation::char_string
    { { AttributeTag(11), "manufacturingDate", ItemType::kDefault },
      kInvalidNodeIndex },                                                         // BridgedDeviceBasicInformation::char_string
    { { AttributeTag(12), "partNumber", ItemType::kDefault }, kInvalidNodeIndex }, // BridgedDeviceBasicInformation::char_string
    { { AttributeTag(13), "productURL", ItemType::kDefault },
      kInvalidNodeIndex }, // BridgedDeviceBasicInformation::long_char_string
    { { AttributeTag(14), "productLabel", ItemType::kDefault }, kInvalidNodeIndex }, // BridgedDeviceBasicInformation::char_string
    { { AttributeTag(15), "serialNumber", ItemType::kDefault }, kInvalidNodeIndex }, // BridgedDeviceBasicInformation::char_string
    { { AttributeTag(17), "reachable", ItemType::kDefault }, kInvalidNodeIndex },    // BridgedDeviceBasicInformation::boolean
    { { AttributeTag(18), "uniqueID", ItemType::kDefault }, kInvalidNodeIndex },     // BridgedDeviceBasicInformation::char_string
    { { AttributeTag(20), "productAppearance", ItemType::kDefault },
      190 }, // BridgedDeviceBasicInformation::ProductAppearanceStruct
    { { AttributeTag(65528), "generatedCommandList", ItemType::kList },
      kInvalidNodeIndex }, // BridgedDeviceBasicInformation::command_id[]
    { { AttributeTag(65529), "acceptedCommandList", ItemType::kList },
      kInvalidNodeIndex },                                                            // BridgedDeviceBasicInformation::command_id[]
    { { AttributeTag(65530), "eventList", ItemType::kList }, kInvalidNodeIndex },     // BridgedDeviceBasicInformation::event_id[]
    { { AttributeTag(65531), "attributeList", ItemType::kList }, kInvalidNodeIndex }, // BridgedDeviceBasicInformation::attrib_id[]
    { { AttributeTag(65532), "featureMap", ItemType::kDefault }, kInvalidNodeIndex }, // BridgedDeviceBasicInformation::bitmap32
    { { AttributeTag(65533), "clusterRevision", ItemType::kDefault }, kInvalidNodeIndex }, // BridgedDeviceBasicInformation::int16u
    { { EventTag(0), "StartUp", ItemType::kDefault }, 191 },                               // BridgedDeviceBasicInformation::StartUp
    { { EventTag(1), "ShutDown", ItemType::kDefault }, 192 },         // BridgedDeviceBasicInformation::ShutDown
    { { EventTag(2), "Leave", ItemType::kDefault }, 193 },            // BridgedDeviceBasicInformation::Leave
    { { EventTag(3), "ReachableChanged", ItemType::kDefault }, 194 }, // BridgedDeviceBasicInformation::ReachableChanged
};

const Entry<ItemInfo> _BridgedDeviceBasicInformation_ProductAppearanceStruct[] = {
    { { ContextTag(0), "finish", ItemType::kEnum }, kInvalidNodeIndex },       // BridgedDeviceBasicInformation::ProductFinishEnum
    { { ContextTag(1), "primaryColor", ItemType::kEnum }, kInvalidNodeIndex }, // BridgedDeviceBasicInformation::ColorEnum
};

const Entry<ItemInfo> _BridgedDeviceBasicInformation_StartUp[] = {
    { { ContextTag(0), "softwareVersion", ItemType::kDefault }, kInvalidNodeIndex }, // BridgedDeviceBasicInformation::INT32U
};

const Entry<ItemInfo> _BridgedDeviceBasicInformation_ShutDown[] = {};

const Entry<ItemInfo> _BridgedDeviceBasicInformation_Leave[] = {};

const Entry<ItemInfo> _BridgedDeviceBasicInformation_ReachableChanged[] = {
    { { ContextTag(0), "reachableNewValue", ItemType::kDefault }, kInvalidNodeIndex }, // BridgedDeviceBasicInformation::boolean
};

const Entry<ItemInfo> _Switch[] = {
    { { AttributeTag(0), "numberOfPositions", ItemType::kDefault }, kInvalidNodeIndex },     // Switch::int8u
    { { AttributeTag(1), "currentPosition", ItemType::kDefault }, kInvalidNodeIndex },       // Switch::int8u
    { { AttributeTag(2), "multiPressMax", ItemType::kDefault }, kInvalidNodeIndex },         // Switch::int8u
    { { AttributeTag(65528), "generatedCommandList", ItemType::kList }, kInvalidNodeIndex }, // Switch::command_id[]
    { { AttributeTag(65529), "acceptedCommandList", ItemType::kList }, kInvalidNodeIndex },  // Switch::command_id[]
    { { AttributeTag(65530), "eventList", ItemType::kList }, kInvalidNodeIndex },            // Switch::event_id[]
    { { AttributeTag(65531), "attributeList", ItemType::kList }, kInvalidNodeIndex },        // Switch::attrib_id[]
    { { AttributeTag(65532), "featureMap", ItemType::kDefault }, kInvalidNodeIndex },        // Switch::bitmap32
    { { AttributeTag(65533), "clusterRevision", ItemType::kDefault }, kInvalidNodeIndex },   // Switch::int16u
    { { EventTag(0), "SwitchLatched", ItemType::kDefault }, 196 },                           // Switch::SwitchLatched
    { { EventTag(1), "InitialPress", ItemType::kDefault }, 197 },                            // Switch::InitialPress
    { { EventTag(2), "LongPress", ItemType::kDefault }, 198 },                               // Switch::LongPress
    { { EventTag(3), "ShortRelease", ItemType::kDefault }, 199 },                            // Switch::ShortRelease
    { { EventTag(4), "LongRelease", ItemType::kDefault }, 200 },                             // Switch::LongRelease
    { { EventTag(5), "MultiPressOngoing", ItemType::kDefault }, 201 },                       // Switch::MultiPressOngoing
    { { EventTag(6), "MultiPressComplete", ItemType::kDefault }, 202 },                      // Switch::MultiPressComplete
};

const Entry<ItemInfo> _Switch_SwitchLatched[] = {
    { { ContextTag(0), "newPosition", ItemType::kDefault }, kInvalidNodeIndex }, // Switch::INT8U
};

const Entry<ItemInfo> _Switch_InitialPress[] = {
    { { ContextTag(0), "newPosition", ItemType::kDefault }, kInvalidNodeIndex }, // Switch::INT8U
};

const Entry<ItemInfo> _Switch_LongPress[] = {
    { { ContextTag(0), "newPosition", ItemType::kDefault }, kInvalidNodeIndex }, // Switch::INT8U
};

const Entry<ItemInfo> _Switch_ShortRelease[] = {
    { { ContextTag(0), "previousPosition", ItemType::kDefault }, kInvalidNodeIndex }, // Switch::INT8U
};

const Entry<ItemInfo> _Switch_LongRelease[] = {
    { { ContextTag(0), "previousPosition", ItemType::kDefault }, kInvalidNodeIndex }, // Switch::INT8U
};

const Entry<ItemInfo> _Switch_MultiPressOngoing[] = {
    { { ContextTag(0), "newPosition", ItemType::kDefault }, kInvalidNodeIndex },                   // Switch::INT8U
    { { ContextTag(1), "currentNumberOfPressesCounted", ItemType::kDefault }, kInvalidNodeIndex }, // Switch::INT8U
};

const Entry<ItemInfo> _Switch_MultiPressComplete[] = {
    { { ContextTag(0), "previousPosition", ItemType::kDefault }, kInvalidNodeIndex },            // Switch::INT8U
    { { ContextTag(1), "totalNumberOfPressesCounted", ItemType::kDefault }, kInvalidNodeIndex }, // Switch::INT8U
};

const Entry<ItemInfo> _AdministratorCommissioning[] = {
    { { AttributeTag(0), "windowStatus", ItemType::kEnum },
      kInvalidNodeIndex }, // AdministratorCommissioning::CommissioningWindowStatusEnum
    { { AttributeTag(1), "adminFabricIndex", ItemType::kDefault }, kInvalidNodeIndex }, // AdministratorCommissioning::fabric_idx
    { { AttributeTag(2), "adminVendorId", ItemType::kDefault }, kInvalidNodeIndex },    // AdministratorCommissioning::int16u
    { { AttributeTag(65528), "generatedCommandList", ItemType::kList },
      kInvalidNodeIndex }, // AdministratorCommissioning::command_id[]
    { { AttributeTag(65529), "acceptedCommandList", ItemType::kList },
      kInvalidNodeIndex },                                                            // AdministratorCommissioning::command_id[]
    { { AttributeTag(65530), "eventList", ItemType::kList }, kInvalidNodeIndex },     // AdministratorCommissioning::event_id[]
    { { AttributeTag(65531), "attributeList", ItemType::kList }, kInvalidNodeIndex }, // AdministratorCommissioning::attrib_id[]
    { { AttributeTag(65532), "featureMap", ItemType::kDefault }, kInvalidNodeIndex }, // AdministratorCommissioning::bitmap32
    { { AttributeTag(65533), "clusterRevision", ItemType::kDefault }, kInvalidNodeIndex }, // AdministratorCommissioning::int16u
    { { CommandTag(0), "OpenCommissioningWindowRequest", ItemType::kDefault },
      204 }, // AdministratorCommissioning::OpenCommissioningWindow::OpenCommissioningWindowRequest
    { { CommandTag(1), "OpenBasicCommissioningWindowRequest", ItemType::kDefault },
      205 }, // AdministratorCommissioning::OpenBasicCommissioningWindow::OpenBasicCommissioningWindowRequest
};

const Entry<ItemInfo> _AdministratorCommissioning_OpenCommissioningWindowRequest[] = {
    { { ContextTag(0), "commissioningTimeout", ItemType::kDefault }, kInvalidNodeIndex }, // AdministratorCommissioning::INT16U
    { { ContextTag(1), "PAKEPasscodeVerifier", ItemType::kDefault },
      kInvalidNodeIndex },                                                         // AdministratorCommissioning::OCTET_STRING
    { { ContextTag(2), "discriminator", ItemType::kDefault }, kInvalidNodeIndex }, // AdministratorCommissioning::INT16U
    { { ContextTag(3), "iterations", ItemType::kDefault }, kInvalidNodeIndex },    // AdministratorCommissioning::INT32U
    { { ContextTag(4), "salt", ItemType::kDefault }, kInvalidNodeIndex },          // AdministratorCommissioning::OCTET_STRING
};

const Entry<ItemInfo> _AdministratorCommissioning_OpenBasicCommissioningWindowRequest[] = {
    { { ContextTag(0), "commissioningTimeout", ItemType::kDefault }, kInvalidNodeIndex }, // AdministratorCommissioning::INT16U
};

const Entry<ItemInfo> _OperationalCredentials[] = {
    { { AttributeTag(0), "NOCs", ItemType::kList }, 222 },    // OperationalCredentials::NOCStruct[]
    { { AttributeTag(1), "fabrics", ItemType::kList }, 221 }, // OperationalCredentials::FabricDescriptorStruct[]
    { { AttributeTag(2), "supportedFabrics", ItemType::kDefault }, kInvalidNodeIndex },    // OperationalCredentials::int8u
    { { AttributeTag(3), "commissionedFabrics", ItemType::kDefault }, kInvalidNodeIndex }, // OperationalCredentials::int8u
    { { AttributeTag(4), "trustedRootCertificates", ItemType::kList },
      kInvalidNodeIndex },                                                                // OperationalCredentials::OCTET_STRING[]
    { { AttributeTag(5), "currentFabricIndex", ItemType::kDefault }, kInvalidNodeIndex }, // OperationalCredentials::int8u
    { { AttributeTag(65528), "generatedCommandList", ItemType::kList }, kInvalidNodeIndex }, // OperationalCredentials::command_id[]
    { { AttributeTag(65529), "acceptedCommandList", ItemType::kList }, kInvalidNodeIndex },  // OperationalCredentials::command_id[]
    { { AttributeTag(65530), "eventList", ItemType::kList }, kInvalidNodeIndex },            // OperationalCredentials::event_id[]
    { { AttributeTag(65531), "attributeList", ItemType::kList }, kInvalidNodeIndex },        // OperationalCredentials::attrib_id[]
    { { AttributeTag(65532), "featureMap", ItemType::kDefault }, kInvalidNodeIndex },        // OperationalCredentials::bitmap32
    { { AttributeTag(65533), "clusterRevision", ItemType::kDefault }, kInvalidNodeIndex },   // OperationalCredentials::int16u
    { { CommandTag(0), "AttestationRequestRequest", ItemType::kDefault },
      209 }, // OperationalCredentials::AttestationRequest::AttestationRequestRequest
    { { CommandTag(2), "CertificateChainRequestRequest", ItemType::kDefault },
      211 }, // OperationalCredentials::CertificateChainRequest::CertificateChainRequestRequest
    { { CommandTag(4), "CSRRequestRequest", ItemType::kDefault }, 213 }, // OperationalCredentials::CSRRequest::CSRRequestRequest
    { { CommandTag(6), "AddNOCRequest", ItemType::kDefault }, 215 },     // OperationalCredentials::AddNOC::AddNOCRequest
    { { CommandTag(7), "UpdateNOCRequest", ItemType::kDefault }, 216 },  // OperationalCredentials::UpdateNOC::UpdateNOCRequest
    { { CommandTag(9), "UpdateFabricLabelRequest", ItemType::kDefault },
      218 }, // OperationalCredentials::UpdateFabricLabel::UpdateFabricLabelRequest
    { { CommandTag(10), "RemoveFabricRequest", ItemType::kDefault },
      219 }, // OperationalCredentials::RemoveFabric::RemoveFabricRequest
    { { CommandTag(11), "AddTrustedRootCertificateRequest", ItemType::kDefault },
      220 }, // OperationalCredentials::AddTrustedRootCertificate::AddTrustedRootCertificateRequest
    { { CommandTag(1), "AttestationResponse", ItemType::kDefault }, 210 },      // OperationalCredentials::AttestationResponse
    { { CommandTag(3), "CertificateChainResponse", ItemType::kDefault }, 212 }, // OperationalCredentials::CertificateChainResponse
    { { CommandTag(5), "CSRResponse", ItemType::kDefault }, 214 },              // OperationalCredentials::CSRResponse
    { { CommandTag(8), "NOCResponse", ItemType::kDefault }, 217 },              // OperationalCredentials::NOCResponse
};

const Entry<ItemInfo> _OperationalCredentials_FabricDescriptorStruct[] = {
    { { ContextTag(1), "rootPublicKey", ItemType::kDefault }, kInvalidNodeIndex }, // OperationalCredentials::octet_string
    { { ContextTag(2), "vendorID", ItemType::kDefault }, kInvalidNodeIndex },      // OperationalCredentials::vendor_id
    { { ContextTag(3), "fabricID", ItemType::kDefault }, kInvalidNodeIndex },      // OperationalCredentials::fabric_id
    { { ContextTag(4), "nodeID", ItemType::kDefault }, kInvalidNodeIndex },        // OperationalCredentials::node_id
    { { ContextTag(5), "label", ItemType::kDefault }, kInvalidNodeIndex },         // OperationalCredentials::char_string
    { { ContextTag(254), "fabricIndex", ItemType::kDefault }, kInvalidNodeIndex }, // OperationalCredentials::fabric_idx
};

const Entry<ItemInfo> _OperationalCredentials_NOCStruct[] = {
    { { ContextTag(1), "noc", ItemType::kDefault }, kInvalidNodeIndex },           // OperationalCredentials::octet_string
    { { ContextTag(2), "icac", ItemType::kDefault }, kInvalidNodeIndex },          // OperationalCredentials::octet_string
    { { ContextTag(254), "fabricIndex", ItemType::kDefault }, kInvalidNodeIndex }, // OperationalCredentials::fabric_idx
};

const Entry<ItemInfo> _OperationalCredentials_AttestationRequestRequest[] = {
    { { ContextTag(0), "attestationNonce", ItemType::kDefault }, kInvalidNodeIndex }, // OperationalCredentials::OCTET_STRING
};

const Entry<ItemInfo> _OperationalCredentials_AttestationResponse[] = {
    { { ContextTag(0), "attestationElements", ItemType::kDefault }, kInvalidNodeIndex },  // OperationalCredentials::OCTET_STRING
    { { ContextTag(1), "attestationSignature", ItemType::kDefault }, kInvalidNodeIndex }, // OperationalCredentials::OCTET_STRING
};

const Entry<ItemInfo> _OperationalCredentials_CertificateChainRequestRequest[] = {
    { { ContextTag(0), "certificateType", ItemType::kEnum },
      kInvalidNodeIndex }, // OperationalCredentials::CertificateChainTypeEnum
};

const Entry<ItemInfo> _OperationalCredentials_CertificateChainResponse[] = {
    { { ContextTag(0), "certificate", ItemType::kDefault }, kInvalidNodeIndex }, // OperationalCredentials::OCTET_STRING
};

const Entry<ItemInfo> _OperationalCredentials_CSRRequestRequest[] = {
    { { ContextTag(0), "CSRNonce", ItemType::kDefault }, kInvalidNodeIndex },       // OperationalCredentials::OCTET_STRING
    { { ContextTag(1), "isForUpdateNOC", ItemType::kDefault }, kInvalidNodeIndex }, // OperationalCredentials::boolean
};

const Entry<ItemInfo> _OperationalCredentials_CSRResponse[] = {
    { { ContextTag(0), "NOCSRElements", ItemType::kDefault }, kInvalidNodeIndex },        // OperationalCredentials::OCTET_STRING
    { { ContextTag(1), "attestationSignature", ItemType::kDefault }, kInvalidNodeIndex }, // OperationalCredentials::OCTET_STRING
};

const Entry<ItemInfo> _OperationalCredentials_AddNOCRequest[] = {
    { { ContextTag(0), "NOCValue", ItemType::kDefault }, kInvalidNodeIndex },         // OperationalCredentials::OCTET_STRING
    { { ContextTag(1), "ICACValue", ItemType::kDefault }, kInvalidNodeIndex },        // OperationalCredentials::OCTET_STRING
    { { ContextTag(2), "IPKValue", ItemType::kDefault }, kInvalidNodeIndex },         // OperationalCredentials::OCTET_STRING
    { { ContextTag(3), "caseAdminSubject", ItemType::kDefault }, kInvalidNodeIndex }, // OperationalCredentials::Int64u
    { { ContextTag(4), "adminVendorId", ItemType::kDefault }, kInvalidNodeIndex },    // OperationalCredentials::VENDOR_ID
};

const Entry<ItemInfo> _OperationalCredentials_UpdateNOCRequest[] = {
    { { ContextTag(0), "NOCValue", ItemType::kDefault }, kInvalidNodeIndex },  // OperationalCredentials::OCTET_STRING
    { { ContextTag(1), "ICACValue", ItemType::kDefault }, kInvalidNodeIndex }, // OperationalCredentials::OCTET_STRING
};

const Entry<ItemInfo> _OperationalCredentials_NOCResponse[] = {
    { { ContextTag(0), "statusCode", ItemType::kEnum },
      kInvalidNodeIndex }, // OperationalCredentials::NodeOperationalCertStatusEnum
    { { ContextTag(1), "fabricIndex", ItemType::kDefault }, kInvalidNodeIndex }, // OperationalCredentials::fabric_idx
    { { ContextTag(2), "debugText", ItemType::kDefault }, kInvalidNodeIndex },   // OperationalCredentials::CHAR_STRING
};

const Entry<ItemInfo> _OperationalCredentials_UpdateFabricLabelRequest[] = {
    { { ContextTag(0), "label", ItemType::kDefault }, kInvalidNodeIndex }, // OperationalCredentials::CHAR_STRING
};

const Entry<ItemInfo> _OperationalCredentials_RemoveFabricRequest[] = {
    { { ContextTag(0), "fabricIndex", ItemType::kDefault }, kInvalidNodeIndex }, // OperationalCredentials::fabric_idx
};

const Entry<ItemInfo> _OperationalCredentials_AddTrustedRootCertificateRequest[] = {
    { { ContextTag(0), "rootCACertificate", ItemType::kDefault }, kInvalidNodeIndex }, // OperationalCredentials::OCTET_STRING
};

const Entry<ItemInfo> _OperationalCredentials_FabricDescriptorStruct_list_[] = {
    { { AnonymousTag(), "[]", ItemType::kDefault }, 207 }, // OperationalCredentials_FabricDescriptorStruct[]
};

const Entry<ItemInfo> _OperationalCredentials_NOCStruct_list_[] = {
    { { AnonymousTag(), "[]", ItemType::kDefault }, 208 }, // OperationalCredentials_NOCStruct[]
};

const Entry<ItemInfo> _GroupKeyManagement[] = {
    { { AttributeTag(0), "groupKeyMap", ItemType::kList }, 232 }, // GroupKeyManagement::GroupKeyMapStruct[]
    { { AttributeTag(1), "groupTable", ItemType::kList }, 233 },  // GroupKeyManagement::GroupInfoMapStruct[]
    { { AttributeTag(2), "maxGroupsPerFabric", ItemType::kDefault }, kInvalidNodeIndex },    // GroupKeyManagement::int16u
    { { AttributeTag(3), "maxGroupKeysPerFabric", ItemType::kDefault }, kInvalidNodeIndex }, // GroupKeyManagement::int16u
    { { AttributeTag(65528), "generatedCommandList", ItemType::kList }, kInvalidNodeIndex }, // GroupKeyManagement::command_id[]
    { { AttributeTag(65529), "acceptedCommandList", ItemType::kList }, kInvalidNodeIndex },  // GroupKeyManagement::command_id[]
    { { AttributeTag(65530), "eventList", ItemType::kList }, kInvalidNodeIndex },            // GroupKeyManagement::event_id[]
    { { AttributeTag(65531), "attributeList", ItemType::kList }, kInvalidNodeIndex },        // GroupKeyManagement::attrib_id[]
    { { AttributeTag(65532), "featureMap", ItemType::kDefault }, kInvalidNodeIndex },        // GroupKeyManagement::bitmap32
    { { AttributeTag(65533), "clusterRevision", ItemType::kDefault }, kInvalidNodeIndex },   // GroupKeyManagement::int16u
    { { CommandTag(0), "KeySetWriteRequest", ItemType::kDefault }, 227 },  // GroupKeyManagement::KeySetWrite::KeySetWriteRequest
    { { CommandTag(1), "KeySetReadRequest", ItemType::kDefault }, 228 },   // GroupKeyManagement::KeySetRead::KeySetReadRequest
    { { CommandTag(3), "KeySetRemoveRequest", ItemType::kDefault }, 230 }, // GroupKeyManagement::KeySetRemove::KeySetRemoveRequest
    { { CommandTag(2), "KeySetReadResponse", ItemType::kDefault }, 229 },  // GroupKeyManagement::KeySetReadResponse
    { { CommandTag(5), "KeySetReadAllIndicesResponse", ItemType::kDefault },
      231 }, // GroupKeyManagement::KeySetReadAllIndicesResponse
};

const Entry<ItemInfo> _GroupKeyManagement_GroupInfoMapStruct[] = {
    { { ContextTag(1), "groupId", ItemType::kDefault }, kInvalidNodeIndex },       // GroupKeyManagement::group_id
    { { ContextTag(2), "endpoints", ItemType::kList }, kInvalidNodeIndex },        // GroupKeyManagement::endpoint_no[]
    { { ContextTag(3), "groupName", ItemType::kDefault }, kInvalidNodeIndex },     // GroupKeyManagement::char_string
    { { ContextTag(254), "fabricIndex", ItemType::kDefault }, kInvalidNodeIndex }, // GroupKeyManagement::fabric_idx
};

const Entry<ItemInfo> _GroupKeyManagement_GroupKeyMapStruct[] = {
    { { ContextTag(1), "groupId", ItemType::kDefault }, kInvalidNodeIndex },       // GroupKeyManagement::group_id
    { { ContextTag(2), "groupKeySetID", ItemType::kDefault }, kInvalidNodeIndex }, // GroupKeyManagement::int16u
    { { ContextTag(254), "fabricIndex", ItemType::kDefault }, kInvalidNodeIndex }, // GroupKeyManagement::fabric_idx
};

const Entry<ItemInfo> _GroupKeyManagement_GroupKeySetStruct[] = {
    { { ContextTag(0), "groupKeySetID", ItemType::kDefault }, kInvalidNodeIndex }, // GroupKeyManagement::int16u
    { { ContextTag(1), "groupKeySecurityPolicy", ItemType::kEnum },
      kInvalidNodeIndex },                                                     // GroupKeyManagement::GroupKeySecurityPolicyEnum
    { { ContextTag(2), "epochKey0", ItemType::kDefault }, kInvalidNodeIndex }, // GroupKeyManagement::octet_string
    { { ContextTag(3), "epochStartTime0", ItemType::kDefault }, kInvalidNodeIndex }, // GroupKeyManagement::epoch_us
    { { ContextTag(4), "epochKey1", ItemType::kDefault }, kInvalidNodeIndex },       // GroupKeyManagement::octet_string
    { { ContextTag(5), "epochStartTime1", ItemType::kDefault }, kInvalidNodeIndex }, // GroupKeyManagement::epoch_us
    { { ContextTag(6), "epochKey2", ItemType::kDefault }, kInvalidNodeIndex },       // GroupKeyManagement::octet_string
    { { ContextTag(7), "epochStartTime2", ItemType::kDefault }, kInvalidNodeIndex }, // GroupKeyManagement::epoch_us
};

const Entry<ItemInfo> _GroupKeyManagement_KeySetWriteRequest[] = {
    { { ContextTag(0), "groupKeySet", ItemType::kDefault }, 226 }, // GroupKeyManagement::GroupKeySetStruct
};

const Entry<ItemInfo> _GroupKeyManagement_KeySetReadRequest[] = {
    { { ContextTag(0), "groupKeySetID", ItemType::kDefault }, kInvalidNodeIndex }, // GroupKeyManagement::INT16U
};

const Entry<ItemInfo> _GroupKeyManagement_KeySetReadResponse[] = {
    { { ContextTag(0), "groupKeySet", ItemType::kDefault }, 226 }, // GroupKeyManagement::GroupKeySetStruct
};

const Entry<ItemInfo> _GroupKeyManagement_KeySetRemoveRequest[] = {
    { { ContextTag(0), "groupKeySetID", ItemType::kDefault }, kInvalidNodeIndex }, // GroupKeyManagement::INT16U
};

const Entry<ItemInfo> _GroupKeyManagement_KeySetReadAllIndicesResponse[] = {
    { { ContextTag(0), "groupKeySetIDs", ItemType::kList }, kInvalidNodeIndex }, // GroupKeyManagement::INT16U[]
};

const Entry<ItemInfo> _GroupKeyManagement_GroupKeyMapStruct_list_[] = {
    { { AnonymousTag(), "[]", ItemType::kDefault }, 225 }, // GroupKeyManagement_GroupKeyMapStruct[]
};

const Entry<ItemInfo> _GroupKeyManagement_GroupInfoMapStruct_list_[] = {
    { { AnonymousTag(), "[]", ItemType::kDefault }, 224 }, // GroupKeyManagement_GroupInfoMapStruct[]
};

const Entry<ItemInfo> _FixedLabel[] = {
    { { AttributeTag(0), "labelList", ItemType::kList }, 236 },                              // FixedLabel::LabelStruct[]
    { { AttributeTag(65528), "generatedCommandList", ItemType::kList }, kInvalidNodeIndex }, // FixedLabel::command_id[]
    { { AttributeTag(65529), "acceptedCommandList", ItemType::kList }, kInvalidNodeIndex },  // FixedLabel::command_id[]
    { { AttributeTag(65530), "eventList", ItemType::kList }, kInvalidNodeIndex },            // FixedLabel::event_id[]
    { { AttributeTag(65531), "attributeList", ItemType::kList }, kInvalidNodeIndex },        // FixedLabel::attrib_id[]
    { { AttributeTag(65532), "featureMap", ItemType::kDefault }, kInvalidNodeIndex },        // FixedLabel::bitmap32
    { { AttributeTag(65533), "clusterRevision", ItemType::kDefault }, kInvalidNodeIndex },   // FixedLabel::int16u
};

const Entry<ItemInfo> _FixedLabel_LabelStruct[] = {
    { { ContextTag(0), "label", ItemType::kDefault }, kInvalidNodeIndex }, // FixedLabel::char_string
    { { ContextTag(1), "value", ItemType::kDefault }, kInvalidNodeIndex }, // FixedLabel::char_string
};

const Entry<ItemInfo> _FixedLabel_LabelStruct_list_[] = {
    { { AnonymousTag(), "[]", ItemType::kDefault }, 235 }, // FixedLabel_LabelStruct[]
};

const Entry<ItemInfo> _UserLabel[] = {
    { { AttributeTag(0), "labelList", ItemType::kList }, 239 },                              // UserLabel::LabelStruct[]
    { { AttributeTag(65528), "generatedCommandList", ItemType::kList }, kInvalidNodeIndex }, // UserLabel::command_id[]
    { { AttributeTag(65529), "acceptedCommandList", ItemType::kList }, kInvalidNodeIndex },  // UserLabel::command_id[]
    { { AttributeTag(65530), "eventList", ItemType::kList }, kInvalidNodeIndex },            // UserLabel::event_id[]
    { { AttributeTag(65531), "attributeList", ItemType::kList }, kInvalidNodeIndex },        // UserLabel::attrib_id[]
    { { AttributeTag(65532), "featureMap", ItemType::kDefault }, kInvalidNodeIndex },        // UserLabel::bitmap32
    { { AttributeTag(65533), "clusterRevision", ItemType::kDefault }, kInvalidNodeIndex },   // UserLabel::int16u
};

const Entry<ItemInfo> _UserLabel_LabelStruct[] = {
    { { ContextTag(0), "label", ItemType::kDefault }, kInvalidNodeIndex }, // UserLabel::char_string
    { { ContextTag(1), "value", ItemType::kDefault }, kInvalidNodeIndex }, // UserLabel::char_string
};

const Entry<ItemInfo> _UserLabel_LabelStruct_list_[] = {
    { { AnonymousTag(), "[]", ItemType::kDefault }, 238 }, // UserLabel_LabelStruct[]
};

const Entry<ItemInfo> _ProxyConfiguration[] = {
    { { AttributeTag(65528), "generatedCommandList", ItemType::kList }, kInvalidNodeIndex }, // ProxyConfiguration::command_id[]
    { { AttributeTag(65529), "acceptedCommandList", ItemType::kList }, kInvalidNodeIndex },  // ProxyConfiguration::command_id[]
    { { AttributeTag(65530), "eventList", ItemType::kList }, kInvalidNodeIndex },            // ProxyConfiguration::event_id[]
    { { AttributeTag(65531), "attributeList", ItemType::kList }, kInvalidNodeIndex },        // ProxyConfiguration::attrib_id[]
    { { AttributeTag(65532), "featureMap", ItemType::kDefault }, kInvalidNodeIndex },        // ProxyConfiguration::bitmap32
    { { AttributeTag(65533), "clusterRevision", ItemType::kDefault }, kInvalidNodeIndex },   // ProxyConfiguration::int16u
};

const Entry<ItemInfo> _ProxyDiscovery[] = {
    { { AttributeTag(65528), "generatedCommandList", ItemType::kList }, kInvalidNodeIndex }, // ProxyDiscovery::command_id[]
    { { AttributeTag(65529), "acceptedCommandList", ItemType::kList }, kInvalidNodeIndex },  // ProxyDiscovery::command_id[]
    { { AttributeTag(65530), "eventList", ItemType::kList }, kInvalidNodeIndex },            // ProxyDiscovery::event_id[]
    { { AttributeTag(65531), "attributeList", ItemType::kList }, kInvalidNodeIndex },        // ProxyDiscovery::attrib_id[]
    { { AttributeTag(65532), "featureMap", ItemType::kDefault }, kInvalidNodeIndex },        // ProxyDiscovery::bitmap32
    { { AttributeTag(65533), "clusterRevision", ItemType::kDefault }, kInvalidNodeIndex },   // ProxyDiscovery::int16u
};

const Entry<ItemInfo> _ProxyValid[] = {
    { { AttributeTag(65528), "generatedCommandList", ItemType::kList }, kInvalidNodeIndex }, // ProxyValid::command_id[]
    { { AttributeTag(65529), "acceptedCommandList", ItemType::kList }, kInvalidNodeIndex },  // ProxyValid::command_id[]
    { { AttributeTag(65530), "eventList", ItemType::kList }, kInvalidNodeIndex },            // ProxyValid::event_id[]
    { { AttributeTag(65531), "attributeList", ItemType::kList }, kInvalidNodeIndex },        // ProxyValid::attrib_id[]
    { { AttributeTag(65532), "featureMap", ItemType::kDefault }, kInvalidNodeIndex },        // ProxyValid::bitmap32
    { { AttributeTag(65533), "clusterRevision", ItemType::kDefault }, kInvalidNodeIndex },   // ProxyValid::int16u
};

const Entry<ItemInfo> _BooleanState[] = {
    { { AttributeTag(0), "stateValue", ItemType::kDefault }, kInvalidNodeIndex },            // BooleanState::boolean
    { { AttributeTag(65528), "generatedCommandList", ItemType::kList }, kInvalidNodeIndex }, // BooleanState::command_id[]
    { { AttributeTag(65529), "acceptedCommandList", ItemType::kList }, kInvalidNodeIndex },  // BooleanState::command_id[]
    { { AttributeTag(65530), "eventList", ItemType::kList }, kInvalidNodeIndex },            // BooleanState::event_id[]
    { { AttributeTag(65531), "attributeList", ItemType::kList }, kInvalidNodeIndex },        // BooleanState::attrib_id[]
    { { AttributeTag(65532), "featureMap", ItemType::kDefault }, kInvalidNodeIndex },        // BooleanState::bitmap32
    { { AttributeTag(65533), "clusterRevision", ItemType::kDefault }, kInvalidNodeIndex },   // BooleanState::int16u
    { { EventTag(0), "StateChange", ItemType::kDefault }, 244 },                             // BooleanState::StateChange
};

const Entry<ItemInfo> _BooleanState_StateChange[] = {
    { { ContextTag(0), "stateValue", ItemType::kDefault }, kInvalidNodeIndex }, // BooleanState::boolean
};

const Entry<ItemInfo> _IcdManagement[] = {
    { { AttributeTag(0), "idleModeInterval", ItemType::kDefault }, kInvalidNodeIndex },    // IcdManagement::int32u
    { { AttributeTag(1), "activeModeInterval", ItemType::kDefault }, kInvalidNodeIndex },  // IcdManagement::int32u
    { { AttributeTag(2), "activeModeThreshold", ItemType::kDefault }, kInvalidNodeIndex }, // IcdManagement::int16u
    { { AttributeTag(3), "registeredClients", ItemType::kList }, 250 },           // IcdManagement::MonitoringRegistrationStruct[]
    { { AttributeTag(4), "ICDCounter", ItemType::kDefault }, kInvalidNodeIndex }, // IcdManagement::int32u
    { { AttributeTag(5), "clientsSupportedPerFabric", ItemType::kDefault }, kInvalidNodeIndex }, // IcdManagement::int16u
    { { AttributeTag(65528), "generatedCommandList", ItemType::kList }, kInvalidNodeIndex },     // IcdManagement::command_id[]
    { { AttributeTag(65529), "acceptedCommandList", ItemType::kList }, kInvalidNodeIndex },      // IcdManagement::command_id[]
    { { AttributeTag(65530), "eventList", ItemType::kList }, kInvalidNodeIndex },                // IcdManagement::event_id[]
    { { AttributeTag(65531), "attributeList", ItemType::kList }, kInvalidNodeIndex },            // IcdManagement::attrib_id[]
    { { AttributeTag(65532), "featureMap", ItemType::kDefault }, kInvalidNodeIndex },            // IcdManagement::bitmap32
    { { AttributeTag(65533), "clusterRevision", ItemType::kDefault }, kInvalidNodeIndex },       // IcdManagement::int16u
    { { CommandTag(0), "RegisterClientRequest", ItemType::kDefault }, 247 }, // IcdManagement::RegisterClient::RegisterClientRequest
    { { CommandTag(2), "UnregisterClientRequest", ItemType::kDefault },
      249 }, // IcdManagement::UnregisterClient::UnregisterClientRequest
    { { CommandTag(1), "RegisterClientResponse", ItemType::kDefault }, 248 }, // IcdManagement::RegisterClientResponse
};

const Entry<ItemInfo> _IcdManagement_MonitoringRegistrationStruct[] = {
    { { ContextTag(1), "checkInNodeID", ItemType::kDefault }, kInvalidNodeIndex },    // IcdManagement::node_id
    { { ContextTag(2), "monitoredSubject", ItemType::kDefault }, kInvalidNodeIndex }, // IcdManagement::int64u
    { { ContextTag(3), "key", ItemType::kDefault }, kInvalidNodeIndex },              // IcdManagement::octet_string
    { { ContextTag(254), "fabricIndex", ItemType::kDefault }, kInvalidNodeIndex },    // IcdManagement::fabric_idx
};

const Entry<ItemInfo> _IcdManagement_RegisterClientRequest[] = {
    { { ContextTag(0), "checkInNodeID", ItemType::kDefault }, kInvalidNodeIndex },    // IcdManagement::node_id
    { { ContextTag(1), "monitoredSubject", ItemType::kDefault }, kInvalidNodeIndex }, // IcdManagement::INT64U
    { { ContextTag(2), "key", ItemType::kDefault }, kInvalidNodeIndex },              // IcdManagement::OCTET_STRING
    { { ContextTag(3), "verificationKey", ItemType::kDefault }, kInvalidNodeIndex },  // IcdManagement::OCTET_STRING
};

const Entry<ItemInfo> _IcdManagement_RegisterClientResponse[] = {
    { { ContextTag(0), "ICDCounter", ItemType::kDefault }, kInvalidNodeIndex }, // IcdManagement::INT32U
};

const Entry<ItemInfo> _IcdManagement_UnregisterClientRequest[] = {
    { { ContextTag(0), "checkInNodeID", ItemType::kDefault }, kInvalidNodeIndex }, // IcdManagement::node_id
    { { ContextTag(1), "key", ItemType::kDefault }, kInvalidNodeIndex },           // IcdManagement::OCTET_STRING
};

const Entry<ItemInfo> _IcdManagement_MonitoringRegistrationStruct_list_[] = {
    { { AnonymousTag(), "[]", ItemType::kDefault }, 246 }, // IcdManagement_MonitoringRegistrationStruct[]
};

const Entry<ItemInfo> _ModeSelect[] = {
    { { AttributeTag(0), "description", ItemType::kDefault }, kInvalidNodeIndex },           // ModeSelect::char_string
    { { AttributeTag(1), "standardNamespace", ItemType::kDefault }, kInvalidNodeIndex },     // ModeSelect::enum16
    { { AttributeTag(2), "supportedModes", ItemType::kList }, 256 },                         // ModeSelect::ModeOptionStruct[]
    { { AttributeTag(3), "currentMode", ItemType::kDefault }, kInvalidNodeIndex },           // ModeSelect::int8u
    { { AttributeTag(4), "startUpMode", ItemType::kDefault }, kInvalidNodeIndex },           // ModeSelect::int8u
    { { AttributeTag(5), "onMode", ItemType::kDefault }, kInvalidNodeIndex },                // ModeSelect::int8u
    { { AttributeTag(65528), "generatedCommandList", ItemType::kList }, kInvalidNodeIndex }, // ModeSelect::command_id[]
    { { AttributeTag(65529), "acceptedCommandList", ItemType::kList }, kInvalidNodeIndex },  // ModeSelect::command_id[]
    { { AttributeTag(65530), "eventList", ItemType::kList }, kInvalidNodeIndex },            // ModeSelect::event_id[]
    { { AttributeTag(65531), "attributeList", ItemType::kList }, kInvalidNodeIndex },        // ModeSelect::attrib_id[]
    { { AttributeTag(65532), "featureMap", ItemType::kDefault }, kInvalidNodeIndex },        // ModeSelect::bitmap32
    { { AttributeTag(65533), "clusterRevision", ItemType::kDefault }, kInvalidNodeIndex },   // ModeSelect::int16u
    { { CommandTag(0), "ChangeToModeRequest", ItemType::kDefault }, 254 }, // ModeSelect::ChangeToMode::ChangeToModeRequest
};

const Entry<ItemInfo> _ModeSelect_SemanticTagStruct[] = {
    { { ContextTag(0), "mfgCode", ItemType::kDefault }, kInvalidNodeIndex }, // ModeSelect::vendor_id
    { { ContextTag(1), "value", ItemType::kDefault }, kInvalidNodeIndex },   // ModeSelect::enum16
};

const Entry<ItemInfo> _ModeSelect_ModeOptionStruct[] = {
    { { ContextTag(0), "label", ItemType::kDefault }, kInvalidNodeIndex }, // ModeSelect::char_string
    { { ContextTag(1), "mode", ItemType::kDefault }, kInvalidNodeIndex },  // ModeSelect::int8u
    { { ContextTag(2), "semanticTags", ItemType::kList }, 255 },           // ModeSelect::SemanticTagStruct[]
};

const Entry<ItemInfo> _ModeSelect_ChangeToModeRequest[] = {
    { { ContextTag(0), "newMode", ItemType::kDefault }, kInvalidNodeIndex }, // ModeSelect::INT8U
};

const Entry<ItemInfo> _ModeSelect_SemanticTagStruct_list_[] = {
    { { AnonymousTag(), "[]", ItemType::kDefault }, 252 }, // ModeSelect_SemanticTagStruct[]
};

const Entry<ItemInfo> _ModeSelect_ModeOptionStruct_list_[] = {
    { { AnonymousTag(), "[]", ItemType::kDefault }, 253 }, // ModeSelect_ModeOptionStruct[]
};

const Entry<ItemInfo> _LaundryWasherMode[] = {
    { { AttributeTag(0), "supportedModes", ItemType::kList }, 262 },               // LaundryWasherMode::ModeOptionStruct[]
    { { AttributeTag(1), "currentMode", ItemType::kDefault }, kInvalidNodeIndex }, // LaundryWasherMode::int8u
    { { AttributeTag(2), "startUpMode", ItemType::kDefault }, kInvalidNodeIndex }, // LaundryWasherMode::int8u
    { { AttributeTag(3), "onMode", ItemType::kDefault }, kInvalidNodeIndex },      // LaundryWasherMode::int8u
    { { AttributeTag(65528), "generatedCommandList", ItemType::kList }, kInvalidNodeIndex }, // LaundryWasherMode::command_id[]
    { { AttributeTag(65529), "acceptedCommandList", ItemType::kList }, kInvalidNodeIndex },  // LaundryWasherMode::command_id[]
    { { AttributeTag(65530), "eventList", ItemType::kList }, kInvalidNodeIndex },            // LaundryWasherMode::event_id[]
    { { AttributeTag(65531), "attributeList", ItemType::kList }, kInvalidNodeIndex },        // LaundryWasherMode::attrib_id[]
    { { AttributeTag(65532), "featureMap", ItemType::kDefault }, kInvalidNodeIndex },        // LaundryWasherMode::bitmap32
    { { AttributeTag(65533), "clusterRevision", ItemType::kDefault }, kInvalidNodeIndex },   // LaundryWasherMode::int16u
    { { CommandTag(0), "ChangeToModeRequest", ItemType::kDefault }, 260 },  // LaundryWasherMode::ChangeToMode::ChangeToModeRequest
    { { CommandTag(1), "ChangeToModeResponse", ItemType::kDefault }, 261 }, // LaundryWasherMode::ChangeToModeResponse
};

const Entry<ItemInfo> _LaundryWasherMode_ModeTagStruct[] = {
    { { ContextTag(0), "mfgCode", ItemType::kDefault }, kInvalidNodeIndex }, // LaundryWasherMode::vendor_id
    { { ContextTag(1), "value", ItemType::kDefault }, kInvalidNodeIndex },   // LaundryWasherMode::enum16
};

const Entry<ItemInfo> _LaundryWasherMode_ModeOptionStruct[] = {
    { { ContextTag(0), "label", ItemType::kDefault }, kInvalidNodeIndex }, // LaundryWasherMode::char_string
    { { ContextTag(1), "mode", ItemType::kDefault }, kInvalidNodeIndex },  // LaundryWasherMode::int8u
    { { ContextTag(2), "modeTags", ItemType::kList }, 263 },               // LaundryWasherMode::ModeTagStruct[]
};

const Entry<ItemInfo> _LaundryWasherMode_ChangeToModeRequest[] = {
    { { ContextTag(0), "newMode", ItemType::kDefault }, kInvalidNodeIndex }, // LaundryWasherMode::INT8U
};

const Entry<ItemInfo> _LaundryWasherMode_ChangeToModeResponse[] = {
    { { ContextTag(0), "status", ItemType::kDefault }, kInvalidNodeIndex },     // LaundryWasherMode::ENUM8
    { { ContextTag(1), "statusText", ItemType::kDefault }, kInvalidNodeIndex }, // LaundryWasherMode::CHAR_STRING
};

const Entry<ItemInfo> _LaundryWasherMode_ModeOptionStruct_list_[] = {
    { { AnonymousTag(), "[]", ItemType::kDefault }, 259 }, // LaundryWasherMode_ModeOptionStruct[]
};

const Entry<ItemInfo> _LaundryWasherMode_ModeTagStruct_list_[] = {
    { { AnonymousTag(), "[]", ItemType::kDefault }, 258 }, // LaundryWasherMode_ModeTagStruct[]
};

const Entry<ItemInfo> _RefrigeratorAndTemperatureControlledCabinetMode[] = {
    { { AttributeTag(0), "supportedModes", ItemType::kList },
      270 }, // RefrigeratorAndTemperatureControlledCabinetMode::ModeOptionStruct[]
    { { AttributeTag(1), "currentMode", ItemType::kDefault },
      kInvalidNodeIndex }, // RefrigeratorAndTemperatureControlledCabinetMode::int8u
    { { AttributeTag(2), "startUpMode", ItemType::kDefault },
      kInvalidNodeIndex }, // RefrigeratorAndTemperatureControlledCabinetMode::int8u
    { { AttributeTag(3), "onMode", ItemType::kDefault },
      kInvalidNodeIndex }, // RefrigeratorAndTemperatureControlledCabinetMode::int8u
    { { AttributeTag(65528), "generatedCommandList", ItemType::kList },
      kInvalidNodeIndex }, // RefrigeratorAndTemperatureControlledCabinetMode::command_id[]
    { { AttributeTag(65529), "acceptedCommandList", ItemType::kList },
      kInvalidNodeIndex }, // RefrigeratorAndTemperatureControlledCabinetMode::command_id[]
    { { AttributeTag(65530), "eventList", ItemType::kList },
      kInvalidNodeIndex }, // RefrigeratorAndTemperatureControlledCabinetMode::event_id[]
    { { AttributeTag(65531), "attributeList", ItemType::kList },
      kInvalidNodeIndex }, // RefrigeratorAndTemperatureControlledCabinetMode::attrib_id[]
    { { AttributeTag(65532), "featureMap", ItemType::kDefault },
      kInvalidNodeIndex }, // RefrigeratorAndTemperatureControlledCabinetMode::bitmap32
    { { AttributeTag(65533), "clusterRevision", ItemType::kDefault },
      kInvalidNodeIndex }, // RefrigeratorAndTemperatureControlledCabinetMode::int16u
    { { CommandTag(0), "ChangeToModeRequest", ItemType::kDefault },
      267 }, // RefrigeratorAndTemperatureControlledCabinetMode::ChangeToMode::ChangeToModeRequest
    { { CommandTag(1), "ChangeToModeResponse", ItemType::kDefault },
      268 }, // RefrigeratorAndTemperatureControlledCabinetMode::ChangeToModeResponse
};

const Entry<ItemInfo> _RefrigeratorAndTemperatureControlledCabinetMode_ModeTagStruct[] = {
    { { ContextTag(0), "mfgCode", ItemType::kDefault },
      kInvalidNodeIndex }, // RefrigeratorAndTemperatureControlledCabinetMode::vendor_id
    { { ContextTag(1), "value", ItemType::kDefault },
      kInvalidNodeIndex }, // RefrigeratorAndTemperatureControlledCabinetMode::enum16
};

const Entry<ItemInfo> _RefrigeratorAndTemperatureControlledCabinetMode_ModeOptionStruct[] = {
    { { ContextTag(0), "label", ItemType::kDefault },
      kInvalidNodeIndex }, // RefrigeratorAndTemperatureControlledCabinetMode::char_string
    { { ContextTag(1), "mode", ItemType::kDefault }, kInvalidNodeIndex }, // RefrigeratorAndTemperatureControlledCabinetMode::int8u
    { { ContextTag(2), "modeTags", ItemType::kList }, 269 }, // RefrigeratorAndTemperatureControlledCabinetMode::ModeTagStruct[]
};

const Entry<ItemInfo> _RefrigeratorAndTemperatureControlledCabinetMode_ChangeToModeRequest[] = {
    { { ContextTag(0), "newMode", ItemType::kDefault },
      kInvalidNodeIndex }, // RefrigeratorAndTemperatureControlledCabinetMode::INT8U
};

const Entry<ItemInfo> _RefrigeratorAndTemperatureControlledCabinetMode_ChangeToModeResponse[] = {
    { { ContextTag(0), "status", ItemType::kDefault },
      kInvalidNodeIndex }, // RefrigeratorAndTemperatureControlledCabinetMode::ENUM8
    { { ContextTag(1), "statusText", ItemType::kDefault },
      kInvalidNodeIndex }, // RefrigeratorAndTemperatureControlledCabinetMode::CHAR_STRING
};

const Entry<ItemInfo> _RefrigeratorAndTemperatureControlledCabinetMode_ModeTagStruct_list_[] = {
    { { AnonymousTag(), "[]", ItemType::kDefault }, 265 }, // RefrigeratorAndTemperatureControlledCabinetMode_ModeTagStruct[]
};

const Entry<ItemInfo> _RefrigeratorAndTemperatureControlledCabinetMode_ModeOptionStruct_list_[] = {
    { { AnonymousTag(), "[]", ItemType::kDefault }, 266 }, // RefrigeratorAndTemperatureControlledCabinetMode_ModeOptionStruct[]
};

const Entry<ItemInfo> _LaundryWasherControls[] = {
    { { AttributeTag(0), "spinSpeeds", ItemType::kList }, kInvalidNodeIndex },          // LaundryWasherControls::CHAR_STRING[]
    { { AttributeTag(1), "spinSpeedCurrent", ItemType::kDefault }, kInvalidNodeIndex }, // LaundryWasherControls::int8u
    { { AttributeTag(2), "numberOfRinses", ItemType::kEnum }, kInvalidNodeIndex },      // LaundryWasherControls::NumberOfRinsesEnum
    { { AttributeTag(3), "supportedRinses", ItemType::kList }, kInvalidNodeIndex }, // LaundryWasherControls::NumberOfRinsesEnum[]
    { { AttributeTag(65528), "generatedCommandList", ItemType::kList }, kInvalidNodeIndex }, // LaundryWasherControls::command_id[]
    { { AttributeTag(65529), "acceptedCommandList", ItemType::kList }, kInvalidNodeIndex },  // LaundryWasherControls::command_id[]
    { { AttributeTag(65530), "eventList", ItemType::kList }, kInvalidNodeIndex },            // LaundryWasherControls::event_id[]
    { { AttributeTag(65531), "attributeList", ItemType::kList }, kInvalidNodeIndex },        // LaundryWasherControls::attrib_id[]
    { { AttributeTag(65532), "featureMap", ItemType::kDefault }, kInvalidNodeIndex },        // LaundryWasherControls::bitmap32
    { { AttributeTag(65533), "clusterRevision", ItemType::kDefault }, kInvalidNodeIndex },   // LaundryWasherControls::int16u
};

const Entry<ItemInfo> _RvcRunMode[] = {
    { { AttributeTag(0), "supportedModes", ItemType::kList }, 278 },                         // RvcRunMode::ModeOptionStruct[]
    { { AttributeTag(1), "currentMode", ItemType::kDefault }, kInvalidNodeIndex },           // RvcRunMode::int8u
    { { AttributeTag(2), "startUpMode", ItemType::kDefault }, kInvalidNodeIndex },           // RvcRunMode::int8u
    { { AttributeTag(3), "onMode", ItemType::kDefault }, kInvalidNodeIndex },                // RvcRunMode::int8u
    { { AttributeTag(65528), "generatedCommandList", ItemType::kList }, kInvalidNodeIndex }, // RvcRunMode::command_id[]
    { { AttributeTag(65529), "acceptedCommandList", ItemType::kList }, kInvalidNodeIndex },  // RvcRunMode::command_id[]
    { { AttributeTag(65530), "eventList", ItemType::kList }, kInvalidNodeIndex },            // RvcRunMode::event_id[]
    { { AttributeTag(65531), "attributeList", ItemType::kList }, kInvalidNodeIndex },        // RvcRunMode::attrib_id[]
    { { AttributeTag(65532), "featureMap", ItemType::kDefault }, kInvalidNodeIndex },        // RvcRunMode::bitmap32
    { { AttributeTag(65533), "clusterRevision", ItemType::kDefault }, kInvalidNodeIndex },   // RvcRunMode::int16u
    { { CommandTag(0), "ChangeToModeRequest", ItemType::kDefault }, 275 },  // RvcRunMode::ChangeToMode::ChangeToModeRequest
    { { CommandTag(1), "ChangeToModeResponse", ItemType::kDefault }, 276 }, // RvcRunMode::ChangeToModeResponse
};

const Entry<ItemInfo> _RvcRunMode_ModeTagStruct[] = {
    { { ContextTag(0), "mfgCode", ItemType::kDefault }, kInvalidNodeIndex }, // RvcRunMode::vendor_id
    { { ContextTag(1), "value", ItemType::kDefault }, kInvalidNodeIndex },   // RvcRunMode::enum16
};

const Entry<ItemInfo> _RvcRunMode_ModeOptionStruct[] = {
    { { ContextTag(0), "label", ItemType::kDefault }, kInvalidNodeIndex }, // RvcRunMode::char_string
    { { ContextTag(1), "mode", ItemType::kDefault }, kInvalidNodeIndex },  // RvcRunMode::int8u
    { { ContextTag(2), "modeTags", ItemType::kList }, 277 },               // RvcRunMode::ModeTagStruct[]
};

const Entry<ItemInfo> _RvcRunMode_ChangeToModeRequest[] = {
    { { ContextTag(0), "newMode", ItemType::kDefault }, kInvalidNodeIndex }, // RvcRunMode::INT8U
};

const Entry<ItemInfo> _RvcRunMode_ChangeToModeResponse[] = {
    { { ContextTag(0), "status", ItemType::kDefault }, kInvalidNodeIndex },     // RvcRunMode::ENUM8
    { { ContextTag(1), "statusText", ItemType::kDefault }, kInvalidNodeIndex }, // RvcRunMode::CHAR_STRING
};

const Entry<ItemInfo> _RvcRunMode_ModeTagStruct_list_[] = {
    { { AnonymousTag(), "[]", ItemType::kDefault }, 273 }, // RvcRunMode_ModeTagStruct[]
};

const Entry<ItemInfo> _RvcRunMode_ModeOptionStruct_list_[] = {
    { { AnonymousTag(), "[]", ItemType::kDefault }, 274 }, // RvcRunMode_ModeOptionStruct[]
};

const Entry<ItemInfo> _RvcCleanMode[] = {
    { { AttributeTag(0), "supportedModes", ItemType::kList }, 284 },                         // RvcCleanMode::ModeOptionStruct[]
    { { AttributeTag(1), "currentMode", ItemType::kDefault }, kInvalidNodeIndex },           // RvcCleanMode::int8u
    { { AttributeTag(2), "startUpMode", ItemType::kDefault }, kInvalidNodeIndex },           // RvcCleanMode::int8u
    { { AttributeTag(3), "onMode", ItemType::kDefault }, kInvalidNodeIndex },                // RvcCleanMode::int8u
    { { AttributeTag(65528), "generatedCommandList", ItemType::kList }, kInvalidNodeIndex }, // RvcCleanMode::command_id[]
    { { AttributeTag(65529), "acceptedCommandList", ItemType::kList }, kInvalidNodeIndex },  // RvcCleanMode::command_id[]
    { { AttributeTag(65530), "eventList", ItemType::kList }, kInvalidNodeIndex },            // RvcCleanMode::event_id[]
    { { AttributeTag(65531), "attributeList", ItemType::kList }, kInvalidNodeIndex },        // RvcCleanMode::attrib_id[]
    { { AttributeTag(65532), "featureMap", ItemType::kDefault }, kInvalidNodeIndex },        // RvcCleanMode::bitmap32
    { { AttributeTag(65533), "clusterRevision", ItemType::kDefault }, kInvalidNodeIndex },   // RvcCleanMode::int16u
    { { CommandTag(0), "ChangeToModeRequest", ItemType::kDefault }, 282 },  // RvcCleanMode::ChangeToMode::ChangeToModeRequest
    { { CommandTag(1), "ChangeToModeResponse", ItemType::kDefault }, 283 }, // RvcCleanMode::ChangeToModeResponse
};

const Entry<ItemInfo> _RvcCleanMode_ModeTagStruct[] = {
    { { ContextTag(0), "mfgCode", ItemType::kDefault }, kInvalidNodeIndex }, // RvcCleanMode::vendor_id
    { { ContextTag(1), "value", ItemType::kDefault }, kInvalidNodeIndex },   // RvcCleanMode::enum16
};

const Entry<ItemInfo> _RvcCleanMode_ModeOptionStruct[] = {
    { { ContextTag(0), "label", ItemType::kDefault }, kInvalidNodeIndex }, // RvcCleanMode::char_string
    { { ContextTag(1), "mode", ItemType::kDefault }, kInvalidNodeIndex },  // RvcCleanMode::int8u
    { { ContextTag(2), "modeTags", ItemType::kList }, 285 },               // RvcCleanMode::ModeTagStruct[]
};

const Entry<ItemInfo> _RvcCleanMode_ChangeToModeRequest[] = {
    { { ContextTag(0), "newMode", ItemType::kDefault }, kInvalidNodeIndex }, // RvcCleanMode::INT8U
};

const Entry<ItemInfo> _RvcCleanMode_ChangeToModeResponse[] = {
    { { ContextTag(0), "status", ItemType::kDefault }, kInvalidNodeIndex },     // RvcCleanMode::ENUM8
    { { ContextTag(1), "statusText", ItemType::kDefault }, kInvalidNodeIndex }, // RvcCleanMode::CHAR_STRING
};

const Entry<ItemInfo> _RvcCleanMode_ModeOptionStruct_list_[] = {
    { { AnonymousTag(), "[]", ItemType::kDefault }, 281 }, // RvcCleanMode_ModeOptionStruct[]
};

const Entry<ItemInfo> _RvcCleanMode_ModeTagStruct_list_[] = {
    { { AnonymousTag(), "[]", ItemType::kDefault }, 280 }, // RvcCleanMode_ModeTagStruct[]
};

const Entry<ItemInfo> _TemperatureControl[] = {
    { { AttributeTag(0), "temperatureSetpoint", ItemType::kDefault }, kInvalidNodeIndex },      // TemperatureControl::temperature
    { { AttributeTag(1), "minTemperature", ItemType::kDefault }, kInvalidNodeIndex },           // TemperatureControl::temperature
    { { AttributeTag(2), "maxTemperature", ItemType::kDefault }, kInvalidNodeIndex },           // TemperatureControl::temperature
    { { AttributeTag(3), "step", ItemType::kDefault }, kInvalidNodeIndex },                     // TemperatureControl::temperature
    { { AttributeTag(4), "selectedTemperatureLevel", ItemType::kDefault }, kInvalidNodeIndex }, // TemperatureControl::int8u
    { { AttributeTag(5), "supportedTemperatureLevels", ItemType::kList }, kInvalidNodeIndex },  // TemperatureControl::char_string[]
    { { AttributeTag(65528), "generatedCommandList", ItemType::kList }, kInvalidNodeIndex },    // TemperatureControl::command_id[]
    { { AttributeTag(65529), "acceptedCommandList", ItemType::kList }, kInvalidNodeIndex },     // TemperatureControl::command_id[]
    { { AttributeTag(65530), "eventList", ItemType::kList }, kInvalidNodeIndex },               // TemperatureControl::event_id[]
    { { AttributeTag(65531), "attributeList", ItemType::kList }, kInvalidNodeIndex },           // TemperatureControl::attrib_id[]
    { { AttributeTag(65532), "featureMap", ItemType::kDefault }, kInvalidNodeIndex },           // TemperatureControl::bitmap32
    { { AttributeTag(65533), "clusterRevision", ItemType::kDefault }, kInvalidNodeIndex },      // TemperatureControl::int16u
    { { CommandTag(0), "SetTemperatureRequest", ItemType::kDefault },
      287 }, // TemperatureControl::SetTemperature::SetTemperatureRequest
};

const Entry<ItemInfo> _TemperatureControl_SetTemperatureRequest[] = {
    { { ContextTag(0), "targetTemperature", ItemType::kDefault }, kInvalidNodeIndex },      // TemperatureControl::temperature
    { { ContextTag(1), "targetTemperatureLevel", ItemType::kDefault }, kInvalidNodeIndex }, // TemperatureControl::INT8U
};

const Entry<ItemInfo> _RefrigeratorAlarm[] = {
    { { AttributeTag(0), "mask", ItemType::kBitmap }, kInvalidNodeIndex },                   // RefrigeratorAlarm::AlarmMap
    { { AttributeTag(2), "state", ItemType::kBitmap }, kInvalidNodeIndex },                  // RefrigeratorAlarm::AlarmMap
    { { AttributeTag(3), "supported", ItemType::kBitmap }, kInvalidNodeIndex },              // RefrigeratorAlarm::AlarmMap
    { { AttributeTag(65528), "generatedCommandList", ItemType::kList }, kInvalidNodeIndex }, // RefrigeratorAlarm::command_id[]
    { { AttributeTag(65529), "acceptedCommandList", ItemType::kList }, kInvalidNodeIndex },  // RefrigeratorAlarm::command_id[]
    { { AttributeTag(65530), "eventList", ItemType::kList }, kInvalidNodeIndex },            // RefrigeratorAlarm::event_id[]
    { { AttributeTag(65531), "attributeList", ItemType::kList }, kInvalidNodeIndex },        // RefrigeratorAlarm::attrib_id[]
    { { AttributeTag(65532), "featureMap", ItemType::kDefault }, kInvalidNodeIndex },        // RefrigeratorAlarm::bitmap32
    { { AttributeTag(65533), "clusterRevision", ItemType::kDefault }, kInvalidNodeIndex },   // RefrigeratorAlarm::int16u
    { { EventTag(0), "Notify", ItemType::kDefault }, 289 },                                  // RefrigeratorAlarm::Notify
};

const Entry<ItemInfo> _RefrigeratorAlarm_Notify[] = {
    { { ContextTag(0), "active", ItemType::kBitmap }, kInvalidNodeIndex },   // RefrigeratorAlarm::AlarmMap
    { { ContextTag(1), "inactive", ItemType::kBitmap }, kInvalidNodeIndex }, // RefrigeratorAlarm::AlarmMap
    { { ContextTag(2), "state", ItemType::kBitmap }, kInvalidNodeIndex },    // RefrigeratorAlarm::AlarmMap
    { { ContextTag(3), "mask", ItemType::kBitmap }, kInvalidNodeIndex },     // RefrigeratorAlarm::AlarmMap
};

const Entry<ItemInfo> _DishwasherMode[] = {
    { { AttributeTag(0), "supportedModes", ItemType::kList }, 295 },                         // DishwasherMode::ModeOptionStruct[]
    { { AttributeTag(1), "currentMode", ItemType::kDefault }, kInvalidNodeIndex },           // DishwasherMode::int8u
    { { AttributeTag(2), "startUpMode", ItemType::kDefault }, kInvalidNodeIndex },           // DishwasherMode::int8u
    { { AttributeTag(3), "onMode", ItemType::kDefault }, kInvalidNodeIndex },                // DishwasherMode::int8u
    { { AttributeTag(65528), "generatedCommandList", ItemType::kList }, kInvalidNodeIndex }, // DishwasherMode::command_id[]
    { { AttributeTag(65529), "acceptedCommandList", ItemType::kList }, kInvalidNodeIndex },  // DishwasherMode::command_id[]
    { { AttributeTag(65530), "eventList", ItemType::kList }, kInvalidNodeIndex },            // DishwasherMode::event_id[]
    { { AttributeTag(65531), "attributeList", ItemType::kList }, kInvalidNodeIndex },        // DishwasherMode::attrib_id[]
    { { AttributeTag(65532), "featureMap", ItemType::kDefault }, kInvalidNodeIndex },        // DishwasherMode::bitmap32
    { { AttributeTag(65533), "clusterRevision", ItemType::kDefault }, kInvalidNodeIndex },   // DishwasherMode::int16u
    { { CommandTag(0), "ChangeToModeRequest", ItemType::kDefault }, 293 },  // DishwasherMode::ChangeToMode::ChangeToModeRequest
    { { CommandTag(1), "ChangeToModeResponse", ItemType::kDefault }, 294 }, // DishwasherMode::ChangeToModeResponse
};

const Entry<ItemInfo> _DishwasherMode_ModeTagStruct[] = {
    { { ContextTag(0), "mfgCode", ItemType::kDefault }, kInvalidNodeIndex }, // DishwasherMode::vendor_id
    { { ContextTag(1), "value", ItemType::kDefault }, kInvalidNodeIndex },   // DishwasherMode::enum16
};

const Entry<ItemInfo> _DishwasherMode_ModeOptionStruct[] = {
    { { ContextTag(0), "label", ItemType::kDefault }, kInvalidNodeIndex }, // DishwasherMode::char_string
    { { ContextTag(1), "mode", ItemType::kDefault }, kInvalidNodeIndex },  // DishwasherMode::int8u
    { { ContextTag(2), "modeTags", ItemType::kList }, 296 },               // DishwasherMode::ModeTagStruct[]
};

const Entry<ItemInfo> _DishwasherMode_ChangeToModeRequest[] = {
    { { ContextTag(0), "newMode", ItemType::kDefault }, kInvalidNodeIndex }, // DishwasherMode::INT8U
};

const Entry<ItemInfo> _DishwasherMode_ChangeToModeResponse[] = {
    { { ContextTag(0), "status", ItemType::kDefault }, kInvalidNodeIndex },     // DishwasherMode::ENUM8
    { { ContextTag(1), "statusText", ItemType::kDefault }, kInvalidNodeIndex }, // DishwasherMode::CHAR_STRING
};

const Entry<ItemInfo> _DishwasherMode_ModeOptionStruct_list_[] = {
    { { AnonymousTag(), "[]", ItemType::kDefault }, 292 }, // DishwasherMode_ModeOptionStruct[]
};

const Entry<ItemInfo> _DishwasherMode_ModeTagStruct_list_[] = {
    { { AnonymousTag(), "[]", ItemType::kDefault }, 291 }, // DishwasherMode_ModeTagStruct[]
};

const Entry<ItemInfo> _AirQuality[] = {
    { { AttributeTag(0), "airQuality", ItemType::kEnum }, kInvalidNodeIndex },               // AirQuality::AirQualityEnum
    { { AttributeTag(65528), "generatedCommandList", ItemType::kList }, kInvalidNodeIndex }, // AirQuality::command_id[]
    { { AttributeTag(65529), "acceptedCommandList", ItemType::kList }, kInvalidNodeIndex },  // AirQuality::command_id[]
    { { AttributeTag(65530), "eventList", ItemType::kList }, kInvalidNodeIndex },            // AirQuality::event_id[]
    { { AttributeTag(65531), "attributeList", ItemType::kList }, kInvalidNodeIndex },        // AirQuality::attrib_id[]
    { { AttributeTag(65532), "featureMap", ItemType::kDefault }, kInvalidNodeIndex },        // AirQuality::bitmap32
    { { AttributeTag(65533), "clusterRevision", ItemType::kDefault }, kInvalidNodeIndex },   // AirQuality::int16u
};

const Entry<ItemInfo> _SmokeCoAlarm[] = {
    { { AttributeTag(0), "expressedState", ItemType::kEnum }, kInvalidNodeIndex },           // SmokeCoAlarm::ExpressedStateEnum
    { { AttributeTag(1), "smokeState", ItemType::kEnum }, kInvalidNodeIndex },               // SmokeCoAlarm::AlarmStateEnum
    { { AttributeTag(2), "COState", ItemType::kEnum }, kInvalidNodeIndex },                  // SmokeCoAlarm::AlarmStateEnum
    { { AttributeTag(3), "batteryAlert", ItemType::kEnum }, kInvalidNodeIndex },             // SmokeCoAlarm::AlarmStateEnum
    { { AttributeTag(4), "deviceMuted", ItemType::kEnum }, kInvalidNodeIndex },              // SmokeCoAlarm::MuteStateEnum
    { { AttributeTag(5), "testInProgress", ItemType::kDefault }, kInvalidNodeIndex },        // SmokeCoAlarm::boolean
    { { AttributeTag(6), "hardwareFaultAlert", ItemType::kDefault }, kInvalidNodeIndex },    // SmokeCoAlarm::boolean
    { { AttributeTag(7), "endOfServiceAlert", ItemType::kEnum }, kInvalidNodeIndex },        // SmokeCoAlarm::EndOfServiceEnum
    { { AttributeTag(8), "interconnectSmokeAlarm", ItemType::kEnum }, kInvalidNodeIndex },   // SmokeCoAlarm::AlarmStateEnum
    { { AttributeTag(9), "interconnectCOAlarm", ItemType::kEnum }, kInvalidNodeIndex },      // SmokeCoAlarm::AlarmStateEnum
    { { AttributeTag(10), "contaminationState", ItemType::kEnum }, kInvalidNodeIndex },      // SmokeCoAlarm::ContaminationStateEnum
    { { AttributeTag(11), "sensitivityLevel", ItemType::kEnum }, kInvalidNodeIndex },        // SmokeCoAlarm::SensitivityEnum
    { { AttributeTag(12), "expiryDate", ItemType::kDefault }, kInvalidNodeIndex },           // SmokeCoAlarm::epoch_s
    { { AttributeTag(65528), "generatedCommandList", ItemType::kList }, kInvalidNodeIndex }, // SmokeCoAlarm::command_id[]
    { { AttributeTag(65529), "acceptedCommandList", ItemType::kList }, kInvalidNodeIndex },  // SmokeCoAlarm::command_id[]
    { { AttributeTag(65530), "eventList", ItemType::kList }, kInvalidNodeIndex },            // SmokeCoAlarm::event_id[]
    { { AttributeTag(65531), "attributeList", ItemType::kList }, kInvalidNodeIndex },        // SmokeCoAlarm::attrib_id[]
    { { AttributeTag(65532), "featureMap", ItemType::kDefault }, kInvalidNodeIndex },        // SmokeCoAlarm::bitmap32
    { { AttributeTag(65533), "clusterRevision", ItemType::kDefault }, kInvalidNodeIndex },   // SmokeCoAlarm::int16u
    { { EventTag(0), "SmokeAlarm", ItemType::kDefault }, 299 },                              // SmokeCoAlarm::SmokeAlarm
    { { EventTag(1), "COAlarm", ItemType::kDefault }, 300 },                                 // SmokeCoAlarm::COAlarm
    { { EventTag(2), "LowBattery", ItemType::kDefault }, 301 },                              // SmokeCoAlarm::LowBattery
    { { EventTag(3), "HardwareFault", ItemType::kDefault }, 302 },                           // SmokeCoAlarm::HardwareFault
    { { EventTag(4), "EndOfService", ItemType::kDefault }, 303 },                            // SmokeCoAlarm::EndOfService
    { { EventTag(5), "SelfTestComplete", ItemType::kDefault }, 304 },                        // SmokeCoAlarm::SelfTestComplete
    { { EventTag(6), "AlarmMuted", ItemType::kDefault }, 305 },                              // SmokeCoAlarm::AlarmMuted
    { { EventTag(7), "MuteEnded", ItemType::kDefault }, 306 },                               // SmokeCoAlarm::MuteEnded
    { { EventTag(8), "InterconnectSmokeAlarm", ItemType::kDefault }, 307 },                  // SmokeCoAlarm::InterconnectSmokeAlarm
    { { EventTag(9), "InterconnectCOAlarm", ItemType::kDefault }, 308 },                     // SmokeCoAlarm::InterconnectCOAlarm
    { { EventTag(10), "AllClear", ItemType::kDefault }, 309 },                               // SmokeCoAlarm::AllClear
};

const Entry<ItemInfo> _SmokeCoAlarm_SmokeAlarm[] = {};

const Entry<ItemInfo> _SmokeCoAlarm_COAlarm[] = {};

const Entry<ItemInfo> _SmokeCoAlarm_LowBattery[] = {};

const Entry<ItemInfo> _SmokeCoAlarm_HardwareFault[] = {};

const Entry<ItemInfo> _SmokeCoAlarm_EndOfService[] = {};

const Entry<ItemInfo> _SmokeCoAlarm_SelfTestComplete[] = {};

const Entry<ItemInfo> _SmokeCoAlarm_AlarmMuted[] = {};

const Entry<ItemInfo> _SmokeCoAlarm_MuteEnded[] = {};

const Entry<ItemInfo> _SmokeCoAlarm_InterconnectSmokeAlarm[] = {};

const Entry<ItemInfo> _SmokeCoAlarm_InterconnectCOAlarm[] = {};

const Entry<ItemInfo> _SmokeCoAlarm_AllClear[] = {};

const Entry<ItemInfo> _DishwasherAlarm[] = {
    { { AttributeTag(0), "mask", ItemType::kBitmap }, kInvalidNodeIndex },                   // DishwasherAlarm::AlarmMap
    { { AttributeTag(1), "latch", ItemType::kBitmap }, kInvalidNodeIndex },                  // DishwasherAlarm::AlarmMap
    { { AttributeTag(2), "state", ItemType::kBitmap }, kInvalidNodeIndex },                  // DishwasherAlarm::AlarmMap
    { { AttributeTag(3), "supported", ItemType::kBitmap }, kInvalidNodeIndex },              // DishwasherAlarm::AlarmMap
    { { AttributeTag(65528), "generatedCommandList", ItemType::kList }, kInvalidNodeIndex }, // DishwasherAlarm::command_id[]
    { { AttributeTag(65529), "acceptedCommandList", ItemType::kList }, kInvalidNodeIndex },  // DishwasherAlarm::command_id[]
    { { AttributeTag(65530), "eventList", ItemType::kList }, kInvalidNodeIndex },            // DishwasherAlarm::event_id[]
    { { AttributeTag(65531), "attributeList", ItemType::kList }, kInvalidNodeIndex },        // DishwasherAlarm::attrib_id[]
    { { AttributeTag(65532), "featureMap", ItemType::kDefault }, kInvalidNodeIndex },        // DishwasherAlarm::bitmap32
    { { AttributeTag(65533), "clusterRevision", ItemType::kDefault }, kInvalidNodeIndex },   // DishwasherAlarm::int16u
    { { EventTag(0), "Notify", ItemType::kDefault }, 313 },                                  // DishwasherAlarm::Notify
    { { CommandTag(0), "ResetRequest", ItemType::kDefault }, 311 },                          // DishwasherAlarm::Reset::ResetRequest
    { { CommandTag(1), "ModifyEnabledAlarmsRequest", ItemType::kDefault },
      312 }, // DishwasherAlarm::ModifyEnabledAlarms::ModifyEnabledAlarmsRequest
};

const Entry<ItemInfo> _DishwasherAlarm_ResetRequest[] = {
    { { ContextTag(0), "alarms", ItemType::kBitmap }, kInvalidNodeIndex }, // DishwasherAlarm::AlarmMap
};

const Entry<ItemInfo> _DishwasherAlarm_ModifyEnabledAlarmsRequest[] = {
    { { ContextTag(0), "mask", ItemType::kBitmap }, kInvalidNodeIndex }, // DishwasherAlarm::AlarmMap
};

const Entry<ItemInfo> _DishwasherAlarm_Notify[] = {
    { { ContextTag(0), "active", ItemType::kBitmap }, kInvalidNodeIndex },   // DishwasherAlarm::AlarmMap
    { { ContextTag(1), "inactive", ItemType::kBitmap }, kInvalidNodeIndex }, // DishwasherAlarm::AlarmMap
    { { ContextTag(2), "state", ItemType::kBitmap }, kInvalidNodeIndex },    // DishwasherAlarm::AlarmMap
    { { ContextTag(3), "mask", ItemType::kBitmap }, kInvalidNodeIndex },     // DishwasherAlarm::AlarmMap
};

const Entry<ItemInfo> _OperationalState[] = {
    { { AttributeTag(0), "phaseList", ItemType::kList }, kInvalidNodeIndex },        // OperationalState::CHAR_STRING[]
    { { AttributeTag(1), "currentPhase", ItemType::kDefault }, kInvalidNodeIndex },  // OperationalState::int8u
    { { AttributeTag(2), "countdownTime", ItemType::kDefault }, kInvalidNodeIndex }, // OperationalState::elapsed_s
    { { AttributeTag(3), "operationalStateList", ItemType::kList }, 320 },           // OperationalState::OperationalStateStruct[]
    { { AttributeTag(4), "operationalState", ItemType::kDefault }, 316 },            // OperationalState::OperationalStateStruct
    { { AttributeTag(5), "operationalError", ItemType::kDefault }, 315 },            // OperationalState::ErrorStateStruct
    { { AttributeTag(65528), "generatedCommandList", ItemType::kList }, kInvalidNodeIndex }, // OperationalState::command_id[]
    { { AttributeTag(65529), "acceptedCommandList", ItemType::kList }, kInvalidNodeIndex },  // OperationalState::command_id[]
    { { AttributeTag(65530), "eventList", ItemType::kList }, kInvalidNodeIndex },            // OperationalState::event_id[]
    { { AttributeTag(65531), "attributeList", ItemType::kList }, kInvalidNodeIndex },        // OperationalState::attrib_id[]
    { { AttributeTag(65532), "featureMap", ItemType::kDefault }, kInvalidNodeIndex },        // OperationalState::bitmap32
    { { AttributeTag(65533), "clusterRevision", ItemType::kDefault }, kInvalidNodeIndex },   // OperationalState::int16u
    { { EventTag(0), "OperationalError", ItemType::kDefault }, 318 },                        // OperationalState::OperationalError
    { { EventTag(1), "OperationCompletion", ItemType::kDefault }, 319 },          // OperationalState::OperationCompletion
    { { CommandTag(4), "OperationalCommandResponse", ItemType::kDefault }, 317 }, // OperationalState::OperationalCommandResponse
};

const Entry<ItemInfo> _OperationalState_ErrorStateStruct[] = {
    { { ContextTag(0), "errorStateID", ItemType::kDefault }, kInvalidNodeIndex },      // OperationalState::enum8
    { { ContextTag(1), "errorStateLabel", ItemType::kDefault }, kInvalidNodeIndex },   // OperationalState::char_string
    { { ContextTag(2), "errorStateDetails", ItemType::kDefault }, kInvalidNodeIndex }, // OperationalState::char_string
};

const Entry<ItemInfo> _OperationalState_OperationalStateStruct[] = {
    { { ContextTag(0), "operationalStateID", ItemType::kDefault }, kInvalidNodeIndex },    // OperationalState::enum8
    { { ContextTag(1), "operationalStateLabel", ItemType::kDefault }, kInvalidNodeIndex }, // OperationalState::char_string
};

const Entry<ItemInfo> _OperationalState_OperationalCommandResponse[] = {
    { { ContextTag(0), "commandResponseState", ItemType::kDefault }, 315 }, // OperationalState::ErrorStateStruct
};

const Entry<ItemInfo> _OperationalState_OperationalError[] = {
    { { ContextTag(0), "errorState", ItemType::kDefault }, 315 }, // OperationalState::ErrorStateStruct
};

const Entry<ItemInfo> _OperationalState_OperationCompletion[] = {
    { { ContextTag(0), "completionErrorCode", ItemType::kDefault }, kInvalidNodeIndex },  // OperationalState::ENUM8
    { { ContextTag(1), "totalOperationalTime", ItemType::kDefault }, kInvalidNodeIndex }, // OperationalState::elapsed_s
    { { ContextTag(2), "pausedTime", ItemType::kDefault }, kInvalidNodeIndex },           // OperationalState::elapsed_s
};

const Entry<ItemInfo> _OperationalState_OperationalStateStruct_list_[] = {
    { { AnonymousTag(), "[]", ItemType::kDefault }, 316 }, // OperationalState_OperationalStateStruct[]
};

const Entry<ItemInfo> _RvcOperationalState[] = {
    { { AttributeTag(0), "phaseList", ItemType::kList }, kInvalidNodeIndex },        // RvcOperationalState::CHAR_STRING[]
    { { AttributeTag(1), "currentPhase", ItemType::kDefault }, kInvalidNodeIndex },  // RvcOperationalState::int8u
    { { AttributeTag(2), "countdownTime", ItemType::kDefault }, kInvalidNodeIndex }, // RvcOperationalState::elapsed_s
    { { AttributeTag(3), "operationalStateList", ItemType::kList }, 327 }, // RvcOperationalState::OperationalStateStruct[]
    { { AttributeTag(4), "operationalState", ItemType::kDefault }, 323 },  // RvcOperationalState::OperationalStateStruct
    { { AttributeTag(5), "operationalError", ItemType::kDefault }, 322 },  // RvcOperationalState::ErrorStateStruct
    { { AttributeTag(65528), "generatedCommandList", ItemType::kList }, kInvalidNodeIndex }, // RvcOperationalState::command_id[]
    { { AttributeTag(65529), "acceptedCommandList", ItemType::kList }, kInvalidNodeIndex },  // RvcOperationalState::command_id[]
    { { AttributeTag(65530), "eventList", ItemType::kList }, kInvalidNodeIndex },            // RvcOperationalState::event_id[]
    { { AttributeTag(65531), "attributeList", ItemType::kList }, kInvalidNodeIndex },        // RvcOperationalState::attrib_id[]
    { { AttributeTag(65532), "featureMap", ItemType::kDefault }, kInvalidNodeIndex },        // RvcOperationalState::bitmap32
    { { AttributeTag(65533), "clusterRevision", ItemType::kDefault }, kInvalidNodeIndex },   // RvcOperationalState::int16u
    { { EventTag(0), "OperationalError", ItemType::kDefault }, 325 },             // RvcOperationalState::OperationalError
    { { EventTag(1), "OperationCompletion", ItemType::kDefault }, 326 },          // RvcOperationalState::OperationCompletion
    { { CommandTag(4), "OperationalCommandResponse", ItemType::kDefault }, 324 }, // RvcOperationalState::OperationalCommandResponse
};

const Entry<ItemInfo> _RvcOperationalState_ErrorStateStruct[] = {
    { { ContextTag(0), "errorStateID", ItemType::kDefault }, kInvalidNodeIndex },      // RvcOperationalState::enum8
    { { ContextTag(1), "errorStateLabel", ItemType::kDefault }, kInvalidNodeIndex },   // RvcOperationalState::char_string
    { { ContextTag(2), "errorStateDetails", ItemType::kDefault }, kInvalidNodeIndex }, // RvcOperationalState::char_string
};

const Entry<ItemInfo> _RvcOperationalState_OperationalStateStruct[] = {
    { { ContextTag(0), "operationalStateID", ItemType::kDefault }, kInvalidNodeIndex },    // RvcOperationalState::enum8
    { { ContextTag(1), "operationalStateLabel", ItemType::kDefault }, kInvalidNodeIndex }, // RvcOperationalState::char_string
};

const Entry<ItemInfo> _RvcOperationalState_OperationalCommandResponse[] = {
    { { ContextTag(0), "commandResponseState", ItemType::kDefault }, 322 }, // RvcOperationalState::ErrorStateStruct
};

const Entry<ItemInfo> _RvcOperationalState_OperationalError[] = {
    { { ContextTag(0), "errorState", ItemType::kDefault }, 322 }, // RvcOperationalState::ErrorStateStruct
};

const Entry<ItemInfo> _RvcOperationalState_OperationCompletion[] = {
    { { ContextTag(0), "completionErrorCode", ItemType::kDefault }, kInvalidNodeIndex },  // RvcOperationalState::ENUM8
    { { ContextTag(1), "totalOperationalTime", ItemType::kDefault }, kInvalidNodeIndex }, // RvcOperationalState::elapsed_s
    { { ContextTag(2), "pausedTime", ItemType::kDefault }, kInvalidNodeIndex },           // RvcOperationalState::elapsed_s
};

const Entry<ItemInfo> _RvcOperationalState_OperationalStateStruct_list_[] = {
    { { AnonymousTag(), "[]", ItemType::kDefault }, 323 }, // RvcOperationalState_OperationalStateStruct[]
};

const Entry<ItemInfo> _HepaFilterMonitoring[] = {
    { { AttributeTag(0), "condition", ItemType::kDefault }, kInvalidNodeIndex }, // HepaFilterMonitoring::percent
    { { AttributeTag(1), "degradationDirection", ItemType::kEnum },
      kInvalidNodeIndex }, // HepaFilterMonitoring::DegradationDirectionEnum
    { { AttributeTag(2), "changeIndication", ItemType::kEnum }, kInvalidNodeIndex }, // HepaFilterMonitoring::ChangeIndicationEnum
    { { AttributeTag(3), "inPlaceIndicator", ItemType::kDefault }, kInvalidNodeIndex },      // HepaFilterMonitoring::boolean
    { { AttributeTag(65528), "generatedCommandList", ItemType::kList }, kInvalidNodeIndex }, // HepaFilterMonitoring::command_id[]
    { { AttributeTag(65529), "acceptedCommandList", ItemType::kList }, kInvalidNodeIndex },  // HepaFilterMonitoring::command_id[]
    { { AttributeTag(65530), "eventList", ItemType::kList }, kInvalidNodeIndex },            // HepaFilterMonitoring::event_id[]
    { { AttributeTag(65531), "attributeList", ItemType::kList }, kInvalidNodeIndex },        // HepaFilterMonitoring::attrib_id[]
    { { AttributeTag(65532), "featureMap", ItemType::kDefault }, kInvalidNodeIndex },        // HepaFilterMonitoring::bitmap32
    { { AttributeTag(65533), "clusterRevision", ItemType::kDefault }, kInvalidNodeIndex },   // HepaFilterMonitoring::int16u
};

const Entry<ItemInfo> _ActivatedCarbonFilterMonitoring[] = {
    { { AttributeTag(0), "condition", ItemType::kDefault }, kInvalidNodeIndex }, // ActivatedCarbonFilterMonitoring::percent
    { { AttributeTag(1), "degradationDirection", ItemType::kEnum },
      kInvalidNodeIndex }, // ActivatedCarbonFilterMonitoring::DegradationDirectionEnum
    { { AttributeTag(2), "changeIndication", ItemType::kEnum },
      kInvalidNodeIndex }, // ActivatedCarbonFilterMonitoring::ChangeIndicationEnum
    { { AttributeTag(3), "inPlaceIndicator", ItemType::kDefault }, kInvalidNodeIndex }, // ActivatedCarbonFilterMonitoring::boolean
    { { AttributeTag(65528), "generatedCommandList", ItemType::kList },
      kInvalidNodeIndex }, // ActivatedCarbonFilterMonitoring::command_id[]
    { { AttributeTag(65529), "acceptedCommandList", ItemType::kList },
      kInvalidNodeIndex },                                                        // ActivatedCarbonFilterMonitoring::command_id[]
    { { AttributeTag(65530), "eventList", ItemType::kList }, kInvalidNodeIndex }, // ActivatedCarbonFilterMonitoring::event_id[]
    { { AttributeTag(65531), "attributeList", ItemType::kList },
      kInvalidNodeIndex }, // ActivatedCarbonFilterMonitoring::attrib_id[]
    { { AttributeTag(65532), "featureMap", ItemType::kDefault }, kInvalidNodeIndex }, // ActivatedCarbonFilterMonitoring::bitmap32
    { { AttributeTag(65533), "clusterRevision", ItemType::kDefault },
      kInvalidNodeIndex }, // ActivatedCarbonFilterMonitoring::int16u
};

const Entry<ItemInfo> _CeramicFilterMonitoring[] = {
    { { AttributeTag(0), "condition", ItemType::kDefault }, kInvalidNodeIndex }, // CeramicFilterMonitoring::percent
    { { AttributeTag(1), "degradationDirection", ItemType::kEnum },
      kInvalidNodeIndex }, // CeramicFilterMonitoring::DegradationDirectionEnum
    { { AttributeTag(2), "changeIndication", ItemType::kEnum },
      kInvalidNodeIndex }, // CeramicFilterMonitoring::ChangeIndicationEnum
    { { AttributeTag(3), "inPlaceIndicator", ItemType::kDefault }, kInvalidNodeIndex }, // CeramicFilterMonitoring::boolean
    { { AttributeTag(65528), "generatedCommandList", ItemType::kList },
      kInvalidNodeIndex },                                                                  // CeramicFilterMonitoring::command_id[]
    { { AttributeTag(65529), "acceptedCommandList", ItemType::kList }, kInvalidNodeIndex }, // CeramicFilterMonitoring::command_id[]
    { { AttributeTag(65530), "eventList", ItemType::kList }, kInvalidNodeIndex },           // CeramicFilterMonitoring::event_id[]
    { { AttributeTag(65531), "attributeList", ItemType::kList }, kInvalidNodeIndex },       // CeramicFilterMonitoring::attrib_id[]
    { { AttributeTag(65532), "featureMap", ItemType::kDefault }, kInvalidNodeIndex },       // CeramicFilterMonitoring::bitmap32
    { { AttributeTag(65533), "clusterRevision", ItemType::kDefault }, kInvalidNodeIndex },  // CeramicFilterMonitoring::int16u
};

const Entry<ItemInfo> _ElectrostaticFilterMonitoring[] = {
    { { AttributeTag(0), "condition", ItemType::kDefault }, kInvalidNodeIndex }, // ElectrostaticFilterMonitoring::percent
    { { AttributeTag(1), "degradationDirection", ItemType::kEnum },
      kInvalidNodeIndex }, // ElectrostaticFilterMonitoring::DegradationDirectionEnum
    { { AttributeTag(2), "changeIndication", ItemType::kEnum },
      kInvalidNodeIndex }, // ElectrostaticFilterMonitoring::ChangeIndicationEnum
    { { AttributeTag(3), "inPlaceIndicator", ItemType::kDefault }, kInvalidNodeIndex }, // ElectrostaticFilterMonitoring::boolean
    { { AttributeTag(65528), "generatedCommandList", ItemType::kList },
      kInvalidNodeIndex }, // ElectrostaticFilterMonitoring::command_id[]
    { { AttributeTag(65529), "acceptedCommandList", ItemType::kList },
      kInvalidNodeIndex },                                                            // ElectrostaticFilterMonitoring::command_id[]
    { { AttributeTag(65530), "eventList", ItemType::kList }, kInvalidNodeIndex },     // ElectrostaticFilterMonitoring::event_id[]
    { { AttributeTag(65531), "attributeList", ItemType::kList }, kInvalidNodeIndex }, // ElectrostaticFilterMonitoring::attrib_id[]
    { { AttributeTag(65532), "featureMap", ItemType::kDefault }, kInvalidNodeIndex }, // ElectrostaticFilterMonitoring::bitmap32
    { { AttributeTag(65533), "clusterRevision", ItemType::kDefault }, kInvalidNodeIndex }, // ElectrostaticFilterMonitoring::int16u
};

const Entry<ItemInfo> _UvFilterMonitoring[] = {
    { { AttributeTag(0), "condition", ItemType::kDefault }, kInvalidNodeIndex }, // UvFilterMonitoring::percent
    { { AttributeTag(1), "degradationDirection", ItemType::kEnum },
      kInvalidNodeIndex },                                                           // UvFilterMonitoring::DegradationDirectionEnum
    { { AttributeTag(2), "changeIndication", ItemType::kEnum }, kInvalidNodeIndex }, // UvFilterMonitoring::ChangeIndicationEnum
    { { AttributeTag(3), "inPlaceIndicator", ItemType::kDefault }, kInvalidNodeIndex },      // UvFilterMonitoring::boolean
    { { AttributeTag(65528), "generatedCommandList", ItemType::kList }, kInvalidNodeIndex }, // UvFilterMonitoring::command_id[]
    { { AttributeTag(65529), "acceptedCommandList", ItemType::kList }, kInvalidNodeIndex },  // UvFilterMonitoring::command_id[]
    { { AttributeTag(65530), "eventList", ItemType::kList }, kInvalidNodeIndex },            // UvFilterMonitoring::event_id[]
    { { AttributeTag(65531), "attributeList", ItemType::kList }, kInvalidNodeIndex },        // UvFilterMonitoring::attrib_id[]
    { { AttributeTag(65532), "featureMap", ItemType::kDefault }, kInvalidNodeIndex },        // UvFilterMonitoring::bitmap32
    { { AttributeTag(65533), "clusterRevision", ItemType::kDefault }, kInvalidNodeIndex },   // UvFilterMonitoring::int16u
};

const Entry<ItemInfo> _IonizingFilterMonitoring[] = {
    { { AttributeTag(0), "condition", ItemType::kDefault }, kInvalidNodeIndex }, // IonizingFilterMonitoring::percent
    { { AttributeTag(1), "degradationDirection", ItemType::kEnum },
      kInvalidNodeIndex }, // IonizingFilterMonitoring::DegradationDirectionEnum
    { { AttributeTag(2), "changeIndication", ItemType::kEnum },
      kInvalidNodeIndex }, // IonizingFilterMonitoring::ChangeIndicationEnum
    { { AttributeTag(3), "inPlaceIndicator", ItemType::kDefault }, kInvalidNodeIndex }, // IonizingFilterMonitoring::boolean
    { { AttributeTag(65528), "generatedCommandList", ItemType::kList },
      kInvalidNodeIndex }, // IonizingFilterMonitoring::command_id[]
    { { AttributeTag(65529), "acceptedCommandList", ItemType::kList },
      kInvalidNodeIndex },                                                                 // IonizingFilterMonitoring::command_id[]
    { { AttributeTag(65530), "eventList", ItemType::kList }, kInvalidNodeIndex },          // IonizingFilterMonitoring::event_id[]
    { { AttributeTag(65531), "attributeList", ItemType::kList }, kInvalidNodeIndex },      // IonizingFilterMonitoring::attrib_id[]
    { { AttributeTag(65532), "featureMap", ItemType::kDefault }, kInvalidNodeIndex },      // IonizingFilterMonitoring::bitmap32
    { { AttributeTag(65533), "clusterRevision", ItemType::kDefault }, kInvalidNodeIndex }, // IonizingFilterMonitoring::int16u
};

const Entry<ItemInfo> _ZeoliteFilterMonitoring[] = {
    { { AttributeTag(0), "condition", ItemType::kDefault }, kInvalidNodeIndex }, // ZeoliteFilterMonitoring::percent
    { { AttributeTag(1), "degradationDirection", ItemType::kEnum },
      kInvalidNodeIndex }, // ZeoliteFilterMonitoring::DegradationDirectionEnum
    { { AttributeTag(2), "changeIndication", ItemType::kEnum },
      kInvalidNodeIndex }, // ZeoliteFilterMonitoring::ChangeIndicationEnum
    { { AttributeTag(3), "inPlaceIndicator", ItemType::kDefault }, kInvalidNodeIndex }, // ZeoliteFilterMonitoring::boolean
    { { AttributeTag(65528), "generatedCommandList", ItemType::kList },
      kInvalidNodeIndex },                                                                  // ZeoliteFilterMonitoring::command_id[]
    { { AttributeTag(65529), "acceptedCommandList", ItemType::kList }, kInvalidNodeIndex }, // ZeoliteFilterMonitoring::command_id[]
    { { AttributeTag(65530), "eventList", ItemType::kList }, kInvalidNodeIndex },           // ZeoliteFilterMonitoring::event_id[]
    { { AttributeTag(65531), "attributeList", ItemType::kList }, kInvalidNodeIndex },       // ZeoliteFilterMonitoring::attrib_id[]
    { { AttributeTag(65532), "featureMap", ItemType::kDefault }, kInvalidNodeIndex },       // ZeoliteFilterMonitoring::bitmap32
    { { AttributeTag(65533), "clusterRevision", ItemType::kDefault }, kInvalidNodeIndex },  // ZeoliteFilterMonitoring::int16u
};

const Entry<ItemInfo> _OzoneFilterMonitoring[] = {
    { { AttributeTag(0), "condition", ItemType::kDefault }, kInvalidNodeIndex }, // OzoneFilterMonitoring::percent
    { { AttributeTag(1), "degradationDirection", ItemType::kEnum },
      kInvalidNodeIndex }, // OzoneFilterMonitoring::DegradationDirectionEnum
    { { AttributeTag(2), "changeIndication", ItemType::kEnum }, kInvalidNodeIndex }, // OzoneFilterMonitoring::ChangeIndicationEnum
    { { AttributeTag(3), "inPlaceIndicator", ItemType::kDefault }, kInvalidNodeIndex },      // OzoneFilterMonitoring::boolean
    { { AttributeTag(65528), "generatedCommandList", ItemType::kList }, kInvalidNodeIndex }, // OzoneFilterMonitoring::command_id[]
    { { AttributeTag(65529), "acceptedCommandList", ItemType::kList }, kInvalidNodeIndex },  // OzoneFilterMonitoring::command_id[]
    { { AttributeTag(65530), "eventList", ItemType::kList }, kInvalidNodeIndex },            // OzoneFilterMonitoring::event_id[]
    { { AttributeTag(65531), "attributeList", ItemType::kList }, kInvalidNodeIndex },        // OzoneFilterMonitoring::attrib_id[]
    { { AttributeTag(65532), "featureMap", ItemType::kDefault }, kInvalidNodeIndex },        // OzoneFilterMonitoring::bitmap32
    { { AttributeTag(65533), "clusterRevision", ItemType::kDefault }, kInvalidNodeIndex },   // OzoneFilterMonitoring::int16u
};

const Entry<ItemInfo> _WaterTankMonitoring[] = {
    { { AttributeTag(0), "condition", ItemType::kDefault }, kInvalidNodeIndex }, // WaterTankMonitoring::percent
    { { AttributeTag(1), "degradationDirection", ItemType::kEnum },
      kInvalidNodeIndex }, // WaterTankMonitoring::DegradationDirectionEnum
    { { AttributeTag(2), "changeIndication", ItemType::kEnum }, kInvalidNodeIndex },    // WaterTankMonitoring::ChangeIndicationEnum
    { { AttributeTag(3), "inPlaceIndicator", ItemType::kDefault }, kInvalidNodeIndex }, // WaterTankMonitoring::boolean
    { { AttributeTag(65528), "generatedCommandList", ItemType::kList }, kInvalidNodeIndex }, // WaterTankMonitoring::command_id[]
    { { AttributeTag(65529), "acceptedCommandList", ItemType::kList }, kInvalidNodeIndex },  // WaterTankMonitoring::command_id[]
    { { AttributeTag(65530), "eventList", ItemType::kList }, kInvalidNodeIndex },            // WaterTankMonitoring::event_id[]
    { { AttributeTag(65531), "attributeList", ItemType::kList }, kInvalidNodeIndex },        // WaterTankMonitoring::attrib_id[]
    { { AttributeTag(65532), "featureMap", ItemType::kDefault }, kInvalidNodeIndex },        // WaterTankMonitoring::bitmap32
    { { AttributeTag(65533), "clusterRevision", ItemType::kDefault }, kInvalidNodeIndex },   // WaterTankMonitoring::int16u
};

const Entry<ItemInfo> _FuelTankMonitoring[] = {
    { { AttributeTag(0), "condition", ItemType::kDefault }, kInvalidNodeIndex }, // FuelTankMonitoring::percent
    { { AttributeTag(1), "degradationDirection", ItemType::kEnum },
      kInvalidNodeIndex },                                                           // FuelTankMonitoring::DegradationDirectionEnum
    { { AttributeTag(2), "changeIndication", ItemType::kEnum }, kInvalidNodeIndex }, // FuelTankMonitoring::ChangeIndicationEnum
    { { AttributeTag(3), "inPlaceIndicator", ItemType::kDefault }, kInvalidNodeIndex },      // FuelTankMonitoring::boolean
    { { AttributeTag(65528), "generatedCommandList", ItemType::kList }, kInvalidNodeIndex }, // FuelTankMonitoring::command_id[]
    { { AttributeTag(65529), "acceptedCommandList", ItemType::kList }, kInvalidNodeIndex },  // FuelTankMonitoring::command_id[]
    { { AttributeTag(65530), "eventList", ItemType::kList }, kInvalidNodeIndex },            // FuelTankMonitoring::event_id[]
    { { AttributeTag(65531), "attributeList", ItemType::kList }, kInvalidNodeIndex },        // FuelTankMonitoring::attrib_id[]
    { { AttributeTag(65532), "featureMap", ItemType::kDefault }, kInvalidNodeIndex },        // FuelTankMonitoring::bitmap32
    { { AttributeTag(65533), "clusterRevision", ItemType::kDefault }, kInvalidNodeIndex },   // FuelTankMonitoring::int16u
};

const Entry<ItemInfo> _InkCartridgeMonitoring[] = {
    { { AttributeTag(0), "condition", ItemType::kDefault }, kInvalidNodeIndex }, // InkCartridgeMonitoring::percent
    { { AttributeTag(1), "degradationDirection", ItemType::kEnum },
      kInvalidNodeIndex }, // InkCartridgeMonitoring::DegradationDirectionEnum
    { { AttributeTag(2), "changeIndication", ItemType::kEnum }, kInvalidNodeIndex }, // InkCartridgeMonitoring::ChangeIndicationEnum
    { { AttributeTag(3), "inPlaceIndicator", ItemType::kDefault }, kInvalidNodeIndex },      // InkCartridgeMonitoring::boolean
    { { AttributeTag(65528), "generatedCommandList", ItemType::kList }, kInvalidNodeIndex }, // InkCartridgeMonitoring::command_id[]
    { { AttributeTag(65529), "acceptedCommandList", ItemType::kList }, kInvalidNodeIndex },  // InkCartridgeMonitoring::command_id[]
    { { AttributeTag(65530), "eventList", ItemType::kList }, kInvalidNodeIndex },            // InkCartridgeMonitoring::event_id[]
    { { AttributeTag(65531), "attributeList", ItemType::kList }, kInvalidNodeIndex },        // InkCartridgeMonitoring::attrib_id[]
    { { AttributeTag(65532), "featureMap", ItemType::kDefault }, kInvalidNodeIndex },        // InkCartridgeMonitoring::bitmap32
    { { AttributeTag(65533), "clusterRevision", ItemType::kDefault }, kInvalidNodeIndex },   // InkCartridgeMonitoring::int16u
};

const Entry<ItemInfo> _TonerCartridgeMonitoring[] = {
    { { AttributeTag(0), "condition", ItemType::kDefault }, kInvalidNodeIndex }, // TonerCartridgeMonitoring::percent
    { { AttributeTag(1), "degradationDirection", ItemType::kEnum },
      kInvalidNodeIndex }, // TonerCartridgeMonitoring::DegradationDirectionEnum
    { { AttributeTag(2), "changeIndication", ItemType::kEnum },
      kInvalidNodeIndex }, // TonerCartridgeMonitoring::ChangeIndicationEnum
    { { AttributeTag(3), "inPlaceIndicator", ItemType::kDefault }, kInvalidNodeIndex }, // TonerCartridgeMonitoring::boolean
    { { AttributeTag(65528), "generatedCommandList", ItemType::kList },
      kInvalidNodeIndex }, // TonerCartridgeMonitoring::command_id[]
    { { AttributeTag(65529), "acceptedCommandList", ItemType::kList },
      kInvalidNodeIndex },                                                                 // TonerCartridgeMonitoring::command_id[]
    { { AttributeTag(65530), "eventList", ItemType::kList }, kInvalidNodeIndex },          // TonerCartridgeMonitoring::event_id[]
    { { AttributeTag(65531), "attributeList", ItemType::kList }, kInvalidNodeIndex },      // TonerCartridgeMonitoring::attrib_id[]
    { { AttributeTag(65532), "featureMap", ItemType::kDefault }, kInvalidNodeIndex },      // TonerCartridgeMonitoring::bitmap32
    { { AttributeTag(65533), "clusterRevision", ItemType::kDefault }, kInvalidNodeIndex }, // TonerCartridgeMonitoring::int16u
};

const Entry<ItemInfo> _DoorLock[] = {
    { { AttributeTag(0), "lockState", ItemType::kEnum }, kInvalidNodeIndex },                       // DoorLock::DlLockState
    { { AttributeTag(1), "lockType", ItemType::kEnum }, kInvalidNodeIndex },                        // DoorLock::DlLockType
    { { AttributeTag(2), "actuatorEnabled", ItemType::kDefault }, kInvalidNodeIndex },              // DoorLock::boolean
    { { AttributeTag(3), "doorState", ItemType::kEnum }, kInvalidNodeIndex },                       // DoorLock::DoorStateEnum
    { { AttributeTag(4), "doorOpenEvents", ItemType::kDefault }, kInvalidNodeIndex },               // DoorLock::int32u
    { { AttributeTag(5), "doorClosedEvents", ItemType::kDefault }, kInvalidNodeIndex },             // DoorLock::int32u
    { { AttributeTag(6), "openPeriod", ItemType::kDefault }, kInvalidNodeIndex },                   // DoorLock::int16u
    { { AttributeTag(17), "numberOfTotalUsersSupported", ItemType::kDefault }, kInvalidNodeIndex }, // DoorLock::int16u
    { { AttributeTag(18), "numberOfPINUsersSupported", ItemType::kDefault }, kInvalidNodeIndex },   // DoorLock::int16u
    { { AttributeTag(19), "numberOfRFIDUsersSupported", ItemType::kDefault }, kInvalidNodeIndex },  // DoorLock::int16u
    { { AttributeTag(20), "numberOfWeekDaySchedulesSupportedPerUser", ItemType::kDefault }, kInvalidNodeIndex }, // DoorLock::int8u
    { { AttributeTag(21), "numberOfYearDaySchedulesSupportedPerUser", ItemType::kDefault }, kInvalidNodeIndex }, // DoorLock::int8u
    { { AttributeTag(22), "numberOfHolidaySchedulesSupported", ItemType::kDefault }, kInvalidNodeIndex },        // DoorLock::int8u
    { { AttributeTag(23), "maxPINCodeLength", ItemType::kDefault }, kInvalidNodeIndex },                         // DoorLock::int8u
    { { AttributeTag(24), "minPINCodeLength", ItemType::kDefault }, kInvalidNodeIndex },                         // DoorLock::int8u
    { { AttributeTag(25), "maxRFIDCodeLength", ItemType::kDefault }, kInvalidNodeIndex },                        // DoorLock::int8u
    { { AttributeTag(26), "minRFIDCodeLength", ItemType::kDefault }, kInvalidNodeIndex },                        // DoorLock::int8u
    { { AttributeTag(27), "credentialRulesSupport", ItemType::kBitmap }, kInvalidNodeIndex }, // DoorLock::DlCredentialRuleMask
    { { AttributeTag(28), "numberOfCredentialsSupportedPerUser", ItemType::kDefault }, kInvalidNodeIndex }, // DoorLock::int8u
    { { AttributeTag(33), "language", ItemType::kDefault }, kInvalidNodeIndex },                            // DoorLock::char_string
    { { AttributeTag(34), "LEDSettings", ItemType::kDefault }, kInvalidNodeIndex },                         // DoorLock::int8u
    { { AttributeTag(35), "autoRelockTime", ItemType::kDefault }, kInvalidNodeIndex },                      // DoorLock::int32u
    { { AttributeTag(36), "soundVolume", ItemType::kDefault }, kInvalidNodeIndex },                         // DoorLock::int8u
    { { AttributeTag(37), "operatingMode", ItemType::kEnum }, kInvalidNodeIndex }, // DoorLock::OperatingModeEnum
    { { AttributeTag(38), "supportedOperatingModes", ItemType::kBitmap },
      kInvalidNodeIndex }, // DoorLock::DlSupportedOperatingModes
    { { AttributeTag(39), "defaultConfigurationRegister", ItemType::kBitmap },
      kInvalidNodeIndex }, // DoorLock::DlDefaultConfigurationRegister
    { { AttributeTag(40), "enableLocalProgramming", ItemType::kDefault }, kInvalidNodeIndex },  // DoorLock::boolean
    { { AttributeTag(41), "enableOneTouchLocking", ItemType::kDefault }, kInvalidNodeIndex },   // DoorLock::boolean
    { { AttributeTag(42), "enableInsideStatusLED", ItemType::kDefault }, kInvalidNodeIndex },   // DoorLock::boolean
    { { AttributeTag(43), "enablePrivacyModeButton", ItemType::kDefault }, kInvalidNodeIndex }, // DoorLock::boolean
    { { AttributeTag(44), "localProgrammingFeatures", ItemType::kBitmap },
      kInvalidNodeIndex },                                                                  // DoorLock::DlLocalProgrammingFeatures
    { { AttributeTag(48), "wrongCodeEntryLimit", ItemType::kDefault }, kInvalidNodeIndex }, // DoorLock::int8u
    { { AttributeTag(49), "userCodeTemporaryDisableTime", ItemType::kDefault }, kInvalidNodeIndex }, // DoorLock::int8u
    { { AttributeTag(50), "sendPINOverTheAir", ItemType::kDefault }, kInvalidNodeIndex },            // DoorLock::boolean
    { { AttributeTag(51), "requirePINforRemoteOperation", ItemType::kDefault }, kInvalidNodeIndex }, // DoorLock::boolean
    { { AttributeTag(53), "expiringUserTimeout", ItemType::kDefault }, kInvalidNodeIndex },          // DoorLock::int16u
    { { AttributeTag(65528), "generatedCommandList", ItemType::kList }, kInvalidNodeIndex },         // DoorLock::command_id[]
    { { AttributeTag(65529), "acceptedCommandList", ItemType::kList }, kInvalidNodeIndex },          // DoorLock::command_id[]
    { { AttributeTag(65530), "eventList", ItemType::kList }, kInvalidNodeIndex },                    // DoorLock::event_id[]
    { { AttributeTag(65531), "attributeList", ItemType::kList }, kInvalidNodeIndex },                // DoorLock::attrib_id[]
    { { AttributeTag(65532), "featureMap", ItemType::kDefault }, kInvalidNodeIndex },                // DoorLock::bitmap32
    { { AttributeTag(65533), "clusterRevision", ItemType::kDefault }, kInvalidNodeIndex },           // DoorLock::int16u
    { { EventTag(0), "DoorLockAlarm", ItemType::kDefault }, 367 },                                   // DoorLock::DoorLockAlarm
    { { EventTag(1), "DoorStateChange", ItemType::kDefault }, 368 },                                 // DoorLock::DoorStateChange
    { { EventTag(2), "LockOperation", ItemType::kDefault }, 369 },                                   // DoorLock::LockOperation
    { { EventTag(3), "LockOperationError", ItemType::kDefault }, 370 },                              // DoorLock::LockOperationError
    { { EventTag(4), "LockUserChange", ItemType::kDefault }, 371 },                                  // DoorLock::LockUserChange
    { { CommandTag(0), "LockDoorRequest", ItemType::kDefault }, 342 },   // DoorLock::LockDoor::LockDoorRequest
    { { CommandTag(1), "UnlockDoorRequest", ItemType::kDefault }, 343 }, // DoorLock::UnlockDoor::UnlockDoorRequest
    { { CommandTag(3), "UnlockWithTimeoutRequest", ItemType::kDefault },
      344 }, // DoorLock::UnlockWithTimeout::UnlockWithTimeoutRequest
    { { CommandTag(11), "SetWeekDayScheduleRequest", ItemType::kDefault },
      345 }, // DoorLock::SetWeekDaySchedule::SetWeekDayScheduleRequest
    { { CommandTag(12), "GetWeekDayScheduleRequest", ItemType::kDefault },
      346 }, // DoorLock::GetWeekDaySchedule::GetWeekDayScheduleRequest
    { { CommandTag(13), "ClearWeekDayScheduleRequest", ItemType::kDefault },
      348 }, // DoorLock::ClearWeekDaySchedule::ClearWeekDayScheduleRequest
    { { CommandTag(14), "SetYearDayScheduleRequest", ItemType::kDefault },
      349 }, // DoorLock::SetYearDaySchedule::SetYearDayScheduleRequest
    { { CommandTag(15), "GetYearDayScheduleRequest", ItemType::kDefault },
      350 }, // DoorLock::GetYearDaySchedule::GetYearDayScheduleRequest
    { { CommandTag(16), "ClearYearDayScheduleRequest", ItemType::kDefault },
      352 }, // DoorLock::ClearYearDaySchedule::ClearYearDayScheduleRequest
    { { CommandTag(17), "SetHolidayScheduleRequest", ItemType::kDefault },
      353 }, // DoorLock::SetHolidaySchedule::SetHolidayScheduleRequest
    { { CommandTag(18), "GetHolidayScheduleRequest", ItemType::kDefault },
      354 }, // DoorLock::GetHolidaySchedule::GetHolidayScheduleRequest
    { { CommandTag(19), "ClearHolidayScheduleRequest", ItemType::kDefault },
      356 }, // DoorLock::ClearHolidaySchedule::ClearHolidayScheduleRequest
    { { CommandTag(26), "SetUserRequest", ItemType::kDefault }, 357 },       // DoorLock::SetUser::SetUserRequest
    { { CommandTag(27), "GetUserRequest", ItemType::kDefault }, 358 },       // DoorLock::GetUser::GetUserRequest
    { { CommandTag(29), "ClearUserRequest", ItemType::kDefault }, 360 },     // DoorLock::ClearUser::ClearUserRequest
    { { CommandTag(34), "SetCredentialRequest", ItemType::kDefault }, 361 }, // DoorLock::SetCredential::SetCredentialRequest
    { { CommandTag(36), "GetCredentialStatusRequest", ItemType::kDefault },
      363 }, // DoorLock::GetCredentialStatus::GetCredentialStatusRequest
    { { CommandTag(38), "ClearCredentialRequest", ItemType::kDefault }, 365 }, // DoorLock::ClearCredential::ClearCredentialRequest
    { { CommandTag(39), "UnboltDoorRequest", ItemType::kDefault }, 366 },      // DoorLock::UnboltDoor::UnboltDoorRequest
    { { CommandTag(12), "GetWeekDayScheduleResponse", ItemType::kDefault }, 347 },  // DoorLock::GetWeekDayScheduleResponse
    { { CommandTag(15), "GetYearDayScheduleResponse", ItemType::kDefault }, 351 },  // DoorLock::GetYearDayScheduleResponse
    { { CommandTag(18), "GetHolidayScheduleResponse", ItemType::kDefault }, 355 },  // DoorLock::GetHolidayScheduleResponse
    { { CommandTag(28), "GetUserResponse", ItemType::kDefault }, 359 },             // DoorLock::GetUserResponse
    { { CommandTag(35), "SetCredentialResponse", ItemType::kDefault }, 362 },       // DoorLock::SetCredentialResponse
    { { CommandTag(37), "GetCredentialStatusResponse", ItemType::kDefault }, 364 }, // DoorLock::GetCredentialStatusResponse
};

const Entry<ItemInfo> _DoorLock_CredentialStruct[] = {
    { { ContextTag(0), "credentialType", ItemType::kEnum }, kInvalidNodeIndex },     // DoorLock::CredentialTypeEnum
    { { ContextTag(1), "credentialIndex", ItemType::kDefault }, kInvalidNodeIndex }, // DoorLock::int16u
};

const Entry<ItemInfo> _DoorLock_LockDoorRequest[] = {
    { { ContextTag(0), "PINCode", ItemType::kDefault }, kInvalidNodeIndex }, // DoorLock::OCTET_STRING
};

const Entry<ItemInfo> _DoorLock_UnlockDoorRequest[] = {
    { { ContextTag(0), "PINCode", ItemType::kDefault }, kInvalidNodeIndex }, // DoorLock::OCTET_STRING
};

const Entry<ItemInfo> _DoorLock_UnlockWithTimeoutRequest[] = {
    { { ContextTag(0), "timeout", ItemType::kDefault }, kInvalidNodeIndex }, // DoorLock::INT16U
    { { ContextTag(1), "PINCode", ItemType::kDefault }, kInvalidNodeIndex }, // DoorLock::OCTET_STRING
};

const Entry<ItemInfo> _DoorLock_SetWeekDayScheduleRequest[] = {
    { { ContextTag(0), "weekDayIndex", ItemType::kDefault }, kInvalidNodeIndex }, // DoorLock::INT8U
    { { ContextTag(1), "userIndex", ItemType::kDefault }, kInvalidNodeIndex },    // DoorLock::INT16U
    { { ContextTag(2), "daysMask", ItemType::kBitmap }, kInvalidNodeIndex },      // DoorLock::DaysMaskMap
    { { ContextTag(3), "startHour", ItemType::kDefault }, kInvalidNodeIndex },    // DoorLock::INT8U
    { { ContextTag(4), "startMinute", ItemType::kDefault }, kInvalidNodeIndex },  // DoorLock::INT8U
    { { ContextTag(5), "endHour", ItemType::kDefault }, kInvalidNodeIndex },      // DoorLock::INT8U
    { { ContextTag(6), "endMinute", ItemType::kDefault }, kInvalidNodeIndex },    // DoorLock::INT8U
};

const Entry<ItemInfo> _DoorLock_GetWeekDayScheduleRequest[] = {
    { { ContextTag(0), "weekDayIndex", ItemType::kDefault }, kInvalidNodeIndex }, // DoorLock::INT8U
    { { ContextTag(1), "userIndex", ItemType::kDefault }, kInvalidNodeIndex },    // DoorLock::INT16U
};

const Entry<ItemInfo> _DoorLock_GetWeekDayScheduleResponse[] = {
    { { ContextTag(0), "weekDayIndex", ItemType::kDefault }, kInvalidNodeIndex }, // DoorLock::INT8U
    { { ContextTag(1), "userIndex", ItemType::kDefault }, kInvalidNodeIndex },    // DoorLock::INT16U
    { { ContextTag(2), "status", ItemType::kEnum }, kInvalidNodeIndex },          // DoorLock::DlStatus
    { { ContextTag(3), "daysMask", ItemType::kBitmap }, kInvalidNodeIndex },      // DoorLock::DaysMaskMap
    { { ContextTag(4), "startHour", ItemType::kDefault }, kInvalidNodeIndex },    // DoorLock::INT8U
    { { ContextTag(5), "startMinute", ItemType::kDefault }, kInvalidNodeIndex },  // DoorLock::INT8U
    { { ContextTag(6), "endHour", ItemType::kDefault }, kInvalidNodeIndex },      // DoorLock::INT8U
    { { ContextTag(7), "endMinute", ItemType::kDefault }, kInvalidNodeIndex },    // DoorLock::INT8U
};

const Entry<ItemInfo> _DoorLock_ClearWeekDayScheduleRequest[] = {
    { { ContextTag(0), "weekDayIndex", ItemType::kDefault }, kInvalidNodeIndex }, // DoorLock::INT8U
    { { ContextTag(1), "userIndex", ItemType::kDefault }, kInvalidNodeIndex },    // DoorLock::INT16U
};

const Entry<ItemInfo> _DoorLock_SetYearDayScheduleRequest[] = {
    { { ContextTag(0), "yearDayIndex", ItemType::kDefault }, kInvalidNodeIndex },   // DoorLock::INT8U
    { { ContextTag(1), "userIndex", ItemType::kDefault }, kInvalidNodeIndex },      // DoorLock::INT16U
    { { ContextTag(2), "localStartTime", ItemType::kDefault }, kInvalidNodeIndex }, // DoorLock::epoch_s
    { { ContextTag(3), "localEndTime", ItemType::kDefault }, kInvalidNodeIndex },   // DoorLock::epoch_s
};

const Entry<ItemInfo> _DoorLock_GetYearDayScheduleRequest[] = {
    { { ContextTag(0), "yearDayIndex", ItemType::kDefault }, kInvalidNodeIndex }, // DoorLock::INT8U
    { { ContextTag(1), "userIndex", ItemType::kDefault }, kInvalidNodeIndex },    // DoorLock::INT16U
};

const Entry<ItemInfo> _DoorLock_GetYearDayScheduleResponse[] = {
    { { ContextTag(0), "yearDayIndex", ItemType::kDefault }, kInvalidNodeIndex },   // DoorLock::INT8U
    { { ContextTag(1), "userIndex", ItemType::kDefault }, kInvalidNodeIndex },      // DoorLock::INT16U
    { { ContextTag(2), "status", ItemType::kEnum }, kInvalidNodeIndex },            // DoorLock::DlStatus
    { { ContextTag(3), "localStartTime", ItemType::kDefault }, kInvalidNodeIndex }, // DoorLock::epoch_s
    { { ContextTag(4), "localEndTime", ItemType::kDefault }, kInvalidNodeIndex },   // DoorLock::epoch_s
};

const Entry<ItemInfo> _DoorLock_ClearYearDayScheduleRequest[] = {
    { { ContextTag(0), "yearDayIndex", ItemType::kDefault }, kInvalidNodeIndex }, // DoorLock::INT8U
    { { ContextTag(1), "userIndex", ItemType::kDefault }, kInvalidNodeIndex },    // DoorLock::INT16U
};

const Entry<ItemInfo> _DoorLock_SetHolidayScheduleRequest[] = {
    { { ContextTag(0), "holidayIndex", ItemType::kDefault }, kInvalidNodeIndex },   // DoorLock::INT8U
    { { ContextTag(1), "localStartTime", ItemType::kDefault }, kInvalidNodeIndex }, // DoorLock::epoch_s
    { { ContextTag(2), "localEndTime", ItemType::kDefault }, kInvalidNodeIndex },   // DoorLock::epoch_s
    { { ContextTag(3), "operatingMode", ItemType::kEnum }, kInvalidNodeIndex },     // DoorLock::OperatingModeEnum
};

const Entry<ItemInfo> _DoorLock_GetHolidayScheduleRequest[] = {
    { { ContextTag(0), "holidayIndex", ItemType::kDefault }, kInvalidNodeIndex }, // DoorLock::INT8U
};

const Entry<ItemInfo> _DoorLock_GetHolidayScheduleResponse[] = {
    { { ContextTag(0), "holidayIndex", ItemType::kDefault }, kInvalidNodeIndex },   // DoorLock::INT8U
    { { ContextTag(1), "status", ItemType::kEnum }, kInvalidNodeIndex },            // DoorLock::DlStatus
    { { ContextTag(2), "localStartTime", ItemType::kDefault }, kInvalidNodeIndex }, // DoorLock::epoch_s
    { { ContextTag(3), "localEndTime", ItemType::kDefault }, kInvalidNodeIndex },   // DoorLock::epoch_s
    { { ContextTag(4), "operatingMode", ItemType::kEnum }, kInvalidNodeIndex },     // DoorLock::OperatingModeEnum
};

const Entry<ItemInfo> _DoorLock_ClearHolidayScheduleRequest[] = {
    { { ContextTag(0), "holidayIndex", ItemType::kDefault }, kInvalidNodeIndex }, // DoorLock::INT8U
};

const Entry<ItemInfo> _DoorLock_SetUserRequest[] = {
    { { ContextTag(0), "operationType", ItemType::kEnum }, kInvalidNodeIndex },   // DoorLock::DataOperationTypeEnum
    { { ContextTag(1), "userIndex", ItemType::kDefault }, kInvalidNodeIndex },    // DoorLock::INT16U
    { { ContextTag(2), "userName", ItemType::kDefault }, kInvalidNodeIndex },     // DoorLock::CHAR_STRING
    { { ContextTag(3), "userUniqueID", ItemType::kDefault }, kInvalidNodeIndex }, // DoorLock::INT32U
    { { ContextTag(4), "userStatus", ItemType::kEnum }, kInvalidNodeIndex },      // DoorLock::UserStatusEnum
    { { ContextTag(5), "userType", ItemType::kEnum }, kInvalidNodeIndex },        // DoorLock::UserTypeEnum
    { { ContextTag(6), "credentialRule", ItemType::kEnum }, kInvalidNodeIndex },  // DoorLock::CredentialRuleEnum
};

const Entry<ItemInfo> _DoorLock_GetUserRequest[] = {
    { { ContextTag(0), "userIndex", ItemType::kDefault }, kInvalidNodeIndex }, // DoorLock::INT16U
};

const Entry<ItemInfo> _DoorLock_GetUserResponse[] = {
    { { ContextTag(0), "userIndex", ItemType::kDefault }, kInvalidNodeIndex },               // DoorLock::INT16U
    { { ContextTag(1), "userName", ItemType::kDefault }, kInvalidNodeIndex },                // DoorLock::CHAR_STRING
    { { ContextTag(2), "userUniqueID", ItemType::kDefault }, kInvalidNodeIndex },            // DoorLock::INT32U
    { { ContextTag(3), "userStatus", ItemType::kEnum }, kInvalidNodeIndex },                 // DoorLock::UserStatusEnum
    { { ContextTag(4), "userType", ItemType::kEnum }, kInvalidNodeIndex },                   // DoorLock::UserTypeEnum
    { { ContextTag(5), "credentialRule", ItemType::kEnum }, kInvalidNodeIndex },             // DoorLock::CredentialRuleEnum
    { { ContextTag(6), "credentials", ItemType::kList }, 372 },                              // DoorLock::CredentialStruct[]
    { { ContextTag(7), "creatorFabricIndex", ItemType::kDefault }, kInvalidNodeIndex },      // DoorLock::fabric_idx
    { { ContextTag(8), "lastModifiedFabricIndex", ItemType::kDefault }, kInvalidNodeIndex }, // DoorLock::fabric_idx
    { { ContextTag(9), "nextUserIndex", ItemType::kDefault }, kInvalidNodeIndex },           // DoorLock::INT16U
};

const Entry<ItemInfo> _DoorLock_ClearUserRequest[] = {
    { { ContextTag(0), "userIndex", ItemType::kDefault }, kInvalidNodeIndex }, // DoorLock::INT16U
};

const Entry<ItemInfo> _DoorLock_SetCredentialRequest[] = {
    { { ContextTag(0), "operationType", ItemType::kEnum }, kInvalidNodeIndex },     // DoorLock::DataOperationTypeEnum
    { { ContextTag(1), "credential", ItemType::kDefault }, 341 },                   // DoorLock::CredentialStruct
    { { ContextTag(2), "credentialData", ItemType::kDefault }, kInvalidNodeIndex }, // DoorLock::LONG_OCTET_STRING
    { { ContextTag(3), "userIndex", ItemType::kDefault }, kInvalidNodeIndex },      // DoorLock::INT16U
    { { ContextTag(4), "userStatus", ItemType::kEnum }, kInvalidNodeIndex },        // DoorLock::UserStatusEnum
    { { ContextTag(5), "userType", ItemType::kEnum }, kInvalidNodeIndex },          // DoorLock::UserTypeEnum
};

const Entry<ItemInfo> _DoorLock_SetCredentialResponse[] = {
    { { ContextTag(0), "status", ItemType::kEnum }, kInvalidNodeIndex },                 // DoorLock::DlStatus
    { { ContextTag(1), "userIndex", ItemType::kDefault }, kInvalidNodeIndex },           // DoorLock::INT16U
    { { ContextTag(2), "nextCredentialIndex", ItemType::kDefault }, kInvalidNodeIndex }, // DoorLock::INT16U
};

const Entry<ItemInfo> _DoorLock_GetCredentialStatusRequest[] = {
    { { ContextTag(0), "credential", ItemType::kDefault }, 341 }, // DoorLock::CredentialStruct
};

const Entry<ItemInfo> _DoorLock_GetCredentialStatusResponse[] = {
    { { ContextTag(0), "credentialExists", ItemType::kDefault }, kInvalidNodeIndex },        // DoorLock::boolean
    { { ContextTag(1), "userIndex", ItemType::kDefault }, kInvalidNodeIndex },               // DoorLock::INT16U
    { { ContextTag(2), "creatorFabricIndex", ItemType::kDefault }, kInvalidNodeIndex },      // DoorLock::fabric_idx
    { { ContextTag(3), "lastModifiedFabricIndex", ItemType::kDefault }, kInvalidNodeIndex }, // DoorLock::fabric_idx
    { { ContextTag(4), "nextCredentialIndex", ItemType::kDefault }, kInvalidNodeIndex },     // DoorLock::INT16U
};

const Entry<ItemInfo> _DoorLock_ClearCredentialRequest[] = {
    { { ContextTag(0), "credential", ItemType::kDefault }, 341 }, // DoorLock::CredentialStruct
};

const Entry<ItemInfo> _DoorLock_UnboltDoorRequest[] = {
    { { ContextTag(0), "PINCode", ItemType::kDefault }, kInvalidNodeIndex }, // DoorLock::OCTET_STRING
};

const Entry<ItemInfo> _DoorLock_DoorLockAlarm[] = {
    { { ContextTag(0), "alarmCode", ItemType::kEnum }, kInvalidNodeIndex }, // DoorLock::AlarmCodeEnum
};

const Entry<ItemInfo> _DoorLock_DoorStateChange[] = {
    { { ContextTag(0), "doorState", ItemType::kEnum }, kInvalidNodeIndex }, // DoorLock::DoorStateEnum
};

const Entry<ItemInfo> _DoorLock_LockOperation[] = {
    { { ContextTag(0), "lockOperationType", ItemType::kEnum }, kInvalidNodeIndex }, // DoorLock::LockOperationTypeEnum
    { { ContextTag(1), "operationSource", ItemType::kEnum }, kInvalidNodeIndex },   // DoorLock::OperationSourceEnum
    { { ContextTag(2), "userIndex", ItemType::kDefault }, kInvalidNodeIndex },      // DoorLock::INT16U
    { { ContextTag(3), "fabricIndex", ItemType::kDefault }, kInvalidNodeIndex },    // DoorLock::fabric_idx
    { { ContextTag(4), "sourceNode", ItemType::kDefault }, kInvalidNodeIndex },     // DoorLock::NODE_ID
    { { ContextTag(5), "credentials", ItemType::kList }, 372 },                     // DoorLock::CredentialStruct[]
};

const Entry<ItemInfo> _DoorLock_LockOperationError[] = {
    { { ContextTag(0), "lockOperationType", ItemType::kEnum }, kInvalidNodeIndex }, // DoorLock::LockOperationTypeEnum
    { { ContextTag(1), "operationSource", ItemType::kEnum }, kInvalidNodeIndex },   // DoorLock::OperationSourceEnum
    { { ContextTag(2), "operationError", ItemType::kEnum }, kInvalidNodeIndex },    // DoorLock::OperationErrorEnum
    { { ContextTag(3), "userIndex", ItemType::kDefault }, kInvalidNodeIndex },      // DoorLock::INT16U
    { { ContextTag(4), "fabricIndex", ItemType::kDefault }, kInvalidNodeIndex },    // DoorLock::fabric_idx
    { { ContextTag(5), "sourceNode", ItemType::kDefault }, kInvalidNodeIndex },     // DoorLock::NODE_ID
    { { ContextTag(6), "credentials", ItemType::kList }, 372 },                     // DoorLock::CredentialStruct[]
};

const Entry<ItemInfo> _DoorLock_LockUserChange[] = {
    { { ContextTag(0), "lockDataType", ItemType::kEnum }, kInvalidNodeIndex },      // DoorLock::LockDataTypeEnum
    { { ContextTag(1), "dataOperationType", ItemType::kEnum }, kInvalidNodeIndex }, // DoorLock::DataOperationTypeEnum
    { { ContextTag(2), "operationSource", ItemType::kEnum }, kInvalidNodeIndex },   // DoorLock::OperationSourceEnum
    { { ContextTag(3), "userIndex", ItemType::kDefault }, kInvalidNodeIndex },      // DoorLock::INT16U
    { { ContextTag(4), "fabricIndex", ItemType::kDefault }, kInvalidNodeIndex },    // DoorLock::fabric_idx
    { { ContextTag(5), "sourceNode", ItemType::kDefault }, kInvalidNodeIndex },     // DoorLock::NODE_ID
    { { ContextTag(6), "dataIndex", ItemType::kDefault }, kInvalidNodeIndex },      // DoorLock::INT16U
};

const Entry<ItemInfo> _DoorLock_CredentialStruct_list_[] = {
    { { AnonymousTag(), "[]", ItemType::kDefault }, 341 }, // DoorLock_CredentialStruct[]
};

const Entry<ItemInfo> _WindowCovering[] = {
    { { AttributeTag(0), "type", ItemType::kEnum }, kInvalidNodeIndex },                             // WindowCovering::Type
    { { AttributeTag(1), "physicalClosedLimitLift", ItemType::kDefault }, kInvalidNodeIndex },       // WindowCovering::int16u
    { { AttributeTag(2), "physicalClosedLimitTilt", ItemType::kDefault }, kInvalidNodeIndex },       // WindowCovering::int16u
    { { AttributeTag(3), "currentPositionLift", ItemType::kDefault }, kInvalidNodeIndex },           // WindowCovering::int16u
    { { AttributeTag(4), "currentPositionTilt", ItemType::kDefault }, kInvalidNodeIndex },           // WindowCovering::int16u
    { { AttributeTag(5), "numberOfActuationsLift", ItemType::kDefault }, kInvalidNodeIndex },        // WindowCovering::int16u
    { { AttributeTag(6), "numberOfActuationsTilt", ItemType::kDefault }, kInvalidNodeIndex },        // WindowCovering::int16u
    { { AttributeTag(7), "configStatus", ItemType::kBitmap }, kInvalidNodeIndex },                   // WindowCovering::ConfigStatus
    { { AttributeTag(8), "currentPositionLiftPercentage", ItemType::kDefault }, kInvalidNodeIndex }, // WindowCovering::Percent
    { { AttributeTag(9), "currentPositionTiltPercentage", ItemType::kDefault }, kInvalidNodeIndex }, // WindowCovering::Percent
    { { AttributeTag(10), "operationalStatus", ItemType::kBitmap }, kInvalidNodeIndex }, // WindowCovering::OperationalStatus
    { { AttributeTag(11), "targetPositionLiftPercent100ths", ItemType::kDefault },
      kInvalidNodeIndex }, // WindowCovering::Percent100ths
    { { AttributeTag(12), "targetPositionTiltPercent100ths", ItemType::kDefault },
      kInvalidNodeIndex },                                                          // WindowCovering::Percent100ths
    { { AttributeTag(13), "endProductType", ItemType::kEnum }, kInvalidNodeIndex }, // WindowCovering::EndProductType
    { { AttributeTag(14), "currentPositionLiftPercent100ths", ItemType::kDefault },
      kInvalidNodeIndex }, // WindowCovering::Percent100ths
    { { AttributeTag(15), "currentPositionTiltPercent100ths", ItemType::kDefault },
      kInvalidNodeIndex },                                                                       // WindowCovering::Percent100ths
    { { AttributeTag(16), "installedOpenLimitLift", ItemType::kDefault }, kInvalidNodeIndex },   // WindowCovering::int16u
    { { AttributeTag(17), "installedClosedLimitLift", ItemType::kDefault }, kInvalidNodeIndex }, // WindowCovering::int16u
    { { AttributeTag(18), "installedOpenLimitTilt", ItemType::kDefault }, kInvalidNodeIndex },   // WindowCovering::int16u
    { { AttributeTag(19), "installedClosedLimitTilt", ItemType::kDefault }, kInvalidNodeIndex }, // WindowCovering::int16u
    { { AttributeTag(23), "mode", ItemType::kBitmap }, kInvalidNodeIndex },                      // WindowCovering::Mode
    { { AttributeTag(26), "safetyStatus", ItemType::kBitmap }, kInvalidNodeIndex },              // WindowCovering::SafetyStatus
    { { AttributeTag(65528), "generatedCommandList", ItemType::kList }, kInvalidNodeIndex },     // WindowCovering::command_id[]
    { { AttributeTag(65529), "acceptedCommandList", ItemType::kList }, kInvalidNodeIndex },      // WindowCovering::command_id[]
    { { AttributeTag(65530), "eventList", ItemType::kList }, kInvalidNodeIndex },                // WindowCovering::event_id[]
    { { AttributeTag(65531), "attributeList", ItemType::kList }, kInvalidNodeIndex },            // WindowCovering::attrib_id[]
    { { AttributeTag(65532), "featureMap", ItemType::kDefault }, kInvalidNodeIndex },            // WindowCovering::bitmap32
    { { AttributeTag(65533), "clusterRevision", ItemType::kDefault }, kInvalidNodeIndex },       // WindowCovering::int16u
    { { CommandTag(4), "GoToLiftValueRequest", ItemType::kDefault }, 374 }, // WindowCovering::GoToLiftValue::GoToLiftValueRequest
    { { CommandTag(5), "GoToLiftPercentageRequest", ItemType::kDefault },
      375 }, // WindowCovering::GoToLiftPercentage::GoToLiftPercentageRequest
    { { CommandTag(7), "GoToTiltValueRequest", ItemType::kDefault }, 376 }, // WindowCovering::GoToTiltValue::GoToTiltValueRequest
    { { CommandTag(8), "GoToTiltPercentageRequest", ItemType::kDefault },
      377 }, // WindowCovering::GoToTiltPercentage::GoToTiltPercentageRequest
};

const Entry<ItemInfo> _WindowCovering_GoToLiftValueRequest[] = {
    { { ContextTag(0), "liftValue", ItemType::kDefault }, kInvalidNodeIndex }, // WindowCovering::INT16U
};

const Entry<ItemInfo> _WindowCovering_GoToLiftPercentageRequest[] = {
    { { ContextTag(0), "liftPercent100thsValue", ItemType::kDefault }, kInvalidNodeIndex }, // WindowCovering::Percent100ths
};

const Entry<ItemInfo> _WindowCovering_GoToTiltValueRequest[] = {
    { { ContextTag(0), "tiltValue", ItemType::kDefault }, kInvalidNodeIndex }, // WindowCovering::INT16U
};

const Entry<ItemInfo> _WindowCovering_GoToTiltPercentageRequest[] = {
    { { ContextTag(0), "tiltPercent100thsValue", ItemType::kDefault }, kInvalidNodeIndex }, // WindowCovering::Percent100ths
};

const Entry<ItemInfo> _BarrierControl[] = {
    { { AttributeTag(1), "barrierMovingState", ItemType::kDefault }, kInvalidNodeIndex },        // BarrierControl::enum8
    { { AttributeTag(2), "barrierSafetyStatus", ItemType::kDefault }, kInvalidNodeIndex },       // BarrierControl::bitmap16
    { { AttributeTag(3), "barrierCapabilities", ItemType::kDefault }, kInvalidNodeIndex },       // BarrierControl::bitmap8
    { { AttributeTag(4), "barrierOpenEvents", ItemType::kDefault }, kInvalidNodeIndex },         // BarrierControl::int16u
    { { AttributeTag(5), "barrierCloseEvents", ItemType::kDefault }, kInvalidNodeIndex },        // BarrierControl::int16u
    { { AttributeTag(6), "barrierCommandOpenEvents", ItemType::kDefault }, kInvalidNodeIndex },  // BarrierControl::int16u
    { { AttributeTag(7), "barrierCommandCloseEvents", ItemType::kDefault }, kInvalidNodeIndex }, // BarrierControl::int16u
    { { AttributeTag(8), "barrierOpenPeriod", ItemType::kDefault }, kInvalidNodeIndex },         // BarrierControl::int16u
    { { AttributeTag(9), "barrierClosePeriod", ItemType::kDefault }, kInvalidNodeIndex },        // BarrierControl::int16u
    { { AttributeTag(10), "barrierPosition", ItemType::kDefault }, kInvalidNodeIndex },          // BarrierControl::int8u
    { { AttributeTag(65528), "generatedCommandList", ItemType::kList }, kInvalidNodeIndex },     // BarrierControl::command_id[]
    { { AttributeTag(65529), "acceptedCommandList", ItemType::kList }, kInvalidNodeIndex },      // BarrierControl::command_id[]
    { { AttributeTag(65530), "eventList", ItemType::kList }, kInvalidNodeIndex },                // BarrierControl::event_id[]
    { { AttributeTag(65531), "attributeList", ItemType::kList }, kInvalidNodeIndex },            // BarrierControl::attrib_id[]
    { { AttributeTag(65532), "featureMap", ItemType::kDefault }, kInvalidNodeIndex },            // BarrierControl::bitmap32
    { { AttributeTag(65533), "clusterRevision", ItemType::kDefault }, kInvalidNodeIndex },       // BarrierControl::int16u
    { { CommandTag(0), "BarrierControlGoToPercentRequest", ItemType::kDefault },
      379 }, // BarrierControl::BarrierControlGoToPercent::BarrierControlGoToPercentRequest
};

const Entry<ItemInfo> _BarrierControl_BarrierControlGoToPercentRequest[] = {
    { { ContextTag(0), "percentOpen", ItemType::kDefault }, kInvalidNodeIndex }, // BarrierControl::INT8U
};

const Entry<ItemInfo> _PumpConfigurationAndControl[] = {
    { { AttributeTag(0), "maxPressure", ItemType::kDefault }, kInvalidNodeIndex },      // PumpConfigurationAndControl::int16s
    { { AttributeTag(1), "maxSpeed", ItemType::kDefault }, kInvalidNodeIndex },         // PumpConfigurationAndControl::int16u
    { { AttributeTag(2), "maxFlow", ItemType::kDefault }, kInvalidNodeIndex },          // PumpConfigurationAndControl::int16u
    { { AttributeTag(3), "minConstPressure", ItemType::kDefault }, kInvalidNodeIndex }, // PumpConfigurationAndControl::int16s
    { { AttributeTag(4), "maxConstPressure", ItemType::kDefault }, kInvalidNodeIndex }, // PumpConfigurationAndControl::int16s
    { { AttributeTag(5), "minCompPressure", ItemType::kDefault }, kInvalidNodeIndex },  // PumpConfigurationAndControl::int16s
    { { AttributeTag(6), "maxCompPressure", ItemType::kDefault }, kInvalidNodeIndex },  // PumpConfigurationAndControl::int16s
    { { AttributeTag(7), "minConstSpeed", ItemType::kDefault }, kInvalidNodeIndex },    // PumpConfigurationAndControl::int16u
    { { AttributeTag(8), "maxConstSpeed", ItemType::kDefault }, kInvalidNodeIndex },    // PumpConfigurationAndControl::int16u
    { { AttributeTag(9), "minConstFlow", ItemType::kDefault }, kInvalidNodeIndex },     // PumpConfigurationAndControl::int16u
    { { AttributeTag(10), "maxConstFlow", ItemType::kDefault }, kInvalidNodeIndex },    // PumpConfigurationAndControl::int16u
    { { AttributeTag(11), "minConstTemp", ItemType::kDefault }, kInvalidNodeIndex },    // PumpConfigurationAndControl::int16s
    { { AttributeTag(12), "maxConstTemp", ItemType::kDefault }, kInvalidNodeIndex },    // PumpConfigurationAndControl::int16s
    { { AttributeTag(16), "pumpStatus", ItemType::kBitmap }, kInvalidNodeIndex }, // PumpConfigurationAndControl::PumpStatusBitmap
    { { AttributeTag(17), "effectiveOperationMode", ItemType::kEnum },
      kInvalidNodeIndex }, // PumpConfigurationAndControl::OperationModeEnum
    { { AttributeTag(18), "effectiveControlMode", ItemType::kEnum },
      kInvalidNodeIndex },                                                       // PumpConfigurationAndControl::ControlModeEnum
    { { AttributeTag(19), "capacity", ItemType::kDefault }, kInvalidNodeIndex }, // PumpConfigurationAndControl::int16s
    { { AttributeTag(20), "speed", ItemType::kDefault }, kInvalidNodeIndex },    // PumpConfigurationAndControl::int16u
    { { AttributeTag(21), "lifetimeRunningHours", ItemType::kDefault }, kInvalidNodeIndex }, // PumpConfigurationAndControl::int24u
    { { AttributeTag(22), "power", ItemType::kDefault }, kInvalidNodeIndex },                // PumpConfigurationAndControl::int24u
    { { AttributeTag(23), "lifetimeEnergyConsumed", ItemType::kDefault },
      kInvalidNodeIndex },                                                         // PumpConfigurationAndControl::int32u
    { { AttributeTag(32), "operationMode", ItemType::kEnum }, kInvalidNodeIndex }, // PumpConfigurationAndControl::OperationModeEnum
    { { AttributeTag(33), "controlMode", ItemType::kEnum }, kInvalidNodeIndex },   // PumpConfigurationAndControl::ControlModeEnum
    { { AttributeTag(65528), "generatedCommandList", ItemType::kList },
      kInvalidNodeIndex }, // PumpConfigurationAndControl::command_id[]
    { { AttributeTag(65529), "acceptedCommandList", ItemType::kList },
      kInvalidNodeIndex },                                                            // PumpConfigurationAndControl::command_id[]
    { { AttributeTag(65530), "eventList", ItemType::kList }, kInvalidNodeIndex },     // PumpConfigurationAndControl::event_id[]
    { { AttributeTag(65531), "attributeList", ItemType::kList }, kInvalidNodeIndex }, // PumpConfigurationAndControl::attrib_id[]
    { { AttributeTag(65532), "featureMap", ItemType::kDefault }, kInvalidNodeIndex }, // PumpConfigurationAndControl::bitmap32
    { { AttributeTag(65533), "clusterRevision", ItemType::kDefault }, kInvalidNodeIndex }, // PumpConfigurationAndControl::int16u
    { { EventTag(0), "SupplyVoltageLow", ItemType::kDefault }, 381 },      // PumpConfigurationAndControl::SupplyVoltageLow
    { { EventTag(1), "SupplyVoltageHigh", ItemType::kDefault }, 382 },     // PumpConfigurationAndControl::SupplyVoltageHigh
    { { EventTag(2), "PowerMissingPhase", ItemType::kDefault }, 383 },     // PumpConfigurationAndControl::PowerMissingPhase
    { { EventTag(3), "SystemPressureLow", ItemType::kDefault }, 384 },     // PumpConfigurationAndControl::SystemPressureLow
    { { EventTag(4), "SystemPressureHigh", ItemType::kDefault }, 385 },    // PumpConfigurationAndControl::SystemPressureHigh
    { { EventTag(5), "DryRunning", ItemType::kDefault }, 386 },            // PumpConfigurationAndControl::DryRunning
    { { EventTag(6), "MotorTemperatureHigh", ItemType::kDefault }, 387 },  // PumpConfigurationAndControl::MotorTemperatureHigh
    { { EventTag(7), "PumpMotorFatalFailure", ItemType::kDefault }, 388 }, // PumpConfigurationAndControl::PumpMotorFatalFailure
    { { EventTag(8), "ElectronicTemperatureHigh", ItemType::kDefault },
      389 },                                                        // PumpConfigurationAndControl::ElectronicTemperatureHigh
    { { EventTag(9), "PumpBlocked", ItemType::kDefault }, 390 },    // PumpConfigurationAndControl::PumpBlocked
    { { EventTag(10), "SensorFailure", ItemType::kDefault }, 391 }, // PumpConfigurationAndControl::SensorFailure
    { { EventTag(11), "ElectronicNonFatalFailure", ItemType::kDefault },
      392 }, // PumpConfigurationAndControl::ElectronicNonFatalFailure
    { { EventTag(12), "ElectronicFatalFailure", ItemType::kDefault }, 393 }, // PumpConfigurationAndControl::ElectronicFatalFailure
    { { EventTag(13), "GeneralFault", ItemType::kDefault }, 394 },           // PumpConfigurationAndControl::GeneralFault
    { { EventTag(14), "Leakage", ItemType::kDefault }, 395 },                // PumpConfigurationAndControl::Leakage
    { { EventTag(15), "AirDetection", ItemType::kDefault }, 396 },           // PumpConfigurationAndControl::AirDetection
    { { EventTag(16), "TurbineOperation", ItemType::kDefault }, 397 },       // PumpConfigurationAndControl::TurbineOperation
};

const Entry<ItemInfo> _PumpConfigurationAndControl_SupplyVoltageLow[] = {};

const Entry<ItemInfo> _PumpConfigurationAndControl_SupplyVoltageHigh[] = {};

const Entry<ItemInfo> _PumpConfigurationAndControl_PowerMissingPhase[] = {};

const Entry<ItemInfo> _PumpConfigurationAndControl_SystemPressureLow[] = {};

const Entry<ItemInfo> _PumpConfigurationAndControl_SystemPressureHigh[] = {};

const Entry<ItemInfo> _PumpConfigurationAndControl_DryRunning[] = {};

const Entry<ItemInfo> _PumpConfigurationAndControl_MotorTemperatureHigh[] = {};

const Entry<ItemInfo> _PumpConfigurationAndControl_PumpMotorFatalFailure[] = {};

const Entry<ItemInfo> _PumpConfigurationAndControl_ElectronicTemperatureHigh[] = {};

const Entry<ItemInfo> _PumpConfigurationAndControl_PumpBlocked[] = {};

const Entry<ItemInfo> _PumpConfigurationAndControl_SensorFailure[] = {};

const Entry<ItemInfo> _PumpConfigurationAndControl_ElectronicNonFatalFailure[] = {};

const Entry<ItemInfo> _PumpConfigurationAndControl_ElectronicFatalFailure[] = {};

const Entry<ItemInfo> _PumpConfigurationAndControl_GeneralFault[] = {};

const Entry<ItemInfo> _PumpConfigurationAndControl_Leakage[] = {};

const Entry<ItemInfo> _PumpConfigurationAndControl_AirDetection[] = {};

const Entry<ItemInfo> _PumpConfigurationAndControl_TurbineOperation[] = {};

const Entry<ItemInfo> _Thermostat[] = {
    { { AttributeTag(0), "localTemperature", ItemType::kDefault }, kInvalidNodeIndex },             // Thermostat::int16s
    { { AttributeTag(1), "outdoorTemperature", ItemType::kDefault }, kInvalidNodeIndex },           // Thermostat::int16s
    { { AttributeTag(2), "occupancy", ItemType::kDefault }, kInvalidNodeIndex },                    // Thermostat::bitmap8
    { { AttributeTag(3), "absMinHeatSetpointLimit", ItemType::kDefault }, kInvalidNodeIndex },      // Thermostat::int16s
    { { AttributeTag(4), "absMaxHeatSetpointLimit", ItemType::kDefault }, kInvalidNodeIndex },      // Thermostat::int16s
    { { AttributeTag(5), "absMinCoolSetpointLimit", ItemType::kDefault }, kInvalidNodeIndex },      // Thermostat::int16s
    { { AttributeTag(6), "absMaxCoolSetpointLimit", ItemType::kDefault }, kInvalidNodeIndex },      // Thermostat::int16s
    { { AttributeTag(7), "PICoolingDemand", ItemType::kDefault }, kInvalidNodeIndex },              // Thermostat::int8u
    { { AttributeTag(8), "PIHeatingDemand", ItemType::kDefault }, kInvalidNodeIndex },              // Thermostat::int8u
    { { AttributeTag(9), "HVACSystemTypeConfiguration", ItemType::kDefault }, kInvalidNodeIndex },  // Thermostat::bitmap8
    { { AttributeTag(16), "localTemperatureCalibration", ItemType::kDefault }, kInvalidNodeIndex }, // Thermostat::int8s
    { { AttributeTag(17), "occupiedCoolingSetpoint", ItemType::kDefault }, kInvalidNodeIndex },     // Thermostat::int16s
    { { AttributeTag(18), "occupiedHeatingSetpoint", ItemType::kDefault }, kInvalidNodeIndex },     // Thermostat::int16s
    { { AttributeTag(19), "unoccupiedCoolingSetpoint", ItemType::kDefault }, kInvalidNodeIndex },   // Thermostat::int16s
    { { AttributeTag(20), "unoccupiedHeatingSetpoint", ItemType::kDefault }, kInvalidNodeIndex },   // Thermostat::int16s
    { { AttributeTag(21), "minHeatSetpointLimit", ItemType::kDefault }, kInvalidNodeIndex },        // Thermostat::int16s
    { { AttributeTag(22), "maxHeatSetpointLimit", ItemType::kDefault }, kInvalidNodeIndex },        // Thermostat::int16s
    { { AttributeTag(23), "minCoolSetpointLimit", ItemType::kDefault }, kInvalidNodeIndex },        // Thermostat::int16s
    { { AttributeTag(24), "maxCoolSetpointLimit", ItemType::kDefault }, kInvalidNodeIndex },        // Thermostat::int16s
    { { AttributeTag(25), "minSetpointDeadBand", ItemType::kDefault }, kInvalidNodeIndex },         // Thermostat::int8s
    { { AttributeTag(26), "remoteSensing", ItemType::kDefault }, kInvalidNodeIndex },               // Thermostat::bitmap8
    { { AttributeTag(27), "controlSequenceOfOperation", ItemType::kEnum },
      kInvalidNodeIndex },                                                         // Thermostat::ThermostatControlSequence
    { { AttributeTag(28), "systemMode", ItemType::kDefault }, kInvalidNodeIndex }, // Thermostat::enum8
    { { AttributeTag(30), "thermostatRunningMode", ItemType::kDefault }, kInvalidNodeIndex },              // Thermostat::enum8
    { { AttributeTag(32), "startOfWeek", ItemType::kDefault }, kInvalidNodeIndex },                        // Thermostat::enum8
    { { AttributeTag(33), "numberOfWeeklyTransitions", ItemType::kDefault }, kInvalidNodeIndex },          // Thermostat::int8u
    { { AttributeTag(34), "numberOfDailyTransitions", ItemType::kDefault }, kInvalidNodeIndex },           // Thermostat::int8u
    { { AttributeTag(35), "temperatureSetpointHold", ItemType::kDefault }, kInvalidNodeIndex },            // Thermostat::enum8
    { { AttributeTag(36), "temperatureSetpointHoldDuration", ItemType::kDefault }, kInvalidNodeIndex },    // Thermostat::int16u
    { { AttributeTag(37), "thermostatProgrammingOperationMode", ItemType::kDefault }, kInvalidNodeIndex }, // Thermostat::bitmap8
    { { AttributeTag(41), "thermostatRunningState", ItemType::kDefault }, kInvalidNodeIndex },             // Thermostat::bitmap16
    { { AttributeTag(48), "setpointChangeSource", ItemType::kDefault }, kInvalidNodeIndex },               // Thermostat::enum8
    { { AttributeTag(49), "setpointChangeAmount", ItemType::kDefault }, kInvalidNodeIndex },               // Thermostat::int16s
    { { AttributeTag(50), "setpointChangeSourceTimestamp", ItemType::kDefault }, kInvalidNodeIndex },      // Thermostat::epoch_s
    { { AttributeTag(52), "occupiedSetback", ItemType::kDefault }, kInvalidNodeIndex },                    // Thermostat::int8u
    { { AttributeTag(53), "occupiedSetbackMin", ItemType::kDefault }, kInvalidNodeIndex },                 // Thermostat::int8u
    { { AttributeTag(54), "occupiedSetbackMax", ItemType::kDefault }, kInvalidNodeIndex },                 // Thermostat::int8u
    { { AttributeTag(55), "unoccupiedSetback", ItemType::kDefault }, kInvalidNodeIndex },                  // Thermostat::int8u
    { { AttributeTag(56), "unoccupiedSetbackMin", ItemType::kDefault }, kInvalidNodeIndex },               // Thermostat::int8u
    { { AttributeTag(57), "unoccupiedSetbackMax", ItemType::kDefault }, kInvalidNodeIndex },               // Thermostat::int8u
    { { AttributeTag(58), "emergencyHeatDelta", ItemType::kDefault }, kInvalidNodeIndex },                 // Thermostat::int8u
    { { AttributeTag(64), "ACType", ItemType::kDefault }, kInvalidNodeIndex },                             // Thermostat::enum8
    { { AttributeTag(65), "ACCapacity", ItemType::kDefault }, kInvalidNodeIndex },                         // Thermostat::int16u
    { { AttributeTag(66), "ACRefrigerantType", ItemType::kDefault }, kInvalidNodeIndex },                  // Thermostat::enum8
    { { AttributeTag(67), "ACCompressorType", ItemType::kDefault }, kInvalidNodeIndex },                   // Thermostat::enum8
    { { AttributeTag(68), "ACErrorCode", ItemType::kDefault }, kInvalidNodeIndex },                        // Thermostat::bitmap32
    { { AttributeTag(69), "ACLouverPosition", ItemType::kDefault }, kInvalidNodeIndex },                   // Thermostat::enum8
    { { AttributeTag(70), "ACCoilTemperature", ItemType::kDefault }, kInvalidNodeIndex },                  // Thermostat::int16s
    { { AttributeTag(71), "ACCapacityformat", ItemType::kDefault }, kInvalidNodeIndex },                   // Thermostat::enum8
    { { AttributeTag(65528), "generatedCommandList", ItemType::kList }, kInvalidNodeIndex }, // Thermostat::command_id[]
    { { AttributeTag(65529), "acceptedCommandList", ItemType::kList }, kInvalidNodeIndex },  // Thermostat::command_id[]
    { { AttributeTag(65530), "eventList", ItemType::kList }, kInvalidNodeIndex },            // Thermostat::event_id[]
    { { AttributeTag(65531), "attributeList", ItemType::kList }, kInvalidNodeIndex },        // Thermostat::attrib_id[]
    { { AttributeTag(65532), "featureMap", ItemType::kDefault }, kInvalidNodeIndex },        // Thermostat::bitmap32
    { { AttributeTag(65533), "clusterRevision", ItemType::kDefault }, kInvalidNodeIndex },   // Thermostat::int16u
    { { CommandTag(0), "SetpointRaiseLowerRequest", ItemType::kDefault },
      400 }, // Thermostat::SetpointRaiseLower::SetpointRaiseLowerRequest
    { { CommandTag(1), "SetWeeklyScheduleRequest", ItemType::kDefault },
      402 }, // Thermostat::SetWeeklySchedule::SetWeeklyScheduleRequest
    { { CommandTag(2), "GetWeeklyScheduleRequest", ItemType::kDefault },
      403 }, // Thermostat::GetWeeklySchedule::GetWeeklyScheduleRequest
    { { CommandTag(0), "GetWeeklyScheduleResponse", ItemType::kDefault }, 401 }, // Thermostat::GetWeeklyScheduleResponse
};

const Entry<ItemInfo> _Thermostat_ThermostatScheduleTransition[] = {
    { { ContextTag(0), "transitionTime", ItemType::kDefault }, kInvalidNodeIndex }, // Thermostat::int16u
    { { ContextTag(1), "heatSetpoint", ItemType::kDefault }, kInvalidNodeIndex },   // Thermostat::int16s
    { { ContextTag(2), "coolSetpoint", ItemType::kDefault }, kInvalidNodeIndex },   // Thermostat::int16s
};

const Entry<ItemInfo> _Thermostat_SetpointRaiseLowerRequest[] = {
    { { ContextTag(0), "mode", ItemType::kEnum }, kInvalidNodeIndex },      // Thermostat::SetpointAdjustMode
    { { ContextTag(1), "amount", ItemType::kDefault }, kInvalidNodeIndex }, // Thermostat::INT8S
};

const Entry<ItemInfo> _Thermostat_GetWeeklyScheduleResponse[] = {
    { { ContextTag(0), "numberOfTransitionsForSequence", ItemType::kDefault }, kInvalidNodeIndex }, // Thermostat::INT8U
    { { ContextTag(1), "dayOfWeekForSequence", ItemType::kBitmap }, kInvalidNodeIndex },            // Thermostat::DayOfWeek
    { { ContextTag(2), "modeForSequence", ItemType::kBitmap }, kInvalidNodeIndex },                 // Thermostat::ModeForSequence
    { { ContextTag(3), "transitions", ItemType::kList }, 404 }, // Thermostat::ThermostatScheduleTransition[]
};

const Entry<ItemInfo> _Thermostat_SetWeeklyScheduleRequest[] = {
    { { ContextTag(0), "numberOfTransitionsForSequence", ItemType::kDefault }, kInvalidNodeIndex }, // Thermostat::INT8U
    { { ContextTag(1), "dayOfWeekForSequence", ItemType::kBitmap }, kInvalidNodeIndex },            // Thermostat::DayOfWeek
    { { ContextTag(2), "modeForSequence", ItemType::kBitmap }, kInvalidNodeIndex },                 // Thermostat::ModeForSequence
    { { ContextTag(3), "transitions", ItemType::kList }, 404 }, // Thermostat::ThermostatScheduleTransition[]
};

const Entry<ItemInfo> _Thermostat_GetWeeklyScheduleRequest[] = {
    { { ContextTag(0), "daysToReturn", ItemType::kBitmap }, kInvalidNodeIndex }, // Thermostat::DayOfWeek
    { { ContextTag(1), "modeToReturn", ItemType::kBitmap }, kInvalidNodeIndex }, // Thermostat::ModeForSequence
};

const Entry<ItemInfo> _Thermostat_ThermostatScheduleTransition_list_[] = {
    { { AnonymousTag(), "[]", ItemType::kDefault }, 399 }, // Thermostat_ThermostatScheduleTransition[]
};

const Entry<ItemInfo> _FanControl[] = {
    { { AttributeTag(0), "fanMode", ItemType::kEnum }, kInvalidNodeIndex },                  // FanControl::FanModeEnum
    { { AttributeTag(1), "fanModeSequence", ItemType::kEnum }, kInvalidNodeIndex },          // FanControl::FanModeSequenceEnum
    { { AttributeTag(2), "percentSetting", ItemType::kDefault }, kInvalidNodeIndex },        // FanControl::Percent
    { { AttributeTag(3), "percentCurrent", ItemType::kDefault }, kInvalidNodeIndex },        // FanControl::Percent
    { { AttributeTag(4), "speedMax", ItemType::kDefault }, kInvalidNodeIndex },              // FanControl::int8u
    { { AttributeTag(5), "speedSetting", ItemType::kDefault }, kInvalidNodeIndex },          // FanControl::int8u
    { { AttributeTag(6), "speedCurrent", ItemType::kDefault }, kInvalidNodeIndex },          // FanControl::int8u
    { { AttributeTag(7), "rockSupport", ItemType::kBitmap }, kInvalidNodeIndex },            // FanControl::RockBitmap
    { { AttributeTag(8), "rockSetting", ItemType::kBitmap }, kInvalidNodeIndex },            // FanControl::RockBitmap
    { { AttributeTag(9), "windSupport", ItemType::kBitmap }, kInvalidNodeIndex },            // FanControl::WindBitmap
    { { AttributeTag(10), "windSetting", ItemType::kBitmap }, kInvalidNodeIndex },           // FanControl::WindBitmap
    { { AttributeTag(11), "airflowDirection", ItemType::kEnum }, kInvalidNodeIndex },        // FanControl::AirflowDirectionEnum
    { { AttributeTag(65528), "generatedCommandList", ItemType::kList }, kInvalidNodeIndex }, // FanControl::command_id[]
    { { AttributeTag(65529), "acceptedCommandList", ItemType::kList }, kInvalidNodeIndex },  // FanControl::command_id[]
    { { AttributeTag(65530), "eventList", ItemType::kList }, kInvalidNodeIndex },            // FanControl::event_id[]
    { { AttributeTag(65531), "attributeList", ItemType::kList }, kInvalidNodeIndex },        // FanControl::attrib_id[]
    { { AttributeTag(65532), "featureMap", ItemType::kDefault }, kInvalidNodeIndex },        // FanControl::bitmap32
    { { AttributeTag(65533), "clusterRevision", ItemType::kDefault }, kInvalidNodeIndex },   // FanControl::int16u
    { { CommandTag(0), "StepRequest", ItemType::kDefault }, 406 },                           // FanControl::Step::StepRequest
};

const Entry<ItemInfo> _FanControl_StepRequest[] = {
    { { ContextTag(0), "direction", ItemType::kEnum }, kInvalidNodeIndex },    // FanControl::StepDirectionEnum
    { { ContextTag(1), "wrap", ItemType::kDefault }, kInvalidNodeIndex },      // FanControl::boolean
    { { ContextTag(2), "lowestOff", ItemType::kDefault }, kInvalidNodeIndex }, // FanControl::boolean
};

const Entry<ItemInfo> _ThermostatUserInterfaceConfiguration[] = {
    { { AttributeTag(0), "temperatureDisplayMode", ItemType::kDefault },
      kInvalidNodeIndex },                                                           // ThermostatUserInterfaceConfiguration::enum8
    { { AttributeTag(1), "keypadLockout", ItemType::kDefault }, kInvalidNodeIndex }, // ThermostatUserInterfaceConfiguration::enum8
    { { AttributeTag(2), "scheduleProgrammingVisibility", ItemType::kDefault },
      kInvalidNodeIndex }, // ThermostatUserInterfaceConfiguration::enum8
    { { AttributeTag(65528), "generatedCommandList", ItemType::kList },
      kInvalidNodeIndex }, // ThermostatUserInterfaceConfiguration::command_id[]
    { { AttributeTag(65529), "acceptedCommandList", ItemType::kList },
      kInvalidNodeIndex }, // ThermostatUserInterfaceConfiguration::command_id[]
    { { AttributeTag(65530), "eventList", ItemType::kList },
      kInvalidNodeIndex }, // ThermostatUserInterfaceConfiguration::event_id[]
    { { AttributeTag(65531), "attributeList", ItemType::kList },
      kInvalidNodeIndex }, // ThermostatUserInterfaceConfiguration::attrib_id[]
    { { AttributeTag(65532), "featureMap", ItemType::kDefault },
      kInvalidNodeIndex }, // ThermostatUserInterfaceConfiguration::bitmap32
    { { AttributeTag(65533), "clusterRevision", ItemType::kDefault },
      kInvalidNodeIndex }, // ThermostatUserInterfaceConfiguration::int16u
};

const Entry<ItemInfo> _ColorControl[] = {
    { { AttributeTag(0), "currentHue", ItemType::kDefault }, kInvalidNodeIndex },                     // ColorControl::int8u
    { { AttributeTag(1), "currentSaturation", ItemType::kDefault }, kInvalidNodeIndex },              // ColorControl::int8u
    { { AttributeTag(2), "remainingTime", ItemType::kDefault }, kInvalidNodeIndex },                  // ColorControl::int16u
    { { AttributeTag(3), "currentX", ItemType::kDefault }, kInvalidNodeIndex },                       // ColorControl::int16u
    { { AttributeTag(4), "currentY", ItemType::kDefault }, kInvalidNodeIndex },                       // ColorControl::int16u
    { { AttributeTag(5), "driftCompensation", ItemType::kDefault }, kInvalidNodeIndex },              // ColorControl::enum8
    { { AttributeTag(6), "compensationText", ItemType::kDefault }, kInvalidNodeIndex },               // ColorControl::char_string
    { { AttributeTag(7), "colorTemperatureMireds", ItemType::kDefault }, kInvalidNodeIndex },         // ColorControl::int16u
    { { AttributeTag(8), "colorMode", ItemType::kDefault }, kInvalidNodeIndex },                      // ColorControl::enum8
    { { AttributeTag(15), "options", ItemType::kDefault }, kInvalidNodeIndex },                       // ColorControl::bitmap8
    { { AttributeTag(16), "numberOfPrimaries", ItemType::kDefault }, kInvalidNodeIndex },             // ColorControl::int8u
    { { AttributeTag(17), "primary1X", ItemType::kDefault }, kInvalidNodeIndex },                     // ColorControl::int16u
    { { AttributeTag(18), "primary1Y", ItemType::kDefault }, kInvalidNodeIndex },                     // ColorControl::int16u
    { { AttributeTag(19), "primary1Intensity", ItemType::kDefault }, kInvalidNodeIndex },             // ColorControl::int8u
    { { AttributeTag(21), "primary2X", ItemType::kDefault }, kInvalidNodeIndex },                     // ColorControl::int16u
    { { AttributeTag(22), "primary2Y", ItemType::kDefault }, kInvalidNodeIndex },                     // ColorControl::int16u
    { { AttributeTag(23), "primary2Intensity", ItemType::kDefault }, kInvalidNodeIndex },             // ColorControl::int8u
    { { AttributeTag(25), "primary3X", ItemType::kDefault }, kInvalidNodeIndex },                     // ColorControl::int16u
    { { AttributeTag(26), "primary3Y", ItemType::kDefault }, kInvalidNodeIndex },                     // ColorControl::int16u
    { { AttributeTag(27), "primary3Intensity", ItemType::kDefault }, kInvalidNodeIndex },             // ColorControl::int8u
    { { AttributeTag(32), "primary4X", ItemType::kDefault }, kInvalidNodeIndex },                     // ColorControl::int16u
    { { AttributeTag(33), "primary4Y", ItemType::kDefault }, kInvalidNodeIndex },                     // ColorControl::int16u
    { { AttributeTag(34), "primary4Intensity", ItemType::kDefault }, kInvalidNodeIndex },             // ColorControl::int8u
    { { AttributeTag(36), "primary5X", ItemType::kDefault }, kInvalidNodeIndex },                     // ColorControl::int16u
    { { AttributeTag(37), "primary5Y", ItemType::kDefault }, kInvalidNodeIndex },                     // ColorControl::int16u
    { { AttributeTag(38), "primary5Intensity", ItemType::kDefault }, kInvalidNodeIndex },             // ColorControl::int8u
    { { AttributeTag(40), "primary6X", ItemType::kDefault }, kInvalidNodeIndex },                     // ColorControl::int16u
    { { AttributeTag(41), "primary6Y", ItemType::kDefault }, kInvalidNodeIndex },                     // ColorControl::int16u
    { { AttributeTag(42), "primary6Intensity", ItemType::kDefault }, kInvalidNodeIndex },             // ColorControl::int8u
    { { AttributeTag(48), "whitePointX", ItemType::kDefault }, kInvalidNodeIndex },                   // ColorControl::int16u
    { { AttributeTag(49), "whitePointY", ItemType::kDefault }, kInvalidNodeIndex },                   // ColorControl::int16u
    { { AttributeTag(50), "colorPointRX", ItemType::kDefault }, kInvalidNodeIndex },                  // ColorControl::int16u
    { { AttributeTag(51), "colorPointRY", ItemType::kDefault }, kInvalidNodeIndex },                  // ColorControl::int16u
    { { AttributeTag(52), "colorPointRIntensity", ItemType::kDefault }, kInvalidNodeIndex },          // ColorControl::int8u
    { { AttributeTag(54), "colorPointGX", ItemType::kDefault }, kInvalidNodeIndex },                  // ColorControl::int16u
    { { AttributeTag(55), "colorPointGY", ItemType::kDefault }, kInvalidNodeIndex },                  // ColorControl::int16u
    { { AttributeTag(56), "colorPointGIntensity", ItemType::kDefault }, kInvalidNodeIndex },          // ColorControl::int8u
    { { AttributeTag(58), "colorPointBX", ItemType::kDefault }, kInvalidNodeIndex },                  // ColorControl::int16u
    { { AttributeTag(59), "colorPointBY", ItemType::kDefault }, kInvalidNodeIndex },                  // ColorControl::int16u
    { { AttributeTag(60), "colorPointBIntensity", ItemType::kDefault }, kInvalidNodeIndex },          // ColorControl::int8u
    { { AttributeTag(16384), "enhancedCurrentHue", ItemType::kDefault }, kInvalidNodeIndex },         // ColorControl::int16u
    { { AttributeTag(16385), "enhancedColorMode", ItemType::kDefault }, kInvalidNodeIndex },          // ColorControl::enum8
    { { AttributeTag(16386), "colorLoopActive", ItemType::kDefault }, kInvalidNodeIndex },            // ColorControl::int8u
    { { AttributeTag(16387), "colorLoopDirection", ItemType::kDefault }, kInvalidNodeIndex },         // ColorControl::int8u
    { { AttributeTag(16388), "colorLoopTime", ItemType::kDefault }, kInvalidNodeIndex },              // ColorControl::int16u
    { { AttributeTag(16389), "colorLoopStartEnhancedHue", ItemType::kDefault }, kInvalidNodeIndex },  // ColorControl::int16u
    { { AttributeTag(16390), "colorLoopStoredEnhancedHue", ItemType::kDefault }, kInvalidNodeIndex }, // ColorControl::int16u
    { { AttributeTag(16394), "colorCapabilities", ItemType::kDefault }, kInvalidNodeIndex },          // ColorControl::bitmap16
    { { AttributeTag(16395), "colorTempPhysicalMinMireds", ItemType::kDefault }, kInvalidNodeIndex }, // ColorControl::int16u
    { { AttributeTag(16396), "colorTempPhysicalMaxMireds", ItemType::kDefault }, kInvalidNodeIndex }, // ColorControl::int16u
    { { AttributeTag(16397), "coupleColorTempToLevelMinMireds", ItemType::kDefault }, kInvalidNodeIndex }, // ColorControl::int16u
    { { AttributeTag(16400), "startUpColorTemperatureMireds", ItemType::kDefault }, kInvalidNodeIndex },   // ColorControl::int16u
    { { AttributeTag(65528), "generatedCommandList", ItemType::kList }, kInvalidNodeIndex }, // ColorControl::command_id[]
    { { AttributeTag(65529), "acceptedCommandList", ItemType::kList }, kInvalidNodeIndex },  // ColorControl::command_id[]
    { { AttributeTag(65530), "eventList", ItemType::kList }, kInvalidNodeIndex },            // ColorControl::event_id[]
    { { AttributeTag(65531), "attributeList", ItemType::kList }, kInvalidNodeIndex },        // ColorControl::attrib_id[]
    { { AttributeTag(65532), "featureMap", ItemType::kDefault }, kInvalidNodeIndex },        // ColorControl::bitmap32
    { { AttributeTag(65533), "clusterRevision", ItemType::kDefault }, kInvalidNodeIndex },   // ColorControl::int16u
    { { CommandTag(0), "MoveToHueRequest", ItemType::kDefault }, 409 }, // ColorControl::MoveToHue::MoveToHueRequest
    { { CommandTag(1), "MoveHueRequest", ItemType::kDefault }, 410 },   // ColorControl::MoveHue::MoveHueRequest
    { { CommandTag(2), "StepHueRequest", ItemType::kDefault }, 411 },   // ColorControl::StepHue::StepHueRequest
    { { CommandTag(3), "MoveToSaturationRequest", ItemType::kDefault },
      412 }, // ColorControl::MoveToSaturation::MoveToSaturationRequest
    { { CommandTag(4), "MoveSaturationRequest", ItemType::kDefault }, 413 }, // ColorControl::MoveSaturation::MoveSaturationRequest
    { { CommandTag(5), "StepSaturationRequest", ItemType::kDefault }, 414 }, // ColorControl::StepSaturation::StepSaturationRequest
    { { CommandTag(6), "MoveToHueAndSaturationRequest", ItemType::kDefault },
      415 }, // ColorControl::MoveToHueAndSaturation::MoveToHueAndSaturationRequest
    { { CommandTag(7), "MoveToColorRequest", ItemType::kDefault }, 416 }, // ColorControl::MoveToColor::MoveToColorRequest
    { { CommandTag(8), "MoveColorRequest", ItemType::kDefault }, 417 },   // ColorControl::MoveColor::MoveColorRequest
    { { CommandTag(9), "StepColorRequest", ItemType::kDefault }, 418 },   // ColorControl::StepColor::StepColorRequest
    { { CommandTag(10), "MoveToColorTemperatureRequest", ItemType::kDefault },
      419 }, // ColorControl::MoveToColorTemperature::MoveToColorTemperatureRequest
    { { CommandTag(64), "EnhancedMoveToHueRequest", ItemType::kDefault },
      420 }, // ColorControl::EnhancedMoveToHue::EnhancedMoveToHueRequest
    { { CommandTag(65), "EnhancedMoveHueRequest", ItemType::kDefault },
      421 }, // ColorControl::EnhancedMoveHue::EnhancedMoveHueRequest
    { { CommandTag(66), "EnhancedStepHueRequest", ItemType::kDefault },
      422 }, // ColorControl::EnhancedStepHue::EnhancedStepHueRequest
    { { CommandTag(67), "EnhancedMoveToHueAndSaturationRequest", ItemType::kDefault },
      423 }, // ColorControl::EnhancedMoveToHueAndSaturation::EnhancedMoveToHueAndSaturationRequest
    { { CommandTag(68), "ColorLoopSetRequest", ItemType::kDefault }, 424 }, // ColorControl::ColorLoopSet::ColorLoopSetRequest
    { { CommandTag(71), "StopMoveStepRequest", ItemType::kDefault }, 425 }, // ColorControl::StopMoveStep::StopMoveStepRequest
    { { CommandTag(75), "MoveColorTemperatureRequest", ItemType::kDefault },
      426 }, // ColorControl::MoveColorTemperature::MoveColorTemperatureRequest
    { { CommandTag(76), "StepColorTemperatureRequest", ItemType::kDefault },
      427 }, // ColorControl::StepColorTemperature::StepColorTemperatureRequest
};

const Entry<ItemInfo> _ColorControl_MoveToHueRequest[] = {
    { { ContextTag(0), "hue", ItemType::kDefault }, kInvalidNodeIndex },             // ColorControl::INT8U
    { { ContextTag(1), "direction", ItemType::kEnum }, kInvalidNodeIndex },          // ColorControl::HueDirection
    { { ContextTag(2), "transitionTime", ItemType::kDefault }, kInvalidNodeIndex },  // ColorControl::INT16U
    { { ContextTag(3), "optionsMask", ItemType::kDefault }, kInvalidNodeIndex },     // ColorControl::BITMAP8
    { { ContextTag(4), "optionsOverride", ItemType::kDefault }, kInvalidNodeIndex }, // ColorControl::BITMAP8
};

const Entry<ItemInfo> _ColorControl_MoveHueRequest[] = {
    { { ContextTag(0), "moveMode", ItemType::kEnum }, kInvalidNodeIndex },           // ColorControl::HueMoveMode
    { { ContextTag(1), "rate", ItemType::kDefault }, kInvalidNodeIndex },            // ColorControl::INT8U
    { { ContextTag(2), "optionsMask", ItemType::kDefault }, kInvalidNodeIndex },     // ColorControl::BITMAP8
    { { ContextTag(3), "optionsOverride", ItemType::kDefault }, kInvalidNodeIndex }, // ColorControl::BITMAP8
};

const Entry<ItemInfo> _ColorControl_StepHueRequest[] = {
    { { ContextTag(0), "stepMode", ItemType::kEnum }, kInvalidNodeIndex },           // ColorControl::HueStepMode
    { { ContextTag(1), "stepSize", ItemType::kDefault }, kInvalidNodeIndex },        // ColorControl::INT8U
    { { ContextTag(2), "transitionTime", ItemType::kDefault }, kInvalidNodeIndex },  // ColorControl::INT8U
    { { ContextTag(3), "optionsMask", ItemType::kDefault }, kInvalidNodeIndex },     // ColorControl::BITMAP8
    { { ContextTag(4), "optionsOverride", ItemType::kDefault }, kInvalidNodeIndex }, // ColorControl::BITMAP8
};

const Entry<ItemInfo> _ColorControl_MoveToSaturationRequest[] = {
    { { ContextTag(0), "saturation", ItemType::kDefault }, kInvalidNodeIndex },      // ColorControl::INT8U
    { { ContextTag(1), "transitionTime", ItemType::kDefault }, kInvalidNodeIndex },  // ColorControl::INT16U
    { { ContextTag(2), "optionsMask", ItemType::kDefault }, kInvalidNodeIndex },     // ColorControl::BITMAP8
    { { ContextTag(3), "optionsOverride", ItemType::kDefault }, kInvalidNodeIndex }, // ColorControl::BITMAP8
};

const Entry<ItemInfo> _ColorControl_MoveSaturationRequest[] = {
    { { ContextTag(0), "moveMode", ItemType::kEnum }, kInvalidNodeIndex },           // ColorControl::SaturationMoveMode
    { { ContextTag(1), "rate", ItemType::kDefault }, kInvalidNodeIndex },            // ColorControl::INT8U
    { { ContextTag(2), "optionsMask", ItemType::kDefault }, kInvalidNodeIndex },     // ColorControl::BITMAP8
    { { ContextTag(3), "optionsOverride", ItemType::kDefault }, kInvalidNodeIndex }, // ColorControl::BITMAP8
};

const Entry<ItemInfo> _ColorControl_StepSaturationRequest[] = {
    { { ContextTag(0), "stepMode", ItemType::kEnum }, kInvalidNodeIndex },           // ColorControl::SaturationStepMode
    { { ContextTag(1), "stepSize", ItemType::kDefault }, kInvalidNodeIndex },        // ColorControl::INT8U
    { { ContextTag(2), "transitionTime", ItemType::kDefault }, kInvalidNodeIndex },  // ColorControl::INT8U
    { { ContextTag(3), "optionsMask", ItemType::kDefault }, kInvalidNodeIndex },     // ColorControl::BITMAP8
    { { ContextTag(4), "optionsOverride", ItemType::kDefault }, kInvalidNodeIndex }, // ColorControl::BITMAP8
};

const Entry<ItemInfo> _ColorControl_MoveToHueAndSaturationRequest[] = {
    { { ContextTag(0), "hue", ItemType::kDefault }, kInvalidNodeIndex },             // ColorControl::INT8U
    { { ContextTag(1), "saturation", ItemType::kDefault }, kInvalidNodeIndex },      // ColorControl::INT8U
    { { ContextTag(2), "transitionTime", ItemType::kDefault }, kInvalidNodeIndex },  // ColorControl::INT16U
    { { ContextTag(3), "optionsMask", ItemType::kDefault }, kInvalidNodeIndex },     // ColorControl::BITMAP8
    { { ContextTag(4), "optionsOverride", ItemType::kDefault }, kInvalidNodeIndex }, // ColorControl::BITMAP8
};

const Entry<ItemInfo> _ColorControl_MoveToColorRequest[] = {
    { { ContextTag(0), "colorX", ItemType::kDefault }, kInvalidNodeIndex },          // ColorControl::INT16U
    { { ContextTag(1), "colorY", ItemType::kDefault }, kInvalidNodeIndex },          // ColorControl::INT16U
    { { ContextTag(2), "transitionTime", ItemType::kDefault }, kInvalidNodeIndex },  // ColorControl::INT16U
    { { ContextTag(3), "optionsMask", ItemType::kDefault }, kInvalidNodeIndex },     // ColorControl::BITMAP8
    { { ContextTag(4), "optionsOverride", ItemType::kDefault }, kInvalidNodeIndex }, // ColorControl::BITMAP8
};

const Entry<ItemInfo> _ColorControl_MoveColorRequest[] = {
    { { ContextTag(0), "rateX", ItemType::kDefault }, kInvalidNodeIndex },           // ColorControl::INT16S
    { { ContextTag(1), "rateY", ItemType::kDefault }, kInvalidNodeIndex },           // ColorControl::INT16S
    { { ContextTag(2), "optionsMask", ItemType::kDefault }, kInvalidNodeIndex },     // ColorControl::BITMAP8
    { { ContextTag(3), "optionsOverride", ItemType::kDefault }, kInvalidNodeIndex }, // ColorControl::BITMAP8
};

const Entry<ItemInfo> _ColorControl_StepColorRequest[] = {
    { { ContextTag(0), "stepX", ItemType::kDefault }, kInvalidNodeIndex },           // ColorControl::INT16S
    { { ContextTag(1), "stepY", ItemType::kDefault }, kInvalidNodeIndex },           // ColorControl::INT16S
    { { ContextTag(2), "transitionTime", ItemType::kDefault }, kInvalidNodeIndex },  // ColorControl::INT16U
    { { ContextTag(3), "optionsMask", ItemType::kDefault }, kInvalidNodeIndex },     // ColorControl::BITMAP8
    { { ContextTag(4), "optionsOverride", ItemType::kDefault }, kInvalidNodeIndex }, // ColorControl::BITMAP8
};

const Entry<ItemInfo> _ColorControl_MoveToColorTemperatureRequest[] = {
    { { ContextTag(0), "colorTemperatureMireds", ItemType::kDefault }, kInvalidNodeIndex }, // ColorControl::INT16U
    { { ContextTag(1), "transitionTime", ItemType::kDefault }, kInvalidNodeIndex },         // ColorControl::INT16U
    { { ContextTag(2), "optionsMask", ItemType::kDefault }, kInvalidNodeIndex },            // ColorControl::BITMAP8
    { { ContextTag(3), "optionsOverride", ItemType::kDefault }, kInvalidNodeIndex },        // ColorControl::BITMAP8
};

const Entry<ItemInfo> _ColorControl_EnhancedMoveToHueRequest[] = {
    { { ContextTag(0), "enhancedHue", ItemType::kDefault }, kInvalidNodeIndex },     // ColorControl::INT16U
    { { ContextTag(1), "direction", ItemType::kEnum }, kInvalidNodeIndex },          // ColorControl::HueDirection
    { { ContextTag(2), "transitionTime", ItemType::kDefault }, kInvalidNodeIndex },  // ColorControl::INT16U
    { { ContextTag(3), "optionsMask", ItemType::kDefault }, kInvalidNodeIndex },     // ColorControl::BITMAP8
    { { ContextTag(4), "optionsOverride", ItemType::kDefault }, kInvalidNodeIndex }, // ColorControl::BITMAP8
};

const Entry<ItemInfo> _ColorControl_EnhancedMoveHueRequest[] = {
    { { ContextTag(0), "moveMode", ItemType::kEnum }, kInvalidNodeIndex },           // ColorControl::HueMoveMode
    { { ContextTag(1), "rate", ItemType::kDefault }, kInvalidNodeIndex },            // ColorControl::INT16U
    { { ContextTag(2), "optionsMask", ItemType::kDefault }, kInvalidNodeIndex },     // ColorControl::BITMAP8
    { { ContextTag(3), "optionsOverride", ItemType::kDefault }, kInvalidNodeIndex }, // ColorControl::BITMAP8
};

const Entry<ItemInfo> _ColorControl_EnhancedStepHueRequest[] = {
    { { ContextTag(0), "stepMode", ItemType::kEnum }, kInvalidNodeIndex },           // ColorControl::HueStepMode
    { { ContextTag(1), "stepSize", ItemType::kDefault }, kInvalidNodeIndex },        // ColorControl::INT16U
    { { ContextTag(2), "transitionTime", ItemType::kDefault }, kInvalidNodeIndex },  // ColorControl::INT16U
    { { ContextTag(3), "optionsMask", ItemType::kDefault }, kInvalidNodeIndex },     // ColorControl::BITMAP8
    { { ContextTag(4), "optionsOverride", ItemType::kDefault }, kInvalidNodeIndex }, // ColorControl::BITMAP8
};

const Entry<ItemInfo> _ColorControl_EnhancedMoveToHueAndSaturationRequest[] = {
    { { ContextTag(0), "enhancedHue", ItemType::kDefault }, kInvalidNodeIndex },     // ColorControl::INT16U
    { { ContextTag(1), "saturation", ItemType::kDefault }, kInvalidNodeIndex },      // ColorControl::INT8U
    { { ContextTag(2), "transitionTime", ItemType::kDefault }, kInvalidNodeIndex },  // ColorControl::INT16U
    { { ContextTag(3), "optionsMask", ItemType::kDefault }, kInvalidNodeIndex },     // ColorControl::BITMAP8
    { { ContextTag(4), "optionsOverride", ItemType::kDefault }, kInvalidNodeIndex }, // ColorControl::BITMAP8
};

const Entry<ItemInfo> _ColorControl_ColorLoopSetRequest[] = {
    { { ContextTag(0), "updateFlags", ItemType::kBitmap }, kInvalidNodeIndex },      // ColorControl::ColorLoopUpdateFlags
    { { ContextTag(1), "action", ItemType::kEnum }, kInvalidNodeIndex },             // ColorControl::ColorLoopAction
    { { ContextTag(2), "direction", ItemType::kEnum }, kInvalidNodeIndex },          // ColorControl::ColorLoopDirection
    { { ContextTag(3), "time", ItemType::kDefault }, kInvalidNodeIndex },            // ColorControl::INT16U
    { { ContextTag(4), "startHue", ItemType::kDefault }, kInvalidNodeIndex },        // ColorControl::INT16U
    { { ContextTag(5), "optionsMask", ItemType::kDefault }, kInvalidNodeIndex },     // ColorControl::BITMAP8
    { { ContextTag(6), "optionsOverride", ItemType::kDefault }, kInvalidNodeIndex }, // ColorControl::BITMAP8
};

const Entry<ItemInfo> _ColorControl_StopMoveStepRequest[] = {
    { { ContextTag(0), "optionsMask", ItemType::kDefault }, kInvalidNodeIndex },     // ColorControl::BITMAP8
    { { ContextTag(1), "optionsOverride", ItemType::kDefault }, kInvalidNodeIndex }, // ColorControl::BITMAP8
};

const Entry<ItemInfo> _ColorControl_MoveColorTemperatureRequest[] = {
    { { ContextTag(0), "moveMode", ItemType::kEnum }, kInvalidNodeIndex },                         // ColorControl::HueMoveMode
    { { ContextTag(1), "rate", ItemType::kDefault }, kInvalidNodeIndex },                          // ColorControl::INT16U
    { { ContextTag(2), "colorTemperatureMinimumMireds", ItemType::kDefault }, kInvalidNodeIndex }, // ColorControl::INT16U
    { { ContextTag(3), "colorTemperatureMaximumMireds", ItemType::kDefault }, kInvalidNodeIndex }, // ColorControl::INT16U
    { { ContextTag(4), "optionsMask", ItemType::kDefault }, kInvalidNodeIndex },                   // ColorControl::BITMAP8
    { { ContextTag(5), "optionsOverride", ItemType::kDefault }, kInvalidNodeIndex },               // ColorControl::BITMAP8
};

const Entry<ItemInfo> _ColorControl_StepColorTemperatureRequest[] = {
    { { ContextTag(0), "stepMode", ItemType::kEnum }, kInvalidNodeIndex },                         // ColorControl::HueStepMode
    { { ContextTag(1), "stepSize", ItemType::kDefault }, kInvalidNodeIndex },                      // ColorControl::INT16U
    { { ContextTag(2), "transitionTime", ItemType::kDefault }, kInvalidNodeIndex },                // ColorControl::INT16U
    { { ContextTag(3), "colorTemperatureMinimumMireds", ItemType::kDefault }, kInvalidNodeIndex }, // ColorControl::INT16U
    { { ContextTag(4), "colorTemperatureMaximumMireds", ItemType::kDefault }, kInvalidNodeIndex }, // ColorControl::INT16U
    { { ContextTag(5), "optionsMask", ItemType::kDefault }, kInvalidNodeIndex },                   // ColorControl::BITMAP8
    { { ContextTag(6), "optionsOverride", ItemType::kDefault }, kInvalidNodeIndex },               // ColorControl::BITMAP8
};

const Entry<ItemInfo> _BallastConfiguration[] = {
    { { AttributeTag(0), "physicalMinLevel", ItemType::kDefault }, kInvalidNodeIndex },         // BallastConfiguration::int8u
    { { AttributeTag(1), "physicalMaxLevel", ItemType::kDefault }, kInvalidNodeIndex },         // BallastConfiguration::int8u
    { { AttributeTag(2), "ballastStatus", ItemType::kDefault }, kInvalidNodeIndex },            // BallastConfiguration::bitmap8
    { { AttributeTag(16), "minLevel", ItemType::kDefault }, kInvalidNodeIndex },                // BallastConfiguration::int8u
    { { AttributeTag(17), "maxLevel", ItemType::kDefault }, kInvalidNodeIndex },                // BallastConfiguration::int8u
    { { AttributeTag(20), "intrinsicBallastFactor", ItemType::kDefault }, kInvalidNodeIndex },  // BallastConfiguration::int8u
    { { AttributeTag(21), "ballastFactorAdjustment", ItemType::kDefault }, kInvalidNodeIndex }, // BallastConfiguration::int8u
    { { AttributeTag(32), "lampQuantity", ItemType::kDefault }, kInvalidNodeIndex },            // BallastConfiguration::int8u
    { { AttributeTag(48), "lampType", ItemType::kDefault }, kInvalidNodeIndex },                // BallastConfiguration::char_string
    { { AttributeTag(49), "lampManufacturer", ItemType::kDefault }, kInvalidNodeIndex },        // BallastConfiguration::char_string
    { { AttributeTag(50), "lampRatedHours", ItemType::kDefault }, kInvalidNodeIndex },          // BallastConfiguration::int24u
    { { AttributeTag(51), "lampBurnHours", ItemType::kDefault }, kInvalidNodeIndex },           // BallastConfiguration::int24u
    { { AttributeTag(52), "lampAlarmMode", ItemType::kDefault }, kInvalidNodeIndex },           // BallastConfiguration::bitmap8
    { { AttributeTag(53), "lampBurnHoursTripPoint", ItemType::kDefault }, kInvalidNodeIndex },  // BallastConfiguration::int24u
    { { AttributeTag(65528), "generatedCommandList", ItemType::kList }, kInvalidNodeIndex }, // BallastConfiguration::command_id[]
    { { AttributeTag(65529), "acceptedCommandList", ItemType::kList }, kInvalidNodeIndex },  // BallastConfiguration::command_id[]
    { { AttributeTag(65530), "eventList", ItemType::kList }, kInvalidNodeIndex },            // BallastConfiguration::event_id[]
    { { AttributeTag(65531), "attributeList", ItemType::kList }, kInvalidNodeIndex },        // BallastConfiguration::attrib_id[]
    { { AttributeTag(65532), "featureMap", ItemType::kDefault }, kInvalidNodeIndex },        // BallastConfiguration::bitmap32
    { { AttributeTag(65533), "clusterRevision", ItemType::kDefault }, kInvalidNodeIndex },   // BallastConfiguration::int16u
};

const Entry<ItemInfo> _IlluminanceMeasurement[] = {
    { { AttributeTag(0), "measuredValue", ItemType::kDefault }, kInvalidNodeIndex },    // IlluminanceMeasurement::int16u
    { { AttributeTag(1), "minMeasuredValue", ItemType::kDefault }, kInvalidNodeIndex }, // IlluminanceMeasurement::int16u
    { { AttributeTag(2), "maxMeasuredValue", ItemType::kDefault }, kInvalidNodeIndex }, // IlluminanceMeasurement::int16u
    { { AttributeTag(3), "tolerance", ItemType::kDefault }, kInvalidNodeIndex },        // IlluminanceMeasurement::int16u
    { { AttributeTag(4), "lightSensorType", ItemType::kEnum }, kInvalidNodeIndex }, // IlluminanceMeasurement::LightSensorTypeEnum
    { { AttributeTag(65528), "generatedCommandList", ItemType::kList }, kInvalidNodeIndex }, // IlluminanceMeasurement::command_id[]
    { { AttributeTag(65529), "acceptedCommandList", ItemType::kList }, kInvalidNodeIndex },  // IlluminanceMeasurement::command_id[]
    { { AttributeTag(65530), "eventList", ItemType::kList }, kInvalidNodeIndex },            // IlluminanceMeasurement::event_id[]
    { { AttributeTag(65531), "attributeList", ItemType::kList }, kInvalidNodeIndex },        // IlluminanceMeasurement::attrib_id[]
    { { AttributeTag(65532), "featureMap", ItemType::kDefault }, kInvalidNodeIndex },        // IlluminanceMeasurement::bitmap32
    { { AttributeTag(65533), "clusterRevision", ItemType::kDefault }, kInvalidNodeIndex },   // IlluminanceMeasurement::int16u
};

const Entry<ItemInfo> _TemperatureMeasurement[] = {
    { { AttributeTag(0), "measuredValue", ItemType::kDefault }, kInvalidNodeIndex },         // TemperatureMeasurement::int16s
    { { AttributeTag(1), "minMeasuredValue", ItemType::kDefault }, kInvalidNodeIndex },      // TemperatureMeasurement::int16s
    { { AttributeTag(2), "maxMeasuredValue", ItemType::kDefault }, kInvalidNodeIndex },      // TemperatureMeasurement::int16s
    { { AttributeTag(3), "tolerance", ItemType::kDefault }, kInvalidNodeIndex },             // TemperatureMeasurement::int16u
    { { AttributeTag(65528), "generatedCommandList", ItemType::kList }, kInvalidNodeIndex }, // TemperatureMeasurement::command_id[]
    { { AttributeTag(65529), "acceptedCommandList", ItemType::kList }, kInvalidNodeIndex },  // TemperatureMeasurement::command_id[]
    { { AttributeTag(65530), "eventList", ItemType::kList }, kInvalidNodeIndex },            // TemperatureMeasurement::event_id[]
    { { AttributeTag(65531), "attributeList", ItemType::kList }, kInvalidNodeIndex },        // TemperatureMeasurement::attrib_id[]
    { { AttributeTag(65532), "featureMap", ItemType::kDefault }, kInvalidNodeIndex },        // TemperatureMeasurement::bitmap32
    { { AttributeTag(65533), "clusterRevision", ItemType::kDefault }, kInvalidNodeIndex },   // TemperatureMeasurement::int16u
};

const Entry<ItemInfo> _PressureMeasurement[] = {
    { { AttributeTag(0), "measuredValue", ItemType::kDefault }, kInvalidNodeIndex },         // PressureMeasurement::int16s
    { { AttributeTag(1), "minMeasuredValue", ItemType::kDefault }, kInvalidNodeIndex },      // PressureMeasurement::int16s
    { { AttributeTag(2), "maxMeasuredValue", ItemType::kDefault }, kInvalidNodeIndex },      // PressureMeasurement::int16s
    { { AttributeTag(3), "tolerance", ItemType::kDefault }, kInvalidNodeIndex },             // PressureMeasurement::int16u
    { { AttributeTag(16), "scaledValue", ItemType::kDefault }, kInvalidNodeIndex },          // PressureMeasurement::int16s
    { { AttributeTag(17), "minScaledValue", ItemType::kDefault }, kInvalidNodeIndex },       // PressureMeasurement::int16s
    { { AttributeTag(18), "maxScaledValue", ItemType::kDefault }, kInvalidNodeIndex },       // PressureMeasurement::int16s
    { { AttributeTag(19), "scaledTolerance", ItemType::kDefault }, kInvalidNodeIndex },      // PressureMeasurement::int16u
    { { AttributeTag(20), "scale", ItemType::kDefault }, kInvalidNodeIndex },                // PressureMeasurement::int8s
    { { AttributeTag(65528), "generatedCommandList", ItemType::kList }, kInvalidNodeIndex }, // PressureMeasurement::command_id[]
    { { AttributeTag(65529), "acceptedCommandList", ItemType::kList }, kInvalidNodeIndex },  // PressureMeasurement::command_id[]
    { { AttributeTag(65530), "eventList", ItemType::kList }, kInvalidNodeIndex },            // PressureMeasurement::event_id[]
    { { AttributeTag(65531), "attributeList", ItemType::kList }, kInvalidNodeIndex },        // PressureMeasurement::attrib_id[]
    { { AttributeTag(65532), "featureMap", ItemType::kDefault }, kInvalidNodeIndex },        // PressureMeasurement::bitmap32
    { { AttributeTag(65533), "clusterRevision", ItemType::kDefault }, kInvalidNodeIndex },   // PressureMeasurement::int16u
};

const Entry<ItemInfo> _FlowMeasurement[] = {
    { { AttributeTag(0), "measuredValue", ItemType::kDefault }, kInvalidNodeIndex },         // FlowMeasurement::int16u
    { { AttributeTag(1), "minMeasuredValue", ItemType::kDefault }, kInvalidNodeIndex },      // FlowMeasurement::int16u
    { { AttributeTag(2), "maxMeasuredValue", ItemType::kDefault }, kInvalidNodeIndex },      // FlowMeasurement::int16u
    { { AttributeTag(3), "tolerance", ItemType::kDefault }, kInvalidNodeIndex },             // FlowMeasurement::int16u
    { { AttributeTag(65528), "generatedCommandList", ItemType::kList }, kInvalidNodeIndex }, // FlowMeasurement::command_id[]
    { { AttributeTag(65529), "acceptedCommandList", ItemType::kList }, kInvalidNodeIndex },  // FlowMeasurement::command_id[]
    { { AttributeTag(65530), "eventList", ItemType::kList }, kInvalidNodeIndex },            // FlowMeasurement::event_id[]
    { { AttributeTag(65531), "attributeList", ItemType::kList }, kInvalidNodeIndex },        // FlowMeasurement::attrib_id[]
    { { AttributeTag(65532), "featureMap", ItemType::kDefault }, kInvalidNodeIndex },        // FlowMeasurement::bitmap32
    { { AttributeTag(65533), "clusterRevision", ItemType::kDefault }, kInvalidNodeIndex },   // FlowMeasurement::int16u
};

const Entry<ItemInfo> _RelativeHumidityMeasurement[] = {
    { { AttributeTag(0), "measuredValue", ItemType::kDefault }, kInvalidNodeIndex },    // RelativeHumidityMeasurement::int16u
    { { AttributeTag(1), "minMeasuredValue", ItemType::kDefault }, kInvalidNodeIndex }, // RelativeHumidityMeasurement::int16u
    { { AttributeTag(2), "maxMeasuredValue", ItemType::kDefault }, kInvalidNodeIndex }, // RelativeHumidityMeasurement::int16u
    { { AttributeTag(3), "tolerance", ItemType::kDefault }, kInvalidNodeIndex },        // RelativeHumidityMeasurement::int16u
    { { AttributeTag(65528), "generatedCommandList", ItemType::kList },
      kInvalidNodeIndex }, // RelativeHumidityMeasurement::command_id[]
    { { AttributeTag(65529), "acceptedCommandList", ItemType::kList },
      kInvalidNodeIndex },                                                            // RelativeHumidityMeasurement::command_id[]
    { { AttributeTag(65530), "eventList", ItemType::kList }, kInvalidNodeIndex },     // RelativeHumidityMeasurement::event_id[]
    { { AttributeTag(65531), "attributeList", ItemType::kList }, kInvalidNodeIndex }, // RelativeHumidityMeasurement::attrib_id[]
    { { AttributeTag(65532), "featureMap", ItemType::kDefault }, kInvalidNodeIndex }, // RelativeHumidityMeasurement::bitmap32
    { { AttributeTag(65533), "clusterRevision", ItemType::kDefault }, kInvalidNodeIndex }, // RelativeHumidityMeasurement::int16u
};

const Entry<ItemInfo> _OccupancySensing[] = {
    { { AttributeTag(0), "occupancy", ItemType::kBitmap }, kInvalidNodeIndex },         // OccupancySensing::OccupancyBitmap
    { { AttributeTag(1), "occupancySensorType", ItemType::kEnum }, kInvalidNodeIndex }, // OccupancySensing::OccupancySensorTypeEnum
    { { AttributeTag(2), "occupancySensorTypeBitmap", ItemType::kBitmap },
      kInvalidNodeIndex }, // OccupancySensing::OccupancySensorTypeBitmap
    { { AttributeTag(16), "PIROccupiedToUnoccupiedDelay", ItemType::kDefault }, kInvalidNodeIndex },     // OccupancySensing::int16u
    { { AttributeTag(17), "PIRUnoccupiedToOccupiedDelay", ItemType::kDefault }, kInvalidNodeIndex },     // OccupancySensing::int16u
    { { AttributeTag(18), "PIRUnoccupiedToOccupiedThreshold", ItemType::kDefault }, kInvalidNodeIndex }, // OccupancySensing::int8u
    { { AttributeTag(32), "ultrasonicOccupiedToUnoccupiedDelay", ItemType::kDefault },
      kInvalidNodeIndex }, // OccupancySensing::int16u
    { { AttributeTag(33), "ultrasonicUnoccupiedToOccupiedDelay", ItemType::kDefault },
      kInvalidNodeIndex }, // OccupancySensing::int16u
    { { AttributeTag(34), "ultrasonicUnoccupiedToOccupiedThreshold", ItemType::kDefault },
      kInvalidNodeIndex }, // OccupancySensing::int8u
    { { AttributeTag(48), "physicalContactOccupiedToUnoccupiedDelay", ItemType::kDefault },
      kInvalidNodeIndex }, // OccupancySensing::int16u
    { { AttributeTag(49), "physicalContactUnoccupiedToOccupiedDelay", ItemType::kDefault },
      kInvalidNodeIndex }, // OccupancySensing::int16u
    { { AttributeTag(50), "physicalContactUnoccupiedToOccupiedThreshold", ItemType::kDefault },
      kInvalidNodeIndex },                                                                   // OccupancySensing::int8u
    { { AttributeTag(65528), "generatedCommandList", ItemType::kList }, kInvalidNodeIndex }, // OccupancySensing::command_id[]
    { { AttributeTag(65529), "acceptedCommandList", ItemType::kList }, kInvalidNodeIndex },  // OccupancySensing::command_id[]
    { { AttributeTag(65530), "eventList", ItemType::kList }, kInvalidNodeIndex },            // OccupancySensing::event_id[]
    { { AttributeTag(65531), "attributeList", ItemType::kList }, kInvalidNodeIndex },        // OccupancySensing::attrib_id[]
    { { AttributeTag(65532), "featureMap", ItemType::kDefault }, kInvalidNodeIndex },        // OccupancySensing::bitmap32
    { { AttributeTag(65533), "clusterRevision", ItemType::kDefault }, kInvalidNodeIndex },   // OccupancySensing::int16u
};

const Entry<ItemInfo> _CarbonMonoxideConcentrationMeasurement[] = {
    { { AttributeTag(0), "measuredValue", ItemType::kDefault },
      kInvalidNodeIndex }, // CarbonMonoxideConcentrationMeasurement::single
    { { AttributeTag(1), "minMeasuredValue", ItemType::kDefault },
      kInvalidNodeIndex }, // CarbonMonoxideConcentrationMeasurement::single
    { { AttributeTag(2), "maxMeasuredValue", ItemType::kDefault },
      kInvalidNodeIndex }, // CarbonMonoxideConcentrationMeasurement::single
    { { AttributeTag(3), "peakMeasuredValue", ItemType::kDefault },
      kInvalidNodeIndex }, // CarbonMonoxideConcentrationMeasurement::single
    { { AttributeTag(4), "peakMeasuredValueWindow", ItemType::kDefault },
      kInvalidNodeIndex }, // CarbonMonoxideConcentrationMeasurement::elapsed_s
    { { AttributeTag(5), "averageMeasuredValue", ItemType::kDefault },
      kInvalidNodeIndex }, // CarbonMonoxideConcentrationMeasurement::single
    { { AttributeTag(6), "averageMeasuredValueWindow", ItemType::kDefault },
      kInvalidNodeIndex }, // CarbonMonoxideConcentrationMeasurement::elapsed_s
    { { AttributeTag(7), "uncertainty", ItemType::kDefault }, kInvalidNodeIndex }, // CarbonMonoxideConcentrationMeasurement::single
    { { AttributeTag(8), "measurementUnit", ItemType::kEnum },
      kInvalidNodeIndex }, // CarbonMonoxideConcentrationMeasurement::MeasurementUnitEnum
    { { AttributeTag(9), "measurementMedium", ItemType::kEnum },
      kInvalidNodeIndex }, // CarbonMonoxideConcentrationMeasurement::MeasurementMediumEnum
    { { AttributeTag(10), "levelValue", ItemType::kEnum },
      kInvalidNodeIndex }, // CarbonMonoxideConcentrationMeasurement::LevelValueEnum
    { { AttributeTag(65528), "generatedCommandList", ItemType::kList },
      kInvalidNodeIndex }, // CarbonMonoxideConcentrationMeasurement::command_id[]
    { { AttributeTag(65529), "acceptedCommandList", ItemType::kList },
      kInvalidNodeIndex }, // CarbonMonoxideConcentrationMeasurement::command_id[]
    { { AttributeTag(65530), "eventList", ItemType::kList },
      kInvalidNodeIndex }, // CarbonMonoxideConcentrationMeasurement::event_id[]
    { { AttributeTag(65531), "attributeList", ItemType::kList },
      kInvalidNodeIndex }, // CarbonMonoxideConcentrationMeasurement::attrib_id[]
    { { AttributeTag(65532), "featureMap", ItemType::kDefault },
      kInvalidNodeIndex }, // CarbonMonoxideConcentrationMeasurement::bitmap32
    { { AttributeTag(65533), "clusterRevision", ItemType::kDefault },
      kInvalidNodeIndex }, // CarbonMonoxideConcentrationMeasurement::int16u
};

const Entry<ItemInfo> _CarbonDioxideConcentrationMeasurement[] = {
    { { AttributeTag(0), "measuredValue", ItemType::kDefault },
      kInvalidNodeIndex }, // CarbonDioxideConcentrationMeasurement::single
    { { AttributeTag(1), "minMeasuredValue", ItemType::kDefault },
      kInvalidNodeIndex }, // CarbonDioxideConcentrationMeasurement::single
    { { AttributeTag(2), "maxMeasuredValue", ItemType::kDefault },
      kInvalidNodeIndex }, // CarbonDioxideConcentrationMeasurement::single
    { { AttributeTag(3), "peakMeasuredValue", ItemType::kDefault },
      kInvalidNodeIndex }, // CarbonDioxideConcentrationMeasurement::single
    { { AttributeTag(4), "peakMeasuredValueWindow", ItemType::kDefault },
      kInvalidNodeIndex }, // CarbonDioxideConcentrationMeasurement::elapsed_s
    { { AttributeTag(5), "averageMeasuredValue", ItemType::kDefault },
      kInvalidNodeIndex }, // CarbonDioxideConcentrationMeasurement::single
    { { AttributeTag(6), "averageMeasuredValueWindow", ItemType::kDefault },
      kInvalidNodeIndex }, // CarbonDioxideConcentrationMeasurement::elapsed_s
    { { AttributeTag(7), "uncertainty", ItemType::kDefault }, kInvalidNodeIndex }, // CarbonDioxideConcentrationMeasurement::single
    { { AttributeTag(8), "measurementUnit", ItemType::kEnum },
      kInvalidNodeIndex }, // CarbonDioxideConcentrationMeasurement::MeasurementUnitEnum
    { { AttributeTag(9), "measurementMedium", ItemType::kEnum },
      kInvalidNodeIndex }, // CarbonDioxideConcentrationMeasurement::MeasurementMediumEnum
    { { AttributeTag(10), "levelValue", ItemType::kEnum },
      kInvalidNodeIndex }, // CarbonDioxideConcentrationMeasurement::LevelValueEnum
    { { AttributeTag(65528), "generatedCommandList", ItemType::kList },
      kInvalidNodeIndex }, // CarbonDioxideConcentrationMeasurement::command_id[]
    { { AttributeTag(65529), "acceptedCommandList", ItemType::kList },
      kInvalidNodeIndex }, // CarbonDioxideConcentrationMeasurement::command_id[]
    { { AttributeTag(65530), "eventList", ItemType::kList },
      kInvalidNodeIndex }, // CarbonDioxideConcentrationMeasurement::event_id[]
    { { AttributeTag(65531), "attributeList", ItemType::kList },
      kInvalidNodeIndex }, // CarbonDioxideConcentrationMeasurement::attrib_id[]
    { { AttributeTag(65532), "featureMap", ItemType::kDefault },
      kInvalidNodeIndex }, // CarbonDioxideConcentrationMeasurement::bitmap32
    { { AttributeTag(65533), "clusterRevision", ItemType::kDefault },
      kInvalidNodeIndex }, // CarbonDioxideConcentrationMeasurement::int16u
};

const Entry<ItemInfo> _EthyleneConcentrationMeasurement[] = {
    { { AttributeTag(0), "measuredValue", ItemType::kDefault }, kInvalidNodeIndex },     // EthyleneConcentrationMeasurement::single
    { { AttributeTag(1), "minMeasuredValue", ItemType::kDefault }, kInvalidNodeIndex },  // EthyleneConcentrationMeasurement::single
    { { AttributeTag(2), "maxMeasuredValue", ItemType::kDefault }, kInvalidNodeIndex },  // EthyleneConcentrationMeasurement::single
    { { AttributeTag(3), "peakMeasuredValue", ItemType::kDefault }, kInvalidNodeIndex }, // EthyleneConcentrationMeasurement::single
    { { AttributeTag(4), "peakMeasuredValueWindow", ItemType::kDefault },
      kInvalidNodeIndex }, // EthyleneConcentrationMeasurement::elapsed_s
    { { AttributeTag(5), "averageMeasuredValue", ItemType::kDefault },
      kInvalidNodeIndex }, // EthyleneConcentrationMeasurement::single
    { { AttributeTag(6), "averageMeasuredValueWindow", ItemType::kDefault },
      kInvalidNodeIndex },                                                         // EthyleneConcentrationMeasurement::elapsed_s
    { { AttributeTag(7), "uncertainty", ItemType::kDefault }, kInvalidNodeIndex }, // EthyleneConcentrationMeasurement::single
    { { AttributeTag(8), "measurementUnit", ItemType::kEnum },
      kInvalidNodeIndex }, // EthyleneConcentrationMeasurement::MeasurementUnitEnum
    { { AttributeTag(9), "measurementMedium", ItemType::kEnum },
      kInvalidNodeIndex }, // EthyleneConcentrationMeasurement::MeasurementMediumEnum
    { { AttributeTag(10), "levelValue", ItemType::kEnum }, kInvalidNodeIndex }, // EthyleneConcentrationMeasurement::LevelValueEnum
    { { AttributeTag(65528), "generatedCommandList", ItemType::kList },
      kInvalidNodeIndex }, // EthyleneConcentrationMeasurement::command_id[]
    { { AttributeTag(65529), "acceptedCommandList", ItemType::kList },
      kInvalidNodeIndex },                                                        // EthyleneConcentrationMeasurement::command_id[]
    { { AttributeTag(65530), "eventList", ItemType::kList }, kInvalidNodeIndex }, // EthyleneConcentrationMeasurement::event_id[]
    { { AttributeTag(65531), "attributeList", ItemType::kList },
      kInvalidNodeIndex }, // EthyleneConcentrationMeasurement::attrib_id[]
    { { AttributeTag(65532), "featureMap", ItemType::kDefault }, kInvalidNodeIndex }, // EthyleneConcentrationMeasurement::bitmap32
    { { AttributeTag(65533), "clusterRevision", ItemType::kDefault },
      kInvalidNodeIndex }, // EthyleneConcentrationMeasurement::int16u
};

const Entry<ItemInfo> _EthyleneOxideConcentrationMeasurement[] = {
    { { AttributeTag(0), "measuredValue", ItemType::kDefault },
      kInvalidNodeIndex }, // EthyleneOxideConcentrationMeasurement::single
    { { AttributeTag(1), "minMeasuredValue", ItemType::kDefault },
      kInvalidNodeIndex }, // EthyleneOxideConcentrationMeasurement::single
    { { AttributeTag(2), "maxMeasuredValue", ItemType::kDefault },
      kInvalidNodeIndex }, // EthyleneOxideConcentrationMeasurement::single
    { { AttributeTag(3), "peakMeasuredValue", ItemType::kDefault },
      kInvalidNodeIndex }, // EthyleneOxideConcentrationMeasurement::single
    { { AttributeTag(4), "peakMeasuredValueWindow", ItemType::kDefault },
      kInvalidNodeIndex }, // EthyleneOxideConcentrationMeasurement::elapsed_s
    { { AttributeTag(5), "averageMeasuredValue", ItemType::kDefault },
      kInvalidNodeIndex }, // EthyleneOxideConcentrationMeasurement::single
    { { AttributeTag(6), "averageMeasuredValueWindow", ItemType::kDefault },
      kInvalidNodeIndex }, // EthyleneOxideConcentrationMeasurement::elapsed_s
    { { AttributeTag(7), "uncertainty", ItemType::kDefault }, kInvalidNodeIndex }, // EthyleneOxideConcentrationMeasurement::single
    { { AttributeTag(8), "measurementUnit", ItemType::kEnum },
      kInvalidNodeIndex }, // EthyleneOxideConcentrationMeasurement::MeasurementUnitEnum
    { { AttributeTag(9), "measurementMedium", ItemType::kEnum },
      kInvalidNodeIndex }, // EthyleneOxideConcentrationMeasurement::MeasurementMediumEnum
    { { AttributeTag(10), "levelValue", ItemType::kEnum },
      kInvalidNodeIndex }, // EthyleneOxideConcentrationMeasurement::LevelValueEnum
    { { AttributeTag(65528), "generatedCommandList", ItemType::kList },
      kInvalidNodeIndex }, // EthyleneOxideConcentrationMeasurement::command_id[]
    { { AttributeTag(65529), "acceptedCommandList", ItemType::kList },
      kInvalidNodeIndex }, // EthyleneOxideConcentrationMeasurement::command_id[]
    { { AttributeTag(65530), "eventList", ItemType::kList },
      kInvalidNodeIndex }, // EthyleneOxideConcentrationMeasurement::event_id[]
    { { AttributeTag(65531), "attributeList", ItemType::kList },
      kInvalidNodeIndex }, // EthyleneOxideConcentrationMeasurement::attrib_id[]
    { { AttributeTag(65532), "featureMap", ItemType::kDefault },
      kInvalidNodeIndex }, // EthyleneOxideConcentrationMeasurement::bitmap32
    { { AttributeTag(65533), "clusterRevision", ItemType::kDefault },
      kInvalidNodeIndex }, // EthyleneOxideConcentrationMeasurement::int16u
};

const Entry<ItemInfo> _HydrogenConcentrationMeasurement[] = {
    { { AttributeTag(0), "measuredValue", ItemType::kDefault }, kInvalidNodeIndex },     // HydrogenConcentrationMeasurement::single
    { { AttributeTag(1), "minMeasuredValue", ItemType::kDefault }, kInvalidNodeIndex },  // HydrogenConcentrationMeasurement::single
    { { AttributeTag(2), "maxMeasuredValue", ItemType::kDefault }, kInvalidNodeIndex },  // HydrogenConcentrationMeasurement::single
    { { AttributeTag(3), "peakMeasuredValue", ItemType::kDefault }, kInvalidNodeIndex }, // HydrogenConcentrationMeasurement::single
    { { AttributeTag(4), "peakMeasuredValueWindow", ItemType::kDefault },
      kInvalidNodeIndex }, // HydrogenConcentrationMeasurement::elapsed_s
    { { AttributeTag(5), "averageMeasuredValue", ItemType::kDefault },
      kInvalidNodeIndex }, // HydrogenConcentrationMeasurement::single
    { { AttributeTag(6), "averageMeasuredValueWindow", ItemType::kDefault },
      kInvalidNodeIndex },                                                         // HydrogenConcentrationMeasurement::elapsed_s
    { { AttributeTag(7), "uncertainty", ItemType::kDefault }, kInvalidNodeIndex }, // HydrogenConcentrationMeasurement::single
    { { AttributeTag(8), "measurementUnit", ItemType::kEnum },
      kInvalidNodeIndex }, // HydrogenConcentrationMeasurement::MeasurementUnitEnum
    { { AttributeTag(9), "measurementMedium", ItemType::kEnum },
      kInvalidNodeIndex }, // HydrogenConcentrationMeasurement::MeasurementMediumEnum
    { { AttributeTag(10), "levelValue", ItemType::kEnum }, kInvalidNodeIndex }, // HydrogenConcentrationMeasurement::LevelValueEnum
    { { AttributeTag(65528), "generatedCommandList", ItemType::kList },
      kInvalidNodeIndex }, // HydrogenConcentrationMeasurement::command_id[]
    { { AttributeTag(65529), "acceptedCommandList", ItemType::kList },
      kInvalidNodeIndex },                                                        // HydrogenConcentrationMeasurement::command_id[]
    { { AttributeTag(65530), "eventList", ItemType::kList }, kInvalidNodeIndex }, // HydrogenConcentrationMeasurement::event_id[]
    { { AttributeTag(65531), "attributeList", ItemType::kList },
      kInvalidNodeIndex }, // HydrogenConcentrationMeasurement::attrib_id[]
    { { AttributeTag(65532), "featureMap", ItemType::kDefault }, kInvalidNodeIndex }, // HydrogenConcentrationMeasurement::bitmap32
    { { AttributeTag(65533), "clusterRevision", ItemType::kDefault },
      kInvalidNodeIndex }, // HydrogenConcentrationMeasurement::int16u
};

const Entry<ItemInfo> _HydrogenSulfideConcentrationMeasurement[] = {
    { { AttributeTag(0), "measuredValue", ItemType::kDefault },
      kInvalidNodeIndex }, // HydrogenSulfideConcentrationMeasurement::single
    { { AttributeTag(1), "minMeasuredValue", ItemType::kDefault },
      kInvalidNodeIndex }, // HydrogenSulfideConcentrationMeasurement::single
    { { AttributeTag(2), "maxMeasuredValue", ItemType::kDefault },
      kInvalidNodeIndex }, // HydrogenSulfideConcentrationMeasurement::single
    { { AttributeTag(3), "peakMeasuredValue", ItemType::kDefault },
      kInvalidNodeIndex }, // HydrogenSulfideConcentrationMeasurement::single
    { { AttributeTag(4), "peakMeasuredValueWindow", ItemType::kDefault },
      kInvalidNodeIndex }, // HydrogenSulfideConcentrationMeasurement::elapsed_s
    { { AttributeTag(5), "averageMeasuredValue", ItemType::kDefault },
      kInvalidNodeIndex }, // HydrogenSulfideConcentrationMeasurement::single
    { { AttributeTag(6), "averageMeasuredValueWindow", ItemType::kDefault },
      kInvalidNodeIndex }, // HydrogenSulfideConcentrationMeasurement::elapsed_s
    { { AttributeTag(7), "uncertainty", ItemType::kDefault },
      kInvalidNodeIndex }, // HydrogenSulfideConcentrationMeasurement::single
    { { AttributeTag(8), "measurementUnit", ItemType::kEnum },
      kInvalidNodeIndex }, // HydrogenSulfideConcentrationMeasurement::MeasurementUnitEnum
    { { AttributeTag(9), "measurementMedium", ItemType::kEnum },
      kInvalidNodeIndex }, // HydrogenSulfideConcentrationMeasurement::MeasurementMediumEnum
    { { AttributeTag(10), "levelValue", ItemType::kEnum },
      kInvalidNodeIndex }, // HydrogenSulfideConcentrationMeasurement::LevelValueEnum
    { { AttributeTag(65528), "generatedCommandList", ItemType::kList },
      kInvalidNodeIndex }, // HydrogenSulfideConcentrationMeasurement::command_id[]
    { { AttributeTag(65529), "acceptedCommandList", ItemType::kList },
      kInvalidNodeIndex }, // HydrogenSulfideConcentrationMeasurement::command_id[]
    { { AttributeTag(65530), "eventList", ItemType::kList },
      kInvalidNodeIndex }, // HydrogenSulfideConcentrationMeasurement::event_id[]
    { { AttributeTag(65531), "attributeList", ItemType::kList },
      kInvalidNodeIndex }, // HydrogenSulfideConcentrationMeasurement::attrib_id[]
    { { AttributeTag(65532), "featureMap", ItemType::kDefault },
      kInvalidNodeIndex }, // HydrogenSulfideConcentrationMeasurement::bitmap32
    { { AttributeTag(65533), "clusterRevision", ItemType::kDefault },
      kInvalidNodeIndex }, // HydrogenSulfideConcentrationMeasurement::int16u
};

const Entry<ItemInfo> _NitricOxideConcentrationMeasurement[] = {
    { { AttributeTag(0), "measuredValue", ItemType::kDefault }, kInvalidNodeIndex }, // NitricOxideConcentrationMeasurement::single
    { { AttributeTag(1), "minMeasuredValue", ItemType::kDefault },
      kInvalidNodeIndex }, // NitricOxideConcentrationMeasurement::single
    { { AttributeTag(2), "maxMeasuredValue", ItemType::kDefault },
      kInvalidNodeIndex }, // NitricOxideConcentrationMeasurement::single
    { { AttributeTag(3), "peakMeasuredValue", ItemType::kDefault },
      kInvalidNodeIndex }, // NitricOxideConcentrationMeasurement::single
    { { AttributeTag(4), "peakMeasuredValueWindow", ItemType::kDefault },
      kInvalidNodeIndex }, // NitricOxideConcentrationMeasurement::elapsed_s
    { { AttributeTag(5), "averageMeasuredValue", ItemType::kDefault },
      kInvalidNodeIndex }, // NitricOxideConcentrationMeasurement::single
    { { AttributeTag(6), "averageMeasuredValueWindow", ItemType::kDefault },
      kInvalidNodeIndex },                                                         // NitricOxideConcentrationMeasurement::elapsed_s
    { { AttributeTag(7), "uncertainty", ItemType::kDefault }, kInvalidNodeIndex }, // NitricOxideConcentrationMeasurement::single
    { { AttributeTag(8), "measurementUnit", ItemType::kEnum },
      kInvalidNodeIndex }, // NitricOxideConcentrationMeasurement::MeasurementUnitEnum
    { { AttributeTag(9), "measurementMedium", ItemType::kEnum },
      kInvalidNodeIndex }, // NitricOxideConcentrationMeasurement::MeasurementMediumEnum
    { { AttributeTag(10), "levelValue", ItemType::kEnum },
      kInvalidNodeIndex }, // NitricOxideConcentrationMeasurement::LevelValueEnum
    { { AttributeTag(65528), "generatedCommandList", ItemType::kList },
      kInvalidNodeIndex }, // NitricOxideConcentrationMeasurement::command_id[]
    { { AttributeTag(65529), "acceptedCommandList", ItemType::kList },
      kInvalidNodeIndex }, // NitricOxideConcentrationMeasurement::command_id[]
    { { AttributeTag(65530), "eventList", ItemType::kList }, kInvalidNodeIndex }, // NitricOxideConcentrationMeasurement::event_id[]
    { { AttributeTag(65531), "attributeList", ItemType::kList },
      kInvalidNodeIndex }, // NitricOxideConcentrationMeasurement::attrib_id[]
    { { AttributeTag(65532), "featureMap", ItemType::kDefault },
      kInvalidNodeIndex }, // NitricOxideConcentrationMeasurement::bitmap32
    { { AttributeTag(65533), "clusterRevision", ItemType::kDefault },
      kInvalidNodeIndex }, // NitricOxideConcentrationMeasurement::int16u
};

const Entry<ItemInfo> _NitrogenDioxideConcentrationMeasurement[] = {
    { { AttributeTag(0), "measuredValue", ItemType::kDefault },
      kInvalidNodeIndex }, // NitrogenDioxideConcentrationMeasurement::single
    { { AttributeTag(1), "minMeasuredValue", ItemType::kDefault },
      kInvalidNodeIndex }, // NitrogenDioxideConcentrationMeasurement::single
    { { AttributeTag(2), "maxMeasuredValue", ItemType::kDefault },
      kInvalidNodeIndex }, // NitrogenDioxideConcentrationMeasurement::single
    { { AttributeTag(3), "peakMeasuredValue", ItemType::kDefault },
      kInvalidNodeIndex }, // NitrogenDioxideConcentrationMeasurement::single
    { { AttributeTag(4), "peakMeasuredValueWindow", ItemType::kDefault },
      kInvalidNodeIndex }, // NitrogenDioxideConcentrationMeasurement::elapsed_s
    { { AttributeTag(5), "averageMeasuredValue", ItemType::kDefault },
      kInvalidNodeIndex }, // NitrogenDioxideConcentrationMeasurement::single
    { { AttributeTag(6), "averageMeasuredValueWindow", ItemType::kDefault },
      kInvalidNodeIndex }, // NitrogenDioxideConcentrationMeasurement::elapsed_s
    { { AttributeTag(7), "uncertainty", ItemType::kDefault },
      kInvalidNodeIndex }, // NitrogenDioxideConcentrationMeasurement::single
    { { AttributeTag(8), "measurementUnit", ItemType::kEnum },
      kInvalidNodeIndex }, // NitrogenDioxideConcentrationMeasurement::MeasurementUnitEnum
    { { AttributeTag(9), "measurementMedium", ItemType::kEnum },
      kInvalidNodeIndex }, // NitrogenDioxideConcentrationMeasurement::MeasurementMediumEnum
    { { AttributeTag(10), "levelValue", ItemType::kEnum },
      kInvalidNodeIndex }, // NitrogenDioxideConcentrationMeasurement::LevelValueEnum
    { { AttributeTag(65528), "generatedCommandList", ItemType::kList },
      kInvalidNodeIndex }, // NitrogenDioxideConcentrationMeasurement::command_id[]
    { { AttributeTag(65529), "acceptedCommandList", ItemType::kList },
      kInvalidNodeIndex }, // NitrogenDioxideConcentrationMeasurement::command_id[]
    { { AttributeTag(65530), "eventList", ItemType::kList },
      kInvalidNodeIndex }, // NitrogenDioxideConcentrationMeasurement::event_id[]
    { { AttributeTag(65531), "attributeList", ItemType::kList },
      kInvalidNodeIndex }, // NitrogenDioxideConcentrationMeasurement::attrib_id[]
    { { AttributeTag(65532), "featureMap", ItemType::kDefault },
      kInvalidNodeIndex }, // NitrogenDioxideConcentrationMeasurement::bitmap32
    { { AttributeTag(65533), "clusterRevision", ItemType::kDefault },
      kInvalidNodeIndex }, // NitrogenDioxideConcentrationMeasurement::int16u
};

const Entry<ItemInfo> _OxygenConcentrationMeasurement[] = {
    { { AttributeTag(0), "measuredValue", ItemType::kDefault }, kInvalidNodeIndex },     // OxygenConcentrationMeasurement::single
    { { AttributeTag(1), "minMeasuredValue", ItemType::kDefault }, kInvalidNodeIndex },  // OxygenConcentrationMeasurement::single
    { { AttributeTag(2), "maxMeasuredValue", ItemType::kDefault }, kInvalidNodeIndex },  // OxygenConcentrationMeasurement::single
    { { AttributeTag(3), "peakMeasuredValue", ItemType::kDefault }, kInvalidNodeIndex }, // OxygenConcentrationMeasurement::single
    { { AttributeTag(4), "peakMeasuredValueWindow", ItemType::kDefault },
      kInvalidNodeIndex }, // OxygenConcentrationMeasurement::elapsed_s
    { { AttributeTag(5), "averageMeasuredValue", ItemType::kDefault },
      kInvalidNodeIndex }, // OxygenConcentrationMeasurement::single
    { { AttributeTag(6), "averageMeasuredValueWindow", ItemType::kDefault },
      kInvalidNodeIndex },                                                         // OxygenConcentrationMeasurement::elapsed_s
    { { AttributeTag(7), "uncertainty", ItemType::kDefault }, kInvalidNodeIndex }, // OxygenConcentrationMeasurement::single
    { { AttributeTag(8), "measurementUnit", ItemType::kEnum },
      kInvalidNodeIndex }, // OxygenConcentrationMeasurement::MeasurementUnitEnum
    { { AttributeTag(9), "measurementMedium", ItemType::kEnum },
      kInvalidNodeIndex }, // OxygenConcentrationMeasurement::MeasurementMediumEnum
    { { AttributeTag(10), "levelValue", ItemType::kEnum }, kInvalidNodeIndex }, // OxygenConcentrationMeasurement::LevelValueEnum
    { { AttributeTag(65528), "generatedCommandList", ItemType::kList },
      kInvalidNodeIndex }, // OxygenConcentrationMeasurement::command_id[]
    { { AttributeTag(65529), "acceptedCommandList", ItemType::kList },
      kInvalidNodeIndex },                                                        // OxygenConcentrationMeasurement::command_id[]
    { { AttributeTag(65530), "eventList", ItemType::kList }, kInvalidNodeIndex }, // OxygenConcentrationMeasurement::event_id[]
    { { AttributeTag(65531), "attributeList", ItemType::kList }, kInvalidNodeIndex }, // OxygenConcentrationMeasurement::attrib_id[]
    { { AttributeTag(65532), "featureMap", ItemType::kDefault }, kInvalidNodeIndex }, // OxygenConcentrationMeasurement::bitmap32
    { { AttributeTag(65533), "clusterRevision", ItemType::kDefault }, kInvalidNodeIndex }, // OxygenConcentrationMeasurement::int16u
};

const Entry<ItemInfo> _OzoneConcentrationMeasurement[] = {
    { { AttributeTag(0), "measuredValue", ItemType::kDefault }, kInvalidNodeIndex },     // OzoneConcentrationMeasurement::single
    { { AttributeTag(1), "minMeasuredValue", ItemType::kDefault }, kInvalidNodeIndex },  // OzoneConcentrationMeasurement::single
    { { AttributeTag(2), "maxMeasuredValue", ItemType::kDefault }, kInvalidNodeIndex },  // OzoneConcentrationMeasurement::single
    { { AttributeTag(3), "peakMeasuredValue", ItemType::kDefault }, kInvalidNodeIndex }, // OzoneConcentrationMeasurement::single
    { { AttributeTag(4), "peakMeasuredValueWindow", ItemType::kDefault },
      kInvalidNodeIndex }, // OzoneConcentrationMeasurement::elapsed_s
    { { AttributeTag(5), "averageMeasuredValue", ItemType::kDefault }, kInvalidNodeIndex }, // OzoneConcentrationMeasurement::single
    { { AttributeTag(6), "averageMeasuredValueWindow", ItemType::kDefault },
      kInvalidNodeIndex },                                                         // OzoneConcentrationMeasurement::elapsed_s
    { { AttributeTag(7), "uncertainty", ItemType::kDefault }, kInvalidNodeIndex }, // OzoneConcentrationMeasurement::single
    { { AttributeTag(8), "measurementUnit", ItemType::kEnum },
      kInvalidNodeIndex }, // OzoneConcentrationMeasurement::MeasurementUnitEnum
    { { AttributeTag(9), "measurementMedium", ItemType::kEnum },
      kInvalidNodeIndex }, // OzoneConcentrationMeasurement::MeasurementMediumEnum
    { { AttributeTag(10), "levelValue", ItemType::kEnum }, kInvalidNodeIndex }, // OzoneConcentrationMeasurement::LevelValueEnum
    { { AttributeTag(65528), "generatedCommandList", ItemType::kList },
      kInvalidNodeIndex }, // OzoneConcentrationMeasurement::command_id[]
    { { AttributeTag(65529), "acceptedCommandList", ItemType::kList },
      kInvalidNodeIndex },                                                            // OzoneConcentrationMeasurement::command_id[]
    { { AttributeTag(65530), "eventList", ItemType::kList }, kInvalidNodeIndex },     // OzoneConcentrationMeasurement::event_id[]
    { { AttributeTag(65531), "attributeList", ItemType::kList }, kInvalidNodeIndex }, // OzoneConcentrationMeasurement::attrib_id[]
    { { AttributeTag(65532), "featureMap", ItemType::kDefault }, kInvalidNodeIndex }, // OzoneConcentrationMeasurement::bitmap32
    { { AttributeTag(65533), "clusterRevision", ItemType::kDefault }, kInvalidNodeIndex }, // OzoneConcentrationMeasurement::int16u
};

const Entry<ItemInfo> _SulfurDioxideConcentrationMeasurement[] = {
    { { AttributeTag(0), "measuredValue", ItemType::kDefault },
      kInvalidNodeIndex }, // SulfurDioxideConcentrationMeasurement::single
    { { AttributeTag(1), "minMeasuredValue", ItemType::kDefault },
      kInvalidNodeIndex }, // SulfurDioxideConcentrationMeasurement::single
    { { AttributeTag(2), "maxMeasuredValue", ItemType::kDefault },
      kInvalidNodeIndex }, // SulfurDioxideConcentrationMeasurement::single
    { { AttributeTag(3), "peakMeasuredValue", ItemType::kDefault },
      kInvalidNodeIndex }, // SulfurDioxideConcentrationMeasurement::single
    { { AttributeTag(4), "peakMeasuredValueWindow", ItemType::kDefault },
      kInvalidNodeIndex }, // SulfurDioxideConcentrationMeasurement::elapsed_s
    { { AttributeTag(5), "averageMeasuredValue", ItemType::kDefault },
      kInvalidNodeIndex }, // SulfurDioxideConcentrationMeasurement::single
    { { AttributeTag(6), "averageMeasuredValueWindow", ItemType::kDefault },
      kInvalidNodeIndex }, // SulfurDioxideConcentrationMeasurement::elapsed_s
    { { AttributeTag(7), "uncertainty", ItemType::kDefault }, kInvalidNodeIndex }, // SulfurDioxideConcentrationMeasurement::single
    { { AttributeTag(8), "measurementUnit", ItemType::kEnum },
      kInvalidNodeIndex }, // SulfurDioxideConcentrationMeasurement::MeasurementUnitEnum
    { { AttributeTag(9), "measurementMedium", ItemType::kEnum },
      kInvalidNodeIndex }, // SulfurDioxideConcentrationMeasurement::MeasurementMediumEnum
    { { AttributeTag(10), "levelValue", ItemType::kEnum },
      kInvalidNodeIndex }, // SulfurDioxideConcentrationMeasurement::LevelValueEnum
    { { AttributeTag(65528), "generatedCommandList", ItemType::kList },
      kInvalidNodeIndex }, // SulfurDioxideConcentrationMeasurement::command_id[]
    { { AttributeTag(65529), "acceptedCommandList", ItemType::kList },
      kInvalidNodeIndex }, // SulfurDioxideConcentrationMeasurement::command_id[]
    { { AttributeTag(65530), "eventList", ItemType::kList },
      kInvalidNodeIndex }, // SulfurDioxideConcentrationMeasurement::event_id[]
    { { AttributeTag(65531), "attributeList", ItemType::kList },
      kInvalidNodeIndex }, // SulfurDioxideConcentrationMeasurement::attrib_id[]
    { { AttributeTag(65532), "featureMap", ItemType::kDefault },
      kInvalidNodeIndex }, // SulfurDioxideConcentrationMeasurement::bitmap32
    { { AttributeTag(65533), "clusterRevision", ItemType::kDefault },
      kInvalidNodeIndex }, // SulfurDioxideConcentrationMeasurement::int16u
};

const Entry<ItemInfo> _DissolvedOxygenConcentrationMeasurement[] = {
    { { AttributeTag(0), "measuredValue", ItemType::kDefault },
      kInvalidNodeIndex }, // DissolvedOxygenConcentrationMeasurement::single
    { { AttributeTag(1), "minMeasuredValue", ItemType::kDefault },
      kInvalidNodeIndex }, // DissolvedOxygenConcentrationMeasurement::single
    { { AttributeTag(2), "maxMeasuredValue", ItemType::kDefault },
      kInvalidNodeIndex }, // DissolvedOxygenConcentrationMeasurement::single
    { { AttributeTag(3), "peakMeasuredValue", ItemType::kDefault },
      kInvalidNodeIndex }, // DissolvedOxygenConcentrationMeasurement::single
    { { AttributeTag(4), "peakMeasuredValueWindow", ItemType::kDefault },
      kInvalidNodeIndex }, // DissolvedOxygenConcentrationMeasurement::elapsed_s
    { { AttributeTag(5), "averageMeasuredValue", ItemType::kDefault },
      kInvalidNodeIndex }, // DissolvedOxygenConcentrationMeasurement::single
    { { AttributeTag(6), "averageMeasuredValueWindow", ItemType::kDefault },
      kInvalidNodeIndex }, // DissolvedOxygenConcentrationMeasurement::elapsed_s
    { { AttributeTag(7), "uncertainty", ItemType::kDefault },
      kInvalidNodeIndex }, // DissolvedOxygenConcentrationMeasurement::single
    { { AttributeTag(8), "measurementUnit", ItemType::kEnum },
      kInvalidNodeIndex }, // DissolvedOxygenConcentrationMeasurement::MeasurementUnitEnum
    { { AttributeTag(9), "measurementMedium", ItemType::kEnum },
      kInvalidNodeIndex }, // DissolvedOxygenConcentrationMeasurement::MeasurementMediumEnum
    { { AttributeTag(10), "levelValue", ItemType::kEnum },
      kInvalidNodeIndex }, // DissolvedOxygenConcentrationMeasurement::LevelValueEnum
    { { AttributeTag(65528), "generatedCommandList", ItemType::kList },
      kInvalidNodeIndex }, // DissolvedOxygenConcentrationMeasurement::command_id[]
    { { AttributeTag(65529), "acceptedCommandList", ItemType::kList },
      kInvalidNodeIndex }, // DissolvedOxygenConcentrationMeasurement::command_id[]
    { { AttributeTag(65530), "eventList", ItemType::kList },
      kInvalidNodeIndex }, // DissolvedOxygenConcentrationMeasurement::event_id[]
    { { AttributeTag(65531), "attributeList", ItemType::kList },
      kInvalidNodeIndex }, // DissolvedOxygenConcentrationMeasurement::attrib_id[]
    { { AttributeTag(65532), "featureMap", ItemType::kDefault },
      kInvalidNodeIndex }, // DissolvedOxygenConcentrationMeasurement::bitmap32
    { { AttributeTag(65533), "clusterRevision", ItemType::kDefault },
      kInvalidNodeIndex }, // DissolvedOxygenConcentrationMeasurement::int16u
};

const Entry<ItemInfo> _BromateConcentrationMeasurement[] = {
    { { AttributeTag(0), "measuredValue", ItemType::kDefault }, kInvalidNodeIndex },     // BromateConcentrationMeasurement::single
    { { AttributeTag(1), "minMeasuredValue", ItemType::kDefault }, kInvalidNodeIndex },  // BromateConcentrationMeasurement::single
    { { AttributeTag(2), "maxMeasuredValue", ItemType::kDefault }, kInvalidNodeIndex },  // BromateConcentrationMeasurement::single
    { { AttributeTag(3), "peakMeasuredValue", ItemType::kDefault }, kInvalidNodeIndex }, // BromateConcentrationMeasurement::single
    { { AttributeTag(4), "peakMeasuredValueWindow", ItemType::kDefault },
      kInvalidNodeIndex }, // BromateConcentrationMeasurement::elapsed_s
    { { AttributeTag(5), "averageMeasuredValue", ItemType::kDefault },
      kInvalidNodeIndex }, // BromateConcentrationMeasurement::single
    { { AttributeTag(6), "averageMeasuredValueWindow", ItemType::kDefault },
      kInvalidNodeIndex },                                                         // BromateConcentrationMeasurement::elapsed_s
    { { AttributeTag(7), "uncertainty", ItemType::kDefault }, kInvalidNodeIndex }, // BromateConcentrationMeasurement::single
    { { AttributeTag(8), "measurementUnit", ItemType::kEnum },
      kInvalidNodeIndex }, // BromateConcentrationMeasurement::MeasurementUnitEnum
    { { AttributeTag(9), "measurementMedium", ItemType::kEnum },
      kInvalidNodeIndex }, // BromateConcentrationMeasurement::MeasurementMediumEnum
    { { AttributeTag(10), "levelValue", ItemType::kEnum }, kInvalidNodeIndex }, // BromateConcentrationMeasurement::LevelValueEnum
    { { AttributeTag(65528), "generatedCommandList", ItemType::kList },
      kInvalidNodeIndex }, // BromateConcentrationMeasurement::command_id[]
    { { AttributeTag(65529), "acceptedCommandList", ItemType::kList },
      kInvalidNodeIndex },                                                        // BromateConcentrationMeasurement::command_id[]
    { { AttributeTag(65530), "eventList", ItemType::kList }, kInvalidNodeIndex }, // BromateConcentrationMeasurement::event_id[]
    { { AttributeTag(65531), "attributeList", ItemType::kList },
      kInvalidNodeIndex }, // BromateConcentrationMeasurement::attrib_id[]
    { { AttributeTag(65532), "featureMap", ItemType::kDefault }, kInvalidNodeIndex }, // BromateConcentrationMeasurement::bitmap32
    { { AttributeTag(65533), "clusterRevision", ItemType::kDefault },
      kInvalidNodeIndex }, // BromateConcentrationMeasurement::int16u
};

const Entry<ItemInfo> _ChloraminesConcentrationMeasurement[] = {
    { { AttributeTag(0), "measuredValue", ItemType::kDefault }, kInvalidNodeIndex }, // ChloraminesConcentrationMeasurement::single
    { { AttributeTag(1), "minMeasuredValue", ItemType::kDefault },
      kInvalidNodeIndex }, // ChloraminesConcentrationMeasurement::single
    { { AttributeTag(2), "maxMeasuredValue", ItemType::kDefault },
      kInvalidNodeIndex }, // ChloraminesConcentrationMeasurement::single
    { { AttributeTag(3), "peakMeasuredValue", ItemType::kDefault },
      kInvalidNodeIndex }, // ChloraminesConcentrationMeasurement::single
    { { AttributeTag(4), "peakMeasuredValueWindow", ItemType::kDefault },
      kInvalidNodeIndex }, // ChloraminesConcentrationMeasurement::elapsed_s
    { { AttributeTag(5), "averageMeasuredValue", ItemType::kDefault },
      kInvalidNodeIndex }, // ChloraminesConcentrationMeasurement::single
    { { AttributeTag(6), "averageMeasuredValueWindow", ItemType::kDefault },
      kInvalidNodeIndex },                                                         // ChloraminesConcentrationMeasurement::elapsed_s
    { { AttributeTag(7), "uncertainty", ItemType::kDefault }, kInvalidNodeIndex }, // ChloraminesConcentrationMeasurement::single
    { { AttributeTag(8), "measurementUnit", ItemType::kEnum },
      kInvalidNodeIndex }, // ChloraminesConcentrationMeasurement::MeasurementUnitEnum
    { { AttributeTag(9), "measurementMedium", ItemType::kEnum },
      kInvalidNodeIndex }, // ChloraminesConcentrationMeasurement::MeasurementMediumEnum
    { { AttributeTag(10), "levelValue", ItemType::kEnum },
      kInvalidNodeIndex }, // ChloraminesConcentrationMeasurement::LevelValueEnum
    { { AttributeTag(65528), "generatedCommandList", ItemType::kList },
      kInvalidNodeIndex }, // ChloraminesConcentrationMeasurement::command_id[]
    { { AttributeTag(65529), "acceptedCommandList", ItemType::kList },
      kInvalidNodeIndex }, // ChloraminesConcentrationMeasurement::command_id[]
    { { AttributeTag(65530), "eventList", ItemType::kList }, kInvalidNodeIndex }, // ChloraminesConcentrationMeasurement::event_id[]
    { { AttributeTag(65531), "attributeList", ItemType::kList },
      kInvalidNodeIndex }, // ChloraminesConcentrationMeasurement::attrib_id[]
    { { AttributeTag(65532), "featureMap", ItemType::kDefault },
      kInvalidNodeIndex }, // ChloraminesConcentrationMeasurement::bitmap32
    { { AttributeTag(65533), "clusterRevision", ItemType::kDefault },
      kInvalidNodeIndex }, // ChloraminesConcentrationMeasurement::int16u
};

const Entry<ItemInfo> _ChlorineConcentrationMeasurement[] = {
    { { AttributeTag(0), "measuredValue", ItemType::kDefault }, kInvalidNodeIndex },     // ChlorineConcentrationMeasurement::single
    { { AttributeTag(1), "minMeasuredValue", ItemType::kDefault }, kInvalidNodeIndex },  // ChlorineConcentrationMeasurement::single
    { { AttributeTag(2), "maxMeasuredValue", ItemType::kDefault }, kInvalidNodeIndex },  // ChlorineConcentrationMeasurement::single
    { { AttributeTag(3), "peakMeasuredValue", ItemType::kDefault }, kInvalidNodeIndex }, // ChlorineConcentrationMeasurement::single
    { { AttributeTag(4), "peakMeasuredValueWindow", ItemType::kDefault },
      kInvalidNodeIndex }, // ChlorineConcentrationMeasurement::elapsed_s
    { { AttributeTag(5), "averageMeasuredValue", ItemType::kDefault },
      kInvalidNodeIndex }, // ChlorineConcentrationMeasurement::single
    { { AttributeTag(6), "averageMeasuredValueWindow", ItemType::kDefault },
      kInvalidNodeIndex },                                                         // ChlorineConcentrationMeasurement::elapsed_s
    { { AttributeTag(7), "uncertainty", ItemType::kDefault }, kInvalidNodeIndex }, // ChlorineConcentrationMeasurement::single
    { { AttributeTag(8), "measurementUnit", ItemType::kEnum },
      kInvalidNodeIndex }, // ChlorineConcentrationMeasurement::MeasurementUnitEnum
    { { AttributeTag(9), "measurementMedium", ItemType::kEnum },
      kInvalidNodeIndex }, // ChlorineConcentrationMeasurement::MeasurementMediumEnum
    { { AttributeTag(10), "levelValue", ItemType::kEnum }, kInvalidNodeIndex }, // ChlorineConcentrationMeasurement::LevelValueEnum
    { { AttributeTag(65528), "generatedCommandList", ItemType::kList },
      kInvalidNodeIndex }, // ChlorineConcentrationMeasurement::command_id[]
    { { AttributeTag(65529), "acceptedCommandList", ItemType::kList },
      kInvalidNodeIndex },                                                        // ChlorineConcentrationMeasurement::command_id[]
    { { AttributeTag(65530), "eventList", ItemType::kList }, kInvalidNodeIndex }, // ChlorineConcentrationMeasurement::event_id[]
    { { AttributeTag(65531), "attributeList", ItemType::kList },
      kInvalidNodeIndex }, // ChlorineConcentrationMeasurement::attrib_id[]
    { { AttributeTag(65532), "featureMap", ItemType::kDefault }, kInvalidNodeIndex }, // ChlorineConcentrationMeasurement::bitmap32
    { { AttributeTag(65533), "clusterRevision", ItemType::kDefault },
      kInvalidNodeIndex }, // ChlorineConcentrationMeasurement::int16u
};

const Entry<ItemInfo> _FecalColiformEColiConcentrationMeasurement[] = {
    { { AttributeTag(0), "measuredValue", ItemType::kDefault },
      kInvalidNodeIndex }, // FecalColiformEColiConcentrationMeasurement::single
    { { AttributeTag(1), "minMeasuredValue", ItemType::kDefault },
      kInvalidNodeIndex }, // FecalColiformEColiConcentrationMeasurement::single
    { { AttributeTag(2), "maxMeasuredValue", ItemType::kDefault },
      kInvalidNodeIndex }, // FecalColiformEColiConcentrationMeasurement::single
    { { AttributeTag(3), "peakMeasuredValue", ItemType::kDefault },
      kInvalidNodeIndex }, // FecalColiformEColiConcentrationMeasurement::single
    { { AttributeTag(4), "peakMeasuredValueWindow", ItemType::kDefault },
      kInvalidNodeIndex }, // FecalColiformEColiConcentrationMeasurement::elapsed_s
    { { AttributeTag(5), "averageMeasuredValue", ItemType::kDefault },
      kInvalidNodeIndex }, // FecalColiformEColiConcentrationMeasurement::single
    { { AttributeTag(6), "averageMeasuredValueWindow", ItemType::kDefault },
      kInvalidNodeIndex }, // FecalColiformEColiConcentrationMeasurement::elapsed_s
    { { AttributeTag(7), "uncertainty", ItemType::kDefault },
      kInvalidNodeIndex }, // FecalColiformEColiConcentrationMeasurement::single
    { { AttributeTag(8), "measurementUnit", ItemType::kEnum },
      kInvalidNodeIndex }, // FecalColiformEColiConcentrationMeasurement::MeasurementUnitEnum
    { { AttributeTag(9), "measurementMedium", ItemType::kEnum },
      kInvalidNodeIndex }, // FecalColiformEColiConcentrationMeasurement::MeasurementMediumEnum
    { { AttributeTag(10), "levelValue", ItemType::kEnum },
      kInvalidNodeIndex }, // FecalColiformEColiConcentrationMeasurement::LevelValueEnum
    { { AttributeTag(65528), "generatedCommandList", ItemType::kList },
      kInvalidNodeIndex }, // FecalColiformEColiConcentrationMeasurement::command_id[]
    { { AttributeTag(65529), "acceptedCommandList", ItemType::kList },
      kInvalidNodeIndex }, // FecalColiformEColiConcentrationMeasurement::command_id[]
    { { AttributeTag(65530), "eventList", ItemType::kList },
      kInvalidNodeIndex }, // FecalColiformEColiConcentrationMeasurement::event_id[]
    { { AttributeTag(65531), "attributeList", ItemType::kList },
      kInvalidNodeIndex }, // FecalColiformEColiConcentrationMeasurement::attrib_id[]
    { { AttributeTag(65532), "featureMap", ItemType::kDefault },
      kInvalidNodeIndex }, // FecalColiformEColiConcentrationMeasurement::bitmap32
    { { AttributeTag(65533), "clusterRevision", ItemType::kDefault },
      kInvalidNodeIndex }, // FecalColiformEColiConcentrationMeasurement::int16u
};

const Entry<ItemInfo> _FluorideConcentrationMeasurement[] = {
    { { AttributeTag(0), "measuredValue", ItemType::kDefault }, kInvalidNodeIndex },     // FluorideConcentrationMeasurement::single
    { { AttributeTag(1), "minMeasuredValue", ItemType::kDefault }, kInvalidNodeIndex },  // FluorideConcentrationMeasurement::single
    { { AttributeTag(2), "maxMeasuredValue", ItemType::kDefault }, kInvalidNodeIndex },  // FluorideConcentrationMeasurement::single
    { { AttributeTag(3), "peakMeasuredValue", ItemType::kDefault }, kInvalidNodeIndex }, // FluorideConcentrationMeasurement::single
    { { AttributeTag(4), "peakMeasuredValueWindow", ItemType::kDefault },
      kInvalidNodeIndex }, // FluorideConcentrationMeasurement::elapsed_s
    { { AttributeTag(5), "averageMeasuredValue", ItemType::kDefault },
      kInvalidNodeIndex }, // FluorideConcentrationMeasurement::single
    { { AttributeTag(6), "averageMeasuredValueWindow", ItemType::kDefault },
      kInvalidNodeIndex },                                                         // FluorideConcentrationMeasurement::elapsed_s
    { { AttributeTag(7), "uncertainty", ItemType::kDefault }, kInvalidNodeIndex }, // FluorideConcentrationMeasurement::single
    { { AttributeTag(8), "measurementUnit", ItemType::kEnum },
      kInvalidNodeIndex }, // FluorideConcentrationMeasurement::MeasurementUnitEnum
    { { AttributeTag(9), "measurementMedium", ItemType::kEnum },
      kInvalidNodeIndex }, // FluorideConcentrationMeasurement::MeasurementMediumEnum
    { { AttributeTag(10), "levelValue", ItemType::kEnum }, kInvalidNodeIndex }, // FluorideConcentrationMeasurement::LevelValueEnum
    { { AttributeTag(65528), "generatedCommandList", ItemType::kList },
      kInvalidNodeIndex }, // FluorideConcentrationMeasurement::command_id[]
    { { AttributeTag(65529), "acceptedCommandList", ItemType::kList },
      kInvalidNodeIndex },                                                        // FluorideConcentrationMeasurement::command_id[]
    { { AttributeTag(65530), "eventList", ItemType::kList }, kInvalidNodeIndex }, // FluorideConcentrationMeasurement::event_id[]
    { { AttributeTag(65531), "attributeList", ItemType::kList },
      kInvalidNodeIndex }, // FluorideConcentrationMeasurement::attrib_id[]
    { { AttributeTag(65532), "featureMap", ItemType::kDefault }, kInvalidNodeIndex }, // FluorideConcentrationMeasurement::bitmap32
    { { AttributeTag(65533), "clusterRevision", ItemType::kDefault },
      kInvalidNodeIndex }, // FluorideConcentrationMeasurement::int16u
};

const Entry<ItemInfo> _HaloaceticAcidsConcentrationMeasurement[] = {
    { { AttributeTag(0), "measuredValue", ItemType::kDefault },
      kInvalidNodeIndex }, // HaloaceticAcidsConcentrationMeasurement::single
    { { AttributeTag(1), "minMeasuredValue", ItemType::kDefault },
      kInvalidNodeIndex }, // HaloaceticAcidsConcentrationMeasurement::single
    { { AttributeTag(2), "maxMeasuredValue", ItemType::kDefault },
      kInvalidNodeIndex }, // HaloaceticAcidsConcentrationMeasurement::single
    { { AttributeTag(3), "peakMeasuredValue", ItemType::kDefault },
      kInvalidNodeIndex }, // HaloaceticAcidsConcentrationMeasurement::single
    { { AttributeTag(4), "peakMeasuredValueWindow", ItemType::kDefault },
      kInvalidNodeIndex }, // HaloaceticAcidsConcentrationMeasurement::elapsed_s
    { { AttributeTag(5), "averageMeasuredValue", ItemType::kDefault },
      kInvalidNodeIndex }, // HaloaceticAcidsConcentrationMeasurement::single
    { { AttributeTag(6), "averageMeasuredValueWindow", ItemType::kDefault },
      kInvalidNodeIndex }, // HaloaceticAcidsConcentrationMeasurement::elapsed_s
    { { AttributeTag(7), "uncertainty", ItemType::kDefault },
      kInvalidNodeIndex }, // HaloaceticAcidsConcentrationMeasurement::single
    { { AttributeTag(8), "measurementUnit", ItemType::kEnum },
      kInvalidNodeIndex }, // HaloaceticAcidsConcentrationMeasurement::MeasurementUnitEnum
    { { AttributeTag(9), "measurementMedium", ItemType::kEnum },
      kInvalidNodeIndex }, // HaloaceticAcidsConcentrationMeasurement::MeasurementMediumEnum
    { { AttributeTag(10), "levelValue", ItemType::kEnum },
      kInvalidNodeIndex }, // HaloaceticAcidsConcentrationMeasurement::LevelValueEnum
    { { AttributeTag(65528), "generatedCommandList", ItemType::kList },
      kInvalidNodeIndex }, // HaloaceticAcidsConcentrationMeasurement::command_id[]
    { { AttributeTag(65529), "acceptedCommandList", ItemType::kList },
      kInvalidNodeIndex }, // HaloaceticAcidsConcentrationMeasurement::command_id[]
    { { AttributeTag(65530), "eventList", ItemType::kList },
      kInvalidNodeIndex }, // HaloaceticAcidsConcentrationMeasurement::event_id[]
    { { AttributeTag(65531), "attributeList", ItemType::kList },
      kInvalidNodeIndex }, // HaloaceticAcidsConcentrationMeasurement::attrib_id[]
    { { AttributeTag(65532), "featureMap", ItemType::kDefault },
      kInvalidNodeIndex }, // HaloaceticAcidsConcentrationMeasurement::bitmap32
    { { AttributeTag(65533), "clusterRevision", ItemType::kDefault },
      kInvalidNodeIndex }, // HaloaceticAcidsConcentrationMeasurement::int16u
};

const Entry<ItemInfo> _TotalTrihalomethanesConcentrationMeasurement[] = {
    { { AttributeTag(0), "measuredValue", ItemType::kDefault },
      kInvalidNodeIndex }, // TotalTrihalomethanesConcentrationMeasurement::single
    { { AttributeTag(1), "minMeasuredValue", ItemType::kDefault },
      kInvalidNodeIndex }, // TotalTrihalomethanesConcentrationMeasurement::single
    { { AttributeTag(2), "maxMeasuredValue", ItemType::kDefault },
      kInvalidNodeIndex }, // TotalTrihalomethanesConcentrationMeasurement::single
    { { AttributeTag(3), "peakMeasuredValue", ItemType::kDefault },
      kInvalidNodeIndex }, // TotalTrihalomethanesConcentrationMeasurement::single
    { { AttributeTag(4), "peakMeasuredValueWindow", ItemType::kDefault },
      kInvalidNodeIndex }, // TotalTrihalomethanesConcentrationMeasurement::elapsed_s
    { { AttributeTag(5), "averageMeasuredValue", ItemType::kDefault },
      kInvalidNodeIndex }, // TotalTrihalomethanesConcentrationMeasurement::single
    { { AttributeTag(6), "averageMeasuredValueWindow", ItemType::kDefault },
      kInvalidNodeIndex }, // TotalTrihalomethanesConcentrationMeasurement::elapsed_s
    { { AttributeTag(7), "uncertainty", ItemType::kDefault },
      kInvalidNodeIndex }, // TotalTrihalomethanesConcentrationMeasurement::single
    { { AttributeTag(8), "measurementUnit", ItemType::kEnum },
      kInvalidNodeIndex }, // TotalTrihalomethanesConcentrationMeasurement::MeasurementUnitEnum
    { { AttributeTag(9), "measurementMedium", ItemType::kEnum },
      kInvalidNodeIndex }, // TotalTrihalomethanesConcentrationMeasurement::MeasurementMediumEnum
    { { AttributeTag(10), "levelValue", ItemType::kEnum },
      kInvalidNodeIndex }, // TotalTrihalomethanesConcentrationMeasurement::LevelValueEnum
    { { AttributeTag(65528), "generatedCommandList", ItemType::kList },
      kInvalidNodeIndex }, // TotalTrihalomethanesConcentrationMeasurement::command_id[]
    { { AttributeTag(65529), "acceptedCommandList", ItemType::kList },
      kInvalidNodeIndex }, // TotalTrihalomethanesConcentrationMeasurement::command_id[]
    { { AttributeTag(65530), "eventList", ItemType::kList },
      kInvalidNodeIndex }, // TotalTrihalomethanesConcentrationMeasurement::event_id[]
    { { AttributeTag(65531), "attributeList", ItemType::kList },
      kInvalidNodeIndex }, // TotalTrihalomethanesConcentrationMeasurement::attrib_id[]
    { { AttributeTag(65532), "featureMap", ItemType::kDefault },
      kInvalidNodeIndex }, // TotalTrihalomethanesConcentrationMeasurement::bitmap32
    { { AttributeTag(65533), "clusterRevision", ItemType::kDefault },
      kInvalidNodeIndex }, // TotalTrihalomethanesConcentrationMeasurement::int16u
};

const Entry<ItemInfo> _TotalColiformBacteriaConcentrationMeasurement[] = {
    { { AttributeTag(0), "measuredValue", ItemType::kDefault },
      kInvalidNodeIndex }, // TotalColiformBacteriaConcentrationMeasurement::single
    { { AttributeTag(1), "minMeasuredValue", ItemType::kDefault },
      kInvalidNodeIndex }, // TotalColiformBacteriaConcentrationMeasurement::single
    { { AttributeTag(2), "maxMeasuredValue", ItemType::kDefault },
      kInvalidNodeIndex }, // TotalColiformBacteriaConcentrationMeasurement::single
    { { AttributeTag(3), "peakMeasuredValue", ItemType::kDefault },
      kInvalidNodeIndex }, // TotalColiformBacteriaConcentrationMeasurement::single
    { { AttributeTag(4), "peakMeasuredValueWindow", ItemType::kDefault },
      kInvalidNodeIndex }, // TotalColiformBacteriaConcentrationMeasurement::elapsed_s
    { { AttributeTag(5), "averageMeasuredValue", ItemType::kDefault },
      kInvalidNodeIndex }, // TotalColiformBacteriaConcentrationMeasurement::single
    { { AttributeTag(6), "averageMeasuredValueWindow", ItemType::kDefault },
      kInvalidNodeIndex }, // TotalColiformBacteriaConcentrationMeasurement::elapsed_s
    { { AttributeTag(7), "uncertainty", ItemType::kDefault },
      kInvalidNodeIndex }, // TotalColiformBacteriaConcentrationMeasurement::single
    { { AttributeTag(8), "measurementUnit", ItemType::kEnum },
      kInvalidNodeIndex }, // TotalColiformBacteriaConcentrationMeasurement::MeasurementUnitEnum
    { { AttributeTag(9), "measurementMedium", ItemType::kEnum },
      kInvalidNodeIndex }, // TotalColiformBacteriaConcentrationMeasurement::MeasurementMediumEnum
    { { AttributeTag(10), "levelValue", ItemType::kEnum },
      kInvalidNodeIndex }, // TotalColiformBacteriaConcentrationMeasurement::LevelValueEnum
    { { AttributeTag(65528), "generatedCommandList", ItemType::kList },
      kInvalidNodeIndex }, // TotalColiformBacteriaConcentrationMeasurement::command_id[]
    { { AttributeTag(65529), "acceptedCommandList", ItemType::kList },
      kInvalidNodeIndex }, // TotalColiformBacteriaConcentrationMeasurement::command_id[]
    { { AttributeTag(65530), "eventList", ItemType::kList },
      kInvalidNodeIndex }, // TotalColiformBacteriaConcentrationMeasurement::event_id[]
    { { AttributeTag(65531), "attributeList", ItemType::kList },
      kInvalidNodeIndex }, // TotalColiformBacteriaConcentrationMeasurement::attrib_id[]
    { { AttributeTag(65532), "featureMap", ItemType::kDefault },
      kInvalidNodeIndex }, // TotalColiformBacteriaConcentrationMeasurement::bitmap32
    { { AttributeTag(65533), "clusterRevision", ItemType::kDefault },
      kInvalidNodeIndex }, // TotalColiformBacteriaConcentrationMeasurement::int16u
};

const Entry<ItemInfo> _TurbidityConcentrationMeasurement[] = {
    { { AttributeTag(0), "measuredValue", ItemType::kDefault }, kInvalidNodeIndex },    // TurbidityConcentrationMeasurement::single
    { { AttributeTag(1), "minMeasuredValue", ItemType::kDefault }, kInvalidNodeIndex }, // TurbidityConcentrationMeasurement::single
    { { AttributeTag(2), "maxMeasuredValue", ItemType::kDefault }, kInvalidNodeIndex }, // TurbidityConcentrationMeasurement::single
    { { AttributeTag(3), "peakMeasuredValue", ItemType::kDefault },
      kInvalidNodeIndex }, // TurbidityConcentrationMeasurement::single
    { { AttributeTag(4), "peakMeasuredValueWindow", ItemType::kDefault },
      kInvalidNodeIndex }, // TurbidityConcentrationMeasurement::elapsed_s
    { { AttributeTag(5), "averageMeasuredValue", ItemType::kDefault },
      kInvalidNodeIndex }, // TurbidityConcentrationMeasurement::single
    { { AttributeTag(6), "averageMeasuredValueWindow", ItemType::kDefault },
      kInvalidNodeIndex },                                                         // TurbidityConcentrationMeasurement::elapsed_s
    { { AttributeTag(7), "uncertainty", ItemType::kDefault }, kInvalidNodeIndex }, // TurbidityConcentrationMeasurement::single
    { { AttributeTag(8), "measurementUnit", ItemType::kEnum },
      kInvalidNodeIndex }, // TurbidityConcentrationMeasurement::MeasurementUnitEnum
    { { AttributeTag(9), "measurementMedium", ItemType::kEnum },
      kInvalidNodeIndex }, // TurbidityConcentrationMeasurement::MeasurementMediumEnum
    { { AttributeTag(10), "levelValue", ItemType::kEnum }, kInvalidNodeIndex }, // TurbidityConcentrationMeasurement::LevelValueEnum
    { { AttributeTag(65528), "generatedCommandList", ItemType::kList },
      kInvalidNodeIndex }, // TurbidityConcentrationMeasurement::command_id[]
    { { AttributeTag(65529), "acceptedCommandList", ItemType::kList },
      kInvalidNodeIndex },                                                        // TurbidityConcentrationMeasurement::command_id[]
    { { AttributeTag(65530), "eventList", ItemType::kList }, kInvalidNodeIndex }, // TurbidityConcentrationMeasurement::event_id[]
    { { AttributeTag(65531), "attributeList", ItemType::kList },
      kInvalidNodeIndex }, // TurbidityConcentrationMeasurement::attrib_id[]
    { { AttributeTag(65532), "featureMap", ItemType::kDefault }, kInvalidNodeIndex }, // TurbidityConcentrationMeasurement::bitmap32
    { { AttributeTag(65533), "clusterRevision", ItemType::kDefault },
      kInvalidNodeIndex }, // TurbidityConcentrationMeasurement::int16u
};

const Entry<ItemInfo> _CopperConcentrationMeasurement[] = {
    { { AttributeTag(0), "measuredValue", ItemType::kDefault }, kInvalidNodeIndex },     // CopperConcentrationMeasurement::single
    { { AttributeTag(1), "minMeasuredValue", ItemType::kDefault }, kInvalidNodeIndex },  // CopperConcentrationMeasurement::single
    { { AttributeTag(2), "maxMeasuredValue", ItemType::kDefault }, kInvalidNodeIndex },  // CopperConcentrationMeasurement::single
    { { AttributeTag(3), "peakMeasuredValue", ItemType::kDefault }, kInvalidNodeIndex }, // CopperConcentrationMeasurement::single
    { { AttributeTag(4), "peakMeasuredValueWindow", ItemType::kDefault },
      kInvalidNodeIndex }, // CopperConcentrationMeasurement::elapsed_s
    { { AttributeTag(5), "averageMeasuredValue", ItemType::kDefault },
      kInvalidNodeIndex }, // CopperConcentrationMeasurement::single
    { { AttributeTag(6), "averageMeasuredValueWindow", ItemType::kDefault },
      kInvalidNodeIndex },                                                         // CopperConcentrationMeasurement::elapsed_s
    { { AttributeTag(7), "uncertainty", ItemType::kDefault }, kInvalidNodeIndex }, // CopperConcentrationMeasurement::single
    { { AttributeTag(8), "measurementUnit", ItemType::kEnum },
      kInvalidNodeIndex }, // CopperConcentrationMeasurement::MeasurementUnitEnum
    { { AttributeTag(9), "measurementMedium", ItemType::kEnum },
      kInvalidNodeIndex }, // CopperConcentrationMeasurement::MeasurementMediumEnum
    { { AttributeTag(10), "levelValue", ItemType::kEnum }, kInvalidNodeIndex }, // CopperConcentrationMeasurement::LevelValueEnum
    { { AttributeTag(65528), "generatedCommandList", ItemType::kList },
      kInvalidNodeIndex }, // CopperConcentrationMeasurement::command_id[]
    { { AttributeTag(65529), "acceptedCommandList", ItemType::kList },
      kInvalidNodeIndex },                                                        // CopperConcentrationMeasurement::command_id[]
    { { AttributeTag(65530), "eventList", ItemType::kList }, kInvalidNodeIndex }, // CopperConcentrationMeasurement::event_id[]
    { { AttributeTag(65531), "attributeList", ItemType::kList }, kInvalidNodeIndex }, // CopperConcentrationMeasurement::attrib_id[]
    { { AttributeTag(65532), "featureMap", ItemType::kDefault }, kInvalidNodeIndex }, // CopperConcentrationMeasurement::bitmap32
    { { AttributeTag(65533), "clusterRevision", ItemType::kDefault }, kInvalidNodeIndex }, // CopperConcentrationMeasurement::int16u
};

const Entry<ItemInfo> _LeadConcentrationMeasurement[] = {
    { { AttributeTag(0), "measuredValue", ItemType::kDefault }, kInvalidNodeIndex },     // LeadConcentrationMeasurement::single
    { { AttributeTag(1), "minMeasuredValue", ItemType::kDefault }, kInvalidNodeIndex },  // LeadConcentrationMeasurement::single
    { { AttributeTag(2), "maxMeasuredValue", ItemType::kDefault }, kInvalidNodeIndex },  // LeadConcentrationMeasurement::single
    { { AttributeTag(3), "peakMeasuredValue", ItemType::kDefault }, kInvalidNodeIndex }, // LeadConcentrationMeasurement::single
    { { AttributeTag(4), "peakMeasuredValueWindow", ItemType::kDefault },
      kInvalidNodeIndex }, // LeadConcentrationMeasurement::elapsed_s
    { { AttributeTag(5), "averageMeasuredValue", ItemType::kDefault }, kInvalidNodeIndex }, // LeadConcentrationMeasurement::single
    { { AttributeTag(6), "averageMeasuredValueWindow", ItemType::kDefault },
      kInvalidNodeIndex },                                                         // LeadConcentrationMeasurement::elapsed_s
    { { AttributeTag(7), "uncertainty", ItemType::kDefault }, kInvalidNodeIndex }, // LeadConcentrationMeasurement::single
    { { AttributeTag(8), "measurementUnit", ItemType::kEnum },
      kInvalidNodeIndex }, // LeadConcentrationMeasurement::MeasurementUnitEnum
    { { AttributeTag(9), "measurementMedium", ItemType::kEnum },
      kInvalidNodeIndex }, // LeadConcentrationMeasurement::MeasurementMediumEnum
    { { AttributeTag(10), "levelValue", ItemType::kEnum }, kInvalidNodeIndex }, // LeadConcentrationMeasurement::LevelValueEnum
    { { AttributeTag(65528), "generatedCommandList", ItemType::kList },
      kInvalidNodeIndex }, // LeadConcentrationMeasurement::command_id[]
    { { AttributeTag(65529), "acceptedCommandList", ItemType::kList },
      kInvalidNodeIndex },                                                            // LeadConcentrationMeasurement::command_id[]
    { { AttributeTag(65530), "eventList", ItemType::kList }, kInvalidNodeIndex },     // LeadConcentrationMeasurement::event_id[]
    { { AttributeTag(65531), "attributeList", ItemType::kList }, kInvalidNodeIndex }, // LeadConcentrationMeasurement::attrib_id[]
    { { AttributeTag(65532), "featureMap", ItemType::kDefault }, kInvalidNodeIndex }, // LeadConcentrationMeasurement::bitmap32
    { { AttributeTag(65533), "clusterRevision", ItemType::kDefault }, kInvalidNodeIndex }, // LeadConcentrationMeasurement::int16u
};

const Entry<ItemInfo> _ManganeseConcentrationMeasurement[] = {
    { { AttributeTag(0), "measuredValue", ItemType::kDefault }, kInvalidNodeIndex },    // ManganeseConcentrationMeasurement::single
    { { AttributeTag(1), "minMeasuredValue", ItemType::kDefault }, kInvalidNodeIndex }, // ManganeseConcentrationMeasurement::single
    { { AttributeTag(2), "maxMeasuredValue", ItemType::kDefault }, kInvalidNodeIndex }, // ManganeseConcentrationMeasurement::single
    { { AttributeTag(3), "peakMeasuredValue", ItemType::kDefault },
      kInvalidNodeIndex }, // ManganeseConcentrationMeasurement::single
    { { AttributeTag(4), "peakMeasuredValueWindow", ItemType::kDefault },
      kInvalidNodeIndex }, // ManganeseConcentrationMeasurement::elapsed_s
    { { AttributeTag(5), "averageMeasuredValue", ItemType::kDefault },
      kInvalidNodeIndex }, // ManganeseConcentrationMeasurement::single
    { { AttributeTag(6), "averageMeasuredValueWindow", ItemType::kDefault },
      kInvalidNodeIndex },                                                         // ManganeseConcentrationMeasurement::elapsed_s
    { { AttributeTag(7), "uncertainty", ItemType::kDefault }, kInvalidNodeIndex }, // ManganeseConcentrationMeasurement::single
    { { AttributeTag(8), "measurementUnit", ItemType::kEnum },
      kInvalidNodeIndex }, // ManganeseConcentrationMeasurement::MeasurementUnitEnum
    { { AttributeTag(9), "measurementMedium", ItemType::kEnum },
      kInvalidNodeIndex }, // ManganeseConcentrationMeasurement::MeasurementMediumEnum
    { { AttributeTag(10), "levelValue", ItemType::kEnum }, kInvalidNodeIndex }, // ManganeseConcentrationMeasurement::LevelValueEnum
    { { AttributeTag(65528), "generatedCommandList", ItemType::kList },
      kInvalidNodeIndex }, // ManganeseConcentrationMeasurement::command_id[]
    { { AttributeTag(65529), "acceptedCommandList", ItemType::kList },
      kInvalidNodeIndex },                                                        // ManganeseConcentrationMeasurement::command_id[]
    { { AttributeTag(65530), "eventList", ItemType::kList }, kInvalidNodeIndex }, // ManganeseConcentrationMeasurement::event_id[]
    { { AttributeTag(65531), "attributeList", ItemType::kList },
      kInvalidNodeIndex }, // ManganeseConcentrationMeasurement::attrib_id[]
    { { AttributeTag(65532), "featureMap", ItemType::kDefault }, kInvalidNodeIndex }, // ManganeseConcentrationMeasurement::bitmap32
    { { AttributeTag(65533), "clusterRevision", ItemType::kDefault },
      kInvalidNodeIndex }, // ManganeseConcentrationMeasurement::int16u
};

const Entry<ItemInfo> _SulfateConcentrationMeasurement[] = {
    { { AttributeTag(0), "measuredValue", ItemType::kDefault }, kInvalidNodeIndex },     // SulfateConcentrationMeasurement::single
    { { AttributeTag(1), "minMeasuredValue", ItemType::kDefault }, kInvalidNodeIndex },  // SulfateConcentrationMeasurement::single
    { { AttributeTag(2), "maxMeasuredValue", ItemType::kDefault }, kInvalidNodeIndex },  // SulfateConcentrationMeasurement::single
    { { AttributeTag(3), "peakMeasuredValue", ItemType::kDefault }, kInvalidNodeIndex }, // SulfateConcentrationMeasurement::single
    { { AttributeTag(4), "peakMeasuredValueWindow", ItemType::kDefault },
      kInvalidNodeIndex }, // SulfateConcentrationMeasurement::elapsed_s
    { { AttributeTag(5), "averageMeasuredValue", ItemType::kDefault },
      kInvalidNodeIndex }, // SulfateConcentrationMeasurement::single
    { { AttributeTag(6), "averageMeasuredValueWindow", ItemType::kDefault },
      kInvalidNodeIndex },                                                         // SulfateConcentrationMeasurement::elapsed_s
    { { AttributeTag(7), "uncertainty", ItemType::kDefault }, kInvalidNodeIndex }, // SulfateConcentrationMeasurement::single
    { { AttributeTag(8), "measurementUnit", ItemType::kEnum },
      kInvalidNodeIndex }, // SulfateConcentrationMeasurement::MeasurementUnitEnum
    { { AttributeTag(9), "measurementMedium", ItemType::kEnum },
      kInvalidNodeIndex }, // SulfateConcentrationMeasurement::MeasurementMediumEnum
    { { AttributeTag(10), "levelValue", ItemType::kEnum }, kInvalidNodeIndex }, // SulfateConcentrationMeasurement::LevelValueEnum
    { { AttributeTag(65528), "generatedCommandList", ItemType::kList },
      kInvalidNodeIndex }, // SulfateConcentrationMeasurement::command_id[]
    { { AttributeTag(65529), "acceptedCommandList", ItemType::kList },
      kInvalidNodeIndex },                                                        // SulfateConcentrationMeasurement::command_id[]
    { { AttributeTag(65530), "eventList", ItemType::kList }, kInvalidNodeIndex }, // SulfateConcentrationMeasurement::event_id[]
    { { AttributeTag(65531), "attributeList", ItemType::kList },
      kInvalidNodeIndex }, // SulfateConcentrationMeasurement::attrib_id[]
    { { AttributeTag(65532), "featureMap", ItemType::kDefault }, kInvalidNodeIndex }, // SulfateConcentrationMeasurement::bitmap32
    { { AttributeTag(65533), "clusterRevision", ItemType::kDefault },
      kInvalidNodeIndex }, // SulfateConcentrationMeasurement::int16u
};

const Entry<ItemInfo> _BromodichloromethaneConcentrationMeasurement[] = {
    { { AttributeTag(0), "measuredValue", ItemType::kDefault },
      kInvalidNodeIndex }, // BromodichloromethaneConcentrationMeasurement::single
    { { AttributeTag(1), "minMeasuredValue", ItemType::kDefault },
      kInvalidNodeIndex }, // BromodichloromethaneConcentrationMeasurement::single
    { { AttributeTag(2), "maxMeasuredValue", ItemType::kDefault },
      kInvalidNodeIndex }, // BromodichloromethaneConcentrationMeasurement::single
    { { AttributeTag(3), "peakMeasuredValue", ItemType::kDefault },
      kInvalidNodeIndex }, // BromodichloromethaneConcentrationMeasurement::single
    { { AttributeTag(4), "peakMeasuredValueWindow", ItemType::kDefault },
      kInvalidNodeIndex }, // BromodichloromethaneConcentrationMeasurement::elapsed_s
    { { AttributeTag(5), "averageMeasuredValue", ItemType::kDefault },
      kInvalidNodeIndex }, // BromodichloromethaneConcentrationMeasurement::single
    { { AttributeTag(6), "averageMeasuredValueWindow", ItemType::kDefault },
      kInvalidNodeIndex }, // BromodichloromethaneConcentrationMeasurement::elapsed_s
    { { AttributeTag(7), "uncertainty", ItemType::kDefault },
      kInvalidNodeIndex }, // BromodichloromethaneConcentrationMeasurement::single
    { { AttributeTag(8), "measurementUnit", ItemType::kEnum },
      kInvalidNodeIndex }, // BromodichloromethaneConcentrationMeasurement::MeasurementUnitEnum
    { { AttributeTag(9), "measurementMedium", ItemType::kEnum },
      kInvalidNodeIndex }, // BromodichloromethaneConcentrationMeasurement::MeasurementMediumEnum
    { { AttributeTag(10), "levelValue", ItemType::kEnum },
      kInvalidNodeIndex }, // BromodichloromethaneConcentrationMeasurement::LevelValueEnum
    { { AttributeTag(65528), "generatedCommandList", ItemType::kList },
      kInvalidNodeIndex }, // BromodichloromethaneConcentrationMeasurement::command_id[]
    { { AttributeTag(65529), "acceptedCommandList", ItemType::kList },
      kInvalidNodeIndex }, // BromodichloromethaneConcentrationMeasurement::command_id[]
    { { AttributeTag(65530), "eventList", ItemType::kList },
      kInvalidNodeIndex }, // BromodichloromethaneConcentrationMeasurement::event_id[]
    { { AttributeTag(65531), "attributeList", ItemType::kList },
      kInvalidNodeIndex }, // BromodichloromethaneConcentrationMeasurement::attrib_id[]
    { { AttributeTag(65532), "featureMap", ItemType::kDefault },
      kInvalidNodeIndex }, // BromodichloromethaneConcentrationMeasurement::bitmap32
    { { AttributeTag(65533), "clusterRevision", ItemType::kDefault },
      kInvalidNodeIndex }, // BromodichloromethaneConcentrationMeasurement::int16u
};

const Entry<ItemInfo> _BromoformConcentrationMeasurement[] = {
    { { AttributeTag(0), "measuredValue", ItemType::kDefault }, kInvalidNodeIndex },    // BromoformConcentrationMeasurement::single
    { { AttributeTag(1), "minMeasuredValue", ItemType::kDefault }, kInvalidNodeIndex }, // BromoformConcentrationMeasurement::single
    { { AttributeTag(2), "maxMeasuredValue", ItemType::kDefault }, kInvalidNodeIndex }, // BromoformConcentrationMeasurement::single
    { { AttributeTag(3), "peakMeasuredValue", ItemType::kDefault },
      kInvalidNodeIndex }, // BromoformConcentrationMeasurement::single
    { { AttributeTag(4), "peakMeasuredValueWindow", ItemType::kDefault },
      kInvalidNodeIndex }, // BromoformConcentrationMeasurement::elapsed_s
    { { AttributeTag(5), "averageMeasuredValue", ItemType::kDefault },
      kInvalidNodeIndex }, // BromoformConcentrationMeasurement::single
    { { AttributeTag(6), "averageMeasuredValueWindow", ItemType::kDefault },
      kInvalidNodeIndex },                                                         // BromoformConcentrationMeasurement::elapsed_s
    { { AttributeTag(7), "uncertainty", ItemType::kDefault }, kInvalidNodeIndex }, // BromoformConcentrationMeasurement::single
    { { AttributeTag(8), "measurementUnit", ItemType::kEnum },
      kInvalidNodeIndex }, // BromoformConcentrationMeasurement::MeasurementUnitEnum
    { { AttributeTag(9), "measurementMedium", ItemType::kEnum },
      kInvalidNodeIndex }, // BromoformConcentrationMeasurement::MeasurementMediumEnum
    { { AttributeTag(10), "levelValue", ItemType::kEnum }, kInvalidNodeIndex }, // BromoformConcentrationMeasurement::LevelValueEnum
    { { AttributeTag(65528), "generatedCommandList", ItemType::kList },
      kInvalidNodeIndex }, // BromoformConcentrationMeasurement::command_id[]
    { { AttributeTag(65529), "acceptedCommandList", ItemType::kList },
      kInvalidNodeIndex },                                                        // BromoformConcentrationMeasurement::command_id[]
    { { AttributeTag(65530), "eventList", ItemType::kList }, kInvalidNodeIndex }, // BromoformConcentrationMeasurement::event_id[]
    { { AttributeTag(65531), "attributeList", ItemType::kList },
      kInvalidNodeIndex }, // BromoformConcentrationMeasurement::attrib_id[]
    { { AttributeTag(65532), "featureMap", ItemType::kDefault }, kInvalidNodeIndex }, // BromoformConcentrationMeasurement::bitmap32
    { { AttributeTag(65533), "clusterRevision", ItemType::kDefault },
      kInvalidNodeIndex }, // BromoformConcentrationMeasurement::int16u
};

const Entry<ItemInfo> _ChlorodibromomethaneConcentrationMeasurement[] = {
    { { AttributeTag(0), "measuredValue", ItemType::kDefault },
      kInvalidNodeIndex }, // ChlorodibromomethaneConcentrationMeasurement::single
    { { AttributeTag(1), "minMeasuredValue", ItemType::kDefault },
      kInvalidNodeIndex }, // ChlorodibromomethaneConcentrationMeasurement::single
    { { AttributeTag(2), "maxMeasuredValue", ItemType::kDefault },
      kInvalidNodeIndex }, // ChlorodibromomethaneConcentrationMeasurement::single
    { { AttributeTag(3), "peakMeasuredValue", ItemType::kDefault },
      kInvalidNodeIndex }, // ChlorodibromomethaneConcentrationMeasurement::single
    { { AttributeTag(4), "peakMeasuredValueWindow", ItemType::kDefault },
      kInvalidNodeIndex }, // ChlorodibromomethaneConcentrationMeasurement::elapsed_s
    { { AttributeTag(5), "averageMeasuredValue", ItemType::kDefault },
      kInvalidNodeIndex }, // ChlorodibromomethaneConcentrationMeasurement::single
    { { AttributeTag(6), "averageMeasuredValueWindow", ItemType::kDefault },
      kInvalidNodeIndex }, // ChlorodibromomethaneConcentrationMeasurement::elapsed_s
    { { AttributeTag(7), "uncertainty", ItemType::kDefault },
      kInvalidNodeIndex }, // ChlorodibromomethaneConcentrationMeasurement::single
    { { AttributeTag(8), "measurementUnit", ItemType::kEnum },
      kInvalidNodeIndex }, // ChlorodibromomethaneConcentrationMeasurement::MeasurementUnitEnum
    { { AttributeTag(9), "measurementMedium", ItemType::kEnum },
      kInvalidNodeIndex }, // ChlorodibromomethaneConcentrationMeasurement::MeasurementMediumEnum
    { { AttributeTag(10), "levelValue", ItemType::kEnum },
      kInvalidNodeIndex }, // ChlorodibromomethaneConcentrationMeasurement::LevelValueEnum
    { { AttributeTag(65528), "generatedCommandList", ItemType::kList },
      kInvalidNodeIndex }, // ChlorodibromomethaneConcentrationMeasurement::command_id[]
    { { AttributeTag(65529), "acceptedCommandList", ItemType::kList },
      kInvalidNodeIndex }, // ChlorodibromomethaneConcentrationMeasurement::command_id[]
    { { AttributeTag(65530), "eventList", ItemType::kList },
      kInvalidNodeIndex }, // ChlorodibromomethaneConcentrationMeasurement::event_id[]
    { { AttributeTag(65531), "attributeList", ItemType::kList },
      kInvalidNodeIndex }, // ChlorodibromomethaneConcentrationMeasurement::attrib_id[]
    { { AttributeTag(65532), "featureMap", ItemType::kDefault },
      kInvalidNodeIndex }, // ChlorodibromomethaneConcentrationMeasurement::bitmap32
    { { AttributeTag(65533), "clusterRevision", ItemType::kDefault },
      kInvalidNodeIndex }, // ChlorodibromomethaneConcentrationMeasurement::int16u
};

const Entry<ItemInfo> _ChloroformConcentrationMeasurement[] = {
    { { AttributeTag(0), "measuredValue", ItemType::kDefault }, kInvalidNodeIndex }, // ChloroformConcentrationMeasurement::single
    { { AttributeTag(1), "minMeasuredValue", ItemType::kDefault },
      kInvalidNodeIndex }, // ChloroformConcentrationMeasurement::single
    { { AttributeTag(2), "maxMeasuredValue", ItemType::kDefault },
      kInvalidNodeIndex }, // ChloroformConcentrationMeasurement::single
    { { AttributeTag(3), "peakMeasuredValue", ItemType::kDefault },
      kInvalidNodeIndex }, // ChloroformConcentrationMeasurement::single
    { { AttributeTag(4), "peakMeasuredValueWindow", ItemType::kDefault },
      kInvalidNodeIndex }, // ChloroformConcentrationMeasurement::elapsed_s
    { { AttributeTag(5), "averageMeasuredValue", ItemType::kDefault },
      kInvalidNodeIndex }, // ChloroformConcentrationMeasurement::single
    { { AttributeTag(6), "averageMeasuredValueWindow", ItemType::kDefault },
      kInvalidNodeIndex },                                                         // ChloroformConcentrationMeasurement::elapsed_s
    { { AttributeTag(7), "uncertainty", ItemType::kDefault }, kInvalidNodeIndex }, // ChloroformConcentrationMeasurement::single
    { { AttributeTag(8), "measurementUnit", ItemType::kEnum },
      kInvalidNodeIndex }, // ChloroformConcentrationMeasurement::MeasurementUnitEnum
    { { AttributeTag(9), "measurementMedium", ItemType::kEnum },
      kInvalidNodeIndex }, // ChloroformConcentrationMeasurement::MeasurementMediumEnum
    { { AttributeTag(10), "levelValue", ItemType::kEnum },
      kInvalidNodeIndex }, // ChloroformConcentrationMeasurement::LevelValueEnum
    { { AttributeTag(65528), "generatedCommandList", ItemType::kList },
      kInvalidNodeIndex }, // ChloroformConcentrationMeasurement::command_id[]
    { { AttributeTag(65529), "acceptedCommandList", ItemType::kList },
      kInvalidNodeIndex }, // ChloroformConcentrationMeasurement::command_id[]
    { { AttributeTag(65530), "eventList", ItemType::kList }, kInvalidNodeIndex }, // ChloroformConcentrationMeasurement::event_id[]
    { { AttributeTag(65531), "attributeList", ItemType::kList },
      kInvalidNodeIndex }, // ChloroformConcentrationMeasurement::attrib_id[]
    { { AttributeTag(65532), "featureMap", ItemType::kDefault },
      kInvalidNodeIndex }, // ChloroformConcentrationMeasurement::bitmap32
    { { AttributeTag(65533), "clusterRevision", ItemType::kDefault },
      kInvalidNodeIndex }, // ChloroformConcentrationMeasurement::int16u
};

const Entry<ItemInfo> _SodiumConcentrationMeasurement[] = {
    { { AttributeTag(0), "measuredValue", ItemType::kDefault }, kInvalidNodeIndex },     // SodiumConcentrationMeasurement::single
    { { AttributeTag(1), "minMeasuredValue", ItemType::kDefault }, kInvalidNodeIndex },  // SodiumConcentrationMeasurement::single
    { { AttributeTag(2), "maxMeasuredValue", ItemType::kDefault }, kInvalidNodeIndex },  // SodiumConcentrationMeasurement::single
    { { AttributeTag(3), "peakMeasuredValue", ItemType::kDefault }, kInvalidNodeIndex }, // SodiumConcentrationMeasurement::single
    { { AttributeTag(4), "peakMeasuredValueWindow", ItemType::kDefault },
      kInvalidNodeIndex }, // SodiumConcentrationMeasurement::elapsed_s
    { { AttributeTag(5), "averageMeasuredValue", ItemType::kDefault },
      kInvalidNodeIndex }, // SodiumConcentrationMeasurement::single
    { { AttributeTag(6), "averageMeasuredValueWindow", ItemType::kDefault },
      kInvalidNodeIndex },                                                         // SodiumConcentrationMeasurement::elapsed_s
    { { AttributeTag(7), "uncertainty", ItemType::kDefault }, kInvalidNodeIndex }, // SodiumConcentrationMeasurement::single
    { { AttributeTag(8), "measurementUnit", ItemType::kEnum },
      kInvalidNodeIndex }, // SodiumConcentrationMeasurement::MeasurementUnitEnum
    { { AttributeTag(9), "measurementMedium", ItemType::kEnum },
      kInvalidNodeIndex }, // SodiumConcentrationMeasurement::MeasurementMediumEnum
    { { AttributeTag(10), "levelValue", ItemType::kEnum }, kInvalidNodeIndex }, // SodiumConcentrationMeasurement::LevelValueEnum
    { { AttributeTag(65528), "generatedCommandList", ItemType::kList },
      kInvalidNodeIndex }, // SodiumConcentrationMeasurement::command_id[]
    { { AttributeTag(65529), "acceptedCommandList", ItemType::kList },
      kInvalidNodeIndex },                                                        // SodiumConcentrationMeasurement::command_id[]
    { { AttributeTag(65530), "eventList", ItemType::kList }, kInvalidNodeIndex }, // SodiumConcentrationMeasurement::event_id[]
    { { AttributeTag(65531), "attributeList", ItemType::kList }, kInvalidNodeIndex }, // SodiumConcentrationMeasurement::attrib_id[]
    { { AttributeTag(65532), "featureMap", ItemType::kDefault }, kInvalidNodeIndex }, // SodiumConcentrationMeasurement::bitmap32
    { { AttributeTag(65533), "clusterRevision", ItemType::kDefault }, kInvalidNodeIndex }, // SodiumConcentrationMeasurement::int16u
};

const Entry<ItemInfo> _Pm25ConcentrationMeasurement[] = {
    { { AttributeTag(0), "measuredValue", ItemType::kDefault }, kInvalidNodeIndex },     // Pm25ConcentrationMeasurement::single
    { { AttributeTag(1), "minMeasuredValue", ItemType::kDefault }, kInvalidNodeIndex },  // Pm25ConcentrationMeasurement::single
    { { AttributeTag(2), "maxMeasuredValue", ItemType::kDefault }, kInvalidNodeIndex },  // Pm25ConcentrationMeasurement::single
    { { AttributeTag(3), "peakMeasuredValue", ItemType::kDefault }, kInvalidNodeIndex }, // Pm25ConcentrationMeasurement::single
    { { AttributeTag(4), "peakMeasuredValueWindow", ItemType::kDefault },
      kInvalidNodeIndex }, // Pm25ConcentrationMeasurement::elapsed_s
    { { AttributeTag(5), "averageMeasuredValue", ItemType::kDefault }, kInvalidNodeIndex }, // Pm25ConcentrationMeasurement::single
    { { AttributeTag(6), "averageMeasuredValueWindow", ItemType::kDefault },
      kInvalidNodeIndex },                                                         // Pm25ConcentrationMeasurement::elapsed_s
    { { AttributeTag(7), "uncertainty", ItemType::kDefault }, kInvalidNodeIndex }, // Pm25ConcentrationMeasurement::single
    { { AttributeTag(8), "measurementUnit", ItemType::kEnum },
      kInvalidNodeIndex }, // Pm25ConcentrationMeasurement::MeasurementUnitEnum
    { { AttributeTag(9), "measurementMedium", ItemType::kEnum },
      kInvalidNodeIndex }, // Pm25ConcentrationMeasurement::MeasurementMediumEnum
    { { AttributeTag(10), "levelValue", ItemType::kEnum }, kInvalidNodeIndex }, // Pm25ConcentrationMeasurement::LevelValueEnum
    { { AttributeTag(65528), "generatedCommandList", ItemType::kList },
      kInvalidNodeIndex }, // Pm25ConcentrationMeasurement::command_id[]
    { { AttributeTag(65529), "acceptedCommandList", ItemType::kList },
      kInvalidNodeIndex },                                                            // Pm25ConcentrationMeasurement::command_id[]
    { { AttributeTag(65530), "eventList", ItemType::kList }, kInvalidNodeIndex },     // Pm25ConcentrationMeasurement::event_id[]
    { { AttributeTag(65531), "attributeList", ItemType::kList }, kInvalidNodeIndex }, // Pm25ConcentrationMeasurement::attrib_id[]
    { { AttributeTag(65532), "featureMap", ItemType::kDefault }, kInvalidNodeIndex }, // Pm25ConcentrationMeasurement::bitmap32
    { { AttributeTag(65533), "clusterRevision", ItemType::kDefault }, kInvalidNodeIndex }, // Pm25ConcentrationMeasurement::int16u
};

const Entry<ItemInfo> _FormaldehydeConcentrationMeasurement[] = {
    { { AttributeTag(0), "measuredValue", ItemType::kDefault }, kInvalidNodeIndex }, // FormaldehydeConcentrationMeasurement::single
    { { AttributeTag(1), "minMeasuredValue", ItemType::kDefault },
      kInvalidNodeIndex }, // FormaldehydeConcentrationMeasurement::single
    { { AttributeTag(2), "maxMeasuredValue", ItemType::kDefault },
      kInvalidNodeIndex }, // FormaldehydeConcentrationMeasurement::single
    { { AttributeTag(3), "peakMeasuredValue", ItemType::kDefault },
      kInvalidNodeIndex }, // FormaldehydeConcentrationMeasurement::single
    { { AttributeTag(4), "peakMeasuredValueWindow", ItemType::kDefault },
      kInvalidNodeIndex }, // FormaldehydeConcentrationMeasurement::elapsed_s
    { { AttributeTag(5), "averageMeasuredValue", ItemType::kDefault },
      kInvalidNodeIndex }, // FormaldehydeConcentrationMeasurement::single
    { { AttributeTag(6), "averageMeasuredValueWindow", ItemType::kDefault },
      kInvalidNodeIndex }, // FormaldehydeConcentrationMeasurement::elapsed_s
    { { AttributeTag(7), "uncertainty", ItemType::kDefault }, kInvalidNodeIndex }, // FormaldehydeConcentrationMeasurement::single
    { { AttributeTag(8), "measurementUnit", ItemType::kEnum },
      kInvalidNodeIndex }, // FormaldehydeConcentrationMeasurement::MeasurementUnitEnum
    { { AttributeTag(9), "measurementMedium", ItemType::kEnum },
      kInvalidNodeIndex }, // FormaldehydeConcentrationMeasurement::MeasurementMediumEnum
    { { AttributeTag(10), "levelValue", ItemType::kEnum },
      kInvalidNodeIndex }, // FormaldehydeConcentrationMeasurement::LevelValueEnum
    { { AttributeTag(65528), "generatedCommandList", ItemType::kList },
      kInvalidNodeIndex }, // FormaldehydeConcentrationMeasurement::command_id[]
    { { AttributeTag(65529), "acceptedCommandList", ItemType::kList },
      kInvalidNodeIndex }, // FormaldehydeConcentrationMeasurement::command_id[]
    { { AttributeTag(65530), "eventList", ItemType::kList },
      kInvalidNodeIndex }, // FormaldehydeConcentrationMeasurement::event_id[]
    { { AttributeTag(65531), "attributeList", ItemType::kList },
      kInvalidNodeIndex }, // FormaldehydeConcentrationMeasurement::attrib_id[]
    { { AttributeTag(65532), "featureMap", ItemType::kDefault },
      kInvalidNodeIndex }, // FormaldehydeConcentrationMeasurement::bitmap32
    { { AttributeTag(65533), "clusterRevision", ItemType::kDefault },
      kInvalidNodeIndex }, // FormaldehydeConcentrationMeasurement::int16u
};

const Entry<ItemInfo> _Pm1ConcentrationMeasurement[] = {
    { { AttributeTag(0), "measuredValue", ItemType::kDefault }, kInvalidNodeIndex },     // Pm1ConcentrationMeasurement::single
    { { AttributeTag(1), "minMeasuredValue", ItemType::kDefault }, kInvalidNodeIndex },  // Pm1ConcentrationMeasurement::single
    { { AttributeTag(2), "maxMeasuredValue", ItemType::kDefault }, kInvalidNodeIndex },  // Pm1ConcentrationMeasurement::single
    { { AttributeTag(3), "peakMeasuredValue", ItemType::kDefault }, kInvalidNodeIndex }, // Pm1ConcentrationMeasurement::single
    { { AttributeTag(4), "peakMeasuredValueWindow", ItemType::kDefault },
      kInvalidNodeIndex }, // Pm1ConcentrationMeasurement::elapsed_s
    { { AttributeTag(5), "averageMeasuredValue", ItemType::kDefault }, kInvalidNodeIndex }, // Pm1ConcentrationMeasurement::single
    { { AttributeTag(6), "averageMeasuredValueWindow", ItemType::kDefault },
      kInvalidNodeIndex },                                                         // Pm1ConcentrationMeasurement::elapsed_s
    { { AttributeTag(7), "uncertainty", ItemType::kDefault }, kInvalidNodeIndex }, // Pm1ConcentrationMeasurement::single
    { { AttributeTag(8), "measurementUnit", ItemType::kEnum },
      kInvalidNodeIndex }, // Pm1ConcentrationMeasurement::MeasurementUnitEnum
    { { AttributeTag(9), "measurementMedium", ItemType::kEnum },
      kInvalidNodeIndex }, // Pm1ConcentrationMeasurement::MeasurementMediumEnum
    { { AttributeTag(10), "levelValue", ItemType::kEnum }, kInvalidNodeIndex }, // Pm1ConcentrationMeasurement::LevelValueEnum
    { { AttributeTag(65528), "generatedCommandList", ItemType::kList },
      kInvalidNodeIndex }, // Pm1ConcentrationMeasurement::command_id[]
    { { AttributeTag(65529), "acceptedCommandList", ItemType::kList },
      kInvalidNodeIndex },                                                            // Pm1ConcentrationMeasurement::command_id[]
    { { AttributeTag(65530), "eventList", ItemType::kList }, kInvalidNodeIndex },     // Pm1ConcentrationMeasurement::event_id[]
    { { AttributeTag(65531), "attributeList", ItemType::kList }, kInvalidNodeIndex }, // Pm1ConcentrationMeasurement::attrib_id[]
    { { AttributeTag(65532), "featureMap", ItemType::kDefault }, kInvalidNodeIndex }, // Pm1ConcentrationMeasurement::bitmap32
    { { AttributeTag(65533), "clusterRevision", ItemType::kDefault }, kInvalidNodeIndex }, // Pm1ConcentrationMeasurement::int16u
};

const Entry<ItemInfo> _Pm10ConcentrationMeasurement[] = {
    { { AttributeTag(0), "measuredValue", ItemType::kDefault }, kInvalidNodeIndex },     // Pm10ConcentrationMeasurement::single
    { { AttributeTag(1), "minMeasuredValue", ItemType::kDefault }, kInvalidNodeIndex },  // Pm10ConcentrationMeasurement::single
    { { AttributeTag(2), "maxMeasuredValue", ItemType::kDefault }, kInvalidNodeIndex },  // Pm10ConcentrationMeasurement::single
    { { AttributeTag(3), "peakMeasuredValue", ItemType::kDefault }, kInvalidNodeIndex }, // Pm10ConcentrationMeasurement::single
    { { AttributeTag(4), "peakMeasuredValueWindow", ItemType::kDefault },
      kInvalidNodeIndex }, // Pm10ConcentrationMeasurement::elapsed_s
    { { AttributeTag(5), "averageMeasuredValue", ItemType::kDefault }, kInvalidNodeIndex }, // Pm10ConcentrationMeasurement::single
    { { AttributeTag(6), "averageMeasuredValueWindow", ItemType::kDefault },
      kInvalidNodeIndex },                                                         // Pm10ConcentrationMeasurement::elapsed_s
    { { AttributeTag(7), "uncertainty", ItemType::kDefault }, kInvalidNodeIndex }, // Pm10ConcentrationMeasurement::single
    { { AttributeTag(8), "measurementUnit", ItemType::kEnum },
      kInvalidNodeIndex }, // Pm10ConcentrationMeasurement::MeasurementUnitEnum
    { { AttributeTag(9), "measurementMedium", ItemType::kEnum },
      kInvalidNodeIndex }, // Pm10ConcentrationMeasurement::MeasurementMediumEnum
    { { AttributeTag(10), "levelValue", ItemType::kEnum }, kInvalidNodeIndex }, // Pm10ConcentrationMeasurement::LevelValueEnum
    { { AttributeTag(65528), "generatedCommandList", ItemType::kList },
      kInvalidNodeIndex }, // Pm10ConcentrationMeasurement::command_id[]
    { { AttributeTag(65529), "acceptedCommandList", ItemType::kList },
      kInvalidNodeIndex },                                                            // Pm10ConcentrationMeasurement::command_id[]
    { { AttributeTag(65530), "eventList", ItemType::kList }, kInvalidNodeIndex },     // Pm10ConcentrationMeasurement::event_id[]
    { { AttributeTag(65531), "attributeList", ItemType::kList }, kInvalidNodeIndex }, // Pm10ConcentrationMeasurement::attrib_id[]
    { { AttributeTag(65532), "featureMap", ItemType::kDefault }, kInvalidNodeIndex }, // Pm10ConcentrationMeasurement::bitmap32
    { { AttributeTag(65533), "clusterRevision", ItemType::kDefault }, kInvalidNodeIndex }, // Pm10ConcentrationMeasurement::int16u
};

const Entry<ItemInfo> _TotalVolatileOrganicCompoundsConcentrationMeasurement[] = {
    { { AttributeTag(0), "measuredValue", ItemType::kDefault },
      kInvalidNodeIndex }, // TotalVolatileOrganicCompoundsConcentrationMeasurement::single
    { { AttributeTag(1), "minMeasuredValue", ItemType::kDefault },
      kInvalidNodeIndex }, // TotalVolatileOrganicCompoundsConcentrationMeasurement::single
    { { AttributeTag(2), "maxMeasuredValue", ItemType::kDefault },
      kInvalidNodeIndex }, // TotalVolatileOrganicCompoundsConcentrationMeasurement::single
    { { AttributeTag(3), "peakMeasuredValue", ItemType::kDefault },
      kInvalidNodeIndex }, // TotalVolatileOrganicCompoundsConcentrationMeasurement::single
    { { AttributeTag(4), "peakMeasuredValueWindow", ItemType::kDefault },
      kInvalidNodeIndex }, // TotalVolatileOrganicCompoundsConcentrationMeasurement::elapsed_s
    { { AttributeTag(5), "averageMeasuredValue", ItemType::kDefault },
      kInvalidNodeIndex }, // TotalVolatileOrganicCompoundsConcentrationMeasurement::single
    { { AttributeTag(6), "averageMeasuredValueWindow", ItemType::kDefault },
      kInvalidNodeIndex }, // TotalVolatileOrganicCompoundsConcentrationMeasurement::elapsed_s
    { { AttributeTag(7), "uncertainty", ItemType::kDefault },
      kInvalidNodeIndex }, // TotalVolatileOrganicCompoundsConcentrationMeasurement::single
    { { AttributeTag(8), "measurementUnit", ItemType::kEnum },
      kInvalidNodeIndex }, // TotalVolatileOrganicCompoundsConcentrationMeasurement::MeasurementUnitEnum
    { { AttributeTag(9), "measurementMedium", ItemType::kEnum },
      kInvalidNodeIndex }, // TotalVolatileOrganicCompoundsConcentrationMeasurement::MeasurementMediumEnum
    { { AttributeTag(10), "levelValue", ItemType::kEnum },
      kInvalidNodeIndex }, // TotalVolatileOrganicCompoundsConcentrationMeasurement::LevelValueEnum
    { { AttributeTag(65528), "generatedCommandList", ItemType::kList },
      kInvalidNodeIndex }, // TotalVolatileOrganicCompoundsConcentrationMeasurement::command_id[]
    { { AttributeTag(65529), "acceptedCommandList", ItemType::kList },
      kInvalidNodeIndex }, // TotalVolatileOrganicCompoundsConcentrationMeasurement::command_id[]
    { { AttributeTag(65530), "eventList", ItemType::kList },
      kInvalidNodeIndex }, // TotalVolatileOrganicCompoundsConcentrationMeasurement::event_id[]
    { { AttributeTag(65531), "attributeList", ItemType::kList },
      kInvalidNodeIndex }, // TotalVolatileOrganicCompoundsConcentrationMeasurement::attrib_id[]
    { { AttributeTag(65532), "featureMap", ItemType::kDefault },
      kInvalidNodeIndex }, // TotalVolatileOrganicCompoundsConcentrationMeasurement::bitmap32
    { { AttributeTag(65533), "clusterRevision", ItemType::kDefault },
      kInvalidNodeIndex }, // TotalVolatileOrganicCompoundsConcentrationMeasurement::int16u
};

const Entry<ItemInfo> _RadonConcentrationMeasurement[] = {
    { { AttributeTag(0), "measuredValue", ItemType::kDefault }, kInvalidNodeIndex },     // RadonConcentrationMeasurement::single
    { { AttributeTag(1), "minMeasuredValue", ItemType::kDefault }, kInvalidNodeIndex },  // RadonConcentrationMeasurement::single
    { { AttributeTag(2), "maxMeasuredValue", ItemType::kDefault }, kInvalidNodeIndex },  // RadonConcentrationMeasurement::single
    { { AttributeTag(3), "peakMeasuredValue", ItemType::kDefault }, kInvalidNodeIndex }, // RadonConcentrationMeasurement::single
    { { AttributeTag(4), "peakMeasuredValueWindow", ItemType::kDefault },
      kInvalidNodeIndex }, // RadonConcentrationMeasurement::elapsed_s
    { { AttributeTag(5), "averageMeasuredValue", ItemType::kDefault }, kInvalidNodeIndex }, // RadonConcentrationMeasurement::single
    { { AttributeTag(6), "averageMeasuredValueWindow", ItemType::kDefault },
      kInvalidNodeIndex },                                                         // RadonConcentrationMeasurement::elapsed_s
    { { AttributeTag(7), "uncertainty", ItemType::kDefault }, kInvalidNodeIndex }, // RadonConcentrationMeasurement::single
    { { AttributeTag(8), "measurementUnit", ItemType::kEnum },
      kInvalidNodeIndex }, // RadonConcentrationMeasurement::MeasurementUnitEnum
    { { AttributeTag(9), "measurementMedium", ItemType::kEnum },
      kInvalidNodeIndex }, // RadonConcentrationMeasurement::MeasurementMediumEnum
    { { AttributeTag(10), "levelValue", ItemType::kEnum }, kInvalidNodeIndex }, // RadonConcentrationMeasurement::LevelValueEnum
    { { AttributeTag(65528), "generatedCommandList", ItemType::kList },
      kInvalidNodeIndex }, // RadonConcentrationMeasurement::command_id[]
    { { AttributeTag(65529), "acceptedCommandList", ItemType::kList },
      kInvalidNodeIndex },                                                            // RadonConcentrationMeasurement::command_id[]
    { { AttributeTag(65530), "eventList", ItemType::kList }, kInvalidNodeIndex },     // RadonConcentrationMeasurement::event_id[]
    { { AttributeTag(65531), "attributeList", ItemType::kList }, kInvalidNodeIndex }, // RadonConcentrationMeasurement::attrib_id[]
    { { AttributeTag(65532), "featureMap", ItemType::kDefault }, kInvalidNodeIndex }, // RadonConcentrationMeasurement::bitmap32
    { { AttributeTag(65533), "clusterRevision", ItemType::kDefault }, kInvalidNodeIndex }, // RadonConcentrationMeasurement::int16u
};

const Entry<ItemInfo> _WakeOnLan[] = {
    { { AttributeTag(0), "MACAddress", ItemType::kDefault }, kInvalidNodeIndex },            // WakeOnLan::char_string
    { { AttributeTag(65528), "generatedCommandList", ItemType::kList }, kInvalidNodeIndex }, // WakeOnLan::command_id[]
    { { AttributeTag(65529), "acceptedCommandList", ItemType::kList }, kInvalidNodeIndex },  // WakeOnLan::command_id[]
    { { AttributeTag(65530), "eventList", ItemType::kList }, kInvalidNodeIndex },            // WakeOnLan::event_id[]
    { { AttributeTag(65531), "attributeList", ItemType::kList }, kInvalidNodeIndex },        // WakeOnLan::attrib_id[]
    { { AttributeTag(65532), "featureMap", ItemType::kDefault }, kInvalidNodeIndex },        // WakeOnLan::bitmap32
    { { AttributeTag(65533), "clusterRevision", ItemType::kDefault }, kInvalidNodeIndex },   // WakeOnLan::int16u
};

const Entry<ItemInfo> _Channel[] = {
    { { AttributeTag(0), "channelList", ItemType::kList }, 479 },                            // Channel::ChannelInfoStruct[]
    { { AttributeTag(1), "lineup", ItemType::kDefault }, 474 },                              // Channel::LineupInfoStruct
    { { AttributeTag(2), "currentChannel", ItemType::kDefault }, 473 },                      // Channel::ChannelInfoStruct
    { { AttributeTag(65528), "generatedCommandList", ItemType::kList }, kInvalidNodeIndex }, // Channel::command_id[]
    { { AttributeTag(65529), "acceptedCommandList", ItemType::kList }, kInvalidNodeIndex },  // Channel::command_id[]
    { { AttributeTag(65530), "eventList", ItemType::kList }, kInvalidNodeIndex },            // Channel::event_id[]
    { { AttributeTag(65531), "attributeList", ItemType::kList }, kInvalidNodeIndex },        // Channel::attrib_id[]
    { { AttributeTag(65532), "featureMap", ItemType::kDefault }, kInvalidNodeIndex },        // Channel::bitmap32
    { { AttributeTag(65533), "clusterRevision", ItemType::kDefault }, kInvalidNodeIndex },   // Channel::int16u
    { { CommandTag(0), "ChangeChannelRequest", ItemType::kDefault }, 475 }, // Channel::ChangeChannel::ChangeChannelRequest
    { { CommandTag(2), "ChangeChannelByNumberRequest", ItemType::kDefault },
      477 }, // Channel::ChangeChannelByNumber::ChangeChannelByNumberRequest
    { { CommandTag(3), "SkipChannelRequest", ItemType::kDefault }, 478 },    // Channel::SkipChannel::SkipChannelRequest
    { { CommandTag(1), "ChangeChannelResponse", ItemType::kDefault }, 476 }, // Channel::ChangeChannelResponse
};

const Entry<ItemInfo> _Channel_ChannelInfoStruct[] = {
    { { ContextTag(0), "majorNumber", ItemType::kDefault }, kInvalidNodeIndex },       // Channel::int16u
    { { ContextTag(1), "minorNumber", ItemType::kDefault }, kInvalidNodeIndex },       // Channel::int16u
    { { ContextTag(2), "name", ItemType::kDefault }, kInvalidNodeIndex },              // Channel::char_string
    { { ContextTag(3), "callSign", ItemType::kDefault }, kInvalidNodeIndex },          // Channel::char_string
    { { ContextTag(4), "affiliateCallSign", ItemType::kDefault }, kInvalidNodeIndex }, // Channel::char_string
};

const Entry<ItemInfo> _Channel_LineupInfoStruct[] = {
    { { ContextTag(0), "operatorName", ItemType::kDefault }, kInvalidNodeIndex }, // Channel::char_string
    { { ContextTag(1), "lineupName", ItemType::kDefault }, kInvalidNodeIndex },   // Channel::char_string
    { { ContextTag(2), "postalCode", ItemType::kDefault }, kInvalidNodeIndex },   // Channel::char_string
    { { ContextTag(3), "lineupInfoType", ItemType::kEnum }, kInvalidNodeIndex },  // Channel::LineupInfoTypeEnum
};

const Entry<ItemInfo> _Channel_ChangeChannelRequest[] = {
    { { ContextTag(0), "match", ItemType::kDefault }, kInvalidNodeIndex }, // Channel::CHAR_STRING
};

const Entry<ItemInfo> _Channel_ChangeChannelResponse[] = {
    { { ContextTag(0), "status", ItemType::kEnum }, kInvalidNodeIndex },  // Channel::ChannelStatusEnum
    { { ContextTag(1), "data", ItemType::kDefault }, kInvalidNodeIndex }, // Channel::CHAR_STRING
};

const Entry<ItemInfo> _Channel_ChangeChannelByNumberRequest[] = {
    { { ContextTag(0), "majorNumber", ItemType::kDefault }, kInvalidNodeIndex }, // Channel::INT16U
    { { ContextTag(1), "minorNumber", ItemType::kDefault }, kInvalidNodeIndex }, // Channel::INT16U
};

const Entry<ItemInfo> _Channel_SkipChannelRequest[] = {
    { { ContextTag(0), "count", ItemType::kDefault }, kInvalidNodeIndex }, // Channel::INT16S
};

const Entry<ItemInfo> _Channel_ChannelInfoStruct_list_[] = {
    { { AnonymousTag(), "[]", ItemType::kDefault }, 473 }, // Channel_ChannelInfoStruct[]
};

const Entry<ItemInfo> _TargetNavigator[] = {
    { { AttributeTag(0), "targetList", ItemType::kList }, 484 },                             // TargetNavigator::TargetInfoStruct[]
    { { AttributeTag(1), "currentTarget", ItemType::kDefault }, kInvalidNodeIndex },         // TargetNavigator::int8u
    { { AttributeTag(65528), "generatedCommandList", ItemType::kList }, kInvalidNodeIndex }, // TargetNavigator::command_id[]
    { { AttributeTag(65529), "acceptedCommandList", ItemType::kList }, kInvalidNodeIndex },  // TargetNavigator::command_id[]
    { { AttributeTag(65530), "eventList", ItemType::kList }, kInvalidNodeIndex },            // TargetNavigator::event_id[]
    { { AttributeTag(65531), "attributeList", ItemType::kList }, kInvalidNodeIndex },        // TargetNavigator::attrib_id[]
    { { AttributeTag(65532), "featureMap", ItemType::kDefault }, kInvalidNodeIndex },        // TargetNavigator::bitmap32
    { { AttributeTag(65533), "clusterRevision", ItemType::kDefault }, kInvalidNodeIndex },   // TargetNavigator::int16u
    { { CommandTag(0), "NavigateTargetRequest", ItemType::kDefault },
      482 }, // TargetNavigator::NavigateTarget::NavigateTargetRequest
    { { CommandTag(1), "NavigateTargetResponse", ItemType::kDefault }, 483 }, // TargetNavigator::NavigateTargetResponse
};

const Entry<ItemInfo> _TargetNavigator_TargetInfoStruct[] = {
    { { ContextTag(0), "identifier", ItemType::kDefault }, kInvalidNodeIndex }, // TargetNavigator::int8u
    { { ContextTag(1), "name", ItemType::kDefault }, kInvalidNodeIndex },       // TargetNavigator::char_string
};

const Entry<ItemInfo> _TargetNavigator_NavigateTargetRequest[] = {
    { { ContextTag(0), "target", ItemType::kDefault }, kInvalidNodeIndex }, // TargetNavigator::INT8U
    { { ContextTag(1), "data", ItemType::kDefault }, kInvalidNodeIndex },   // TargetNavigator::CHAR_STRING
};

const Entry<ItemInfo> _TargetNavigator_NavigateTargetResponse[] = {
    { { ContextTag(0), "status", ItemType::kEnum }, kInvalidNodeIndex },  // TargetNavigator::TargetNavigatorStatusEnum
    { { ContextTag(1), "data", ItemType::kDefault }, kInvalidNodeIndex }, // TargetNavigator::CHAR_STRING
};

const Entry<ItemInfo> _TargetNavigator_TargetInfoStruct_list_[] = {
    { { AnonymousTag(), "[]", ItemType::kDefault }, 481 }, // TargetNavigator_TargetInfoStruct[]
};

const Entry<ItemInfo> _MediaPlayback[] = {
    { { AttributeTag(0), "currentState", ItemType::kEnum }, kInvalidNodeIndex },      // MediaPlayback::PlaybackStateEnum
    { { AttributeTag(1), "startTime", ItemType::kDefault }, kInvalidNodeIndex },      // MediaPlayback::epoch_us
    { { AttributeTag(2), "duration", ItemType::kDefault }, kInvalidNodeIndex },       // MediaPlayback::int64u
    { { AttributeTag(3), "sampledPosition", ItemType::kDefault }, 486 },              // MediaPlayback::PlaybackPositionStruct
    { { AttributeTag(4), "playbackSpeed", ItemType::kDefault }, kInvalidNodeIndex },  // MediaPlayback::single
    { { AttributeTag(5), "seekRangeEnd", ItemType::kDefault }, kInvalidNodeIndex },   // MediaPlayback::int64u
    { { AttributeTag(6), "seekRangeStart", ItemType::kDefault }, kInvalidNodeIndex }, // MediaPlayback::int64u
    { { AttributeTag(65528), "generatedCommandList", ItemType::kList }, kInvalidNodeIndex }, // MediaPlayback::command_id[]
    { { AttributeTag(65529), "acceptedCommandList", ItemType::kList }, kInvalidNodeIndex },  // MediaPlayback::command_id[]
    { { AttributeTag(65530), "eventList", ItemType::kList }, kInvalidNodeIndex },            // MediaPlayback::event_id[]
    { { AttributeTag(65531), "attributeList", ItemType::kList }, kInvalidNodeIndex },        // MediaPlayback::attrib_id[]
    { { AttributeTag(65532), "featureMap", ItemType::kDefault }, kInvalidNodeIndex },        // MediaPlayback::bitmap32
    { { AttributeTag(65533), "clusterRevision", ItemType::kDefault }, kInvalidNodeIndex },   // MediaPlayback::int16u
    { { CommandTag(8), "SkipForwardRequest", ItemType::kDefault }, 487 },  // MediaPlayback::SkipForward::SkipForwardRequest
    { { CommandTag(9), "SkipBackwardRequest", ItemType::kDefault }, 488 }, // MediaPlayback::SkipBackward::SkipBackwardRequest
    { { CommandTag(11), "SeekRequest", ItemType::kDefault }, 490 },        // MediaPlayback::Seek::SeekRequest
    { { CommandTag(10), "PlaybackResponse", ItemType::kDefault }, 489 },   // MediaPlayback::PlaybackResponse
};

const Entry<ItemInfo> _MediaPlayback_PlaybackPositionStruct[] = {
    { { ContextTag(0), "updatedAt", ItemType::kDefault }, kInvalidNodeIndex }, // MediaPlayback::epoch_us
    { { ContextTag(1), "position", ItemType::kDefault }, kInvalidNodeIndex },  // MediaPlayback::int64u
};

const Entry<ItemInfo> _MediaPlayback_SkipForwardRequest[] = {
    { { ContextTag(0), "deltaPositionMilliseconds", ItemType::kDefault }, kInvalidNodeIndex }, // MediaPlayback::INT64U
};

const Entry<ItemInfo> _MediaPlayback_SkipBackwardRequest[] = {
    { { ContextTag(0), "deltaPositionMilliseconds", ItemType::kDefault }, kInvalidNodeIndex }, // MediaPlayback::INT64U
};

const Entry<ItemInfo> _MediaPlayback_PlaybackResponse[] = {
    { { ContextTag(0), "status", ItemType::kEnum }, kInvalidNodeIndex },  // MediaPlayback::MediaPlaybackStatusEnum
    { { ContextTag(1), "data", ItemType::kDefault }, kInvalidNodeIndex }, // MediaPlayback::CHAR_STRING
};

const Entry<ItemInfo> _MediaPlayback_SeekRequest[] = {
    { { ContextTag(0), "position", ItemType::kDefault }, kInvalidNodeIndex }, // MediaPlayback::INT64U
};

const Entry<ItemInfo> _MediaInput[] = {
    { { AttributeTag(0), "inputList", ItemType::kList }, 495 },                              // MediaInput::InputInfoStruct[]
    { { AttributeTag(1), "currentInput", ItemType::kDefault }, kInvalidNodeIndex },          // MediaInput::int8u
    { { AttributeTag(65528), "generatedCommandList", ItemType::kList }, kInvalidNodeIndex }, // MediaInput::command_id[]
    { { AttributeTag(65529), "acceptedCommandList", ItemType::kList }, kInvalidNodeIndex },  // MediaInput::command_id[]
    { { AttributeTag(65530), "eventList", ItemType::kList }, kInvalidNodeIndex },            // MediaInput::event_id[]
    { { AttributeTag(65531), "attributeList", ItemType::kList }, kInvalidNodeIndex },        // MediaInput::attrib_id[]
    { { AttributeTag(65532), "featureMap", ItemType::kDefault }, kInvalidNodeIndex },        // MediaInput::bitmap32
    { { AttributeTag(65533), "clusterRevision", ItemType::kDefault }, kInvalidNodeIndex },   // MediaInput::int16u
    { { CommandTag(0), "SelectInputRequest", ItemType::kDefault }, 493 }, // MediaInput::SelectInput::SelectInputRequest
    { { CommandTag(3), "RenameInputRequest", ItemType::kDefault }, 494 }, // MediaInput::RenameInput::RenameInputRequest
};

const Entry<ItemInfo> _MediaInput_InputInfoStruct[] = {
    { { ContextTag(0), "index", ItemType::kDefault }, kInvalidNodeIndex },       // MediaInput::int8u
    { { ContextTag(1), "inputType", ItemType::kEnum }, kInvalidNodeIndex },      // MediaInput::InputTypeEnum
    { { ContextTag(2), "name", ItemType::kDefault }, kInvalidNodeIndex },        // MediaInput::char_string
    { { ContextTag(3), "description", ItemType::kDefault }, kInvalidNodeIndex }, // MediaInput::char_string
};

const Entry<ItemInfo> _MediaInput_SelectInputRequest[] = {
    { { ContextTag(0), "index", ItemType::kDefault }, kInvalidNodeIndex }, // MediaInput::INT8U
};

const Entry<ItemInfo> _MediaInput_RenameInputRequest[] = {
    { { ContextTag(0), "index", ItemType::kDefault }, kInvalidNodeIndex }, // MediaInput::INT8U
    { { ContextTag(1), "name", ItemType::kDefault }, kInvalidNodeIndex },  // MediaInput::CHAR_STRING
};

const Entry<ItemInfo> _MediaInput_InputInfoStruct_list_[] = {
    { { AnonymousTag(), "[]", ItemType::kDefault }, 492 }, // MediaInput_InputInfoStruct[]
};

const Entry<ItemInfo> _LowPower[] = {
    { { AttributeTag(65528), "generatedCommandList", ItemType::kList }, kInvalidNodeIndex }, // LowPower::command_id[]
    { { AttributeTag(65529), "acceptedCommandList", ItemType::kList }, kInvalidNodeIndex },  // LowPower::command_id[]
    { { AttributeTag(65530), "eventList", ItemType::kList }, kInvalidNodeIndex },            // LowPower::event_id[]
    { { AttributeTag(65531), "attributeList", ItemType::kList }, kInvalidNodeIndex },        // LowPower::attrib_id[]
    { { AttributeTag(65532), "featureMap", ItemType::kDefault }, kInvalidNodeIndex },        // LowPower::bitmap32
    { { AttributeTag(65533), "clusterRevision", ItemType::kDefault }, kInvalidNodeIndex },   // LowPower::int16u
};

const Entry<ItemInfo> _KeypadInput[] = {
    { { AttributeTag(65528), "generatedCommandList", ItemType::kList }, kInvalidNodeIndex }, // KeypadInput::command_id[]
    { { AttributeTag(65529), "acceptedCommandList", ItemType::kList }, kInvalidNodeIndex },  // KeypadInput::command_id[]
    { { AttributeTag(65530), "eventList", ItemType::kList }, kInvalidNodeIndex },            // KeypadInput::event_id[]
    { { AttributeTag(65531), "attributeList", ItemType::kList }, kInvalidNodeIndex },        // KeypadInput::attrib_id[]
    { { AttributeTag(65532), "featureMap", ItemType::kDefault }, kInvalidNodeIndex },        // KeypadInput::bitmap32
    { { AttributeTag(65533), "clusterRevision", ItemType::kDefault }, kInvalidNodeIndex },   // KeypadInput::int16u
    { { CommandTag(0), "SendKeyRequest", ItemType::kDefault }, 498 },                        // KeypadInput::SendKey::SendKeyRequest
    { { CommandTag(1), "SendKeyResponse", ItemType::kDefault }, 499 },                       // KeypadInput::SendKeyResponse
};

const Entry<ItemInfo> _KeypadInput_SendKeyRequest[] = {
    { { ContextTag(0), "keyCode", ItemType::kEnum }, kInvalidNodeIndex }, // KeypadInput::CecKeyCode
};

const Entry<ItemInfo> _KeypadInput_SendKeyResponse[] = {
    { { ContextTag(0), "status", ItemType::kEnum }, kInvalidNodeIndex }, // KeypadInput::KeypadInputStatusEnum
};

const Entry<ItemInfo> _ContentLauncher[] = {
    { { AttributeTag(0), "acceptHeader", ItemType::kList }, kInvalidNodeIndex },                   // ContentLauncher::CHAR_STRING[]
    { { AttributeTag(1), "supportedStreamingProtocols", ItemType::kDefault }, kInvalidNodeIndex }, // ContentLauncher::bitmap32
    { { AttributeTag(65528), "generatedCommandList", ItemType::kList }, kInvalidNodeIndex },       // ContentLauncher::command_id[]
    { { AttributeTag(65529), "acceptedCommandList", ItemType::kList }, kInvalidNodeIndex },        // ContentLauncher::command_id[]
    { { AttributeTag(65530), "eventList", ItemType::kList }, kInvalidNodeIndex },                  // ContentLauncher::event_id[]
    { { AttributeTag(65531), "attributeList", ItemType::kList }, kInvalidNodeIndex },              // ContentLauncher::attrib_id[]
    { { AttributeTag(65532), "featureMap", ItemType::kDefault }, kInvalidNodeIndex },              // ContentLauncher::bitmap32
    { { AttributeTag(65533), "clusterRevision", ItemType::kDefault }, kInvalidNodeIndex },         // ContentLauncher::int16u
    { { CommandTag(0), "LaunchContentRequest", ItemType::kDefault }, 507 }, // ContentLauncher::LaunchContent::LaunchContentRequest
    { { CommandTag(1), "LaunchURLRequest", ItemType::kDefault }, 508 },     // ContentLauncher::LaunchURL::LaunchURLRequest
    { { CommandTag(2), "LauncherResponse", ItemType::kDefault }, 509 },     // ContentLauncher::LauncherResponse
};

const Entry<ItemInfo> _ContentLauncher_DimensionStruct[] = {
    { { ContextTag(0), "width", ItemType::kDefault }, kInvalidNodeIndex },  // ContentLauncher::double
    { { ContextTag(1), "height", ItemType::kDefault }, kInvalidNodeIndex }, // ContentLauncher::double
    { { ContextTag(2), "metric", ItemType::kEnum }, kInvalidNodeIndex },    // ContentLauncher::MetricTypeEnum
};

const Entry<ItemInfo> _ContentLauncher_AdditionalInfoStruct[] = {
    { { ContextTag(0), "name", ItemType::kDefault }, kInvalidNodeIndex },  // ContentLauncher::char_string
    { { ContextTag(1), "value", ItemType::kDefault }, kInvalidNodeIndex }, // ContentLauncher::char_string
};

const Entry<ItemInfo> _ContentLauncher_ParameterStruct[] = {
    { { ContextTag(0), "type", ItemType::kEnum }, kInvalidNodeIndex },     // ContentLauncher::ParameterEnum
    { { ContextTag(1), "value", ItemType::kDefault }, kInvalidNodeIndex }, // ContentLauncher::char_string
    { { ContextTag(2), "externalIDList", ItemType::kList }, 510 },         // ContentLauncher::AdditionalInfoStruct[]
};

const Entry<ItemInfo> _ContentLauncher_ContentSearchStruct[] = {
    { { ContextTag(0), "parameterList", ItemType::kList }, 511 }, // ContentLauncher::ParameterStruct[]
};

const Entry<ItemInfo> _ContentLauncher_StyleInformationStruct[] = {
    { { ContextTag(0), "imageURL", ItemType::kDefault }, kInvalidNodeIndex }, // ContentLauncher::char_string
    { { ContextTag(1), "color", ItemType::kDefault }, kInvalidNodeIndex },    // ContentLauncher::char_string
    { { ContextTag(2), "size", ItemType::kDefault }, 501 },                   // ContentLauncher::DimensionStruct
};

const Entry<ItemInfo> _ContentLauncher_BrandingInformationStruct[] = {
    { { ContextTag(0), "providerName", ItemType::kDefault }, kInvalidNodeIndex }, // ContentLauncher::char_string
    { { ContextTag(1), "background", ItemType::kDefault }, 505 },                 // ContentLauncher::StyleInformationStruct
    { { ContextTag(2), "logo", ItemType::kDefault }, 505 },                       // ContentLauncher::StyleInformationStruct
    { { ContextTag(3), "progressBar", ItemType::kDefault }, 505 },                // ContentLauncher::StyleInformationStruct
    { { ContextTag(4), "splash", ItemType::kDefault }, 505 },                     // ContentLauncher::StyleInformationStruct
    { { ContextTag(5), "waterMark", ItemType::kDefault }, 505 },                  // ContentLauncher::StyleInformationStruct
};

const Entry<ItemInfo> _ContentLauncher_LaunchContentRequest[] = {
    { { ContextTag(0), "search", ItemType::kDefault }, 504 },                 // ContentLauncher::ContentSearchStruct
    { { ContextTag(1), "autoPlay", ItemType::kDefault }, kInvalidNodeIndex }, // ContentLauncher::BOOLEAN
    { { ContextTag(2), "data", ItemType::kDefault }, kInvalidNodeIndex },     // ContentLauncher::CHAR_STRING
};

const Entry<ItemInfo> _ContentLauncher_LaunchURLRequest[] = {
    { { ContextTag(0), "contentURL", ItemType::kDefault }, kInvalidNodeIndex },    // ContentLauncher::CHAR_STRING
    { { ContextTag(1), "displayString", ItemType::kDefault }, kInvalidNodeIndex }, // ContentLauncher::CHAR_STRING
    { { ContextTag(2), "brandingInformation", ItemType::kDefault }, 506 },         // ContentLauncher::BrandingInformationStruct
};

const Entry<ItemInfo> _ContentLauncher_LauncherResponse[] = {
    { { ContextTag(0), "status", ItemType::kEnum }, kInvalidNodeIndex },  // ContentLauncher::ContentLaunchStatusEnum
    { { ContextTag(1), "data", ItemType::kDefault }, kInvalidNodeIndex }, // ContentLauncher::CHAR_STRING
};

const Entry<ItemInfo> _ContentLauncher_AdditionalInfoStruct_list_[] = {
    { { AnonymousTag(), "[]", ItemType::kDefault }, 502 }, // ContentLauncher_AdditionalInfoStruct[]
};

const Entry<ItemInfo> _ContentLauncher_ParameterStruct_list_[] = {
    { { AnonymousTag(), "[]", ItemType::kDefault }, 503 }, // ContentLauncher_ParameterStruct[]
};

const Entry<ItemInfo> _AudioOutput[] = {
    { { AttributeTag(0), "outputList", ItemType::kList }, 516 },                             // AudioOutput::OutputInfoStruct[]
    { { AttributeTag(1), "currentOutput", ItemType::kDefault }, kInvalidNodeIndex },         // AudioOutput::int8u
    { { AttributeTag(65528), "generatedCommandList", ItemType::kList }, kInvalidNodeIndex }, // AudioOutput::command_id[]
    { { AttributeTag(65529), "acceptedCommandList", ItemType::kList }, kInvalidNodeIndex },  // AudioOutput::command_id[]
    { { AttributeTag(65530), "eventList", ItemType::kList }, kInvalidNodeIndex },            // AudioOutput::event_id[]
    { { AttributeTag(65531), "attributeList", ItemType::kList }, kInvalidNodeIndex },        // AudioOutput::attrib_id[]
    { { AttributeTag(65532), "featureMap", ItemType::kDefault }, kInvalidNodeIndex },        // AudioOutput::bitmap32
    { { AttributeTag(65533), "clusterRevision", ItemType::kDefault }, kInvalidNodeIndex },   // AudioOutput::int16u
    { { CommandTag(0), "SelectOutputRequest", ItemType::kDefault }, 514 }, // AudioOutput::SelectOutput::SelectOutputRequest
    { { CommandTag(1), "RenameOutputRequest", ItemType::kDefault }, 515 }, // AudioOutput::RenameOutput::RenameOutputRequest
};

const Entry<ItemInfo> _AudioOutput_OutputInfoStruct[] = {
    { { ContextTag(0), "index", ItemType::kDefault }, kInvalidNodeIndex },   // AudioOutput::int8u
    { { ContextTag(1), "outputType", ItemType::kEnum }, kInvalidNodeIndex }, // AudioOutput::OutputTypeEnum
    { { ContextTag(2), "name", ItemType::kDefault }, kInvalidNodeIndex },    // AudioOutput::char_string
};

const Entry<ItemInfo> _AudioOutput_SelectOutputRequest[] = {
    { { ContextTag(0), "index", ItemType::kDefault }, kInvalidNodeIndex }, // AudioOutput::INT8U
};

const Entry<ItemInfo> _AudioOutput_RenameOutputRequest[] = {
    { { ContextTag(0), "index", ItemType::kDefault }, kInvalidNodeIndex }, // AudioOutput::INT8U
    { { ContextTag(1), "name", ItemType::kDefault }, kInvalidNodeIndex },  // AudioOutput::CHAR_STRING
};

const Entry<ItemInfo> _AudioOutput_OutputInfoStruct_list_[] = {
    { { AnonymousTag(), "[]", ItemType::kDefault }, 513 }, // AudioOutput_OutputInfoStruct[]
};

const Entry<ItemInfo> _ApplicationLauncher[] = {
    { { AttributeTag(0), "catalogList", ItemType::kList }, kInvalidNodeIndex }, // ApplicationLauncher::INT16U[]
    { { AttributeTag(1), "currentApp", ItemType::kDefault }, 519 },             // ApplicationLauncher::ApplicationEPStruct
    { { AttributeTag(65528), "generatedCommandList", ItemType::kList }, kInvalidNodeIndex }, // ApplicationLauncher::command_id[]
    { { AttributeTag(65529), "acceptedCommandList", ItemType::kList }, kInvalidNodeIndex },  // ApplicationLauncher::command_id[]
    { { AttributeTag(65530), "eventList", ItemType::kList }, kInvalidNodeIndex },            // ApplicationLauncher::event_id[]
    { { AttributeTag(65531), "attributeList", ItemType::kList }, kInvalidNodeIndex },        // ApplicationLauncher::attrib_id[]
    { { AttributeTag(65532), "featureMap", ItemType::kDefault }, kInvalidNodeIndex },        // ApplicationLauncher::bitmap32
    { { AttributeTag(65533), "clusterRevision", ItemType::kDefault }, kInvalidNodeIndex },   // ApplicationLauncher::int16u
    { { CommandTag(0), "LaunchAppRequest", ItemType::kDefault }, 520 }, // ApplicationLauncher::LaunchApp::LaunchAppRequest
    { { CommandTag(1), "StopAppRequest", ItemType::kDefault }, 521 },   // ApplicationLauncher::StopApp::StopAppRequest
    { { CommandTag(2), "HideAppRequest", ItemType::kDefault }, 522 },   // ApplicationLauncher::HideApp::HideAppRequest
    { { CommandTag(3), "LauncherResponse", ItemType::kDefault }, 523 }, // ApplicationLauncher::LauncherResponse
};

const Entry<ItemInfo> _ApplicationLauncher_ApplicationStruct[] = {
    { { ContextTag(0), "catalogVendorID", ItemType::kDefault }, kInvalidNodeIndex }, // ApplicationLauncher::int16u
    { { ContextTag(1), "applicationID", ItemType::kDefault }, kInvalidNodeIndex },   // ApplicationLauncher::char_string
};

const Entry<ItemInfo> _ApplicationLauncher_ApplicationEPStruct[] = {
    { { ContextTag(0), "application", ItemType::kDefault }, 518 },            // ApplicationLauncher::ApplicationStruct
    { { ContextTag(1), "endpoint", ItemType::kDefault }, kInvalidNodeIndex }, // ApplicationLauncher::endpoint_no
};

const Entry<ItemInfo> _ApplicationLauncher_LaunchAppRequest[] = {
    { { ContextTag(0), "application", ItemType::kDefault }, 518 },        // ApplicationLauncher::ApplicationStruct
    { { ContextTag(1), "data", ItemType::kDefault }, kInvalidNodeIndex }, // ApplicationLauncher::OCTET_STRING
};

const Entry<ItemInfo> _ApplicationLauncher_StopAppRequest[] = {
    { { ContextTag(0), "application", ItemType::kDefault }, 518 }, // ApplicationLauncher::ApplicationStruct
};

const Entry<ItemInfo> _ApplicationLauncher_HideAppRequest[] = {
    { { ContextTag(0), "application", ItemType::kDefault }, 518 }, // ApplicationLauncher::ApplicationStruct
};

const Entry<ItemInfo> _ApplicationLauncher_LauncherResponse[] = {
    { { ContextTag(0), "status", ItemType::kEnum }, kInvalidNodeIndex },  // ApplicationLauncher::ApplicationLauncherStatusEnum
    { { ContextTag(1), "data", ItemType::kDefault }, kInvalidNodeIndex }, // ApplicationLauncher::OCTET_STRING
};

const Entry<ItemInfo> _ApplicationBasic[] = {
    { { AttributeTag(0), "vendorName", ItemType::kDefault }, kInvalidNodeIndex },         // ApplicationBasic::char_string
    { { AttributeTag(1), "vendorID", ItemType::kDefault }, kInvalidNodeIndex },           // ApplicationBasic::vendor_id
    { { AttributeTag(2), "applicationName", ItemType::kDefault }, kInvalidNodeIndex },    // ApplicationBasic::char_string
    { { AttributeTag(3), "productID", ItemType::kDefault }, kInvalidNodeIndex },          // ApplicationBasic::int16u
    { { AttributeTag(4), "application", ItemType::kDefault }, 525 },                      // ApplicationBasic::ApplicationStruct
    { { AttributeTag(5), "status", ItemType::kEnum }, kInvalidNodeIndex },                // ApplicationBasic::ApplicationStatusEnum
    { { AttributeTag(6), "applicationVersion", ItemType::kDefault }, kInvalidNodeIndex }, // ApplicationBasic::char_string
    { { AttributeTag(7), "allowedVendorList", ItemType::kList }, kInvalidNodeIndex },     // ApplicationBasic::vendor_id[]
    { { AttributeTag(65528), "generatedCommandList", ItemType::kList }, kInvalidNodeIndex }, // ApplicationBasic::command_id[]
    { { AttributeTag(65529), "acceptedCommandList", ItemType::kList }, kInvalidNodeIndex },  // ApplicationBasic::command_id[]
    { { AttributeTag(65530), "eventList", ItemType::kList }, kInvalidNodeIndex },            // ApplicationBasic::event_id[]
    { { AttributeTag(65531), "attributeList", ItemType::kList }, kInvalidNodeIndex },        // ApplicationBasic::attrib_id[]
    { { AttributeTag(65532), "featureMap", ItemType::kDefault }, kInvalidNodeIndex },        // ApplicationBasic::bitmap32
    { { AttributeTag(65533), "clusterRevision", ItemType::kDefault }, kInvalidNodeIndex },   // ApplicationBasic::int16u
};

const Entry<ItemInfo> _ApplicationBasic_ApplicationStruct[] = {
    { { ContextTag(0), "catalogVendorID", ItemType::kDefault }, kInvalidNodeIndex }, // ApplicationBasic::int16u
    { { ContextTag(1), "applicationID", ItemType::kDefault }, kInvalidNodeIndex },   // ApplicationBasic::char_string
};

const Entry<ItemInfo> _AccountLogin[] = {
    { { AttributeTag(65528), "generatedCommandList", ItemType::kList }, kInvalidNodeIndex }, // AccountLogin::command_id[]
    { { AttributeTag(65529), "acceptedCommandList", ItemType::kList }, kInvalidNodeIndex },  // AccountLogin::command_id[]
    { { AttributeTag(65530), "eventList", ItemType::kList }, kInvalidNodeIndex },            // AccountLogin::event_id[]
    { { AttributeTag(65531), "attributeList", ItemType::kList }, kInvalidNodeIndex },        // AccountLogin::attrib_id[]
    { { AttributeTag(65532), "featureMap", ItemType::kDefault }, kInvalidNodeIndex },        // AccountLogin::bitmap32
    { { AttributeTag(65533), "clusterRevision", ItemType::kDefault }, kInvalidNodeIndex },   // AccountLogin::int16u
    { { CommandTag(0), "GetSetupPINRequest", ItemType::kDefault }, 527 },  // AccountLogin::GetSetupPIN::GetSetupPINRequest
    { { CommandTag(2), "LoginRequest", ItemType::kDefault }, 529 },        // AccountLogin::Login::LoginRequest
    { { CommandTag(1), "GetSetupPINResponse", ItemType::kDefault }, 528 }, // AccountLogin::GetSetupPINResponse
};

const Entry<ItemInfo> _AccountLogin_GetSetupPINRequest[] = {
    { { ContextTag(0), "tempAccountIdentifier", ItemType::kDefault }, kInvalidNodeIndex }, // AccountLogin::CHAR_STRING
};

const Entry<ItemInfo> _AccountLogin_GetSetupPINResponse[] = {
    { { ContextTag(0), "setupPIN", ItemType::kDefault }, kInvalidNodeIndex }, // AccountLogin::CHAR_STRING
};

const Entry<ItemInfo> _AccountLogin_LoginRequest[] = {
    { { ContextTag(0), "tempAccountIdentifier", ItemType::kDefault }, kInvalidNodeIndex }, // AccountLogin::CHAR_STRING
    { { ContextTag(1), "setupPIN", ItemType::kDefault }, kInvalidNodeIndex },              // AccountLogin::CHAR_STRING
};

const Entry<ItemInfo> _ElectricalMeasurement[] = {
    { { AttributeTag(0), "measurementType", ItemType::kDefault }, kInvalidNodeIndex },       // ElectricalMeasurement::bitmap32
    { { AttributeTag(256), "dcVoltage", ItemType::kDefault }, kInvalidNodeIndex },           // ElectricalMeasurement::int16s
    { { AttributeTag(257), "dcVoltageMin", ItemType::kDefault }, kInvalidNodeIndex },        // ElectricalMeasurement::int16s
    { { AttributeTag(258), "dcVoltageMax", ItemType::kDefault }, kInvalidNodeIndex },        // ElectricalMeasurement::int16s
    { { AttributeTag(259), "dcCurrent", ItemType::kDefault }, kInvalidNodeIndex },           // ElectricalMeasurement::int16s
    { { AttributeTag(260), "dcCurrentMin", ItemType::kDefault }, kInvalidNodeIndex },        // ElectricalMeasurement::int16s
    { { AttributeTag(261), "dcCurrentMax", ItemType::kDefault }, kInvalidNodeIndex },        // ElectricalMeasurement::int16s
    { { AttributeTag(262), "dcPower", ItemType::kDefault }, kInvalidNodeIndex },             // ElectricalMeasurement::int16s
    { { AttributeTag(263), "dcPowerMin", ItemType::kDefault }, kInvalidNodeIndex },          // ElectricalMeasurement::int16s
    { { AttributeTag(264), "dcPowerMax", ItemType::kDefault }, kInvalidNodeIndex },          // ElectricalMeasurement::int16s
    { { AttributeTag(512), "dcVoltageMultiplier", ItemType::kDefault }, kInvalidNodeIndex }, // ElectricalMeasurement::int16u
    { { AttributeTag(513), "dcVoltageDivisor", ItemType::kDefault }, kInvalidNodeIndex },    // ElectricalMeasurement::int16u
    { { AttributeTag(514), "dcCurrentMultiplier", ItemType::kDefault }, kInvalidNodeIndex }, // ElectricalMeasurement::int16u
    { { AttributeTag(515), "dcCurrentDivisor", ItemType::kDefault }, kInvalidNodeIndex },    // ElectricalMeasurement::int16u
    { { AttributeTag(516), "dcPowerMultiplier", ItemType::kDefault }, kInvalidNodeIndex },   // ElectricalMeasurement::int16u
    { { AttributeTag(517), "dcPowerDivisor", ItemType::kDefault }, kInvalidNodeIndex },      // ElectricalMeasurement::int16u
    { { AttributeTag(768), "acFrequency", ItemType::kDefault }, kInvalidNodeIndex },         // ElectricalMeasurement::int16u
    { { AttributeTag(769), "acFrequencyMin", ItemType::kDefault }, kInvalidNodeIndex },      // ElectricalMeasurement::int16u
    { { AttributeTag(770), "acFrequencyMax", ItemType::kDefault }, kInvalidNodeIndex },      // ElectricalMeasurement::int16u
    { { AttributeTag(771), "neutralCurrent", ItemType::kDefault }, kInvalidNodeIndex },      // ElectricalMeasurement::int16u
    { { AttributeTag(772), "totalActivePower", ItemType::kDefault }, kInvalidNodeIndex },    // ElectricalMeasurement::int32s
    { { AttributeTag(773), "totalReactivePower", ItemType::kDefault }, kInvalidNodeIndex },  // ElectricalMeasurement::int32s
    { { AttributeTag(774), "totalApparentPower", ItemType::kDefault }, kInvalidNodeIndex },  // ElectricalMeasurement::int32u
    { { AttributeTag(775), "measured1stHarmonicCurrent", ItemType::kDefault }, kInvalidNodeIndex }, // ElectricalMeasurement::int16s
    { { AttributeTag(776), "measured3rdHarmonicCurrent", ItemType::kDefault }, kInvalidNodeIndex }, // ElectricalMeasurement::int16s
    { { AttributeTag(777), "measured5thHarmonicCurrent", ItemType::kDefault }, kInvalidNodeIndex }, // ElectricalMeasurement::int16s
    { { AttributeTag(778), "measured7thHarmonicCurrent", ItemType::kDefault }, kInvalidNodeIndex }, // ElectricalMeasurement::int16s
    { { AttributeTag(779), "measured9thHarmonicCurrent", ItemType::kDefault }, kInvalidNodeIndex }, // ElectricalMeasurement::int16s
    { { AttributeTag(780), "measured11thHarmonicCurrent", ItemType::kDefault },
      kInvalidNodeIndex }, // ElectricalMeasurement::int16s
    { { AttributeTag(781), "measuredPhase1stHarmonicCurrent", ItemType::kDefault },
      kInvalidNodeIndex }, // ElectricalMeasurement::int16s
    { { AttributeTag(782), "measuredPhase3rdHarmonicCurrent", ItemType::kDefault },
      kInvalidNodeIndex }, // ElectricalMeasurement::int16s
    { { AttributeTag(783), "measuredPhase5thHarmonicCurrent", ItemType::kDefault },
      kInvalidNodeIndex }, // ElectricalMeasurement::int16s
    { { AttributeTag(784), "measuredPhase7thHarmonicCurrent", ItemType::kDefault },
      kInvalidNodeIndex }, // ElectricalMeasurement::int16s
    { { AttributeTag(785), "measuredPhase9thHarmonicCurrent", ItemType::kDefault },
      kInvalidNodeIndex }, // ElectricalMeasurement::int16s
    { { AttributeTag(786), "measuredPhase11thHarmonicCurrent", ItemType::kDefault },
      kInvalidNodeIndex },                                                                          // ElectricalMeasurement::int16s
    { { AttributeTag(1024), "acFrequencyMultiplier", ItemType::kDefault }, kInvalidNodeIndex },     // ElectricalMeasurement::int16u
    { { AttributeTag(1025), "acFrequencyDivisor", ItemType::kDefault }, kInvalidNodeIndex },        // ElectricalMeasurement::int16u
    { { AttributeTag(1026), "powerMultiplier", ItemType::kDefault }, kInvalidNodeIndex },           // ElectricalMeasurement::int32u
    { { AttributeTag(1027), "powerDivisor", ItemType::kDefault }, kInvalidNodeIndex },              // ElectricalMeasurement::int32u
    { { AttributeTag(1028), "harmonicCurrentMultiplier", ItemType::kDefault }, kInvalidNodeIndex }, // ElectricalMeasurement::int8s
    { { AttributeTag(1029), "phaseHarmonicCurrentMultiplier", ItemType::kDefault },
      kInvalidNodeIndex },                                                                         // ElectricalMeasurement::int8s
    { { AttributeTag(1280), "instantaneousVoltage", ItemType::kDefault }, kInvalidNodeIndex },     // ElectricalMeasurement::int16s
    { { AttributeTag(1281), "instantaneousLineCurrent", ItemType::kDefault }, kInvalidNodeIndex }, // ElectricalMeasurement::int16u
    { { AttributeTag(1282), "instantaneousActiveCurrent", ItemType::kDefault },
      kInvalidNodeIndex }, // ElectricalMeasurement::int16s
    { { AttributeTag(1283), "instantaneousReactiveCurrent", ItemType::kDefault },
      kInvalidNodeIndex },                                                                   // ElectricalMeasurement::int16s
    { { AttributeTag(1284), "instantaneousPower", ItemType::kDefault }, kInvalidNodeIndex }, // ElectricalMeasurement::int16s
    { { AttributeTag(1285), "rmsVoltage", ItemType::kDefault }, kInvalidNodeIndex },         // ElectricalMeasurement::int16u
    { { AttributeTag(1286), "rmsVoltageMin", ItemType::kDefault }, kInvalidNodeIndex },      // ElectricalMeasurement::int16u
    { { AttributeTag(1287), "rmsVoltageMax", ItemType::kDefault }, kInvalidNodeIndex },      // ElectricalMeasurement::int16u
    { { AttributeTag(1288), "rmsCurrent", ItemType::kDefault }, kInvalidNodeIndex },         // ElectricalMeasurement::int16u
    { { AttributeTag(1289), "rmsCurrentMin", ItemType::kDefault }, kInvalidNodeIndex },      // ElectricalMeasurement::int16u
    { { AttributeTag(1290), "rmsCurrentMax", ItemType::kDefault }, kInvalidNodeIndex },      // ElectricalMeasurement::int16u
    { { AttributeTag(1291), "activePower", ItemType::kDefault }, kInvalidNodeIndex },        // ElectricalMeasurement::int16s
    { { AttributeTag(1292), "activePowerMin", ItemType::kDefault }, kInvalidNodeIndex },     // ElectricalMeasurement::int16s
    { { AttributeTag(1293), "activePowerMax", ItemType::kDefault }, kInvalidNodeIndex },     // ElectricalMeasurement::int16s
    { { AttributeTag(1294), "reactivePower", ItemType::kDefault }, kInvalidNodeIndex },      // ElectricalMeasurement::int16s
    { { AttributeTag(1295), "apparentPower", ItemType::kDefault }, kInvalidNodeIndex },      // ElectricalMeasurement::int16u
    { { AttributeTag(1296), "powerFactor", ItemType::kDefault }, kInvalidNodeIndex },        // ElectricalMeasurement::int8s
    { { AttributeTag(1297), "averageRmsVoltageMeasurementPeriod", ItemType::kDefault },
      kInvalidNodeIndex }, // ElectricalMeasurement::int16u
    { { AttributeTag(1299), "averageRmsUnderVoltageCounter", ItemType::kDefault },
      kInvalidNodeIndex }, // ElectricalMeasurement::int16u
    { { AttributeTag(1300), "rmsExtremeOverVoltagePeriod", ItemType::kDefault },
      kInvalidNodeIndex }, // ElectricalMeasurement::int16u
    { { AttributeTag(1301), "rmsExtremeUnderVoltagePeriod", ItemType::kDefault },
      kInvalidNodeIndex },                                                                        // ElectricalMeasurement::int16u
    { { AttributeTag(1302), "rmsVoltageSagPeriod", ItemType::kDefault }, kInvalidNodeIndex },     // ElectricalMeasurement::int16u
    { { AttributeTag(1303), "rmsVoltageSwellPeriod", ItemType::kDefault }, kInvalidNodeIndex },   // ElectricalMeasurement::int16u
    { { AttributeTag(1536), "acVoltageMultiplier", ItemType::kDefault }, kInvalidNodeIndex },     // ElectricalMeasurement::int16u
    { { AttributeTag(1537), "acVoltageDivisor", ItemType::kDefault }, kInvalidNodeIndex },        // ElectricalMeasurement::int16u
    { { AttributeTag(1538), "acCurrentMultiplier", ItemType::kDefault }, kInvalidNodeIndex },     // ElectricalMeasurement::int16u
    { { AttributeTag(1539), "acCurrentDivisor", ItemType::kDefault }, kInvalidNodeIndex },        // ElectricalMeasurement::int16u
    { { AttributeTag(1540), "acPowerMultiplier", ItemType::kDefault }, kInvalidNodeIndex },       // ElectricalMeasurement::int16u
    { { AttributeTag(1541), "acPowerDivisor", ItemType::kDefault }, kInvalidNodeIndex },          // ElectricalMeasurement::int16u
    { { AttributeTag(1792), "overloadAlarmsMask", ItemType::kDefault }, kInvalidNodeIndex },      // ElectricalMeasurement::bitmap8
    { { AttributeTag(1793), "voltageOverload", ItemType::kDefault }, kInvalidNodeIndex },         // ElectricalMeasurement::int16s
    { { AttributeTag(1794), "currentOverload", ItemType::kDefault }, kInvalidNodeIndex },         // ElectricalMeasurement::int16s
    { { AttributeTag(2048), "acOverloadAlarmsMask", ItemType::kDefault }, kInvalidNodeIndex },    // ElectricalMeasurement::bitmap16
    { { AttributeTag(2049), "acVoltageOverload", ItemType::kDefault }, kInvalidNodeIndex },       // ElectricalMeasurement::int16s
    { { AttributeTag(2050), "acCurrentOverload", ItemType::kDefault }, kInvalidNodeIndex },       // ElectricalMeasurement::int16s
    { { AttributeTag(2051), "acActivePowerOverload", ItemType::kDefault }, kInvalidNodeIndex },   // ElectricalMeasurement::int16s
    { { AttributeTag(2052), "acReactivePowerOverload", ItemType::kDefault }, kInvalidNodeIndex }, // ElectricalMeasurement::int16s
    { { AttributeTag(2053), "averageRmsOverVoltage", ItemType::kDefault }, kInvalidNodeIndex },   // ElectricalMeasurement::int16s
    { { AttributeTag(2054), "averageRmsUnderVoltage", ItemType::kDefault }, kInvalidNodeIndex },  // ElectricalMeasurement::int16s
    { { AttributeTag(2055), "rmsExtremeOverVoltage", ItemType::kDefault }, kInvalidNodeIndex },   // ElectricalMeasurement::int16s
    { { AttributeTag(2056), "rmsExtremeUnderVoltage", ItemType::kDefault }, kInvalidNodeIndex },  // ElectricalMeasurement::int16s
    { { AttributeTag(2057), "rmsVoltageSag", ItemType::kDefault }, kInvalidNodeIndex },           // ElectricalMeasurement::int16s
    { { AttributeTag(2058), "rmsVoltageSwell", ItemType::kDefault }, kInvalidNodeIndex },         // ElectricalMeasurement::int16s
    { { AttributeTag(2305), "lineCurrentPhaseB", ItemType::kDefault }, kInvalidNodeIndex },       // ElectricalMeasurement::int16u
    { { AttributeTag(2306), "activeCurrentPhaseB", ItemType::kDefault }, kInvalidNodeIndex },     // ElectricalMeasurement::int16s
    { { AttributeTag(2307), "reactiveCurrentPhaseB", ItemType::kDefault }, kInvalidNodeIndex },   // ElectricalMeasurement::int16s
    { { AttributeTag(2309), "rmsVoltagePhaseB", ItemType::kDefault }, kInvalidNodeIndex },        // ElectricalMeasurement::int16u
    { { AttributeTag(2310), "rmsVoltageMinPhaseB", ItemType::kDefault }, kInvalidNodeIndex },     // ElectricalMeasurement::int16u
    { { AttributeTag(2311), "rmsVoltageMaxPhaseB", ItemType::kDefault }, kInvalidNodeIndex },     // ElectricalMeasurement::int16u
    { { AttributeTag(2312), "rmsCurrentPhaseB", ItemType::kDefault }, kInvalidNodeIndex },        // ElectricalMeasurement::int16u
    { { AttributeTag(2313), "rmsCurrentMinPhaseB", ItemType::kDefault }, kInvalidNodeIndex },     // ElectricalMeasurement::int16u
    { { AttributeTag(2314), "rmsCurrentMaxPhaseB", ItemType::kDefault }, kInvalidNodeIndex },     // ElectricalMeasurement::int16u
    { { AttributeTag(2315), "activePowerPhaseB", ItemType::kDefault }, kInvalidNodeIndex },       // ElectricalMeasurement::int16s
    { { AttributeTag(2316), "activePowerMinPhaseB", ItemType::kDefault }, kInvalidNodeIndex },    // ElectricalMeasurement::int16s
    { { AttributeTag(2317), "activePowerMaxPhaseB", ItemType::kDefault }, kInvalidNodeIndex },    // ElectricalMeasurement::int16s
    { { AttributeTag(2318), "reactivePowerPhaseB", ItemType::kDefault }, kInvalidNodeIndex },     // ElectricalMeasurement::int16s
    { { AttributeTag(2319), "apparentPowerPhaseB", ItemType::kDefault }, kInvalidNodeIndex },     // ElectricalMeasurement::int16u
    { { AttributeTag(2320), "powerFactorPhaseB", ItemType::kDefault }, kInvalidNodeIndex },       // ElectricalMeasurement::int8s
    { { AttributeTag(2321), "averageRmsVoltageMeasurementPeriodPhaseB", ItemType::kDefault },
      kInvalidNodeIndex }, // ElectricalMeasurement::int16u
    { { AttributeTag(2322), "averageRmsOverVoltageCounterPhaseB", ItemType::kDefault },
      kInvalidNodeIndex }, // ElectricalMeasurement::int16u
    { { AttributeTag(2323), "averageRmsUnderVoltageCounterPhaseB", ItemType::kDefault },
      kInvalidNodeIndex }, // ElectricalMeasurement::int16u
    { { AttributeTag(2324), "rmsExtremeOverVoltagePeriodPhaseB", ItemType::kDefault },
      kInvalidNodeIndex }, // ElectricalMeasurement::int16u
    { { AttributeTag(2325), "rmsExtremeUnderVoltagePeriodPhaseB", ItemType::kDefault },
      kInvalidNodeIndex },                                                                          // ElectricalMeasurement::int16u
    { { AttributeTag(2326), "rmsVoltageSagPeriodPhaseB", ItemType::kDefault }, kInvalidNodeIndex }, // ElectricalMeasurement::int16u
    { { AttributeTag(2327), "rmsVoltageSwellPeriodPhaseB", ItemType::kDefault },
      kInvalidNodeIndex },                                                                      // ElectricalMeasurement::int16u
    { { AttributeTag(2561), "lineCurrentPhaseC", ItemType::kDefault }, kInvalidNodeIndex },     // ElectricalMeasurement::int16u
    { { AttributeTag(2562), "activeCurrentPhaseC", ItemType::kDefault }, kInvalidNodeIndex },   // ElectricalMeasurement::int16s
    { { AttributeTag(2563), "reactiveCurrentPhaseC", ItemType::kDefault }, kInvalidNodeIndex }, // ElectricalMeasurement::int16s
    { { AttributeTag(2565), "rmsVoltagePhaseC", ItemType::kDefault }, kInvalidNodeIndex },      // ElectricalMeasurement::int16u
    { { AttributeTag(2566), "rmsVoltageMinPhaseC", ItemType::kDefault }, kInvalidNodeIndex },   // ElectricalMeasurement::int16u
    { { AttributeTag(2567), "rmsVoltageMaxPhaseC", ItemType::kDefault }, kInvalidNodeIndex },   // ElectricalMeasurement::int16u
    { { AttributeTag(2568), "rmsCurrentPhaseC", ItemType::kDefault }, kInvalidNodeIndex },      // ElectricalMeasurement::int16u
    { { AttributeTag(2569), "rmsCurrentMinPhaseC", ItemType::kDefault }, kInvalidNodeIndex },   // ElectricalMeasurement::int16u
    { { AttributeTag(2570), "rmsCurrentMaxPhaseC", ItemType::kDefault }, kInvalidNodeIndex },   // ElectricalMeasurement::int16u
    { { AttributeTag(2571), "activePowerPhaseC", ItemType::kDefault }, kInvalidNodeIndex },     // ElectricalMeasurement::int16s
    { { AttributeTag(2572), "activePowerMinPhaseC", ItemType::kDefault }, kInvalidNodeIndex },  // ElectricalMeasurement::int16s
    { { AttributeTag(2573), "activePowerMaxPhaseC", ItemType::kDefault }, kInvalidNodeIndex },  // ElectricalMeasurement::int16s
    { { AttributeTag(2574), "reactivePowerPhaseC", ItemType::kDefault }, kInvalidNodeIndex },   // ElectricalMeasurement::int16s
    { { AttributeTag(2575), "apparentPowerPhaseC", ItemType::kDefault }, kInvalidNodeIndex },   // ElectricalMeasurement::int16u
    { { AttributeTag(2576), "powerFactorPhaseC", ItemType::kDefault }, kInvalidNodeIndex },     // ElectricalMeasurement::int8s
    { { AttributeTag(2577), "averageRmsVoltageMeasurementPeriodPhaseC", ItemType::kDefault },
      kInvalidNodeIndex }, // ElectricalMeasurement::int16u
    { { AttributeTag(2578), "averageRmsOverVoltageCounterPhaseC", ItemType::kDefault },
      kInvalidNodeIndex }, // ElectricalMeasurement::int16u
    { { AttributeTag(2579), "averageRmsUnderVoltageCounterPhaseC", ItemType::kDefault },
      kInvalidNodeIndex }, // ElectricalMeasurement::int16u
    { { AttributeTag(2580), "rmsExtremeOverVoltagePeriodPhaseC", ItemType::kDefault },
      kInvalidNodeIndex }, // ElectricalMeasurement::int16u
    { { AttributeTag(2581), "rmsExtremeUnderVoltagePeriodPhaseC", ItemType::kDefault },
      kInvalidNodeIndex },                                                                          // ElectricalMeasurement::int16u
    { { AttributeTag(2582), "rmsVoltageSagPeriodPhaseC", ItemType::kDefault }, kInvalidNodeIndex }, // ElectricalMeasurement::int16u
    { { AttributeTag(2583), "rmsVoltageSwellPeriodPhaseC", ItemType::kDefault },
      kInvalidNodeIndex },                                                                   // ElectricalMeasurement::int16u
    { { AttributeTag(65528), "generatedCommandList", ItemType::kList }, kInvalidNodeIndex }, // ElectricalMeasurement::command_id[]
    { { AttributeTag(65529), "acceptedCommandList", ItemType::kList }, kInvalidNodeIndex },  // ElectricalMeasurement::command_id[]
    { { AttributeTag(65530), "eventList", ItemType::kList }, kInvalidNodeIndex },            // ElectricalMeasurement::event_id[]
    { { AttributeTag(65531), "attributeList", ItemType::kList }, kInvalidNodeIndex },        // ElectricalMeasurement::attrib_id[]
    { { AttributeTag(65532), "featureMap", ItemType::kDefault }, kInvalidNodeIndex },        // ElectricalMeasurement::bitmap32
    { { AttributeTag(65533), "clusterRevision", ItemType::kDefault }, kInvalidNodeIndex },   // ElectricalMeasurement::int16u
    { { CommandTag(1), "GetMeasurementProfileCommandRequest", ItemType::kDefault },
      533 }, // ElectricalMeasurement::GetMeasurementProfileCommand::GetMeasurementProfileCommandRequest
    { { CommandTag(0), "GetProfileInfoResponseCommand", ItemType::kDefault },
      531 }, // ElectricalMeasurement::GetProfileInfoResponseCommand
    { { CommandTag(1), "GetMeasurementProfileResponseCommand", ItemType::kDefault },
      532 }, // ElectricalMeasurement::GetMeasurementProfileResponseCommand
};

const Entry<ItemInfo> _ElectricalMeasurement_GetProfileInfoResponseCommand[] = {
    { { ContextTag(0), "profileCount", ItemType::kDefault }, kInvalidNodeIndex },          // ElectricalMeasurement::INT8U
    { { ContextTag(1), "profileIntervalPeriod", ItemType::kDefault }, kInvalidNodeIndex }, // ElectricalMeasurement::ENUM8
    { { ContextTag(2), "maxNumberOfIntervals", ItemType::kDefault }, kInvalidNodeIndex },  // ElectricalMeasurement::INT8U
    { { ContextTag(3), "listOfAttributes", ItemType::kList }, kInvalidNodeIndex },         // ElectricalMeasurement::INT16U[]
};

const Entry<ItemInfo> _ElectricalMeasurement_GetMeasurementProfileResponseCommand[] = {
    { { ContextTag(0), "startTime", ItemType::kDefault }, kInvalidNodeIndex },                  // ElectricalMeasurement::INT32U
    { { ContextTag(1), "status", ItemType::kDefault }, kInvalidNodeIndex },                     // ElectricalMeasurement::ENUM8
    { { ContextTag(2), "profileIntervalPeriod", ItemType::kDefault }, kInvalidNodeIndex },      // ElectricalMeasurement::ENUM8
    { { ContextTag(3), "numberOfIntervalsDelivered", ItemType::kDefault }, kInvalidNodeIndex }, // ElectricalMeasurement::INT8U
    { { ContextTag(4), "attributeId", ItemType::kDefault }, kInvalidNodeIndex },                // ElectricalMeasurement::INT16U
    { { ContextTag(5), "intervals", ItemType::kList }, kInvalidNodeIndex },                     // ElectricalMeasurement::INT8U[]
};

const Entry<ItemInfo> _ElectricalMeasurement_GetMeasurementProfileCommandRequest[] = {
    { { ContextTag(0), "attributeId", ItemType::kDefault }, kInvalidNodeIndex },       // ElectricalMeasurement::INT16U
    { { ContextTag(1), "startTime", ItemType::kDefault }, kInvalidNodeIndex },         // ElectricalMeasurement::INT32U
    { { ContextTag(2), "numberOfIntervals", ItemType::kDefault }, kInvalidNodeIndex }, // ElectricalMeasurement::ENUM8
};

const Entry<ItemInfo> _UnitTesting[] = {
    { { AttributeTag(0), "boolean", ItemType::kDefault }, kInvalidNodeIndex },          // UnitTesting::boolean
    { { AttributeTag(1), "bitmap8", ItemType::kBitmap }, kInvalidNodeIndex },           // UnitTesting::Bitmap8MaskMap
    { { AttributeTag(2), "bitmap16", ItemType::kBitmap }, kInvalidNodeIndex },          // UnitTesting::Bitmap16MaskMap
    { { AttributeTag(3), "bitmap32", ItemType::kBitmap }, kInvalidNodeIndex },          // UnitTesting::Bitmap32MaskMap
    { { AttributeTag(4), "bitmap64", ItemType::kBitmap }, kInvalidNodeIndex },          // UnitTesting::Bitmap64MaskMap
    { { AttributeTag(5), "int8u", ItemType::kDefault }, kInvalidNodeIndex },            // UnitTesting::int8u
    { { AttributeTag(6), "int16u", ItemType::kDefault }, kInvalidNodeIndex },           // UnitTesting::int16u
    { { AttributeTag(7), "int24u", ItemType::kDefault }, kInvalidNodeIndex },           // UnitTesting::int24u
    { { AttributeTag(8), "int32u", ItemType::kDefault }, kInvalidNodeIndex },           // UnitTesting::int32u
    { { AttributeTag(9), "int40u", ItemType::kDefault }, kInvalidNodeIndex },           // UnitTesting::int40u
    { { AttributeTag(10), "int48u", ItemType::kDefault }, kInvalidNodeIndex },          // UnitTesting::int48u
    { { AttributeTag(11), "int56u", ItemType::kDefault }, kInvalidNodeIndex },          // UnitTesting::int56u
    { { AttributeTag(12), "int64u", ItemType::kDefault }, kInvalidNodeIndex },          // UnitTesting::int64u
    { { AttributeTag(13), "int8s", ItemType::kDefault }, kInvalidNodeIndex },           // UnitTesting::int8s
    { { AttributeTag(14), "int16s", ItemType::kDefault }, kInvalidNodeIndex },          // UnitTesting::int16s
    { { AttributeTag(15), "int24s", ItemType::kDefault }, kInvalidNodeIndex },          // UnitTesting::int24s
    { { AttributeTag(16), "int32s", ItemType::kDefault }, kInvalidNodeIndex },          // UnitTesting::int32s
    { { AttributeTag(17), "int40s", ItemType::kDefault }, kInvalidNodeIndex },          // UnitTesting::int40s
    { { AttributeTag(18), "int48s", ItemType::kDefault }, kInvalidNodeIndex },          // UnitTesting::int48s
    { { AttributeTag(19), "int56s", ItemType::kDefault }, kInvalidNodeIndex },          // UnitTesting::int56s
    { { AttributeTag(20), "int64s", ItemType::kDefault }, kInvalidNodeIndex },          // UnitTesting::int64s
    { { AttributeTag(21), "enum8", ItemType::kDefault }, kInvalidNodeIndex },           // UnitTesting::enum8
    { { AttributeTag(22), "enum16", ItemType::kDefault }, kInvalidNodeIndex },          // UnitTesting::enum16
    { { AttributeTag(23), "floatSingle", ItemType::kDefault }, kInvalidNodeIndex },     // UnitTesting::single
    { { AttributeTag(24), "floatDouble", ItemType::kDefault }, kInvalidNodeIndex },     // UnitTesting::double
    { { AttributeTag(25), "octetString", ItemType::kDefault }, kInvalidNodeIndex },     // UnitTesting::octet_string
    { { AttributeTag(26), "listInt8u", ItemType::kList }, kInvalidNodeIndex },          // UnitTesting::INT8U[]
    { { AttributeTag(27), "listOctetString", ItemType::kList }, kInvalidNodeIndex },    // UnitTesting::OCTET_STRING[]
    { { AttributeTag(28), "listStructOctetString", ItemType::kList }, 574 },            // UnitTesting::TestListStructOctet[]
    { { AttributeTag(29), "longOctetString", ItemType::kDefault }, kInvalidNodeIndex }, // UnitTesting::long_octet_string
    { { AttributeTag(30), "charString", ItemType::kDefault }, kInvalidNodeIndex },      // UnitTesting::char_string
    { { AttributeTag(31), "longCharString", ItemType::kDefault }, kInvalidNodeIndex },  // UnitTesting::long_char_string
    { { AttributeTag(32), "epochUs", ItemType::kDefault }, kInvalidNodeIndex },         // UnitTesting::epoch_us
    { { AttributeTag(33), "epochS", ItemType::kDefault }, kInvalidNodeIndex },          // UnitTesting::epoch_s
    { { AttributeTag(34), "vendorId", ItemType::kDefault }, kInvalidNodeIndex },        // UnitTesting::vendor_id
    { { AttributeTag(35), "listNullablesAndOptionalsStruct", ItemType::kList }, 576 }, // UnitTesting::NullablesAndOptionalsStruct[]
    { { AttributeTag(36), "enumAttr", ItemType::kEnum }, kInvalidNodeIndex },          // UnitTesting::SimpleEnum
    { { AttributeTag(37), "structAttr", ItemType::kDefault }, 535 },                   // UnitTesting::SimpleStruct
    { { AttributeTag(38), "rangeRestrictedInt8u", ItemType::kDefault }, kInvalidNodeIndex },   // UnitTesting::int8u
    { { AttributeTag(39), "rangeRestrictedInt8s", ItemType::kDefault }, kInvalidNodeIndex },   // UnitTesting::int8s
    { { AttributeTag(40), "rangeRestrictedInt16u", ItemType::kDefault }, kInvalidNodeIndex },  // UnitTesting::int16u
    { { AttributeTag(41), "rangeRestrictedInt16s", ItemType::kDefault }, kInvalidNodeIndex },  // UnitTesting::int16s
    { { AttributeTag(42), "listLongOctetString", ItemType::kList }, kInvalidNodeIndex },       // UnitTesting::LONG_OCTET_STRING[]
    { { AttributeTag(43), "listFabricScoped", ItemType::kList }, 575 },                        // UnitTesting::TestFabricScoped[]
    { { AttributeTag(48), "timedWriteBoolean", ItemType::kDefault }, kInvalidNodeIndex },      // UnitTesting::boolean
    { { AttributeTag(49), "generalErrorBoolean", ItemType::kDefault }, kInvalidNodeIndex },    // UnitTesting::boolean
    { { AttributeTag(50), "clusterErrorBoolean", ItemType::kDefault }, kInvalidNodeIndex },    // UnitTesting::boolean
    { { AttributeTag(255), "unsupported", ItemType::kDefault }, kInvalidNodeIndex },           // UnitTesting::boolean
    { { AttributeTag(16384), "nullableBoolean", ItemType::kDefault }, kInvalidNodeIndex },     // UnitTesting::boolean
    { { AttributeTag(16385), "nullableBitmap8", ItemType::kBitmap }, kInvalidNodeIndex },      // UnitTesting::Bitmap8MaskMap
    { { AttributeTag(16386), "nullableBitmap16", ItemType::kBitmap }, kInvalidNodeIndex },     // UnitTesting::Bitmap16MaskMap
    { { AttributeTag(16387), "nullableBitmap32", ItemType::kBitmap }, kInvalidNodeIndex },     // UnitTesting::Bitmap32MaskMap
    { { AttributeTag(16388), "nullableBitmap64", ItemType::kBitmap }, kInvalidNodeIndex },     // UnitTesting::Bitmap64MaskMap
    { { AttributeTag(16389), "nullableInt8u", ItemType::kDefault }, kInvalidNodeIndex },       // UnitTesting::int8u
    { { AttributeTag(16390), "nullableInt16u", ItemType::kDefault }, kInvalidNodeIndex },      // UnitTesting::int16u
    { { AttributeTag(16391), "nullableInt24u", ItemType::kDefault }, kInvalidNodeIndex },      // UnitTesting::int24u
    { { AttributeTag(16392), "nullableInt32u", ItemType::kDefault }, kInvalidNodeIndex },      // UnitTesting::int32u
    { { AttributeTag(16393), "nullableInt40u", ItemType::kDefault }, kInvalidNodeIndex },      // UnitTesting::int40u
    { { AttributeTag(16394), "nullableInt48u", ItemType::kDefault }, kInvalidNodeIndex },      // UnitTesting::int48u
    { { AttributeTag(16395), "nullableInt56u", ItemType::kDefault }, kInvalidNodeIndex },      // UnitTesting::int56u
    { { AttributeTag(16396), "nullableInt64u", ItemType::kDefault }, kInvalidNodeIndex },      // UnitTesting::int64u
    { { AttributeTag(16397), "nullableInt8s", ItemType::kDefault }, kInvalidNodeIndex },       // UnitTesting::int8s
    { { AttributeTag(16398), "nullableInt16s", ItemType::kDefault }, kInvalidNodeIndex },      // UnitTesting::int16s
    { { AttributeTag(16399), "nullableInt24s", ItemType::kDefault }, kInvalidNodeIndex },      // UnitTesting::int24s
    { { AttributeTag(16400), "nullableInt32s", ItemType::kDefault }, kInvalidNodeIndex },      // UnitTesting::int32s
    { { AttributeTag(16401), "nullableInt40s", ItemType::kDefault }, kInvalidNodeIndex },      // UnitTesting::int40s
    { { AttributeTag(16402), "nullableInt48s", ItemType::kDefault }, kInvalidNodeIndex },      // UnitTesting::int48s
    { { AttributeTag(16403), "nullableInt56s", ItemType::kDefault }, kInvalidNodeIndex },      // UnitTesting::int56s
    { { AttributeTag(16404), "nullableInt64s", ItemType::kDefault }, kInvalidNodeIndex },      // UnitTesting::int64s
    { { AttributeTag(16405), "nullableEnum8", ItemType::kDefault }, kInvalidNodeIndex },       // UnitTesting::enum8
    { { AttributeTag(16406), "nullableEnum16", ItemType::kDefault }, kInvalidNodeIndex },      // UnitTesting::enum16
    { { AttributeTag(16407), "nullableFloatSingle", ItemType::kDefault }, kInvalidNodeIndex }, // UnitTesting::single
    { { AttributeTag(16408), "nullableFloatDouble", ItemType::kDefault }, kInvalidNodeIndex }, // UnitTesting::double
    { { AttributeTag(16409), "nullableOctetString", ItemType::kDefault }, kInvalidNodeIndex }, // UnitTesting::octet_string
    { { AttributeTag(16414), "nullableCharString", ItemType::kDefault }, kInvalidNodeIndex },  // UnitTesting::char_string
    { { AttributeTag(16420), "nullableEnumAttr", ItemType::kEnum }, kInvalidNodeIndex },       // UnitTesting::SimpleEnum
    { { AttributeTag(16421), "nullableStruct", ItemType::kDefault }, 535 },                    // UnitTesting::SimpleStruct
    { { AttributeTag(16422), "nullableRangeRestrictedInt8u", ItemType::kDefault }, kInvalidNodeIndex },  // UnitTesting::int8u
    { { AttributeTag(16423), "nullableRangeRestrictedInt8s", ItemType::kDefault }, kInvalidNodeIndex },  // UnitTesting::int8s
    { { AttributeTag(16424), "nullableRangeRestrictedInt16u", ItemType::kDefault }, kInvalidNodeIndex }, // UnitTesting::int16u
    { { AttributeTag(16425), "nullableRangeRestrictedInt16s", ItemType::kDefault }, kInvalidNodeIndex }, // UnitTesting::int16s
    { { AttributeTag(16426), "writeOnlyInt8u", ItemType::kDefault }, kInvalidNodeIndex },                // UnitTesting::int8u
    { { AttributeTag(65528), "generatedCommandList", ItemType::kList }, kInvalidNodeIndex }, // UnitTesting::command_id[]
    { { AttributeTag(65529), "acceptedCommandList", ItemType::kList }, kInvalidNodeIndex },  // UnitTesting::command_id[]
    { { AttributeTag(65530), "eventList", ItemType::kList }, kInvalidNodeIndex },            // UnitTesting::event_id[]
    { { AttributeTag(65531), "attributeList", ItemType::kList }, kInvalidNodeIndex },        // UnitTesting::attrib_id[]
    { { AttributeTag(65532), "featureMap", ItemType::kDefault }, kInvalidNodeIndex },        // UnitTesting::bitmap32
    { { AttributeTag(65533), "clusterRevision", ItemType::kDefault }, kInvalidNodeIndex },   // UnitTesting::int16u
    { { EventTag(1), "TestEvent", ItemType::kDefault }, 571 },                               // UnitTesting::TestEvent
    { { EventTag(2), "TestFabricScopedEvent", ItemType::kDefault }, 572 },                   // UnitTesting::TestFabricScopedEvent
    { { CommandTag(4), "TestAddArgumentsRequest", ItemType::kDefault },
      546 }, // UnitTesting::TestAddArguments::TestAddArgumentsRequest
    { { CommandTag(5), "TestSimpleArgumentRequestRequest", ItemType::kDefault },
      548 }, // UnitTesting::TestSimpleArgumentRequest::TestSimpleArgumentRequestRequest
    { { CommandTag(6), "TestStructArrayArgumentRequestRequest", ItemType::kDefault },
      550 }, // UnitTesting::TestStructArrayArgumentRequest::TestStructArrayArgumentRequestRequest
    { { CommandTag(7), "TestStructArgumentRequestRequest", ItemType::kDefault },
      552 }, // UnitTesting::TestStructArgumentRequest::TestStructArgumentRequestRequest
    { { CommandTag(8), "TestNestedStructArgumentRequestRequest", ItemType::kDefault },
      554 }, // UnitTesting::TestNestedStructArgumentRequest::TestNestedStructArgumentRequestRequest
    { { CommandTag(9), "TestListStructArgumentRequestRequest", ItemType::kDefault },
      556 }, // UnitTesting::TestListStructArgumentRequest::TestListStructArgumentRequestRequest
    { { CommandTag(10), "TestListInt8UArgumentRequestRequest", ItemType::kDefault },
      558 }, // UnitTesting::TestListInt8UArgumentRequest::TestListInt8UArgumentRequestRequest
    { { CommandTag(11), "TestNestedStructListArgumentRequestRequest", ItemType::kDefault },
      560 }, // UnitTesting::TestNestedStructListArgumentRequest::TestNestedStructListArgumentRequestRequest
    { { CommandTag(12), "TestListNestedStructListArgumentRequestRequest", ItemType::kDefault },
      562 }, // UnitTesting::TestListNestedStructListArgumentRequest::TestListNestedStructListArgumentRequestRequest
    { { CommandTag(13), "TestListInt8UReverseRequestRequest", ItemType::kDefault },
      563 }, // UnitTesting::TestListInt8UReverseRequest::TestListInt8UReverseRequestRequest
    { { CommandTag(14), "TestEnumsRequestRequest", ItemType::kDefault },
      564 }, // UnitTesting::TestEnumsRequest::TestEnumsRequestRequest
    { { CommandTag(15), "TestNullableOptionalRequestRequest", ItemType::kDefault },
      565 }, // UnitTesting::TestNullableOptionalRequest::TestNullableOptionalRequestRequest
    { { CommandTag(16), "TestComplexNullableOptionalRequestRequest", ItemType::kDefault },
      566 }, // UnitTesting::TestComplexNullableOptionalRequest::TestComplexNullableOptionalRequestRequest
    { { CommandTag(17), "SimpleStructEchoRequestRequest", ItemType::kDefault },
      567 }, // UnitTesting::SimpleStructEchoRequest::SimpleStructEchoRequestRequest
    { { CommandTag(19), "TestSimpleOptionalArgumentRequestRequest", ItemType::kDefault },
      568 }, // UnitTesting::TestSimpleOptionalArgumentRequest::TestSimpleOptionalArgumentRequestRequest
    { { CommandTag(20), "TestEmitTestEventRequestRequest", ItemType::kDefault },
      569 }, // UnitTesting::TestEmitTestEventRequest::TestEmitTestEventRequestRequest
    { { CommandTag(21), "TestEmitTestFabricScopedEventRequestRequest", ItemType::kDefault },
      570 }, // UnitTesting::TestEmitTestFabricScopedEventRequest::TestEmitTestFabricScopedEventRequestRequest
    { { CommandTag(0), "TestSpecificResponse", ItemType::kDefault }, 542 },       // UnitTesting::TestSpecificResponse
    { { CommandTag(1), "TestAddArgumentsResponse", ItemType::kDefault }, 543 },   // UnitTesting::TestAddArgumentsResponse
    { { CommandTag(2), "TestSimpleArgumentResponse", ItemType::kDefault }, 544 }, // UnitTesting::TestSimpleArgumentResponse
    { { CommandTag(3), "TestStructArrayArgumentResponse", ItemType::kDefault },
      545 },                                                                        // UnitTesting::TestStructArrayArgumentResponse
    { { CommandTag(4), "TestListInt8UReverseResponse", ItemType::kDefault }, 547 }, // UnitTesting::TestListInt8UReverseResponse
    { { CommandTag(5), "TestEnumsResponse", ItemType::kDefault }, 549 },            // UnitTesting::TestEnumsResponse
    { { CommandTag(6), "TestNullableOptionalResponse", ItemType::kDefault }, 551 }, // UnitTesting::TestNullableOptionalResponse
    { { CommandTag(7), "TestComplexNullableOptionalResponse", ItemType::kDefault },
      553 },                                                                // UnitTesting::TestComplexNullableOptionalResponse
    { { CommandTag(8), "BooleanResponse", ItemType::kDefault }, 555 },      // UnitTesting::BooleanResponse
    { { CommandTag(9), "SimpleStructResponse", ItemType::kDefault }, 557 }, // UnitTesting::SimpleStructResponse
    { { CommandTag(10), "TestEmitTestEventResponse", ItemType::kDefault }, 559 }, // UnitTesting::TestEmitTestEventResponse
    { { CommandTag(11), "TestEmitTestFabricScopedEventResponse", ItemType::kDefault },
      561 }, // UnitTesting::TestEmitTestFabricScopedEventResponse
};

const Entry<ItemInfo> _UnitTesting_SimpleStruct[] = {
    { { ContextTag(0), "a", ItemType::kDefault }, kInvalidNodeIndex }, // UnitTesting::int8u
    { { ContextTag(1), "b", ItemType::kDefault }, kInvalidNodeIndex }, // UnitTesting::boolean
    { { ContextTag(2), "c", ItemType::kEnum }, kInvalidNodeIndex },    // UnitTesting::SimpleEnum
    { { ContextTag(3), "d", ItemType::kDefault }, kInvalidNodeIndex }, // UnitTesting::octet_string
    { { ContextTag(4), "e", ItemType::kDefault }, kInvalidNodeIndex }, // UnitTesting::char_string
    { { ContextTag(5), "f", ItemType::kBitmap }, kInvalidNodeIndex },  // UnitTesting::SimpleBitmap
    { { ContextTag(6), "g", ItemType::kDefault }, kInvalidNodeIndex }, // UnitTesting::single
    { { ContextTag(7), "h", ItemType::kDefault }, kInvalidNodeIndex }, // UnitTesting::double
};

const Entry<ItemInfo> _UnitTesting_TestFabricScoped[] = {
    { { ContextTag(1), "fabricSensitiveInt8u", ItemType::kDefault }, kInvalidNodeIndex },                 // UnitTesting::int8u
    { { ContextTag(2), "optionalFabricSensitiveInt8u", ItemType::kDefault }, kInvalidNodeIndex },         // UnitTesting::int8u
    { { ContextTag(3), "nullableFabricSensitiveInt8u", ItemType::kDefault }, kInvalidNodeIndex },         // UnitTesting::int8u
    { { ContextTag(4), "nullableOptionalFabricSensitiveInt8u", ItemType::kDefault }, kInvalidNodeIndex }, // UnitTesting::int8u
    { { ContextTag(5), "fabricSensitiveCharString", ItemType::kDefault }, kInvalidNodeIndex }, // UnitTesting::char_string
    { { ContextTag(6), "fabricSensitiveStruct", ItemType::kDefault }, 535 },                   // UnitTesting::SimpleStruct
    { { ContextTag(7), "fabricSensitiveInt8uList", ItemType::kList }, kInvalidNodeIndex },     // UnitTesting::int8u[]
    { { ContextTag(254), "fabricIndex", ItemType::kDefault }, kInvalidNodeIndex },             // UnitTesting::fabric_idx
};

const Entry<ItemInfo> _UnitTesting_NullablesAndOptionalsStruct[] = {
    { { ContextTag(0), "nullableInt", ItemType::kDefault }, kInvalidNodeIndex },            // UnitTesting::int16u
    { { ContextTag(1), "optionalInt", ItemType::kDefault }, kInvalidNodeIndex },            // UnitTesting::int16u
    { { ContextTag(2), "nullableOptionalInt", ItemType::kDefault }, kInvalidNodeIndex },    // UnitTesting::int16u
    { { ContextTag(3), "nullableString", ItemType::kDefault }, kInvalidNodeIndex },         // UnitTesting::char_string
    { { ContextTag(4), "optionalString", ItemType::kDefault }, kInvalidNodeIndex },         // UnitTesting::char_string
    { { ContextTag(5), "nullableOptionalString", ItemType::kDefault }, kInvalidNodeIndex }, // UnitTesting::char_string
    { { ContextTag(6), "nullableStruct", ItemType::kDefault }, 535 },                       // UnitTesting::SimpleStruct
    { { ContextTag(7), "optionalStruct", ItemType::kDefault }, 535 },                       // UnitTesting::SimpleStruct
    { { ContextTag(8), "nullableOptionalStruct", ItemType::kDefault }, 535 },               // UnitTesting::SimpleStruct
    { { ContextTag(9), "nullableList", ItemType::kList }, kInvalidNodeIndex },              // UnitTesting::SimpleEnum[]
    { { ContextTag(10), "optionalList", ItemType::kList }, kInvalidNodeIndex },             // UnitTesting::SimpleEnum[]
    { { ContextTag(11), "nullableOptionalList", ItemType::kList }, kInvalidNodeIndex },     // UnitTesting::SimpleEnum[]
};

const Entry<ItemInfo> _UnitTesting_NestedStruct[] = {
    { { ContextTag(0), "a", ItemType::kDefault }, kInvalidNodeIndex }, // UnitTesting::int8u
    { { ContextTag(1), "b", ItemType::kDefault }, kInvalidNodeIndex }, // UnitTesting::boolean
    { { ContextTag(2), "c", ItemType::kDefault }, 535 },               // UnitTesting::SimpleStruct
};

const Entry<ItemInfo> _UnitTesting_NestedStructList[] = {
    { { ContextTag(0), "a", ItemType::kDefault }, kInvalidNodeIndex }, // UnitTesting::int8u
    { { ContextTag(1), "b", ItemType::kDefault }, kInvalidNodeIndex }, // UnitTesting::boolean
    { { ContextTag(2), "c", ItemType::kDefault }, 535 },               // UnitTesting::SimpleStruct
    { { ContextTag(3), "d", ItemType::kList }, 573 },                  // UnitTesting::SimpleStruct[]
    { { ContextTag(4), "e", ItemType::kList }, kInvalidNodeIndex },    // UnitTesting::int32u[]
    { { ContextTag(5), "f", ItemType::kList }, kInvalidNodeIndex },    // UnitTesting::octet_string[]
    { { ContextTag(6), "g", ItemType::kList }, kInvalidNodeIndex },    // UnitTesting::int8u[]
};

const Entry<ItemInfo> _UnitTesting_DoubleNestedStructList[] = {
    { { ContextTag(0), "a", ItemType::kList }, 577 }, // UnitTesting::NestedStructList[]
};

const Entry<ItemInfo> _UnitTesting_TestListStructOctet[] = {
    { { ContextTag(0), "member1", ItemType::kDefault }, kInvalidNodeIndex }, // UnitTesting::int64u
    { { ContextTag(1), "member2", ItemType::kDefault }, kInvalidNodeIndex }, // UnitTesting::octet_string
};

const Entry<ItemInfo> _UnitTesting_TestSpecificResponse[] = {
    { { ContextTag(0), "returnValue", ItemType::kDefault }, kInvalidNodeIndex }, // UnitTesting::INT8U
};

const Entry<ItemInfo> _UnitTesting_TestAddArgumentsResponse[] = {
    { { ContextTag(0), "returnValue", ItemType::kDefault }, kInvalidNodeIndex }, // UnitTesting::INT8U
};

const Entry<ItemInfo> _UnitTesting_TestSimpleArgumentResponse[] = {
    { { ContextTag(0), "returnValue", ItemType::kDefault }, kInvalidNodeIndex }, // UnitTesting::BOOLEAN
};

const Entry<ItemInfo> _UnitTesting_TestStructArrayArgumentResponse[] = {
    { { ContextTag(0), "arg1", ItemType::kList }, 577 },                  // UnitTesting::NestedStructList[]
    { { ContextTag(1), "arg2", ItemType::kList }, 573 },                  // UnitTesting::SimpleStruct[]
    { { ContextTag(2), "arg3", ItemType::kList }, kInvalidNodeIndex },    // UnitTesting::SimpleEnum[]
    { { ContextTag(3), "arg4", ItemType::kList }, kInvalidNodeIndex },    // UnitTesting::BOOLEAN[]
    { { ContextTag(4), "arg5", ItemType::kEnum }, kInvalidNodeIndex },    // UnitTesting::SimpleEnum
    { { ContextTag(5), "arg6", ItemType::kDefault }, kInvalidNodeIndex }, // UnitTesting::BOOLEAN
};

const Entry<ItemInfo> _UnitTesting_TestAddArgumentsRequest[] = {
    { { ContextTag(0), "arg1", ItemType::kDefault }, kInvalidNodeIndex }, // UnitTesting::INT8U
    { { ContextTag(1), "arg2", ItemType::kDefault }, kInvalidNodeIndex }, // UnitTesting::INT8U
};

const Entry<ItemInfo> _UnitTesting_TestListInt8UReverseResponse[] = {
    { { ContextTag(0), "arg1", ItemType::kList }, kInvalidNodeIndex }, // UnitTesting::INT8U[]
};

const Entry<ItemInfo> _UnitTesting_TestSimpleArgumentRequestRequest[] = {
    { { ContextTag(0), "arg1", ItemType::kDefault }, kInvalidNodeIndex }, // UnitTesting::BOOLEAN
};

const Entry<ItemInfo> _UnitTesting_TestEnumsResponse[] = {
    { { ContextTag(0), "arg1", ItemType::kDefault }, kInvalidNodeIndex }, // UnitTesting::vendor_id
    { { ContextTag(1), "arg2", ItemType::kEnum }, kInvalidNodeIndex },    // UnitTesting::SimpleEnum
};

const Entry<ItemInfo> _UnitTesting_TestStructArrayArgumentRequestRequest[] = {
    { { ContextTag(0), "arg1", ItemType::kList }, 577 },                  // UnitTesting::NestedStructList[]
    { { ContextTag(1), "arg2", ItemType::kList }, 573 },                  // UnitTesting::SimpleStruct[]
    { { ContextTag(2), "arg3", ItemType::kList }, kInvalidNodeIndex },    // UnitTesting::SimpleEnum[]
    { { ContextTag(3), "arg4", ItemType::kList }, kInvalidNodeIndex },    // UnitTesting::BOOLEAN[]
    { { ContextTag(4), "arg5", ItemType::kEnum }, kInvalidNodeIndex },    // UnitTesting::SimpleEnum
    { { ContextTag(5), "arg6", ItemType::kDefault }, kInvalidNodeIndex }, // UnitTesting::BOOLEAN
};

const Entry<ItemInfo> _UnitTesting_TestNullableOptionalResponse[] = {
    { { ContextTag(0), "wasPresent", ItemType::kDefault }, kInvalidNodeIndex },    // UnitTesting::BOOLEAN
    { { ContextTag(1), "wasNull", ItemType::kDefault }, kInvalidNodeIndex },       // UnitTesting::BOOLEAN
    { { ContextTag(2), "value", ItemType::kDefault }, kInvalidNodeIndex },         // UnitTesting::INT8U
    { { ContextTag(3), "originalValue", ItemType::kDefault }, kInvalidNodeIndex }, // UnitTesting::INT8U
};

const Entry<ItemInfo> _UnitTesting_TestStructArgumentRequestRequest[] = {
    { { ContextTag(0), "arg1", ItemType::kDefault }, 535 }, // UnitTesting::SimpleStruct
};

const Entry<ItemInfo> _UnitTesting_TestComplexNullableOptionalResponse[] = {
    { { ContextTag(0), "nullableIntWasNull", ItemType::kDefault }, kInvalidNodeIndex },                // UnitTesting::BOOLEAN
    { { ContextTag(1), "nullableIntValue", ItemType::kDefault }, kInvalidNodeIndex },                  // UnitTesting::INT16U
    { { ContextTag(2), "optionalIntWasPresent", ItemType::kDefault }, kInvalidNodeIndex },             // UnitTesting::BOOLEAN
    { { ContextTag(3), "optionalIntValue", ItemType::kDefault }, kInvalidNodeIndex },                  // UnitTesting::INT16U
    { { ContextTag(4), "nullableOptionalIntWasPresent", ItemType::kDefault }, kInvalidNodeIndex },     // UnitTesting::BOOLEAN
    { { ContextTag(5), "nullableOptionalIntWasNull", ItemType::kDefault }, kInvalidNodeIndex },        // UnitTesting::BOOLEAN
    { { ContextTag(6), "nullableOptionalIntValue", ItemType::kDefault }, kInvalidNodeIndex },          // UnitTesting::INT16U
    { { ContextTag(7), "nullableStringWasNull", ItemType::kDefault }, kInvalidNodeIndex },             // UnitTesting::BOOLEAN
    { { ContextTag(8), "nullableStringValue", ItemType::kDefault }, kInvalidNodeIndex },               // UnitTesting::CHAR_STRING
    { { ContextTag(9), "optionalStringWasPresent", ItemType::kDefault }, kInvalidNodeIndex },          // UnitTesting::BOOLEAN
    { { ContextTag(10), "optionalStringValue", ItemType::kDefault }, kInvalidNodeIndex },              // UnitTesting::CHAR_STRING
    { { ContextTag(11), "nullableOptionalStringWasPresent", ItemType::kDefault }, kInvalidNodeIndex }, // UnitTesting::BOOLEAN
    { { ContextTag(12), "nullableOptionalStringWasNull", ItemType::kDefault }, kInvalidNodeIndex },    // UnitTesting::BOOLEAN
    { { ContextTag(13), "nullableOptionalStringValue", ItemType::kDefault }, kInvalidNodeIndex },      // UnitTesting::CHAR_STRING
    { { ContextTag(14), "nullableStructWasNull", ItemType::kDefault }, kInvalidNodeIndex },            // UnitTesting::BOOLEAN
    { { ContextTag(15), "nullableStructValue", ItemType::kDefault }, 535 },                            // UnitTesting::SimpleStruct
    { { ContextTag(16), "optionalStructWasPresent", ItemType::kDefault }, kInvalidNodeIndex },         // UnitTesting::BOOLEAN
    { { ContextTag(17), "optionalStructValue", ItemType::kDefault }, 535 },                            // UnitTesting::SimpleStruct
    { { ContextTag(18), "nullableOptionalStructWasPresent", ItemType::kDefault }, kInvalidNodeIndex }, // UnitTesting::BOOLEAN
    { { ContextTag(19), "nullableOptionalStructWasNull", ItemType::kDefault }, kInvalidNodeIndex },    // UnitTesting::BOOLEAN
    { { ContextTag(20), "nullableOptionalStructValue", ItemType::kDefault }, 535 },                    // UnitTesting::SimpleStruct
    { { ContextTag(21), "nullableListWasNull", ItemType::kDefault }, kInvalidNodeIndex },              // UnitTesting::BOOLEAN
    { { ContextTag(22), "nullableListValue", ItemType::kList }, kInvalidNodeIndex },                   // UnitTesting::SimpleEnum[]
    { { ContextTag(23), "optionalListWasPresent", ItemType::kDefault }, kInvalidNodeIndex },           // UnitTesting::BOOLEAN
    { { ContextTag(24), "optionalListValue", ItemType::kList }, kInvalidNodeIndex },                   // UnitTesting::SimpleEnum[]
    { { ContextTag(25), "nullableOptionalListWasPresent", ItemType::kDefault }, kInvalidNodeIndex },   // UnitTesting::BOOLEAN
    { { ContextTag(26), "nullableOptionalListWasNull", ItemType::kDefault }, kInvalidNodeIndex },      // UnitTesting::BOOLEAN
    { { ContextTag(27), "nullableOptionalListValue", ItemType::kList }, kInvalidNodeIndex },           // UnitTesting::SimpleEnum[]
};

const Entry<ItemInfo> _UnitTesting_TestNestedStructArgumentRequestRequest[] = {
    { { ContextTag(0), "arg1", ItemType::kDefault }, 538 }, // UnitTesting::NestedStruct
};

const Entry<ItemInfo> _UnitTesting_BooleanResponse[] = {
    { { ContextTag(0), "value", ItemType::kDefault }, kInvalidNodeIndex }, // UnitTesting::BOOLEAN
};

const Entry<ItemInfo> _UnitTesting_TestListStructArgumentRequestRequest[] = {
    { { ContextTag(0), "arg1", ItemType::kList }, 573 }, // UnitTesting::SimpleStruct[]
};

const Entry<ItemInfo> _UnitTesting_SimpleStructResponse[] = {
    { { ContextTag(0), "arg1", ItemType::kDefault }, 535 }, // UnitTesting::SimpleStruct
};

const Entry<ItemInfo> _UnitTesting_TestListInt8UArgumentRequestRequest[] = {
    { { ContextTag(0), "arg1", ItemType::kList }, kInvalidNodeIndex }, // UnitTesting::INT8U[]
};

const Entry<ItemInfo> _UnitTesting_TestEmitTestEventResponse[] = {
    { { ContextTag(0), "value", ItemType::kDefault }, kInvalidNodeIndex }, // UnitTesting::INT64U
};

const Entry<ItemInfo> _UnitTesting_TestNestedStructListArgumentRequestRequest[] = {
    { { ContextTag(0), "arg1", ItemType::kDefault }, 539 }, // UnitTesting::NestedStructList
};

const Entry<ItemInfo> _UnitTesting_TestEmitTestFabricScopedEventResponse[] = {
    { { ContextTag(0), "value", ItemType::kDefault }, kInvalidNodeIndex }, // UnitTesting::INT64U
};

const Entry<ItemInfo> _UnitTesting_TestListNestedStructListArgumentRequestRequest[] = {
    { { ContextTag(0), "arg1", ItemType::kList }, 577 }, // UnitTesting::NestedStructList[]
};

const Entry<ItemInfo> _UnitTesting_TestListInt8UReverseRequestRequest[] = {
    { { ContextTag(0), "arg1", ItemType::kList }, kInvalidNodeIndex }, // UnitTesting::INT8U[]
};

const Entry<ItemInfo> _UnitTesting_TestEnumsRequestRequest[] = {
    { { ContextTag(0), "arg1", ItemType::kDefault }, kInvalidNodeIndex }, // UnitTesting::vendor_id
    { { ContextTag(1), "arg2", ItemType::kEnum }, kInvalidNodeIndex },    // UnitTesting::SimpleEnum
};

const Entry<ItemInfo> _UnitTesting_TestNullableOptionalRequestRequest[] = {
    { { ContextTag(0), "arg1", ItemType::kDefault }, kInvalidNodeIndex }, // UnitTesting::INT8U
};

const Entry<ItemInfo> _UnitTesting_TestComplexNullableOptionalRequestRequest[] = {
    { { ContextTag(0), "nullableInt", ItemType::kDefault }, kInvalidNodeIndex },            // UnitTesting::INT16U
    { { ContextTag(1), "optionalInt", ItemType::kDefault }, kInvalidNodeIndex },            // UnitTesting::INT16U
    { { ContextTag(2), "nullableOptionalInt", ItemType::kDefault }, kInvalidNodeIndex },    // UnitTesting::INT16U
    { { ContextTag(3), "nullableString", ItemType::kDefault }, kInvalidNodeIndex },         // UnitTesting::CHAR_STRING
    { { ContextTag(4), "optionalString", ItemType::kDefault }, kInvalidNodeIndex },         // UnitTesting::CHAR_STRING
    { { ContextTag(5), "nullableOptionalString", ItemType::kDefault }, kInvalidNodeIndex }, // UnitTesting::CHAR_STRING
    { { ContextTag(6), "nullableStruct", ItemType::kDefault }, 535 },                       // UnitTesting::SimpleStruct
    { { ContextTag(7), "optionalStruct", ItemType::kDefault }, 535 },                       // UnitTesting::SimpleStruct
    { { ContextTag(8), "nullableOptionalStruct", ItemType::kDefault }, 535 },               // UnitTesting::SimpleStruct
    { { ContextTag(9), "nullableList", ItemType::kList }, kInvalidNodeIndex },              // UnitTesting::SimpleEnum[]
    { { ContextTag(10), "optionalList", ItemType::kList }, kInvalidNodeIndex },             // UnitTesting::SimpleEnum[]
    { { ContextTag(11), "nullableOptionalList", ItemType::kList }, kInvalidNodeIndex },     // UnitTesting::SimpleEnum[]
};

const Entry<ItemInfo> _UnitTesting_SimpleStructEchoRequestRequest[] = {
    { { ContextTag(0), "arg1", ItemType::kDefault }, 535 }, // UnitTesting::SimpleStruct
};

const Entry<ItemInfo> _UnitTesting_TestSimpleOptionalArgumentRequestRequest[] = {
    { { ContextTag(0), "arg1", ItemType::kDefault }, kInvalidNodeIndex }, // UnitTesting::BOOLEAN
};

const Entry<ItemInfo> _UnitTesting_TestEmitTestEventRequestRequest[] = {
    { { ContextTag(0), "arg1", ItemType::kDefault }, kInvalidNodeIndex }, // UnitTesting::INT8U
    { { ContextTag(1), "arg2", ItemType::kEnum }, kInvalidNodeIndex },    // UnitTesting::SimpleEnum
    { { ContextTag(2), "arg3", ItemType::kDefault }, kInvalidNodeIndex }, // UnitTesting::BOOLEAN
};

const Entry<ItemInfo> _UnitTesting_TestEmitTestFabricScopedEventRequestRequest[] = {
    { { ContextTag(0), "arg1", ItemType::kDefault }, kInvalidNodeIndex }, // UnitTesting::INT8U
};

const Entry<ItemInfo> _UnitTesting_TestEvent[] = {
    { { ContextTag(1), "arg1", ItemType::kDefault }, kInvalidNodeIndex }, // UnitTesting::INT8U
    { { ContextTag(2), "arg2", ItemType::kEnum }, kInvalidNodeIndex },    // UnitTesting::SimpleEnum
    { { ContextTag(3), "arg3", ItemType::kDefault }, kInvalidNodeIndex }, // UnitTesting::BOOLEAN
    { { ContextTag(4), "arg4", ItemType::kDefault }, 535 },               // UnitTesting::SimpleStruct
    { { ContextTag(5), "arg5", ItemType::kList }, 573 },                  // UnitTesting::SimpleStruct[]
    { { ContextTag(6), "arg6", ItemType::kList }, kInvalidNodeIndex },    // UnitTesting::SimpleEnum[]
};

const Entry<ItemInfo> _UnitTesting_TestFabricScopedEvent[] = {
    { { ContextTag(254), "fabricIndex", ItemType::kDefault }, kInvalidNodeIndex }, // UnitTesting::fabric_idx
};

const Entry<ItemInfo> _UnitTesting_SimpleStruct_list_[] = {
    { { AnonymousTag(), "[]", ItemType::kDefault }, 535 }, // UnitTesting_SimpleStruct[]
};

const Entry<ItemInfo> _UnitTesting_TestListStructOctet_list_[] = {
    { { AnonymousTag(), "[]", ItemType::kDefault }, 541 }, // UnitTesting_TestListStructOctet[]
};

const Entry<ItemInfo> _UnitTesting_TestFabricScoped_list_[] = {
    { { AnonymousTag(), "[]", ItemType::kDefault }, 536 }, // UnitTesting_TestFabricScoped[]
};

const Entry<ItemInfo> _UnitTesting_NullablesAndOptionalsStruct_list_[] = {
    { { AnonymousTag(), "[]", ItemType::kDefault }, 537 }, // UnitTesting_NullablesAndOptionalsStruct[]
};

const Entry<ItemInfo> _UnitTesting_NestedStructList_list_[] = {
    { { AnonymousTag(), "[]", ItemType::kDefault }, 539 }, // UnitTesting_NestedStructList[]
};

const Entry<ItemInfo> _FaultInjection[] = {
    { { AttributeTag(65528), "generatedCommandList", ItemType::kList }, kInvalidNodeIndex }, // FaultInjection::command_id[]
    { { AttributeTag(65529), "acceptedCommandList", ItemType::kList }, kInvalidNodeIndex },  // FaultInjection::command_id[]
    { { AttributeTag(65530), "eventList", ItemType::kList }, kInvalidNodeIndex },            // FaultInjection::event_id[]
    { { AttributeTag(65531), "attributeList", ItemType::kList }, kInvalidNodeIndex },        // FaultInjection::attrib_id[]
    { { AttributeTag(65532), "featureMap", ItemType::kDefault }, kInvalidNodeIndex },        // FaultInjection::bitmap32
    { { AttributeTag(65533), "clusterRevision", ItemType::kDefault }, kInvalidNodeIndex },   // FaultInjection::int16u
    { { CommandTag(0), "FailAtFaultRequest", ItemType::kDefault }, 579 }, // FaultInjection::FailAtFault::FailAtFaultRequest
    { { CommandTag(1), "FailRandomlyAtFaultRequest", ItemType::kDefault },
      580 }, // FaultInjection::FailRandomlyAtFault::FailRandomlyAtFaultRequest
};

const Entry<ItemInfo> _FaultInjection_FailAtFaultRequest[] = {
    { { ContextTag(0), "type", ItemType::kEnum }, kInvalidNodeIndex },              // FaultInjection::FaultType
    { { ContextTag(1), "id", ItemType::kDefault }, kInvalidNodeIndex },             // FaultInjection::INT32U
    { { ContextTag(2), "numCallsToSkip", ItemType::kDefault }, kInvalidNodeIndex }, // FaultInjection::INT32U
    { { ContextTag(3), "numCallsToFail", ItemType::kDefault }, kInvalidNodeIndex }, // FaultInjection::INT32U
    { { ContextTag(4), "takeMutex", ItemType::kDefault }, kInvalidNodeIndex },      // FaultInjection::BOOLEAN
};

const Entry<ItemInfo> _FaultInjection_FailRandomlyAtFaultRequest[] = {
    { { ContextTag(0), "type", ItemType::kEnum }, kInvalidNodeIndex },          // FaultInjection::FaultType
    { { ContextTag(1), "id", ItemType::kDefault }, kInvalidNodeIndex },         // FaultInjection::INT32U
    { { ContextTag(2), "percentage", ItemType::kDefault }, kInvalidNodeIndex }, // FaultInjection::INT8U
};

const Entry<ItemInfo> _all_clusters[] = {
    { { ClusterTag(0x03), "Identify", ItemType::kDefault }, 1 },
    { { ClusterTag(0x04), "Groups", ItemType::kDefault }, 4 },
    { { ClusterTag(0x05), "Scenes", ItemType::kDefault }, 14 },
    { { ClusterTag(0x06), "OnOff", ItemType::kDefault }, 38 },
    { { ClusterTag(0x07), "OnOffSwitchConfiguration", ItemType::kDefault }, 41 },
    { { ClusterTag(0x08), "LevelControl", ItemType::kDefault }, 42 },
    { { ClusterTag(0x0F), "BinaryInputBasic", ItemType::kDefault }, 52 },
    { { ClusterTag(0x1C), "PulseWidthModulation", ItemType::kDefault }, 53 },
    { { ClusterTag(0x1D), "Descriptor", ItemType::kDefault }, 54 },
    { { ClusterTag(0x1E), "Binding", ItemType::kDefault }, 57 },
    { { ClusterTag(0x1F), "AccessControl", ItemType::kDefault }, 60 },
    { { ClusterTag(0x25), "Actions", ItemType::kDefault }, 69 },
    { { ClusterTag(0x28), "BasicInformation", ItemType::kDefault }, 88 },
    { { ClusterTag(0x29), "OtaSoftwareUpdateProvider", ItemType::kDefault }, 95 },
    { { ClusterTag(0x2A), "OtaSoftwareUpdateRequestor", ItemType::kDefault }, 101 },
    { { ClusterTag(0x2B), "LocalizationConfiguration", ItemType::kDefault }, 108 },
    { { ClusterTag(0x2C), "TimeFormatLocalization", ItemType::kDefault }, 109 },
    { { ClusterTag(0x2D), "UnitLocalization", ItemType::kDefault }, 110 },
    { { ClusterTag(0x2E), "PowerSourceConfiguration", ItemType::kDefault }, 111 },
    { { ClusterTag(0x2F), "PowerSource", ItemType::kDefault }, 112 },
    { { ClusterTag(0x30), "GeneralCommissioning", ItemType::kDefault }, 119 },
    { { ClusterTag(0x31), "NetworkCommissioning", ItemType::kDefault }, 126 },
    { { ClusterTag(0x32), "DiagnosticLogs", ItemType::kDefault }, 142 },
    { { ClusterTag(0x33), "GeneralDiagnostics", ItemType::kDefault }, 145 },
    { { ClusterTag(0x34), "SoftwareDiagnostics", ItemType::kDefault }, 153 },
    { { ClusterTag(0x35), "ThreadNetworkDiagnostics", ItemType::kDefault }, 157 },
    { { ClusterTag(0x36), "WiFiNetworkDiagnostics", ItemType::kDefault }, 166 },
    { { ClusterTag(0x37), "EthernetNetworkDiagnostics", ItemType::kDefault }, 170 },
    { { ClusterTag(0x38), "TimeSynchronization", ItemType::kDefault }, 171 },
    { { ClusterTag(0x39), "BridgedDeviceBasicInformation", ItemType::kDefault }, 189 },
    { { ClusterTag(0x3B), "Switch", ItemType::kDefault }, 195 },
    { { ClusterTag(0x3C), "AdministratorCommissioning", ItemType::kDefault }, 203 },
    { { ClusterTag(0x3E), "OperationalCredentials", ItemType::kDefault }, 206 },
    { { ClusterTag(0x3F), "GroupKeyManagement", ItemType::kDefault }, 223 },
    { { ClusterTag(0x40), "FixedLabel", ItemType::kDefault }, 234 },
    { { ClusterTag(0x41), "UserLabel", ItemType::kDefault }, 237 },
    { { ClusterTag(0x42), "ProxyConfiguration", ItemType::kDefault }, 240 },
    { { ClusterTag(0x43), "ProxyDiscovery", ItemType::kDefault }, 241 },
    { { ClusterTag(0x44), "ProxyValid", ItemType::kDefault }, 242 },
    { { ClusterTag(0x45), "BooleanState", ItemType::kDefault }, 243 },
    { { ClusterTag(0x46), "IcdManagement", ItemType::kDefault }, 245 },
    { { ClusterTag(0x50), "ModeSelect", ItemType::kDefault }, 251 },
    { { ClusterTag(0x51), "LaundryWasherMode", ItemType::kDefault }, 257 },
    { { ClusterTag(0x52), "RefrigeratorAndTemperatureControlledCabinetMode", ItemType::kDefault }, 264 },
    { { ClusterTag(0x53), "LaundryWasherControls", ItemType::kDefault }, 271 },
    { { ClusterTag(0x54), "RvcRunMode", ItemType::kDefault }, 272 },
    { { ClusterTag(0x55), "RvcCleanMode", ItemType::kDefault }, 279 },
    { { ClusterTag(0x56), "TemperatureControl", ItemType::kDefault }, 286 },
    { { ClusterTag(0x57), "RefrigeratorAlarm", ItemType::kDefault }, 288 },
    { { ClusterTag(0x59), "DishwasherMode", ItemType::kDefault }, 290 },
    { { ClusterTag(0x5B), "AirQuality", ItemType::kDefault }, 297 },
    { { ClusterTag(0x5C), "SmokeCoAlarm", ItemType::kDefault }, 298 },
    { { ClusterTag(0x5D), "DishwasherAlarm", ItemType::kDefault }, 310 },
    { { ClusterTag(0x60), "OperationalState", ItemType::kDefault }, 314 },
    { { ClusterTag(0x61), "RvcOperationalState", ItemType::kDefault }, 321 },
    { { ClusterTag(0x71), "HepaFilterMonitoring", ItemType::kDefault }, 328 },
    { { ClusterTag(0x72), "ActivatedCarbonFilterMonitoring", ItemType::kDefault }, 329 },
    { { ClusterTag(0x73), "CeramicFilterMonitoring", ItemType::kDefault }, 330 },
    { { ClusterTag(0x74), "ElectrostaticFilterMonitoring", ItemType::kDefault }, 331 },
    { { ClusterTag(0x75), "UvFilterMonitoring", ItemType::kDefault }, 332 },
    { { ClusterTag(0x76), "IonizingFilterMonitoring", ItemType::kDefault }, 333 },
    { { ClusterTag(0x77), "ZeoliteFilterMonitoring", ItemType::kDefault }, 334 },
    { { ClusterTag(0x78), "OzoneFilterMonitoring", ItemType::kDefault }, 335 },
    { { ClusterTag(0x79), "WaterTankMonitoring", ItemType::kDefault }, 336 },
    { { ClusterTag(0x7A), "FuelTankMonitoring", ItemType::kDefault }, 337 },
    { { ClusterTag(0x7B), "InkCartridgeMonitoring", ItemType::kDefault }, 338 },
    { { ClusterTag(0x7C), "TonerCartridgeMonitoring", ItemType::kDefault }, 339 },
    { { ClusterTag(0x101), "DoorLock", ItemType::kDefault }, 340 },
    { { ClusterTag(0x102), "WindowCovering", ItemType::kDefault }, 373 },
    { { ClusterTag(0x103), "BarrierControl", ItemType::kDefault }, 378 },
    { { ClusterTag(0x200), "PumpConfigurationAndControl", ItemType::kDefault }, 380 },
    { { ClusterTag(0x201), "Thermostat", ItemType::kDefault }, 398 },
    { { ClusterTag(0x202), "FanControl", ItemType::kDefault }, 405 },
    { { ClusterTag(0x204), "ThermostatUserInterfaceConfiguration", ItemType::kDefault }, 407 },
    { { ClusterTag(0x300), "ColorControl", ItemType::kDefault }, 408 },
    { { ClusterTag(0x301), "BallastConfiguration", ItemType::kDefault }, 428 },
    { { ClusterTag(0x400), "IlluminanceMeasurement", ItemType::kDefault }, 429 },
    { { ClusterTag(0x402), "TemperatureMeasurement", ItemType::kDefault }, 430 },
    { { ClusterTag(0x403), "PressureMeasurement", ItemType::kDefault }, 431 },
    { { ClusterTag(0x404), "FlowMeasurement", ItemType::kDefault }, 432 },
    { { ClusterTag(0x405), "RelativeHumidityMeasurement", ItemType::kDefault }, 433 },
    { { ClusterTag(0x406), "OccupancySensing", ItemType::kDefault }, 434 },
    { { ClusterTag(0x40C), "CarbonMonoxideConcentrationMeasurement", ItemType::kDefault }, 435 },
    { { ClusterTag(0x40D), "CarbonDioxideConcentrationMeasurement", ItemType::kDefault }, 436 },
    { { ClusterTag(0x40E), "EthyleneConcentrationMeasurement", ItemType::kDefault }, 437 },
    { { ClusterTag(0x40F), "EthyleneOxideConcentrationMeasurement", ItemType::kDefault }, 438 },
    { { ClusterTag(0x410), "HydrogenConcentrationMeasurement", ItemType::kDefault }, 439 },
    { { ClusterTag(0x411), "HydrogenSulfideConcentrationMeasurement", ItemType::kDefault }, 440 },
    { { ClusterTag(0x412), "NitricOxideConcentrationMeasurement", ItemType::kDefault }, 441 },
    { { ClusterTag(0x413), "NitrogenDioxideConcentrationMeasurement", ItemType::kDefault }, 442 },
    { { ClusterTag(0x414), "OxygenConcentrationMeasurement", ItemType::kDefault }, 443 },
    { { ClusterTag(0x415), "OzoneConcentrationMeasurement", ItemType::kDefault }, 444 },
    { { ClusterTag(0x416), "SulfurDioxideConcentrationMeasurement", ItemType::kDefault }, 445 },
    { { ClusterTag(0x417), "DissolvedOxygenConcentrationMeasurement", ItemType::kDefault }, 446 },
    { { ClusterTag(0x418), "BromateConcentrationMeasurement", ItemType::kDefault }, 447 },
    { { ClusterTag(0x419), "ChloraminesConcentrationMeasurement", ItemType::kDefault }, 448 },
    { { ClusterTag(0x41A), "ChlorineConcentrationMeasurement", ItemType::kDefault }, 449 },
    { { ClusterTag(0x41B), "FecalColiformEColiConcentrationMeasurement", ItemType::kDefault }, 450 },
    { { ClusterTag(0x41C), "FluorideConcentrationMeasurement", ItemType::kDefault }, 451 },
    { { ClusterTag(0x41D), "HaloaceticAcidsConcentrationMeasurement", ItemType::kDefault }, 452 },
    { { ClusterTag(0x41E), "TotalTrihalomethanesConcentrationMeasurement", ItemType::kDefault }, 453 },
    { { ClusterTag(0x41F), "TotalColiformBacteriaConcentrationMeasurement", ItemType::kDefault }, 454 },
    { { ClusterTag(0x420), "TurbidityConcentrationMeasurement", ItemType::kDefault }, 455 },
    { { ClusterTag(0x421), "CopperConcentrationMeasurement", ItemType::kDefault }, 456 },
    { { ClusterTag(0x422), "LeadConcentrationMeasurement", ItemType::kDefault }, 457 },
    { { ClusterTag(0x423), "ManganeseConcentrationMeasurement", ItemType::kDefault }, 458 },
    { { ClusterTag(0x424), "SulfateConcentrationMeasurement", ItemType::kDefault }, 459 },
    { { ClusterTag(0x425), "BromodichloromethaneConcentrationMeasurement", ItemType::kDefault }, 460 },
    { { ClusterTag(0x426), "BromoformConcentrationMeasurement", ItemType::kDefault }, 461 },
    { { ClusterTag(0x427), "ChlorodibromomethaneConcentrationMeasurement", ItemType::kDefault }, 462 },
    { { ClusterTag(0x428), "ChloroformConcentrationMeasurement", ItemType::kDefault }, 463 },
    { { ClusterTag(0x429), "SodiumConcentrationMeasurement", ItemType::kDefault }, 464 },
    { { ClusterTag(0x42A), "Pm25ConcentrationMeasurement", ItemType::kDefault }, 465 },
    { { ClusterTag(0x42B), "FormaldehydeConcentrationMeasurement", ItemType::kDefault }, 466 },
    { { ClusterTag(0x42C), "Pm1ConcentrationMeasurement", ItemType::kDefault }, 467 },
    { { ClusterTag(0x42D), "Pm10ConcentrationMeasurement", ItemType::kDefault }, 468 },
    { { ClusterTag(0x42E), "TotalVolatileOrganicCompoundsConcentrationMeasurement", ItemType::kDefault }, 469 },
    { { ClusterTag(0x42F), "RadonConcentrationMeasurement", ItemType::kDefault }, 470 },
    { { ClusterTag(0x503), "WakeOnLan", ItemType::kDefault }, 471 },
    { { ClusterTag(0x504), "Channel", ItemType::kDefault }, 472 },
    { { ClusterTag(0x505), "TargetNavigator", ItemType::kDefault }, 480 },
    { { ClusterTag(0x506), "MediaPlayback", ItemType::kDefault }, 485 },
    { { ClusterTag(0x507), "MediaInput", ItemType::kDefault }, 491 },
    { { ClusterTag(0x508), "LowPower", ItemType::kDefault }, 496 },
    { { ClusterTag(0x509), "KeypadInput", ItemType::kDefault }, 497 },
    { { ClusterTag(0x50A), "ContentLauncher", ItemType::kDefault }, 500 },
    { { ClusterTag(0x50B), "AudioOutput", ItemType::kDefault }, 512 },
    { { ClusterTag(0x50C), "ApplicationLauncher", ItemType::kDefault }, 517 },
    { { ClusterTag(0x50D), "ApplicationBasic", ItemType::kDefault }, 524 },
    { { ClusterTag(0x50E), "AccountLogin", ItemType::kDefault }, 526 },
    { { ClusterTag(0xB04), "ElectricalMeasurement", ItemType::kDefault }, 530 },
    { { ClusterTag(0xFFF1FC05), "UnitTesting", ItemType::kDefault }, 534 },
    { { ClusterTag(0xFFF1FC06), "FaultInjection", ItemType::kDefault }, 578 },
};

} // namespace

#define _ENTRY(n)                                                                                                                  \
    {                                                                                                                              \
        sizeof(n) / sizeof(n[0]), n                                                                                                \
    }

const std::array<const Node<ItemInfo>, 580 + 1> clusters_meta = { {
    _ENTRY(_all_clusters),
    _ENTRY(_Identify),                                                               // 1
    _ENTRY(_Identify_IdentifyRequest),                                               // 2
    _ENTRY(_Identify_TriggerEffectRequest),                                          // 3
    _ENTRY(_Groups),                                                                 // 4
    _ENTRY(_Groups_AddGroupRequest),                                                 // 5
    _ENTRY(_Groups_AddGroupResponse),                                                // 6
    _ENTRY(_Groups_ViewGroupRequest),                                                // 7
    _ENTRY(_Groups_ViewGroupResponse),                                               // 8
    _ENTRY(_Groups_GetGroupMembershipRequest),                                       // 9
    _ENTRY(_Groups_GetGroupMembershipResponse),                                      // 10
    _ENTRY(_Groups_RemoveGroupRequest),                                              // 11
    _ENTRY(_Groups_RemoveGroupResponse),                                             // 12
    _ENTRY(_Groups_AddGroupIfIdentifyingRequest),                                    // 13
    _ENTRY(_Scenes),                                                                 // 14
    _ENTRY(_Scenes_AttributeValuePair),                                              // 15
    _ENTRY(_Scenes_ExtensionFieldSet),                                               // 16
    _ENTRY(_Scenes_AddSceneRequest),                                                 // 17
    _ENTRY(_Scenes_AddSceneResponse),                                                // 18
    _ENTRY(_Scenes_ViewSceneRequest),                                                // 19
    _ENTRY(_Scenes_ViewSceneResponse),                                               // 20
    _ENTRY(_Scenes_RemoveSceneRequest),                                              // 21
    _ENTRY(_Scenes_RemoveSceneResponse),                                             // 22
    _ENTRY(_Scenes_RemoveAllScenesRequest),                                          // 23
    _ENTRY(_Scenes_RemoveAllScenesResponse),                                         // 24
    _ENTRY(_Scenes_StoreSceneRequest),                                               // 25
    _ENTRY(_Scenes_StoreSceneResponse),                                              // 26
    _ENTRY(_Scenes_RecallSceneRequest),                                              // 27
    _ENTRY(_Scenes_GetSceneMembershipRequest),                                       // 28
    _ENTRY(_Scenes_GetSceneMembershipResponse),                                      // 29
    _ENTRY(_Scenes_EnhancedAddSceneRequest),                                         // 30
    _ENTRY(_Scenes_EnhancedAddSceneResponse),                                        // 31
    _ENTRY(_Scenes_EnhancedViewSceneRequest),                                        // 32
    _ENTRY(_Scenes_EnhancedViewSceneResponse),                                       // 33
    _ENTRY(_Scenes_CopySceneRequest),                                                // 34
    _ENTRY(_Scenes_CopySceneResponse),                                               // 35
    _ENTRY(_Scenes_ExtensionFieldSet_list_),                                         // 36
    _ENTRY(_Scenes_AttributeValuePair_list_),                                        // 37
    _ENTRY(_OnOff),                                                                  // 38
    _ENTRY(_OnOff_OffWithEffectRequest),                                             // 39
    _ENTRY(_OnOff_OnWithTimedOffRequest),                                            // 40
    _ENTRY(_OnOffSwitchConfiguration),                                               // 41
    _ENTRY(_LevelControl),                                                           // 42
    _ENTRY(_LevelControl_MoveToLevelRequest),                                        // 43
    _ENTRY(_LevelControl_MoveRequest),                                               // 44
    _ENTRY(_LevelControl_StepRequest),                                               // 45
    _ENTRY(_LevelControl_StopRequest),                                               // 46
    _ENTRY(_LevelControl_MoveToLevelWithOnOffRequest),                               // 47
    _ENTRY(_LevelControl_MoveWithOnOffRequest),                                      // 48
    _ENTRY(_LevelControl_StepWithOnOffRequest),                                      // 49
    _ENTRY(_LevelControl_StopWithOnOffRequest),                                      // 50
    _ENTRY(_LevelControl_MoveToClosestFrequencyRequest),                             // 51
    _ENTRY(_BinaryInputBasic),                                                       // 52
    _ENTRY(_PulseWidthModulation),                                                   // 53
    _ENTRY(_Descriptor),                                                             // 54
    _ENTRY(_Descriptor_DeviceTypeStruct),                                            // 55
    _ENTRY(_Descriptor_DeviceTypeStruct_list_),                                      // 56
    _ENTRY(_Binding),                                                                // 57
    _ENTRY(_Binding_TargetStruct),                                                   // 58
    _ENTRY(_Binding_TargetStruct_list_),                                             // 59
    _ENTRY(_AccessControl),                                                          // 60
    _ENTRY(_AccessControl_AccessControlTargetStruct),                                // 61
    _ENTRY(_AccessControl_AccessControlEntryStruct),                                 // 62
    _ENTRY(_AccessControl_AccessControlExtensionStruct),                             // 63
    _ENTRY(_AccessControl_AccessControlEntryChanged),                                // 64
    _ENTRY(_AccessControl_AccessControlExtensionChanged),                            // 65
    _ENTRY(_AccessControl_AccessControlExtensionStruct_list_),                       // 66
    _ENTRY(_AccessControl_AccessControlTargetStruct_list_),                          // 67
    _ENTRY(_AccessControl_AccessControlEntryStruct_list_),                           // 68
    _ENTRY(_Actions),                                                                // 69
    _ENTRY(_Actions_ActionStruct),                                                   // 70
    _ENTRY(_Actions_EndpointListStruct),                                             // 71
    _ENTRY(_Actions_InstantActionRequest),                                           // 72
    _ENTRY(_Actions_InstantActionWithTransitionRequest),                             // 73
    _ENTRY(_Actions_StartActionRequest),                                             // 74
    _ENTRY(_Actions_StartActionWithDurationRequest),                                 // 75
    _ENTRY(_Actions_StopActionRequest),                                              // 76
    _ENTRY(_Actions_PauseActionRequest),                                             // 77
    _ENTRY(_Actions_PauseActionWithDurationRequest),                                 // 78
    _ENTRY(_Actions_ResumeActionRequest),                                            // 79
    _ENTRY(_Actions_EnableActionRequest),                                            // 80
    _ENTRY(_Actions_EnableActionWithDurationRequest),                                // 81
    _ENTRY(_Actions_DisableActionRequest),                                           // 82
    _ENTRY(_Actions_DisableActionWithDurationRequest),                               // 83
    _ENTRY(_Actions_StateChanged),                                                   // 84
    _ENTRY(_Actions_ActionFailed),                                                   // 85
    _ENTRY(_Actions_ActionStruct_list_),                                             // 86
    _ENTRY(_Actions_EndpointListStruct_list_),                                       // 87
    _ENTRY(_BasicInformation),                                                       // 88
    _ENTRY(_BasicInformation_CapabilityMinimaStruct),                                // 89
    _ENTRY(_BasicInformation_ProductAppearanceStruct),                               // 90
    _ENTRY(_BasicInformation_StartUp),                                               // 91
    _ENTRY(_BasicInformation_ShutDown),                                              // 92
    _ENTRY(_BasicInformation_Leave),                                                 // 93
    _ENTRY(_BasicInformation_ReachableChanged),                                      // 94
    _ENTRY(_OtaSoftwareUpdateProvider),                                              // 95
    _ENTRY(_OtaSoftwareUpdateProvider_QueryImageRequest),                            // 96
    _ENTRY(_OtaSoftwareUpdateProvider_QueryImageResponse),                           // 97
    _ENTRY(_OtaSoftwareUpdateProvider_ApplyUpdateRequestRequest),                    // 98
    _ENTRY(_OtaSoftwareUpdateProvider_ApplyUpdateResponse),                          // 99
    _ENTRY(_OtaSoftwareUpdateProvider_NotifyUpdateAppliedRequest),                   // 100
    _ENTRY(_OtaSoftwareUpdateRequestor),                                             // 101
    _ENTRY(_OtaSoftwareUpdateRequestor_ProviderLocation),                            // 102
    _ENTRY(_OtaSoftwareUpdateRequestor_AnnounceOTAProviderRequest),                  // 103
    _ENTRY(_OtaSoftwareUpdateRequestor_StateTransition),                             // 104
    _ENTRY(_OtaSoftwareUpdateRequestor_VersionApplied),                              // 105
    _ENTRY(_OtaSoftwareUpdateRequestor_DownloadError),                               // 106
    _ENTRY(_OtaSoftwareUpdateRequestor_ProviderLocation_list_),                      // 107
    _ENTRY(_LocalizationConfiguration),                                              // 108
    _ENTRY(_TimeFormatLocalization),                                                 // 109
    _ENTRY(_UnitLocalization),                                                       // 110
    _ENTRY(_PowerSourceConfiguration),                                               // 111
    _ENTRY(_PowerSource),                                                            // 112
    _ENTRY(_PowerSource_BatChargeFaultChangeType),                                   // 113
    _ENTRY(_PowerSource_BatFaultChangeType),                                         // 114
    _ENTRY(_PowerSource_WiredFaultChangeType),                                       // 115
    _ENTRY(_PowerSource_WiredFaultChange),                                           // 116
    _ENTRY(_PowerSource_BatFaultChange),                                             // 117
    _ENTRY(_PowerSource_BatChargeFaultChange),                                       // 118
    _ENTRY(_GeneralCommissioning),                                                   // 119
    _ENTRY(_GeneralCommissioning_BasicCommissioningInfo),                            // 120
    _ENTRY(_GeneralCommissioning_ArmFailSafeRequest),                                // 121
    _ENTRY(_GeneralCommissioning_ArmFailSafeResponse),                               // 122
    _ENTRY(_GeneralCommissioning_SetRegulatoryConfigRequest),                        // 123
    _ENTRY(_GeneralCommissioning_SetRegulatoryConfigResponse),                       // 124
    _ENTRY(_GeneralCommissioning_CommissioningCompleteResponse),                     // 125
    _ENTRY(_NetworkCommissioning),                                                   // 126
    _ENTRY(_NetworkCommissioning_NetworkInfoStruct),                                 // 127
    _ENTRY(_NetworkCommissioning_ThreadInterfaceScanResultStruct),                   // 128
    _ENTRY(_NetworkCommissioning_WiFiInterfaceScanResultStruct),                     // 129
    _ENTRY(_NetworkCommissioning_ScanNetworksRequest),                               // 130
    _ENTRY(_NetworkCommissioning_ScanNetworksResponse),                              // 131
    _ENTRY(_NetworkCommissioning_AddOrUpdateWiFiNetworkRequest),                     // 132
    _ENTRY(_NetworkCommissioning_AddOrUpdateThreadNetworkRequest),                   // 133
    _ENTRY(_NetworkCommissioning_RemoveNetworkRequest),                              // 134
    _ENTRY(_NetworkCommissioning_NetworkConfigResponse),                             // 135
    _ENTRY(_NetworkCommissioning_ConnectNetworkRequest),                             // 136
    _ENTRY(_NetworkCommissioning_ConnectNetworkResponse),                            // 137
    _ENTRY(_NetworkCommissioning_ReorderNetworkRequest),                             // 138
    _ENTRY(_NetworkCommissioning_WiFiInterfaceScanResultStruct_list_),               // 139
    _ENTRY(_NetworkCommissioning_NetworkInfoStruct_list_),                           // 140
    _ENTRY(_NetworkCommissioning_ThreadInterfaceScanResultStruct_list_),             // 141
    _ENTRY(_DiagnosticLogs),                                                         // 142
    _ENTRY(_DiagnosticLogs_RetrieveLogsRequestRequest),                              // 143
    _ENTRY(_DiagnosticLogs_RetrieveLogsResponse),                                    // 144
    _ENTRY(_GeneralDiagnostics),                                                     // 145
    _ENTRY(_GeneralDiagnostics_NetworkInterface),                                    // 146
    _ENTRY(_GeneralDiagnostics_TestEventTriggerRequest),                             // 147
    _ENTRY(_GeneralDiagnostics_HardwareFaultChange),                                 // 148
    _ENTRY(_GeneralDiagnostics_RadioFaultChange),                                    // 149
    _ENTRY(_GeneralDiagnostics_NetworkFaultChange),                                  // 150
    _ENTRY(_GeneralDiagnostics_BootReason),                                          // 151
    _ENTRY(_GeneralDiagnostics_NetworkInterface_list_),                              // 152
    _ENTRY(_SoftwareDiagnostics),                                                    // 153
    _ENTRY(_SoftwareDiagnostics_ThreadMetricsStruct),                                // 154
    _ENTRY(_SoftwareDiagnostics_SoftwareFault),                                      // 155
    _ENTRY(_SoftwareDiagnostics_ThreadMetricsStruct_list_),                          // 156
    _ENTRY(_ThreadNetworkDiagnostics),                                               // 157
    _ENTRY(_ThreadNetworkDiagnostics_NeighborTableStruct),                           // 158
    _ENTRY(_ThreadNetworkDiagnostics_OperationalDatasetComponents),                  // 159
    _ENTRY(_ThreadNetworkDiagnostics_RouteTableStruct),                              // 160
    _ENTRY(_ThreadNetworkDiagnostics_SecurityPolicy),                                // 161
    _ENTRY(_ThreadNetworkDiagnostics_ConnectionStatus),                              // 162
    _ENTRY(_ThreadNetworkDiagnostics_NetworkFaultChange),                            // 163
    _ENTRY(_ThreadNetworkDiagnostics_NeighborTableStruct_list_),                     // 164
    _ENTRY(_ThreadNetworkDiagnostics_RouteTableStruct_list_),                        // 165
    _ENTRY(_WiFiNetworkDiagnostics),                                                 // 166
    _ENTRY(_WiFiNetworkDiagnostics_Disconnection),                                   // 167
    _ENTRY(_WiFiNetworkDiagnostics_AssociationFailure),                              // 168
    _ENTRY(_WiFiNetworkDiagnostics_ConnectionStatus),                                // 169
    _ENTRY(_EthernetNetworkDiagnostics),                                             // 170
    _ENTRY(_TimeSynchronization),                                                    // 171
    _ENTRY(_TimeSynchronization_DSTOffsetStruct),                                    // 172
    _ENTRY(_TimeSynchronization_FabricScopedTrustedTimeSourceStruct),                // 173
    _ENTRY(_TimeSynchronization_TimeZoneStruct),                                     // 174
    _ENTRY(_TimeSynchronization_TrustedTimeSourceStruct),                            // 175
    _ENTRY(_TimeSynchronization_SetUTCTimeRequest),                                  // 176
    _ENTRY(_TimeSynchronization_SetTrustedTimeSourceRequest),                        // 177
    _ENTRY(_TimeSynchronization_SetTimeZoneRequest),                                 // 178
    _ENTRY(_TimeSynchronization_SetTimeZoneResponse),                                // 179
    _ENTRY(_TimeSynchronization_SetDSTOffsetRequest),                                // 180
    _ENTRY(_TimeSynchronization_SetDefaultNTPRequest),                               // 181
    _ENTRY(_TimeSynchronization_DSTTableEmpty),                                      // 182
    _ENTRY(_TimeSynchronization_DSTStatus),                                          // 183
    _ENTRY(_TimeSynchronization_TimeZoneStatus),                                     // 184
    _ENTRY(_TimeSynchronization_TimeFailure),                                        // 185
    _ENTRY(_TimeSynchronization_MissingTrustedTimeSource),                           // 186
    _ENTRY(_TimeSynchronization_DSTOffsetStruct_list_),                              // 187
    _ENTRY(_TimeSynchronization_TimeZoneStruct_list_),                               // 188
    _ENTRY(_BridgedDeviceBasicInformation),                                          // 189
    _ENTRY(_BridgedDeviceBasicInformation_ProductAppearanceStruct),                  // 190
    _ENTRY(_BridgedDeviceBasicInformation_StartUp),                                  // 191
    _ENTRY(_BridgedDeviceBasicInformation_ShutDown),                                 // 192
    _ENTRY(_BridgedDeviceBasicInformation_Leave),                                    // 193
    _ENTRY(_BridgedDeviceBasicInformation_ReachableChanged),                         // 194
    _ENTRY(_Switch),                                                                 // 195
    _ENTRY(_Switch_SwitchLatched),                                                   // 196
    _ENTRY(_Switch_InitialPress),                                                    // 197
    _ENTRY(_Switch_LongPress),                                                       // 198
    _ENTRY(_Switch_ShortRelease),                                                    // 199
    _ENTRY(_Switch_LongRelease),                                                     // 200
    _ENTRY(_Switch_MultiPressOngoing),                                               // 201
    _ENTRY(_Switch_MultiPressComplete),                                              // 202
    _ENTRY(_AdministratorCommissioning),                                             // 203
    _ENTRY(_AdministratorCommissioning_OpenCommissioningWindowRequest),              // 204
    _ENTRY(_AdministratorCommissioning_OpenBasicCommissioningWindowRequest),         // 205
    _ENTRY(_OperationalCredentials),                                                 // 206
    _ENTRY(_OperationalCredentials_FabricDescriptorStruct),                          // 207
    _ENTRY(_OperationalCredentials_NOCStruct),                                       // 208
    _ENTRY(_OperationalCredentials_AttestationRequestRequest),                       // 209
    _ENTRY(_OperationalCredentials_AttestationResponse),                             // 210
    _ENTRY(_OperationalCredentials_CertificateChainRequestRequest),                  // 211
    _ENTRY(_OperationalCredentials_CertificateChainResponse),                        // 212
    _ENTRY(_OperationalCredentials_CSRRequestRequest),                               // 213
    _ENTRY(_OperationalCredentials_CSRResponse),                                     // 214
    _ENTRY(_OperationalCredentials_AddNOCRequest),                                   // 215
    _ENTRY(_OperationalCredentials_UpdateNOCRequest),                                // 216
    _ENTRY(_OperationalCredentials_NOCResponse),                                     // 217
    _ENTRY(_OperationalCredentials_UpdateFabricLabelRequest),                        // 218
    _ENTRY(_OperationalCredentials_RemoveFabricRequest),                             // 219
    _ENTRY(_OperationalCredentials_AddTrustedRootCertificateRequest),                // 220
    _ENTRY(_OperationalCredentials_FabricDescriptorStruct_list_),                    // 221
    _ENTRY(_OperationalCredentials_NOCStruct_list_),                                 // 222
    _ENTRY(_GroupKeyManagement),                                                     // 223
    _ENTRY(_GroupKeyManagement_GroupInfoMapStruct),                                  // 224
    _ENTRY(_GroupKeyManagement_GroupKeyMapStruct),                                   // 225
    _ENTRY(_GroupKeyManagement_GroupKeySetStruct),                                   // 226
    _ENTRY(_GroupKeyManagement_KeySetWriteRequest),                                  // 227
    _ENTRY(_GroupKeyManagement_KeySetReadRequest),                                   // 228
    _ENTRY(_GroupKeyManagement_KeySetReadResponse),                                  // 229
    _ENTRY(_GroupKeyManagement_KeySetRemoveRequest),                                 // 230
    _ENTRY(_GroupKeyManagement_KeySetReadAllIndicesResponse),                        // 231
    _ENTRY(_GroupKeyManagement_GroupKeyMapStruct_list_),                             // 232
    _ENTRY(_GroupKeyManagement_GroupInfoMapStruct_list_),                            // 233
    _ENTRY(_FixedLabel),                                                             // 234
    _ENTRY(_FixedLabel_LabelStruct),                                                 // 235
    _ENTRY(_FixedLabel_LabelStruct_list_),                                           // 236
    _ENTRY(_UserLabel),                                                              // 237
    _ENTRY(_UserLabel_LabelStruct),                                                  // 238
    _ENTRY(_UserLabel_LabelStruct_list_),                                            // 239
    _ENTRY(_ProxyConfiguration),                                                     // 240
    _ENTRY(_ProxyDiscovery),                                                         // 241
    _ENTRY(_ProxyValid),                                                             // 242
    _ENTRY(_BooleanState),                                                           // 243
    _ENTRY(_BooleanState_StateChange),                                               // 244
    _ENTRY(_IcdManagement),                                                          // 245
    _ENTRY(_IcdManagement_MonitoringRegistrationStruct),                             // 246
    _ENTRY(_IcdManagement_RegisterClientRequest),                                    // 247
    _ENTRY(_IcdManagement_RegisterClientResponse),                                   // 248
    _ENTRY(_IcdManagement_UnregisterClientRequest),                                  // 249
    _ENTRY(_IcdManagement_MonitoringRegistrationStruct_list_),                       // 250
    _ENTRY(_ModeSelect),                                                             // 251
    _ENTRY(_ModeSelect_SemanticTagStruct),                                           // 252
    _ENTRY(_ModeSelect_ModeOptionStruct),                                            // 253
    _ENTRY(_ModeSelect_ChangeToModeRequest),                                         // 254
    _ENTRY(_ModeSelect_SemanticTagStruct_list_),                                     // 255
    _ENTRY(_ModeSelect_ModeOptionStruct_list_),                                      // 256
    _ENTRY(_LaundryWasherMode),                                                      // 257
    _ENTRY(_LaundryWasherMode_ModeTagStruct),                                        // 258
    _ENTRY(_LaundryWasherMode_ModeOptionStruct),                                     // 259
    _ENTRY(_LaundryWasherMode_ChangeToModeRequest),                                  // 260
    _ENTRY(_LaundryWasherMode_ChangeToModeResponse),                                 // 261
    _ENTRY(_LaundryWasherMode_ModeOptionStruct_list_),                               // 262
    _ENTRY(_LaundryWasherMode_ModeTagStruct_list_),                                  // 263
    _ENTRY(_RefrigeratorAndTemperatureControlledCabinetMode),                        // 264
    _ENTRY(_RefrigeratorAndTemperatureControlledCabinetMode_ModeTagStruct),          // 265
    _ENTRY(_RefrigeratorAndTemperatureControlledCabinetMode_ModeOptionStruct),       // 266
    _ENTRY(_RefrigeratorAndTemperatureControlledCabinetMode_ChangeToModeRequest),    // 267
    _ENTRY(_RefrigeratorAndTemperatureControlledCabinetMode_ChangeToModeResponse),   // 268
    _ENTRY(_RefrigeratorAndTemperatureControlledCabinetMode_ModeTagStruct_list_),    // 269
    _ENTRY(_RefrigeratorAndTemperatureControlledCabinetMode_ModeOptionStruct_list_), // 270
    _ENTRY(_LaundryWasherControls),                                                  // 271
    _ENTRY(_RvcRunMode),                                                             // 272
    _ENTRY(_RvcRunMode_ModeTagStruct),                                               // 273
    _ENTRY(_RvcRunMode_ModeOptionStruct),                                            // 274
    _ENTRY(_RvcRunMode_ChangeToModeRequest),                                         // 275
    _ENTRY(_RvcRunMode_ChangeToModeResponse),                                        // 276
    _ENTRY(_RvcRunMode_ModeTagStruct_list_),                                         // 277
    _ENTRY(_RvcRunMode_ModeOptionStruct_list_),                                      // 278
    _ENTRY(_RvcCleanMode),                                                           // 279
    _ENTRY(_RvcCleanMode_ModeTagStruct),                                             // 280
    _ENTRY(_RvcCleanMode_ModeOptionStruct),                                          // 281
    _ENTRY(_RvcCleanMode_ChangeToModeRequest),                                       // 282
    _ENTRY(_RvcCleanMode_ChangeToModeResponse),                                      // 283
    _ENTRY(_RvcCleanMode_ModeOptionStruct_list_),                                    // 284
    _ENTRY(_RvcCleanMode_ModeTagStruct_list_),                                       // 285
    _ENTRY(_TemperatureControl),                                                     // 286
    _ENTRY(_TemperatureControl_SetTemperatureRequest),                               // 287
    _ENTRY(_RefrigeratorAlarm),                                                      // 288
    _ENTRY(_RefrigeratorAlarm_Notify),                                               // 289
    _ENTRY(_DishwasherMode),                                                         // 290
    _ENTRY(_DishwasherMode_ModeTagStruct),                                           // 291
    _ENTRY(_DishwasherMode_ModeOptionStruct),                                        // 292
    _ENTRY(_DishwasherMode_ChangeToModeRequest),                                     // 293
    _ENTRY(_DishwasherMode_ChangeToModeResponse),                                    // 294
    _ENTRY(_DishwasherMode_ModeOptionStruct_list_),                                  // 295
    _ENTRY(_DishwasherMode_ModeTagStruct_list_),                                     // 296
    _ENTRY(_AirQuality),                                                             // 297
    _ENTRY(_SmokeCoAlarm),                                                           // 298
    _ENTRY(_SmokeCoAlarm_SmokeAlarm),                                                // 299
    _ENTRY(_SmokeCoAlarm_COAlarm),                                                   // 300
    _ENTRY(_SmokeCoAlarm_LowBattery),                                                // 301
    _ENTRY(_SmokeCoAlarm_HardwareFault),                                             // 302
    _ENTRY(_SmokeCoAlarm_EndOfService),                                              // 303
    _ENTRY(_SmokeCoAlarm_SelfTestComplete),                                          // 304
    _ENTRY(_SmokeCoAlarm_AlarmMuted),                                                // 305
    _ENTRY(_SmokeCoAlarm_MuteEnded),                                                 // 306
    _ENTRY(_SmokeCoAlarm_InterconnectSmokeAlarm),                                    // 307
    _ENTRY(_SmokeCoAlarm_InterconnectCOAlarm),                                       // 308
    _ENTRY(_SmokeCoAlarm_AllClear),                                                  // 309
    _ENTRY(_DishwasherAlarm),                                                        // 310
    _ENTRY(_DishwasherAlarm_ResetRequest),                                           // 311
    _ENTRY(_DishwasherAlarm_ModifyEnabledAlarmsRequest),                             // 312
    _ENTRY(_DishwasherAlarm_Notify),                                                 // 313
    _ENTRY(_OperationalState),                                                       // 314
    _ENTRY(_OperationalState_ErrorStateStruct),                                      // 315
    _ENTRY(_OperationalState_OperationalStateStruct),                                // 316
    _ENTRY(_OperationalState_OperationalCommandResponse),                            // 317
    _ENTRY(_OperationalState_OperationalError),                                      // 318
    _ENTRY(_OperationalState_OperationCompletion),                                   // 319
    _ENTRY(_OperationalState_OperationalStateStruct_list_),                          // 320
    _ENTRY(_RvcOperationalState),                                                    // 321
    _ENTRY(_RvcOperationalState_ErrorStateStruct),                                   // 322
    _ENTRY(_RvcOperationalState_OperationalStateStruct),                             // 323
    _ENTRY(_RvcOperationalState_OperationalCommandResponse),                         // 324
    _ENTRY(_RvcOperationalState_OperationalError),                                   // 325
    _ENTRY(_RvcOperationalState_OperationCompletion),                                // 326
    _ENTRY(_RvcOperationalState_OperationalStateStruct_list_),                       // 327
    _ENTRY(_HepaFilterMonitoring),                                                   // 328
    _ENTRY(_ActivatedCarbonFilterMonitoring),                                        // 329
    _ENTRY(_CeramicFilterMonitoring),                                                // 330
    _ENTRY(_ElectrostaticFilterMonitoring),                                          // 331
    _ENTRY(_UvFilterMonitoring),                                                     // 332
    _ENTRY(_IonizingFilterMonitoring),                                               // 333
    _ENTRY(_ZeoliteFilterMonitoring),                                                // 334
    _ENTRY(_OzoneFilterMonitoring),                                                  // 335
    _ENTRY(_WaterTankMonitoring),                                                    // 336
    _ENTRY(_FuelTankMonitoring),                                                     // 337
    _ENTRY(_InkCartridgeMonitoring),                                                 // 338
    _ENTRY(_TonerCartridgeMonitoring),                                               // 339
    _ENTRY(_DoorLock),                                                               // 340
    _ENTRY(_DoorLock_CredentialStruct),                                              // 341
    _ENTRY(_DoorLock_LockDoorRequest),                                               // 342
    _ENTRY(_DoorLock_UnlockDoorRequest),                                             // 343
    _ENTRY(_DoorLock_UnlockWithTimeoutRequest),                                      // 344
    _ENTRY(_DoorLock_SetWeekDayScheduleRequest),                                     // 345
    _ENTRY(_DoorLock_GetWeekDayScheduleRequest),                                     // 346
    _ENTRY(_DoorLock_GetWeekDayScheduleResponse),                                    // 347
    _ENTRY(_DoorLock_ClearWeekDayScheduleRequest),                                   // 348
    _ENTRY(_DoorLock_SetYearDayScheduleRequest),                                     // 349
    _ENTRY(_DoorLock_GetYearDayScheduleRequest),                                     // 350
    _ENTRY(_DoorLock_GetYearDayScheduleResponse),                                    // 351
    _ENTRY(_DoorLock_ClearYearDayScheduleRequest),                                   // 352
    _ENTRY(_DoorLock_SetHolidayScheduleRequest),                                     // 353
    _ENTRY(_DoorLock_GetHolidayScheduleRequest),                                     // 354
    _ENTRY(_DoorLock_GetHolidayScheduleResponse),                                    // 355
    _ENTRY(_DoorLock_ClearHolidayScheduleRequest),                                   // 356
    _ENTRY(_DoorLock_SetUserRequest),                                                // 357
    _ENTRY(_DoorLock_GetUserRequest),                                                // 358
    _ENTRY(_DoorLock_GetUserResponse),                                               // 359
    _ENTRY(_DoorLock_ClearUserRequest),                                              // 360
    _ENTRY(_DoorLock_SetCredentialRequest),                                          // 361
    _ENTRY(_DoorLock_SetCredentialResponse),                                         // 362
    _ENTRY(_DoorLock_GetCredentialStatusRequest),                                    // 363
    _ENTRY(_DoorLock_GetCredentialStatusResponse),                                   // 364
    _ENTRY(_DoorLock_ClearCredentialRequest),                                        // 365
    _ENTRY(_DoorLock_UnboltDoorRequest),                                             // 366
    _ENTRY(_DoorLock_DoorLockAlarm),                                                 // 367
    _ENTRY(_DoorLock_DoorStateChange),                                               // 368
    _ENTRY(_DoorLock_LockOperation),                                                 // 369
    _ENTRY(_DoorLock_LockOperationError),                                            // 370
    _ENTRY(_DoorLock_LockUserChange),                                                // 371
    _ENTRY(_DoorLock_CredentialStruct_list_),                                        // 372
    _ENTRY(_WindowCovering),                                                         // 373
    _ENTRY(_WindowCovering_GoToLiftValueRequest),                                    // 374
    _ENTRY(_WindowCovering_GoToLiftPercentageRequest),                               // 375
    _ENTRY(_WindowCovering_GoToTiltValueRequest),                                    // 376
    _ENTRY(_WindowCovering_GoToTiltPercentageRequest),                               // 377
    _ENTRY(_BarrierControl),                                                         // 378
    _ENTRY(_BarrierControl_BarrierControlGoToPercentRequest),                        // 379
    _ENTRY(_PumpConfigurationAndControl),                                            // 380
    _ENTRY(_PumpConfigurationAndControl_SupplyVoltageLow),                           // 381
    _ENTRY(_PumpConfigurationAndControl_SupplyVoltageHigh),                          // 382
    _ENTRY(_PumpConfigurationAndControl_PowerMissingPhase),                          // 383
    _ENTRY(_PumpConfigurationAndControl_SystemPressureLow),                          // 384
    _ENTRY(_PumpConfigurationAndControl_SystemPressureHigh),                         // 385
    _ENTRY(_PumpConfigurationAndControl_DryRunning),                                 // 386
    _ENTRY(_PumpConfigurationAndControl_MotorTemperatureHigh),                       // 387
    _ENTRY(_PumpConfigurationAndControl_PumpMotorFatalFailure),                      // 388
    _ENTRY(_PumpConfigurationAndControl_ElectronicTemperatureHigh),                  // 389
    _ENTRY(_PumpConfigurationAndControl_PumpBlocked),                                // 390
    _ENTRY(_PumpConfigurationAndControl_SensorFailure),                              // 391
    _ENTRY(_PumpConfigurationAndControl_ElectronicNonFatalFailure),                  // 392
    _ENTRY(_PumpConfigurationAndControl_ElectronicFatalFailure),                     // 393
    _ENTRY(_PumpConfigurationAndControl_GeneralFault),                               // 394
    _ENTRY(_PumpConfigurationAndControl_Leakage),                                    // 395
    _ENTRY(_PumpConfigurationAndControl_AirDetection),                               // 396
    _ENTRY(_PumpConfigurationAndControl_TurbineOperation),                           // 397
    _ENTRY(_Thermostat),                                                             // 398
    _ENTRY(_Thermostat_ThermostatScheduleTransition),                                // 399
    _ENTRY(_Thermostat_SetpointRaiseLowerRequest),                                   // 400
    _ENTRY(_Thermostat_GetWeeklyScheduleResponse),                                   // 401
    _ENTRY(_Thermostat_SetWeeklyScheduleRequest),                                    // 402
    _ENTRY(_Thermostat_GetWeeklyScheduleRequest),                                    // 403
    _ENTRY(_Thermostat_ThermostatScheduleTransition_list_),                          // 404
    _ENTRY(_FanControl),                                                             // 405
    _ENTRY(_FanControl_StepRequest),                                                 // 406
    _ENTRY(_ThermostatUserInterfaceConfiguration),                                   // 407
    _ENTRY(_ColorControl),                                                           // 408
    _ENTRY(_ColorControl_MoveToHueRequest),                                          // 409
    _ENTRY(_ColorControl_MoveHueRequest),                                            // 410
    _ENTRY(_ColorControl_StepHueRequest),                                            // 411
    _ENTRY(_ColorControl_MoveToSaturationRequest),                                   // 412
    _ENTRY(_ColorControl_MoveSaturationRequest),                                     // 413
    _ENTRY(_ColorControl_StepSaturationRequest),                                     // 414
    _ENTRY(_ColorControl_MoveToHueAndSaturationRequest),                             // 415
    _ENTRY(_ColorControl_MoveToColorRequest),                                        // 416
    _ENTRY(_ColorControl_MoveColorRequest),                                          // 417
    _ENTRY(_ColorControl_StepColorRequest),                                          // 418
    _ENTRY(_ColorControl_MoveToColorTemperatureRequest),                             // 419
    _ENTRY(_ColorControl_EnhancedMoveToHueRequest),                                  // 420
    _ENTRY(_ColorControl_EnhancedMoveHueRequest),                                    // 421
    _ENTRY(_ColorControl_EnhancedStepHueRequest),                                    // 422
    _ENTRY(_ColorControl_EnhancedMoveToHueAndSaturationRequest),                     // 423
    _ENTRY(_ColorControl_ColorLoopSetRequest),                                       // 424
    _ENTRY(_ColorControl_StopMoveStepRequest),                                       // 425
    _ENTRY(_ColorControl_MoveColorTemperatureRequest),                               // 426
    _ENTRY(_ColorControl_StepColorTemperatureRequest),                               // 427
    _ENTRY(_BallastConfiguration),                                                   // 428
    _ENTRY(_IlluminanceMeasurement),                                                 // 429
    _ENTRY(_TemperatureMeasurement),                                                 // 430
    _ENTRY(_PressureMeasurement),                                                    // 431
    _ENTRY(_FlowMeasurement),                                                        // 432
    _ENTRY(_RelativeHumidityMeasurement),                                            // 433
    _ENTRY(_OccupancySensing),                                                       // 434
    _ENTRY(_CarbonMonoxideConcentrationMeasurement),                                 // 435
    _ENTRY(_CarbonDioxideConcentrationMeasurement),                                  // 436
    _ENTRY(_EthyleneConcentrationMeasurement),                                       // 437
    _ENTRY(_EthyleneOxideConcentrationMeasurement),                                  // 438
    _ENTRY(_HydrogenConcentrationMeasurement),                                       // 439
    _ENTRY(_HydrogenSulfideConcentrationMeasurement),                                // 440
    _ENTRY(_NitricOxideConcentrationMeasurement),                                    // 441
    _ENTRY(_NitrogenDioxideConcentrationMeasurement),                                // 442
    _ENTRY(_OxygenConcentrationMeasurement),                                         // 443
    _ENTRY(_OzoneConcentrationMeasurement),                                          // 444
    _ENTRY(_SulfurDioxideConcentrationMeasurement),                                  // 445
    _ENTRY(_DissolvedOxygenConcentrationMeasurement),                                // 446
    _ENTRY(_BromateConcentrationMeasurement),                                        // 447
    _ENTRY(_ChloraminesConcentrationMeasurement),                                    // 448
    _ENTRY(_ChlorineConcentrationMeasurement),                                       // 449
    _ENTRY(_FecalColiformEColiConcentrationMeasurement),                             // 450
    _ENTRY(_FluorideConcentrationMeasurement),                                       // 451
    _ENTRY(_HaloaceticAcidsConcentrationMeasurement),                                // 452
    _ENTRY(_TotalTrihalomethanesConcentrationMeasurement),                           // 453
    _ENTRY(_TotalColiformBacteriaConcentrationMeasurement),                          // 454
    _ENTRY(_TurbidityConcentrationMeasurement),                                      // 455
    _ENTRY(_CopperConcentrationMeasurement),                                         // 456
    _ENTRY(_LeadConcentrationMeasurement),                                           // 457
    _ENTRY(_ManganeseConcentrationMeasurement),                                      // 458
    _ENTRY(_SulfateConcentrationMeasurement),                                        // 459
    _ENTRY(_BromodichloromethaneConcentrationMeasurement),                           // 460
    _ENTRY(_BromoformConcentrationMeasurement),                                      // 461
    _ENTRY(_ChlorodibromomethaneConcentrationMeasurement),                           // 462
    _ENTRY(_ChloroformConcentrationMeasurement),                                     // 463
    _ENTRY(_SodiumConcentrationMeasurement),                                         // 464
    _ENTRY(_Pm25ConcentrationMeasurement),                                           // 465
    _ENTRY(_FormaldehydeConcentrationMeasurement),                                   // 466
    _ENTRY(_Pm1ConcentrationMeasurement),                                            // 467
    _ENTRY(_Pm10ConcentrationMeasurement),                                           // 468
    _ENTRY(_TotalVolatileOrganicCompoundsConcentrationMeasurement),                  // 469
    _ENTRY(_RadonConcentrationMeasurement),                                          // 470
    _ENTRY(_WakeOnLan),                                                              // 471
    _ENTRY(_Channel),                                                                // 472
    _ENTRY(_Channel_ChannelInfoStruct),                                              // 473
    _ENTRY(_Channel_LineupInfoStruct),                                               // 474
    _ENTRY(_Channel_ChangeChannelRequest),                                           // 475
    _ENTRY(_Channel_ChangeChannelResponse),                                          // 476
    _ENTRY(_Channel_ChangeChannelByNumberRequest),                                   // 477
    _ENTRY(_Channel_SkipChannelRequest),                                             // 478
    _ENTRY(_Channel_ChannelInfoStruct_list_),                                        // 479
    _ENTRY(_TargetNavigator),                                                        // 480
    _ENTRY(_TargetNavigator_TargetInfoStruct),                                       // 481
    _ENTRY(_TargetNavigator_NavigateTargetRequest),                                  // 482
    _ENTRY(_TargetNavigator_NavigateTargetResponse),                                 // 483
    _ENTRY(_TargetNavigator_TargetInfoStruct_list_),                                 // 484
    _ENTRY(_MediaPlayback),                                                          // 485
    _ENTRY(_MediaPlayback_PlaybackPositionStruct),                                   // 486
    _ENTRY(_MediaPlayback_SkipForwardRequest),                                       // 487
    _ENTRY(_MediaPlayback_SkipBackwardRequest),                                      // 488
    _ENTRY(_MediaPlayback_PlaybackResponse),                                         // 489
    _ENTRY(_MediaPlayback_SeekRequest),                                              // 490
    _ENTRY(_MediaInput),                                                             // 491
    _ENTRY(_MediaInput_InputInfoStruct),                                             // 492
    _ENTRY(_MediaInput_SelectInputRequest),                                          // 493
    _ENTRY(_MediaInput_RenameInputRequest),                                          // 494
    _ENTRY(_MediaInput_InputInfoStruct_list_),                                       // 495
    _ENTRY(_LowPower),                                                               // 496
    _ENTRY(_KeypadInput),                                                            // 497
    _ENTRY(_KeypadInput_SendKeyRequest),                                             // 498
    _ENTRY(_KeypadInput_SendKeyResponse),                                            // 499
    _ENTRY(_ContentLauncher),                                                        // 500
    _ENTRY(_ContentLauncher_DimensionStruct),                                        // 501
    _ENTRY(_ContentLauncher_AdditionalInfoStruct),                                   // 502
    _ENTRY(_ContentLauncher_ParameterStruct),                                        // 503
    _ENTRY(_ContentLauncher_ContentSearchStruct),                                    // 504
    _ENTRY(_ContentLauncher_StyleInformationStruct),                                 // 505
    _ENTRY(_ContentLauncher_BrandingInformationStruct),                              // 506
    _ENTRY(_ContentLauncher_LaunchContentRequest),                                   // 507
    _ENTRY(_ContentLauncher_LaunchURLRequest),                                       // 508
    _ENTRY(_ContentLauncher_LauncherResponse),                                       // 509
    _ENTRY(_ContentLauncher_AdditionalInfoStruct_list_),                             // 510
    _ENTRY(_ContentLauncher_ParameterStruct_list_),                                  // 511
    _ENTRY(_AudioOutput),                                                            // 512
    _ENTRY(_AudioOutput_OutputInfoStruct),                                           // 513
    _ENTRY(_AudioOutput_SelectOutputRequest),                                        // 514
    _ENTRY(_AudioOutput_RenameOutputRequest),                                        // 515
    _ENTRY(_AudioOutput_OutputInfoStruct_list_),                                     // 516
    _ENTRY(_ApplicationLauncher),                                                    // 517
    _ENTRY(_ApplicationLauncher_ApplicationStruct),                                  // 518
    _ENTRY(_ApplicationLauncher_ApplicationEPStruct),                                // 519
    _ENTRY(_ApplicationLauncher_LaunchAppRequest),                                   // 520
    _ENTRY(_ApplicationLauncher_StopAppRequest),                                     // 521
    _ENTRY(_ApplicationLauncher_HideAppRequest),                                     // 522
    _ENTRY(_ApplicationLauncher_LauncherResponse),                                   // 523
    _ENTRY(_ApplicationBasic),                                                       // 524
    _ENTRY(_ApplicationBasic_ApplicationStruct),                                     // 525
    _ENTRY(_AccountLogin),                                                           // 526
    _ENTRY(_AccountLogin_GetSetupPINRequest),                                        // 527
    _ENTRY(_AccountLogin_GetSetupPINResponse),                                       // 528
    _ENTRY(_AccountLogin_LoginRequest),                                              // 529
    _ENTRY(_ElectricalMeasurement),                                                  // 530
    _ENTRY(_ElectricalMeasurement_GetProfileInfoResponseCommand),                    // 531
    _ENTRY(_ElectricalMeasurement_GetMeasurementProfileResponseCommand),             // 532
    _ENTRY(_ElectricalMeasurement_GetMeasurementProfileCommandRequest),              // 533
    _ENTRY(_UnitTesting),                                                            // 534
    _ENTRY(_UnitTesting_SimpleStruct),                                               // 535
    _ENTRY(_UnitTesting_TestFabricScoped),                                           // 536
    _ENTRY(_UnitTesting_NullablesAndOptionalsStruct),                                // 537
    _ENTRY(_UnitTesting_NestedStruct),                                               // 538
    _ENTRY(_UnitTesting_NestedStructList),                                           // 539
    _ENTRY(_UnitTesting_DoubleNestedStructList),                                     // 540
    _ENTRY(_UnitTesting_TestListStructOctet),                                        // 541
    _ENTRY(_UnitTesting_TestSpecificResponse),                                       // 542
    _ENTRY(_UnitTesting_TestAddArgumentsResponse),                                   // 543
    _ENTRY(_UnitTesting_TestSimpleArgumentResponse),                                 // 544
    _ENTRY(_UnitTesting_TestStructArrayArgumentResponse),                            // 545
    _ENTRY(_UnitTesting_TestAddArgumentsRequest),                                    // 546
    _ENTRY(_UnitTesting_TestListInt8UReverseResponse),                               // 547
    _ENTRY(_UnitTesting_TestSimpleArgumentRequestRequest),                           // 548
    _ENTRY(_UnitTesting_TestEnumsResponse),                                          // 549
    _ENTRY(_UnitTesting_TestStructArrayArgumentRequestRequest),                      // 550
    _ENTRY(_UnitTesting_TestNullableOptionalResponse),                               // 551
    _ENTRY(_UnitTesting_TestStructArgumentRequestRequest),                           // 552
    _ENTRY(_UnitTesting_TestComplexNullableOptionalResponse),                        // 553
    _ENTRY(_UnitTesting_TestNestedStructArgumentRequestRequest),                     // 554
    _ENTRY(_UnitTesting_BooleanResponse),                                            // 555
    _ENTRY(_UnitTesting_TestListStructArgumentRequestRequest),                       // 556
    _ENTRY(_UnitTesting_SimpleStructResponse),                                       // 557
    _ENTRY(_UnitTesting_TestListInt8UArgumentRequestRequest),                        // 558
    _ENTRY(_UnitTesting_TestEmitTestEventResponse),                                  // 559
    _ENTRY(_UnitTesting_TestNestedStructListArgumentRequestRequest),                 // 560
    _ENTRY(_UnitTesting_TestEmitTestFabricScopedEventResponse),                      // 561
    _ENTRY(_UnitTesting_TestListNestedStructListArgumentRequestRequest),             // 562
    _ENTRY(_UnitTesting_TestListInt8UReverseRequestRequest),                         // 563
    _ENTRY(_UnitTesting_TestEnumsRequestRequest),                                    // 564
    _ENTRY(_UnitTesting_TestNullableOptionalRequestRequest),                         // 565
    _ENTRY(_UnitTesting_TestComplexNullableOptionalRequestRequest),                  // 566
    _ENTRY(_UnitTesting_SimpleStructEchoRequestRequest),                             // 567
    _ENTRY(_UnitTesting_TestSimpleOptionalArgumentRequestRequest),                   // 568
    _ENTRY(_UnitTesting_TestEmitTestEventRequestRequest),                            // 569
    _ENTRY(_UnitTesting_TestEmitTestFabricScopedEventRequestRequest),                // 570
    _ENTRY(_UnitTesting_TestEvent),                                                  // 571
    _ENTRY(_UnitTesting_TestFabricScopedEvent),                                      // 572
    _ENTRY(_UnitTesting_SimpleStruct_list_),                                         // 573
    _ENTRY(_UnitTesting_TestListStructOctet_list_),                                  // 574
    _ENTRY(_UnitTesting_TestFabricScoped_list_),                                     // 575
    _ENTRY(_UnitTesting_NullablesAndOptionalsStruct_list_),                          // 576
    _ENTRY(_UnitTesting_NestedStructList_list_),                                     // 577
    _ENTRY(_FaultInjection),                                                         // 578
    _ENTRY(_FaultInjection_FailAtFaultRequest),                                      // 579
    _ENTRY(_FaultInjection_FailRandomlyAtFaultRequest),                              // 580
} };

} // namespace TLVMeta
} // namespace chip
