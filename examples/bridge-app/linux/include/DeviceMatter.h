#pragma once
#ifndef DEVICEMATTER_H
#define DEVICEMATTER_H
#include <platform/CHIPDeviceLayer.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Events.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/clusters/network-commissioning/network-commissioning.h>
#include <app/util/af-types.h>
#include <app/util/attribute-storage.h>


using namespace chip;
using namespace chip::app;
using namespace chip::Inet;
using namespace chip::Transport;
using namespace chip::DeviceLayer;
using namespace chip::app::Clusters;

namespace MatterDevice{
const int kDescriptorAttributeArraySize = 254;
    // static chip::DeviceLayer::DeviceInfoProviderImpl gExampleDeviceInfoProvider;
    constexpr chip::EndpointId kLightSwitchEndpoint   = 2;
    constexpr chip::EndpointId kGenericSwitchEndpoint = 2;
    DeviceLayer::NetworkCommissioning::LinuxEthernetDriver sEthernetDriver;
    Clusters::NetworkCommissioning::Instance sEthernetNetworkCommissioningInstance(0, &sEthernetDriver);

    // ENDPOINT DEFINITIONS:
    // =================================================================================
    //
    // Endpoint definitions will be reused across multiple endpoints for every instance of the
    // endpoint type.
    // There will be no intrinsic storage for the endpoint attributes declared here.
    // Instead, all attributes will be treated as EXTERNAL, and therefore all reads
    // or writes to the attributes must be handled within the emberAfExternalAttributeWriteCallback
    // and emberAfExternalAttributeReadCallback functions declared herein. This fits
    // the typical model of a bridge, since a bridge typically maintains its own
    // state database representing the devices connected to it.

    // Device types for dynamic endpoints: TODO Need a generated file from ZAP to define these!

    #define DEVICE_TYPE_BRIDGED_NODE                    0x0013

    #define DEVICE_TYPE_LO_ON_OFF_LIGHT                 0x0100

    #define DEVICE_TYPE_DIM_LIGHT                       0x0101

    #define DEVICE_TYPE_COLOR_TEMPERATURE_LIGHT         0x010C

    #define DEVICE_TYPE_LO_SWITCH_LIGHT                 0x0103

    #define DEVICE_TYPE_DIMMER_SWITCH                   0x0104

    #define DEVICE_TYPE_COLOR_DIMMER_SWITCH             0x0105

    #define DEVICE_TYPE_EXTENAL_LIGHT                   0x010D

    #define DEVICE_TYPE_TEMPERATURE_SENSOR              0x0302

    #define DEVICE_TYPE_HUMIDITY_SENSOR                 0x0307

    #define DEVICE_TYPE_CONTACT_SENSOR                  0x0015

    #define DEVICE_TYPE_LIGHT_SENSOR                    0x0106

    #define DEVICE_VERSION_DEFAULT 1


    /*-----------------------Contact Attribute---------------------------*/

    DECLARE_DYNAMIC_ATTRIBUTE_LIST_BEGIN(contactAttrs)
    DECLARE_DYNAMIC_ATTRIBUTE(BooleanState::Attributes::StateValue::Id, BOOLEAN, 1, 0),
    // DECLARE_DYNAMIC_ATTRIBUTE(BooleanState::Attributes::AcceptedCommandList::Id, ARRAY, kDescriptorAttributeArraySize, 0), /* accepted command list */
    // DECLARE_DYNAMIC_ATTRIBUTE(BooleanState::Attributes::GeneratedCommandList::Id, ARRAY, kDescriptorAttributeArraySize, 0), /* generated command list */
    // DECLARE_DYNAMIC_ATTRIBUTE(BooleanState::Attributes::AttributeList::Id, ARRAY, kDescriptorAttributeArraySize, 0), /* attribute list */
    DECLARE_DYNAMIC_ATTRIBUTE(BooleanState::Attributes::FeatureMap::Id, BITMAP32, 4, 0),
        DECLARE_DYNAMIC_ATTRIBUTE_LIST_END();
    
    constexpr EventId eventsContactSensor[] = {
        app::Clusters::BooleanState::Events::StateChange::Id,
        kInvalidEventId,
    };


    /*-----------------------OnOff Attribute---------------------------*/

    DECLARE_DYNAMIC_ATTRIBUTE_LIST_BEGIN(onOffAttrs)
    DECLARE_DYNAMIC_ATTRIBUTE(OnOff::Attributes::OnOff::Id, BOOLEAN, 1, 0),
    DECLARE_DYNAMIC_ATTRIBUTE(OnOff::Attributes::GlobalSceneControl::Id, BOOLEAN, 1, 0),
    DECLARE_DYNAMIC_ATTRIBUTE(OnOff::Attributes::OnTime::Id, INT16U, 2, 0),
    DECLARE_DYNAMIC_ATTRIBUTE(OnOff::Attributes::OffWaitTime::Id, INT16U, 2, 0),
    DECLARE_DYNAMIC_ATTRIBUTE(OnOff::Attributes::StartUpOnOff::Id, BOOLEAN, 1, 0),
    // DECLARE_DYNAMIC_ATTRIBUTE(OnOff::Attributes::AcceptedCommandList::Id, ARRAY, kDescriptorAttributeArraySize, 0),     /* accepted command list */
    // DECLARE_DYNAMIC_ATTRIBUTE(OnOff::Attributes::GeneratedCommandList::Id, ARRAY, kDescriptorAttributeArraySize, 0),    /* generated command list */
    // DECLARE_DYNAMIC_ATTRIBUTE(OnOff::Attributes::AttributeList::Id, ARRAY, kDescriptorAttributeArraySize, 0),           /* attribute list */
    DECLARE_DYNAMIC_ATTRIBUTE(OnOff::Attributes::FeatureMap::Id, BITMAP32, 4, 0),
        DECLARE_DYNAMIC_ATTRIBUTE_LIST_END();

    constexpr CommandId onOffCommands[] = {
        app::Clusters::OnOff::Commands::Off::Id,
        app::Clusters::OnOff::Commands::On::Id,
        app::Clusters::OnOff::Commands::Toggle::Id,
        kInvalidCommandId,
    };


    /*-----------------------Switch Attribute---------------------------*/

    DECLARE_DYNAMIC_ATTRIBUTE_LIST_BEGIN(switchAttrs)
    DECLARE_DYNAMIC_ATTRIBUTE(Switch::Attributes::CurrentPosition::Id, INT8U, 1, 0),
    DECLARE_DYNAMIC_ATTRIBUTE(Switch::Attributes::NumberOfPositions::Id, INT8U, 1, 0),
    DECLARE_DYNAMIC_ATTRIBUTE(Switch::Attributes::MultiPressMax::Id, INT8U, 1, 0),
    // DECLARE_DYNAMIC_ATTRIBUTE(Switch::Attributes::AcceptedCommandList::Id, ARRAY, kDescriptorAttributeArraySize, 0),    /* accepted command list */
    // DECLARE_DYNAMIC_ATTRIBUTE(Switch::Attributes::GeneratedCommandList::Id, ARRAY, kDescriptorAttributeArraySize, 0),   /* generated command list */
    // DECLARE_DYNAMIC_ATTRIBUTE(Switch::Attributes::AttributeList::Id, ARRAY, kDescriptorAttributeArraySize, 0),          /* attribute list */
    DECLARE_DYNAMIC_ATTRIBUTE(Switch::Attributes::EventList::Id, ARRAY, kDescriptorAttributeArraySize, 0),              /* event list */
    DECLARE_DYNAMIC_ATTRIBUTE(Switch::Attributes::FeatureMap::Id, BITMAP32, 4, 0),
        DECLARE_DYNAMIC_ATTRIBUTE_LIST_END();

    constexpr EventId eventsSwitch[] = {
        app::Clusters::Switch::Events::SwitchLatched::Id,
        kInvalidEventId,
    };


    /*-----------------------LevelControl Attribute---------------------------*/

    DECLARE_DYNAMIC_ATTRIBUTE_LIST_BEGIN(levelControlAttrs)
    DECLARE_DYNAMIC_ATTRIBUTE(LevelControl::Attributes::CurrentLevel::Id, INT8U, 1, 0), 
    DECLARE_DYNAMIC_ATTRIBUTE(LevelControl::Attributes::MinLevel::Id, INT8U, 1, 0), 
    DECLARE_DYNAMIC_ATTRIBUTE(LevelControl::Attributes::MaxLevel::Id, INT8U, 1, 0), 
    DECLARE_DYNAMIC_ATTRIBUTE(LevelControl::Attributes::OnLevel::Id, INT8U, 1, 0),
    DECLARE_DYNAMIC_ATTRIBUTE(LevelControl::Attributes::Options::Id, BITMAP8, 1, 0),
    
    // DECLARE_DYNAMIC_ATTRIBUTE(LevelControl::Attributes::AcceptedCommandList::Id, ARRAY, kDescriptorAttributeArraySize, 0),    /* accepted command list */
    // DECLARE_DYNAMIC_ATTRIBUTE(LevelControl::Attributes::GeneratedCommandList::Id, ARRAY, kDescriptorAttributeArraySize, 0),   /* generated command list */
    // DECLARE_DYNAMIC_ATTRIBUTE(LevelControl::Attributes::AttributeList::Id, ARRAY, kDescriptorAttributeArraySize, 0),          /* attribute list */
    DECLARE_DYNAMIC_ATTRIBUTE(LevelControl::Attributes::FeatureMap::Id, BITMAP32, 4, 0),
        DECLARE_DYNAMIC_ATTRIBUTE_LIST_END();

    constexpr CommandId levelControlCommands[] = {
        app::Clusters::LevelControl::Commands::MoveToLevel::Id,
        app::Clusters::LevelControl::Commands::Move::Id,
        app::Clusters::LevelControl::Commands::MoveToLevelWithOnOff::Id,
        app::Clusters::LevelControl::Commands::MoveWithOnOff::Id,
        app::Clusters::LevelControl::Commands::Step::Id,
        app::Clusters::LevelControl::Commands::StepWithOnOff::Id,
        app::Clusters::LevelControl::Commands::Stop::Id,
        app::Clusters::LevelControl::Commands::StopWithOnOff::Id,
        kInvalidCommandId,
    };


    /*-----------------------Temperature Color Attribute---------------------------*/

    DECLARE_DYNAMIC_ATTRIBUTE_LIST_BEGIN(tempColorControlAttrs)
    DECLARE_DYNAMIC_ATTRIBUTE(ColorControl::Attributes::ColorTemperatureMireds::Id, INT8U, 1, 0),
    DECLARE_DYNAMIC_ATTRIBUTE(ColorControl::Attributes::ColorTempPhysicalMinMireds::Id, INT8U, 1, 0),
    DECLARE_DYNAMIC_ATTRIBUTE(ColorControl::Attributes::ColorTempPhysicalMaxMireds::Id, INT8U, 1, 0),
    DECLARE_DYNAMIC_ATTRIBUTE(ColorControl::Attributes::ColorMode::Id, ENUM8, 1, 0),
    DECLARE_DYNAMIC_ATTRIBUTE(ColorControl::Attributes::Options::Id, BITMAP8, 1, 0),
    DECLARE_DYNAMIC_ATTRIBUTE(ColorControl::Attributes::EnhancedColorMode::Id, BOOLEAN, 1, 0),
    DECLARE_DYNAMIC_ATTRIBUTE(ColorControl::Attributes::ColorCapabilities::Id, BITMAP16, 2, 0),
    // DECLARE_DYNAMIC_ATTRIBUTE(ColorControl::Attributes::AcceptedCommandList::Id, ARRAY, kDescriptorAttributeArraySize, 0),    /* accepted command list */
    // DECLARE_DYNAMIC_ATTRIBUTE(ColorControl::Attributes::GeneratedCommandList::Id, ARRAY, kDescriptorAttributeArraySize, 0),   /* generated command list */
    // DECLARE_DYNAMIC_ATTRIBUTE(ColorControl::Attributes::AttributeList::Id, ARRAY, kDescriptorAttributeArraySize, 0),          /* attribute list */
    DECLARE_DYNAMIC_ATTRIBUTE(ColorControl::Attributes::FeatureMap::Id, BITMAP32, 4, 0),
        DECLARE_DYNAMIC_ATTRIBUTE_LIST_END();

    constexpr CommandId tempColorControlCommands[] = {
        app::Clusters::ColorControl::Commands::MoveToColorTemperature ::Id,
        app::Clusters::ColorControl::Commands::MoveColorTemperature ::Id,
        app::Clusters::ColorControl::Commands::StepColorTemperature ::Id,
        app::Clusters::ColorControl::Commands::StopMoveStep ::Id,
        kInvalidCommandId,
    };

    /*-----------------------HS Color Attribute---------------------------*/
    DECLARE_DYNAMIC_ATTRIBUTE_LIST_BEGIN(extendedColorControlAttrs)
    DECLARE_DYNAMIC_ATTRIBUTE(ColorControl::Attributes::CurrentHue::Id, INT8U, 1, 0),
    DECLARE_DYNAMIC_ATTRIBUTE(ColorControl::Attributes::CurrentSaturation::Id, INT8U, 1, 0),
    DECLARE_DYNAMIC_ATTRIBUTE(ColorControl::Attributes::ColorMode::Id, ENUM8, 1, 0),
    DECLARE_DYNAMIC_ATTRIBUTE(ColorControl::Attributes::Options::Id, BITMAP8, 1, 0),
    DECLARE_DYNAMIC_ATTRIBUTE(ColorControl::Attributes::EnhancedColorMode::Id, BOOLEAN, 1, 0),
    DECLARE_DYNAMIC_ATTRIBUTE(ColorControl::Attributes::ColorCapabilities::Id, BITMAP16, 2, 0),
    DECLARE_DYNAMIC_ATTRIBUTE(ColorControl::Attributes::FeatureMap::Id, BITMAP32, 4, 0),
    // DECLARE_DYNAMIC_ATTRIBUTE(ColorControl::Attributes::AcceptedCommandList::Id, ARRAY, kDescriptorAttributeArraySize, 0),    /* accepted command list */
    // DECLARE_DYNAMIC_ATTRIBUTE(ColorControl::Attributes::GeneratedCommandList::Id, ARRAY, kDescriptorAttributeArraySize, 0),   /* generated command list */
    // DECLARE_DYNAMIC_ATTRIBUTE(ColorControl::Attributes::AttributeList::Id, ARRAY, kDescriptorAttributeArraySize, 0),          /* attribute list */
    // DECLARE_DYNAMIC_ATTRIBUTE(ColorControl::Attributes::FeatureMap::Id, BITMAP32, 4, 0),
        DECLARE_DYNAMIC_ATTRIBUTE_LIST_END();

    constexpr CommandId extendedColorControlCommands[] = {
        app::Clusters::ColorControl::Commands::MoveToHue::Id,
        app::Clusters::ColorControl::Commands::MoveHue::Id,
        app::Clusters::ColorControl::Commands::StepHue::Id,  
        app::Clusters::ColorControl::Commands::MoveToSaturation::Id,
        app::Clusters::ColorControl::Commands::MoveSaturation::Id,
        app::Clusters::ColorControl::Commands::StepSaturation::Id,
        app::Clusters::ColorControl::Commands::MoveToHueAndSaturation::Id,
        kInvalidCommandId,
    };

    /*-----------------------Group Attribute---------------------------*/

    DECLARE_DYNAMIC_ATTRIBUTE_LIST_BEGIN(groupsAttrs)
    DECLARE_DYNAMIC_ATTRIBUTE(Groups::Attributes::NameSupport::Id, BITMAP8, 2, 0),
    // DECLARE_DYNAMIC_ATTRIBUTE(ColorControl::Attributes::AcceptedCommandList::Id, ARRAY, kDescriptorAttributeArraySize, 0),    /* accepted command list */
    // DECLARE_DYNAMIC_ATTRIBUTE(ColorControl::Attributes::GeneratedCommandList::Id, ARRAY, kDescriptorAttributeArraySize, 0),   /* generated command list */
    // DECLARE_DYNAMIC_ATTRIBUTE(ColorControl::Attributes::AttributeList::Id, ARRAY, kDescriptorAttributeArraySize, 0),          /* attribute list */
    DECLARE_DYNAMIC_ATTRIBUTE(ColorControl::Attributes::FeatureMap::Id, BITMAP32, 4, 0),
        DECLARE_DYNAMIC_ATTRIBUTE_LIST_END();
    
    constexpr CommandId groupsCommands[] = {
        app::Clusters::Groups::Commands::AddGroup::Id,
        app::Clusters::Groups::Commands::AddGroupIfIdentifying::Id,
        app::Clusters::Groups::Commands::GetGroupMembership::Id,
        app::Clusters::Groups::Commands::RemoveGroup::Id,
        app::Clusters::Groups::Commands::RemoveAllGroups::Id,
        app::Clusters::Groups::Commands::AddGroupResponse::Id,
        app::Clusters::Groups::Commands::ViewGroup::Id,
        app::Clusters::Groups::Commands::GetGroupMembershipResponse::Id,
        app::Clusters::Groups::Commands::RemoveGroupResponse::Id,
        kInvalidCommandId,
    };


    /*-----------------------Scenes Attribute---------------------------*/
    DECLARE_DYNAMIC_ATTRIBUTE_LIST_BEGIN(scenesAttrs)
    // DECLARE_DYNAMIC_ATTRIBUTE(ScenesManagement::Attributes:: ::Id, INT8U, 1, 0),
    // DECLARE_DYNAMIC_ATTRIBUTE(ScenesManagement::Attributes::CurrentScene::Id, INT8U, 1, 0),
    // DECLARE_DYNAMIC_ATTRIBUTE(ScenesManagement::Attributes::CurrentGroup::Id, INT16U, 2, 0),
    // DECLARE_DYNAMIC_ATTRIBUTE(ScenesManagement::Attributes::SceneValid::Id, BOOLEAN, 1, 0),
    // DECLARE_DYNAMIC_ATTRIBUTE(ScenesManagement::Attributes::NameSupport::Id, BITMAP8, 1, 0),
    DECLARE_DYNAMIC_ATTRIBUTE(ScenesManagement::Attributes::LastConfiguredBy::Id, NODE_ID, 1, 0),
    DECLARE_DYNAMIC_ATTRIBUTE(ScenesManagement::Attributes::SceneTableSize::Id, INT8U, 1, 0),
    // DECLARE_DYNAMIC_ATTRIBUTE(ScenesManagement::Attributes::FabricSceneInfo::Id, FABRIC_SCENE_INFO, 1, 0),
    // DECLARE_DYNAMIC_ATTRIBUTE(ScenesManagement::Attributes::AcceptedCommandList::Id, ARRAY, kDescriptorAttributeArraySize, 0),    /* accepted command list */
    // DECLARE_DYNAMIC_ATTRIBUTE(ScenesManagement::Attributes::GeneratedCommandList::Id, ARRAY, kDescriptorAttributeArraySize, 0),   /* generated command list */
    // DECLARE_DYNAMIC_ATTRIBUTE(ScenesManagement::Attributes::AttributeList::Id, ARRAY, kDescriptorAttributeArraySize, 0),          /* attribute list */
    DECLARE_DYNAMIC_ATTRIBUTE(ScenesManagement::Attributes::FeatureMap::Id, BITMAP32, 4, 0),
    // DECLARE_DYNAMIC_ATTRIBUTE(ScenesManagement::Attributes::ClusterRevision::Id, INT16U, 2, 0),
        DECLARE_DYNAMIC_ATTRIBUTE_LIST_END();

    constexpr CommandId scenesCommands[] = {
        app::Clusters::ScenesManagement::Commands::AddScene::Id,
        app::Clusters::ScenesManagement::Commands::GetSceneMembership::Id,
        app::Clusters::ScenesManagement::Commands::RecallScene::Id,
        app::Clusters::ScenesManagement::Commands::RemoveAllScenes::Id,
        app::Clusters::ScenesManagement::Commands::RemoveScene::Id,
        app::Clusters::ScenesManagement::Commands::StoreScene::Id,
        app::Clusters::ScenesManagement::Commands::ViewScene::Id,
        kInvalidCommandId,
    };
  

    /*-----------------------Identify Attribute---------------------------*/
    DECLARE_DYNAMIC_ATTRIBUTE_LIST_BEGIN(identifyAttrs)
    DECLARE_DYNAMIC_ATTRIBUTE(Identify::Attributes::IdentifyTime::Id, INT16U, 2, 0),
    DECLARE_DYNAMIC_ATTRIBUTE(Identify::Attributes::IdentifyType::Id, ENUM8, 1, 0),
    DECLARE_DYNAMIC_ATTRIBUTE(Identify::Attributes::FeatureMap::Id, BITMAP32, 4, 0),
    // DECLARE_DYNAMIC_ATTRIBUTE(Identify::Attributes::AcceptedCommandList::Id, ARRAY, kDescriptorAttributeArraySize, 0),    /* accepted command list */
    // DECLARE_DYNAMIC_ATTRIBUTE(Identify::Attributes::GeneratedCommandList::Id, ARRAY, kDescriptorAttributeArraySize, 0),   /* generated command list */
    // DECLARE_DYNAMIC_ATTRIBUTE(Identify::Attributes::AttributeList::Id, ARRAY, kDescriptorAttributeArraySize, 0),          /* attribute list */
    // DECLARE_DYNAMIC_ATTRIBUTE(Identify::Attributes::FeatureMap::Id, BITMAP32, 4, 0),
        DECLARE_DYNAMIC_ATTRIBUTE_LIST_END();

    constexpr CommandId identifyCommands[] = {
        app::Clusters::Identify::Commands::Identify::Id,
        kInvalidCommandId,
    };

    
    /*-----------------------Binding Attribute---------------------------*/
 
    DECLARE_DYNAMIC_ATTRIBUTE_LIST_BEGIN(bindingAttrs)
    DECLARE_DYNAMIC_ATTRIBUTE(Binding::Attributes::Binding::Id, ARRAY, 256, 1),
    DECLARE_DYNAMIC_ATTRIBUTE(Binding::Attributes::FeatureMap::Id, BITMAP32, 4, 0),
    // DECLARE_DYNAMIC_ATTRIBUTE(Binding::Attributes::AttributeList::Id, ARRAY, kDescriptorAttributeArraySize, 0),          /* attribute list */
    // DECLARE_DYNAMIC_ATTRIBUTE(Binding::Attributes::FeatureMap::Id, BITMAP32, 4, 0),
    // DECLARE_DYNAMIC_ATTRIBUTE(Binding::Attributes::EventList::Id, ARRAY, 4, 0),
        DECLARE_DYNAMIC_ATTRIBUTE_LIST_END();



    /*-----------------------Temperature Attribute---------------------------*/

    DECLARE_DYNAMIC_ATTRIBUTE_LIST_BEGIN(temperatureAttrs)
    DECLARE_DYNAMIC_ATTRIBUTE(TemperatureMeasurement::Attributes::MeasuredValue::Id, INT16S, 2, 0),
    DECLARE_DYNAMIC_ATTRIBUTE(TemperatureMeasurement::Attributes::MinMeasuredValue::Id, INT16S, 2, 0),
    DECLARE_DYNAMIC_ATTRIBUTE(TemperatureMeasurement::Attributes::MaxMeasuredValue::Id, INT16S, 2, 0),
    // DECLARE_DYNAMIC_ATTRIBUTE(TemperatureMeasurement::Attributes::AttributeList::Id, ARRAY, kDescriptorAttributeArraySize, 0),          /* attribute list */
    DECLARE_DYNAMIC_ATTRIBUTE(TemperatureMeasurement::Attributes::FeatureMap::Id, BITMAP32, 4, 0),
    DECLARE_DYNAMIC_ATTRIBUTE(TemperatureMeasurement::Attributes::EventList::Id, ARRAY, 4, 0),
        DECLARE_DYNAMIC_ATTRIBUTE_LIST_END();


    // Dev Declare Humidity cluster attributes
    DECLARE_DYNAMIC_ATTRIBUTE_LIST_BEGIN(humidityAttrs)
    DECLARE_DYNAMIC_ATTRIBUTE(RelativeHumidityMeasurement::Attributes::MeasuredValue::Id, INT8U, 1, 0),
    DECLARE_DYNAMIC_ATTRIBUTE(RelativeHumidityMeasurement::Attributes::MinMeasuredValue::Id, INT8U, 1, 0),
    DECLARE_DYNAMIC_ATTRIBUTE(RelativeHumidityMeasurement::Attributes::MaxMeasuredValue::Id, INT8U, 1, 0),
    // DECLARE_DYNAMIC_ATTRIBUTE(RelativeHumidityMeasurement::Attributes::AttributeList::Id, ARRAY, kDescriptorAttributeArraySize, 0),          /* attribute list */
    DECLARE_DYNAMIC_ATTRIBUTE(RelativeHumidityMeasurement::Attributes::FeatureMap::Id, BITMAP32, 4, 0),
    DECLARE_DYNAMIC_ATTRIBUTE(RelativeHumidityMeasurement::Attributes::EventList::Id, ARRAY, 4, 0),
        DECLARE_DYNAMIC_ATTRIBUTE_LIST_END();
    

    /*-----------------------Illuminance Measurement Attribute---------------------------*/
    DECLARE_DYNAMIC_ATTRIBUTE_LIST_BEGIN(lightAttrs)
    DECLARE_DYNAMIC_ATTRIBUTE(IlluminanceMeasurement::Attributes::MeasuredValue::Id, INT16S, 2, 0),
    DECLARE_DYNAMIC_ATTRIBUTE(IlluminanceMeasurement::Attributes::MinMeasuredValue::Id, INT16S, 2, 0),
    DECLARE_DYNAMIC_ATTRIBUTE(IlluminanceMeasurement::Attributes::MaxMeasuredValue::Id, INT16S, 2, 0),
    // DECLARE_DYNAMIC_ATTRIBUTE(IlluminanceMeasurement::Attributes::AttributeList::Id, ARRAY, kDescriptorAttributeArraySize, 0),          /* attribute list */
    DECLARE_DYNAMIC_ATTRIBUTE(IlluminanceMeasurement::Attributes::FeatureMap::Id, BITMAP32, 4, 0),
    DECLARE_DYNAMIC_ATTRIBUTE(IlluminanceMeasurement::Attributes::EventList::Id, ARRAY, 4, 0),
        DECLARE_DYNAMIC_ATTRIBUTE_LIST_END();


    /*-----------------------Descriptor Attribute---------------------------*/
    DECLARE_DYNAMIC_ATTRIBUTE_LIST_BEGIN(descriptorAttrs)
    DECLARE_DYNAMIC_ATTRIBUTE(Descriptor::Attributes::DeviceTypeList::Id, ARRAY, kDescriptorAttributeArraySize, 0), /* device list */
        DECLARE_DYNAMIC_ATTRIBUTE(Descriptor::Attributes::ServerList::Id, ARRAY, kDescriptorAttributeArraySize, 0), /* server list */
        DECLARE_DYNAMIC_ATTRIBUTE(Descriptor::Attributes::ClientList::Id, ARRAY, kDescriptorAttributeArraySize, 0), /* client list */
        DECLARE_DYNAMIC_ATTRIBUTE(Descriptor::Attributes::PartsList::Id, ARRAY, kDescriptorAttributeArraySize, 0),  /* parts list */
        // DECLARE_DYNAMIC_ATTRIBUTE(Descriptor::Attributes::ClusterRevision::Id, INT16U, 2, 0),                       /* cluster revision */
        // DECLARE_DYNAMIC_ATTRIBUTE(Descriptor::Attributes::AcceptedCommandList::Id, ARRAY, kDescriptorAttributeArraySize, 0), /* accepted command list */
        // DECLARE_DYNAMIC_ATTRIBUTE(Descriptor::Attributes::GeneratedCommandList::Id, ARRAY, kDescriptorAttributeArraySize, 0), /* generated command list */
        // DECLARE_DYNAMIC_ATTRIBUTE(Descriptor::Attributes::AttributeList::Id, ARRAY, kDescriptorAttributeArraySize, 0), /* attribute list */
        DECLARE_DYNAMIC_ATTRIBUTE_LIST_END();

    
    /*-----------------------Bridged Device Basic Information Attribute---------------------------*/
    DECLARE_DYNAMIC_ATTRIBUTE_LIST_BEGIN(bridgedDeviceBasicAttrs)
    DECLARE_DYNAMIC_ATTRIBUTE(BridgedDeviceBasicInformation::Attributes::NodeLabel::Id, CHAR_STRING, 10, 0),                /* NodeLabel */
        DECLARE_DYNAMIC_ATTRIBUTE(BridgedDeviceBasicInformation::Attributes::VendorName::Id, CHAR_STRING, 10, 0),
        DECLARE_DYNAMIC_ATTRIBUTE(BridgedDeviceBasicInformation::Attributes::VendorID::Id, VENDOR_ID, 10, 0),
        DECLARE_DYNAMIC_ATTRIBUTE(BridgedDeviceBasicInformation::Attributes::ProductName::Id, CHAR_STRING, 10, 0),
        DECLARE_DYNAMIC_ATTRIBUTE(BridgedDeviceBasicInformation::Attributes::ProductID::Id, INT16U, 10, 0),
        DECLARE_DYNAMIC_ATTRIBUTE(BridgedDeviceBasicInformation::Attributes::HardwareVersion::Id, INT8U, 10, 0),
        DECLARE_DYNAMIC_ATTRIBUTE(BridgedDeviceBasicInformation::Attributes::HardwareVersionString::Id, CHAR_STRING, 10, 0),
        DECLARE_DYNAMIC_ATTRIBUTE(BridgedDeviceBasicInformation::Attributes::SoftwareVersion::Id, INT8U, 10, 0),
        DECLARE_DYNAMIC_ATTRIBUTE(BridgedDeviceBasicInformation::Attributes::SoftwareVersionString::Id, CHAR_STRING, 10, 0),
        // DECLARE_DYNAMIC_ATTRIBUTE(BridgedDeviceBasicInformation::Attributes::PartNumber::Id, CHAR_STRING, 10, 0),
        // DECLARE_DYNAMIC_ATTRIBUTE(BridgedDeviceBasicInformation::Attributes::ProductURL::Id, CHAR_STRING, 10, 0),
        DECLARE_DYNAMIC_ATTRIBUTE(BridgedDeviceBasicInformation::Attributes::SerialNumber::Id, CHAR_STRING, 10, 0),
        DECLARE_DYNAMIC_ATTRIBUTE(BridgedDeviceBasicInformation::Attributes::UniqueID::Id, CHAR_STRING, 10, 0),
        // DECLARE_DYNAMIC_ATTRIBUTE(BridgedDeviceBasicInformation::Attributes::ManufacturingDate::Id, CHAR_STRING, 10, 0),
        // DECLARE_DYNAMIC_ATTRIBUTE(BridgedDeviceBasicInformation::Attributes::ProductLabel::Id, CHAR_STRING, 10, 0),
        DECLARE_DYNAMIC_ATTRIBUTE(BridgedDeviceBasicInformation::Attributes::Reachable::Id, BOOLEAN, 1, 0),                 /* Reachable */
        DECLARE_DYNAMIC_ATTRIBUTE(BridgedDeviceBasicInformation::Attributes::FeatureMap::Id, BITMAP32, 4, 0),               /* feature map */
        // DECLARE_DYNAMIC_ATTRIBUTE(BridgedDeviceBasicInformation::Attributes::ClusterRevision::Id, INT16U, 2, 0),            /* cluster revision */
        // DECLARE_DYNAMIC_ATTRIBUTE(BridgedDeviceBasicInformation::Attributes::AttributeList::Id, ARRAY, 254, 0),            /* attribute list */
        DECLARE_DYNAMIC_ATTRIBUTE(BridgedDeviceBasicInformation::Attributes::EventList::Id, ARRAY, 254, 0),                /* event list */
        DECLARE_DYNAMIC_ATTRIBUTE_LIST_END();

    constexpr EventId eventsBrigde[] = {
        app::Clusters::BridgedDeviceBasicInformation::Events::ReachableChanged::Id,
        kInvalidEventId,
    };

    /*----------------Color Temperature Light-------------------------*/

    DECLARE_DYNAMIC_CLUSTER_LIST_BEGIN(bridgedTempLightClusters)
    DECLARE_DYNAMIC_CLUSTER(Identify::Id, identifyAttrs, ZAP_CLUSTER_MASK(SERVER), identifyCommands, nullptr),
    DECLARE_DYNAMIC_CLUSTER(Descriptor::Id, descriptorAttrs, ZAP_CLUSTER_MASK(SERVER), nullptr, nullptr),
    DECLARE_DYNAMIC_CLUSTER(BridgedDeviceBasicInformation::Id, bridgedDeviceBasicAttrs, ZAP_CLUSTER_MASK(SERVER), nullptr, nullptr),
    DECLARE_DYNAMIC_CLUSTER(Groups::Id, groupsAttrs, ZAP_CLUSTER_MASK(SERVER), groupsCommands, nullptr),
    DECLARE_DYNAMIC_CLUSTER(ScenesManagement::Id, scenesAttrs, ZAP_CLUSTER_MASK(SERVER), scenesCommands, nullptr),
    DECLARE_DYNAMIC_CLUSTER(OnOff::Id, onOffAttrs, ZAP_CLUSTER_MASK(SERVER), onOffCommands, nullptr),
    DECLARE_DYNAMIC_CLUSTER(LevelControl::Id, levelControlAttrs, ZAP_CLUSTER_MASK(SERVER), levelControlCommands, nullptr),
    DECLARE_DYNAMIC_CLUSTER(ColorControl::Id, tempColorControlAttrs, ZAP_CLUSTER_MASK(SERVER), tempColorControlCommands, nullptr),
        DECLARE_DYNAMIC_CLUSTER_LIST_END;


    /*---------------Extended Temperature Light-------------------------*/

    DECLARE_DYNAMIC_CLUSTER_LIST_BEGIN(bridgedExtendedLightClusters)
    DECLARE_DYNAMIC_CLUSTER(Identify::Id, identifyAttrs, ZAP_CLUSTER_MASK(SERVER), identifyCommands, nullptr),
    DECLARE_DYNAMIC_CLUSTER(Descriptor::Id, descriptorAttrs, ZAP_CLUSTER_MASK(SERVER), nullptr, nullptr),
    DECLARE_DYNAMIC_CLUSTER(BridgedDeviceBasicInformation::Id, bridgedDeviceBasicAttrs, ZAP_CLUSTER_MASK(SERVER), nullptr, nullptr),
    DECLARE_DYNAMIC_CLUSTER(Groups::Id, groupsAttrs, ZAP_CLUSTER_MASK(SERVER), groupsCommands, nullptr),
    DECLARE_DYNAMIC_CLUSTER(ScenesManagement::Id, scenesAttrs, ZAP_CLUSTER_MASK(SERVER), scenesCommands, nullptr),
    DECLARE_DYNAMIC_CLUSTER(OnOff::Id, onOffAttrs, ZAP_CLUSTER_MASK(SERVER), onOffCommands, nullptr),
    DECLARE_DYNAMIC_CLUSTER(LevelControl::Id, levelControlAttrs, ZAP_CLUSTER_MASK(SERVER), levelControlCommands, nullptr),
    DECLARE_DYNAMIC_CLUSTER(ColorControl::Id, extendedColorControlAttrs, ZAP_CLUSTER_MASK(SERVER), extendedColorControlCommands, nullptr),
        DECLARE_DYNAMIC_CLUSTER_LIST_END;

    /*--------------------On/Off Light Switch-------------------------*/
    DECLARE_DYNAMIC_CLUSTER_LIST_BEGIN(bridgedOnOffSwitchClusters)
    DECLARE_DYNAMIC_CLUSTER(Identify::Id, identifyAttrs, ZAP_CLUSTER_MASK(SERVER), identifyCommands, nullptr),
    DECLARE_DYNAMIC_CLUSTER(Identify::Id, identifyAttrs, ZAP_CLUSTER_MASK(CLIENT), nullptr, nullptr),
    DECLARE_DYNAMIC_CLUSTER(Descriptor::Id, descriptorAttrs, ZAP_CLUSTER_MASK(SERVER), nullptr, nullptr),
    DECLARE_DYNAMIC_CLUSTER(BridgedDeviceBasicInformation::Id, bridgedDeviceBasicAttrs, ZAP_CLUSTER_MASK(SERVER), nullptr, nullptr),
    DECLARE_DYNAMIC_CLUSTER(OnOff::Id, onOffAttrs, ZAP_CLUSTER_MASK(CLIENT), nullptr, onOffCommands),
    DECLARE_DYNAMIC_CLUSTER(Binding::Id, bindingAttrs, ZAP_CLUSTER_MASK(SERVER), nullptr, nullptr),
        DECLARE_DYNAMIC_CLUSTER_LIST_END;

    /*----------------------Temperature Sensor-------------------------*/
    DECLARE_DYNAMIC_CLUSTER_LIST_BEGIN(bridgedTemperatureClusters)
    DECLARE_DYNAMIC_CLUSTER(Identify::Id, identifyAttrs, ZAP_CLUSTER_MASK(SERVER), identifyCommands, nullptr),
    DECLARE_DYNAMIC_CLUSTER(Descriptor::Id, descriptorAttrs, ZAP_CLUSTER_MASK(SERVER), nullptr, nullptr),
    DECLARE_DYNAMIC_CLUSTER(BridgedDeviceBasicInformation::Id, bridgedDeviceBasicAttrs, ZAP_CLUSTER_MASK(SERVER), nullptr, nullptr),
    DECLARE_DYNAMIC_CLUSTER(Groups::Id, groupsAttrs, ZAP_CLUSTER_MASK(CLIENT), nullptr, nullptr),
    DECLARE_DYNAMIC_CLUSTER(TemperatureMeasurement::Id, temperatureAttrs, ZAP_CLUSTER_MASK(SERVER), nullptr, nullptr),
        DECLARE_DYNAMIC_CLUSTER_LIST_END;

    /*----------------------Humidity Sensor-------------------------*/
    DECLARE_DYNAMIC_CLUSTER_LIST_BEGIN(bridgedHumidityClusters)
    DECLARE_DYNAMIC_CLUSTER(Identify::Id, identifyAttrs, ZAP_CLUSTER_MASK(SERVER), identifyCommands, nullptr),
    DECLARE_DYNAMIC_CLUSTER(Descriptor::Id, descriptorAttrs, ZAP_CLUSTER_MASK(SERVER), nullptr, nullptr),
    DECLARE_DYNAMIC_CLUSTER(BridgedDeviceBasicInformation::Id, bridgedDeviceBasicAttrs, ZAP_CLUSTER_MASK(SERVER), nullptr, nullptr),
    DECLARE_DYNAMIC_CLUSTER(Groups::Id, groupsAttrs, ZAP_CLUSTER_MASK(CLIENT), nullptr, nullptr),
    DECLARE_DYNAMIC_CLUSTER(RelativeHumidityMeasurement::Id, humidityAttrs, ZAP_CLUSTER_MASK(SERVER),nullptr, nullptr),
        DECLARE_DYNAMIC_CLUSTER_LIST_END;


    /*----------------------Contact Sensor-------------------------*/
    DECLARE_DYNAMIC_CLUSTER_LIST_BEGIN(bridgedContactSensorClusters)
    DECLARE_DYNAMIC_CLUSTER(Identify::Id, identifyAttrs, ZAP_CLUSTER_MASK(SERVER), identifyCommands, nullptr),
    DECLARE_DYNAMIC_CLUSTER(Descriptor::Id, descriptorAttrs, ZAP_CLUSTER_MASK(SERVER), nullptr, nullptr),
    DECLARE_DYNAMIC_CLUSTER(BooleanState::Id, contactAttrs, ZAP_CLUSTER_MASK(SERVER), nullptr, nullptr),
    DECLARE_DYNAMIC_CLUSTER(BridgedDeviceBasicInformation::Id, bridgedDeviceBasicAttrs, ZAP_CLUSTER_MASK(SERVER), nullptr, nullptr),
        DECLARE_DYNAMIC_CLUSTER_LIST_END;
    
    /*----------------------Light Sensor-------------------------*/
    DECLARE_DYNAMIC_CLUSTER_LIST_BEGIN(bridgedLightSensorClusters)
    DECLARE_DYNAMIC_CLUSTER(Identify::Id, identifyAttrs, ZAP_CLUSTER_MASK(SERVER), identifyCommands, nullptr),
    DECLARE_DYNAMIC_CLUSTER(Groups::Id, groupsAttrs, ZAP_CLUSTER_MASK(CLIENT), nullptr, nullptr),
    DECLARE_DYNAMIC_CLUSTER(IlluminanceMeasurement::Id, lightAttrs, ZAP_CLUSTER_MASK(SERVER), nullptr, nullptr),
    DECLARE_DYNAMIC_CLUSTER(Descriptor::Id, descriptorAttrs, ZAP_CLUSTER_MASK(SERVER), nullptr, nullptr),
    DECLARE_DYNAMIC_CLUSTER(BridgedDeviceBasicInformation::Id, bridgedDeviceBasicAttrs, ZAP_CLUSTER_MASK(SERVER), nullptr, nullptr),
        DECLARE_DYNAMIC_CLUSTER_LIST_END;

    /**
     * @brief Create Switchs Endpoint
     * 
     */
    DECLARE_DYNAMIC_ENDPOINT(bridgedOnOffSwitchEndpoint, bridgedOnOffSwitchClusters);
    DataVersion gOnOffSwitchDataVersions[ArraySize(bridgedOnOffSwitchClusters)];


    // DECLARE_DYNAMIC_ENDPOINT(bridgedColorDimmerSwitchEndpoint, bridgedColorDimmerSwitchClusters);
    // DataVersion gColorDimmerSwitchDataVersions[ArraySize(bridgedColorDimmerSwitchClusters)];

    /**
     * @brief Create Lights Endpoint
     * 
     */
    DECLARE_DYNAMIC_ENDPOINT(bridgedTempLightEndpoint, bridgedTempLightClusters);
    DataVersion gTempLightDataVersions[ArraySize(bridgedTempLightClusters)];

    DECLARE_DYNAMIC_ENDPOINT(bridgedExtendedLightEndpoint, bridgedExtendedLightClusters);
    DataVersion gExtendedLightDataVersions[ArraySize(bridgedExtendedLightClusters)];


    /**
     * @brief Create Sensor Endpoint
     * 
     */
    DECLARE_DYNAMIC_ENDPOINT(bridgedTemperatureEndpoint, bridgedTemperatureClusters);
    DataVersion gTemperatureDataVersions[ArraySize(bridgedTemperatureClusters)];

    DECLARE_DYNAMIC_ENDPOINT(bridgedHumidityEndpoint, bridgedHumidityClusters);
    DataVersion gHumidityDataVersions[ArraySize(bridgedHumidityClusters)];

    DECLARE_DYNAMIC_ENDPOINT(bridgedContactEndpoint, bridgedContactSensorClusters);
    DataVersion gContactDataVersions[ArraySize(bridgedContactSensorClusters)];

    DECLARE_DYNAMIC_ENDPOINT(bridgedLightSensorEndpoint, bridgedLightSensorClusters);
    DataVersion gLightSensorDataVersions[ArraySize(bridgedLightSensorClusters)];

}

#endif