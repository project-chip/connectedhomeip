
/**
 *  @file
 *    This file contains declarations for accessors around clusters attributes.
 */

#pragma once

#include <app/data-model/Nullable.h>
#include <app/util/af-types.h>
#include <app-common/zap-generated/cluster-objects.h>
#include <lib/support/Span.h>

namespace unify {
namespace matter_bridge {

namespace Identify {
namespace Attributes {

namespace IdentifyTime {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value);
} // namespace IdentifyTime

namespace IdentifyType {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value);
} // namespace IdentifyType

namespace FeatureMap {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value);
} // namespace FeatureMap

namespace ClusterRevision {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value);
} // namespace ClusterRevision

} // namespace Attributes
} // Identify

namespace Groups {
namespace Attributes {

namespace NameSupport {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value);
} // namespace NameSupport

namespace FeatureMap {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value);
} // namespace FeatureMap

namespace ClusterRevision {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value);
} // namespace ClusterRevision

} // namespace Attributes
} // Groups

namespace Scenes {
namespace Attributes {

namespace SceneCount {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value);
} // namespace SceneCount

namespace CurrentScene {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value);
} // namespace CurrentScene

namespace CurrentGroup {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::GroupId& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::GroupId& value);
} // namespace CurrentGroup

namespace SceneValid {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, bool& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const bool& value);
} // namespace SceneValid

namespace NameSupport {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value);
} // namespace NameSupport

namespace LastConfiguredBy {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<chip::NodeId>& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<chip::NodeId>& value);
} // namespace LastConfiguredBy

namespace FeatureMap {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value);
} // namespace FeatureMap

namespace ClusterRevision {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value);
} // namespace ClusterRevision

} // namespace Attributes
} // Scenes

namespace OnOff {
namespace Attributes {

namespace OnOff {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, bool& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const bool& value);
} // namespace OnOff

namespace GlobalSceneControl {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, bool& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const bool& value);
} // namespace GlobalSceneControl

namespace OnTime {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value);
} // namespace OnTime

namespace OffWaitTime {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value);
} // namespace OffWaitTime

namespace StartUpOnOff {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<chip::app::Clusters::OnOff::OnOffStartUpOnOff>& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<chip::app::Clusters::OnOff::OnOffStartUpOnOff>& value);
} // namespace StartUpOnOff

namespace FeatureMap {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value);
} // namespace FeatureMap

namespace ClusterRevision {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value);
} // namespace ClusterRevision

} // namespace Attributes
} // OnOff

namespace OnOffSwitchConfiguration {
namespace Attributes {

namespace SwitchType {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value);
} // namespace SwitchType

namespace SwitchActions {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value);
} // namespace SwitchActions

namespace FeatureMap {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value);
} // namespace FeatureMap

namespace ClusterRevision {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value);
} // namespace ClusterRevision

} // namespace Attributes
} // OnOffSwitchConfiguration

namespace LevelControl {
namespace Attributes {

namespace CurrentLevel {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<uint8_t>& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<uint8_t>& value);
} // namespace CurrentLevel

namespace RemainingTime {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value);
} // namespace RemainingTime

namespace MinLevel {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value);
} // namespace MinLevel

namespace MaxLevel {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value);
} // namespace MaxLevel

namespace CurrentFrequency {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value);
} // namespace CurrentFrequency

namespace MinFrequency {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value);
} // namespace MinFrequency

namespace MaxFrequency {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value);
} // namespace MaxFrequency

namespace Options {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value);
} // namespace Options

namespace OnOffTransitionTime {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value);
} // namespace OnOffTransitionTime

namespace OnLevel {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<uint8_t>& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<uint8_t>& value);
} // namespace OnLevel

namespace OnTransitionTime {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<uint16_t>& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<uint16_t>& value);
} // namespace OnTransitionTime

namespace OffTransitionTime {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<uint16_t>& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<uint16_t>& value);
} // namespace OffTransitionTime

namespace DefaultMoveRate {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<uint8_t>& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<uint8_t>& value);
} // namespace DefaultMoveRate

namespace StartUpCurrentLevel {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<uint8_t>& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<uint8_t>& value);
} // namespace StartUpCurrentLevel

namespace FeatureMap {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value);
} // namespace FeatureMap

namespace ClusterRevision {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value);
} // namespace ClusterRevision

} // namespace Attributes
} // LevelControl

namespace BinaryInputBasic {
namespace Attributes {

namespace ActiveText {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::CharSpan& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::CharSpan& value);
} // namespace ActiveText

namespace Description {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::CharSpan& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::CharSpan& value);
} // namespace Description

namespace InactiveText {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::CharSpan& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::CharSpan& value);
} // namespace InactiveText

namespace OutOfService {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, bool& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const bool& value);
} // namespace OutOfService

namespace Polarity {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value);
} // namespace Polarity

namespace PresentValue {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, bool& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const bool& value);
} // namespace PresentValue

namespace Reliability {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value);
} // namespace Reliability

namespace StatusFlags {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value);
} // namespace StatusFlags

namespace ApplicationType {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value);
} // namespace ApplicationType

namespace FeatureMap {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value);
} // namespace FeatureMap

namespace ClusterRevision {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value);
} // namespace ClusterRevision

} // namespace Attributes
} // BinaryInputBasic

namespace PulseWidthModulation {
namespace Attributes {

namespace FeatureMap {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value);
} // namespace FeatureMap

namespace ClusterRevision {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value);
} // namespace ClusterRevision

} // namespace Attributes
} // PulseWidthModulation

namespace Descriptor {
namespace Attributes {

namespace FeatureMap {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value);
} // namespace FeatureMap

namespace ClusterRevision {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value);
} // namespace ClusterRevision

} // namespace Attributes
} // Descriptor

namespace Binding {
namespace Attributes {

namespace FeatureMap {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value);
} // namespace FeatureMap

namespace ClusterRevision {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value);
} // namespace ClusterRevision

} // namespace Attributes
} // Binding

namespace AccessControl {
namespace Attributes {

namespace SubjectsPerAccessControlEntry {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value);
} // namespace SubjectsPerAccessControlEntry

namespace TargetsPerAccessControlEntry {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value);
} // namespace TargetsPerAccessControlEntry

namespace AccessControlEntriesPerFabric {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value);
} // namespace AccessControlEntriesPerFabric

namespace FeatureMap {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value);
} // namespace FeatureMap

namespace ClusterRevision {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value);
} // namespace ClusterRevision

} // namespace Attributes
} // AccessControl

namespace Actions {
namespace Attributes {

namespace SetupURL {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::CharSpan& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::CharSpan& value);
} // namespace SetupURL

namespace FeatureMap {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value);
} // namespace FeatureMap

namespace ClusterRevision {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value);
} // namespace ClusterRevision

} // namespace Attributes
} // Actions

namespace Basic {
namespace Attributes {

namespace DataModelRevision {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value);
} // namespace DataModelRevision

namespace VendorName {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::CharSpan& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::CharSpan& value);
} // namespace VendorName

namespace VendorID {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::VendorId& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::VendorId& value);
} // namespace VendorID

namespace ProductName {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::CharSpan& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::CharSpan& value);
} // namespace ProductName

namespace ProductID {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value);
} // namespace ProductID

namespace NodeLabel {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::CharSpan& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::CharSpan& value);
} // namespace NodeLabel

namespace Location {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::CharSpan& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::CharSpan& value);
} // namespace Location

namespace HardwareVersion {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value);
} // namespace HardwareVersion

namespace HardwareVersionString {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::CharSpan& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::CharSpan& value);
} // namespace HardwareVersionString

namespace SoftwareVersion {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value);
} // namespace SoftwareVersion

namespace SoftwareVersionString {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::CharSpan& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::CharSpan& value);
} // namespace SoftwareVersionString

namespace ManufacturingDate {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::CharSpan& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::CharSpan& value);
} // namespace ManufacturingDate

namespace PartNumber {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::CharSpan& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::CharSpan& value);
} // namespace PartNumber

namespace ProductURL {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::CharSpan& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::CharSpan& value);
} // namespace ProductURL

namespace ProductLabel {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::CharSpan& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::CharSpan& value);
} // namespace ProductLabel

namespace SerialNumber {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::CharSpan& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::CharSpan& value);
} // namespace SerialNumber

namespace LocalConfigDisabled {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, bool& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const bool& value);
} // namespace LocalConfigDisabled

namespace Reachable {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, bool& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const bool& value);
} // namespace Reachable

namespace UniqueID {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::CharSpan& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::CharSpan& value);
} // namespace UniqueID

namespace FeatureMap {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value);
} // namespace FeatureMap

namespace ClusterRevision {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value);
} // namespace ClusterRevision

} // namespace Attributes
} // Basic

namespace OtaSoftwareUpdateProvider {
namespace Attributes {

namespace FeatureMap {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value);
} // namespace FeatureMap

namespace ClusterRevision {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value);
} // namespace ClusterRevision

} // namespace Attributes
} // OtaSoftwareUpdateProvider

namespace OtaSoftwareUpdateRequestor {
namespace Attributes {

namespace UpdatePossible {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, bool& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const bool& value);
} // namespace UpdatePossible

namespace UpdateState {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::Clusters::OtaSoftwareUpdateRequestor::OTAUpdateStateEnum& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::Clusters::OtaSoftwareUpdateRequestor::OTAUpdateStateEnum& value);
} // namespace UpdateState

namespace UpdateStateProgress {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<uint8_t>& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<uint8_t>& value);
} // namespace UpdateStateProgress

namespace FeatureMap {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value);
} // namespace FeatureMap

namespace ClusterRevision {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value);
} // namespace ClusterRevision

} // namespace Attributes
} // OtaSoftwareUpdateRequestor

namespace LocalizationConfiguration {
namespace Attributes {

namespace ActiveLocale {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::CharSpan& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::CharSpan& value);
} // namespace ActiveLocale

namespace FeatureMap {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value);
} // namespace FeatureMap

namespace ClusterRevision {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value);
} // namespace ClusterRevision

} // namespace Attributes
} // LocalizationConfiguration

namespace TimeFormatLocalization {
namespace Attributes {

namespace HourFormat {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::Clusters::TimeFormatLocalization::HourFormat& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::Clusters::TimeFormatLocalization::HourFormat& value);
} // namespace HourFormat

namespace ActiveCalendarType {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::Clusters::TimeFormatLocalization::CalendarType& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::Clusters::TimeFormatLocalization::CalendarType& value);
} // namespace ActiveCalendarType

namespace FeatureMap {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value);
} // namespace FeatureMap

namespace ClusterRevision {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value);
} // namespace ClusterRevision

} // namespace Attributes
} // TimeFormatLocalization

namespace UnitLocalization {
namespace Attributes {

namespace TemperatureUnit {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::Clusters::UnitLocalization::TempUnit& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::Clusters::UnitLocalization::TempUnit& value);
} // namespace TemperatureUnit

namespace FeatureMap {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value);
} // namespace FeatureMap

namespace ClusterRevision {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value);
} // namespace ClusterRevision

} // namespace Attributes
} // UnitLocalization

namespace PowerSourceConfiguration {
namespace Attributes {

namespace FeatureMap {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value);
} // namespace FeatureMap

namespace ClusterRevision {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value);
} // namespace ClusterRevision

} // namespace Attributes
} // PowerSourceConfiguration

namespace PowerSource {
namespace Attributes {

namespace Status {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::Clusters::PowerSource::PowerSourceStatus& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::Clusters::PowerSource::PowerSourceStatus& value);
} // namespace Status

namespace Order {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value);
} // namespace Order

namespace Description {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::CharSpan& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::CharSpan& value);
} // namespace Description

namespace WiredAssessedInputVoltage {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<uint32_t>& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<uint32_t>& value);
} // namespace WiredAssessedInputVoltage

namespace WiredAssessedInputFrequency {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<uint16_t>& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<uint16_t>& value);
} // namespace WiredAssessedInputFrequency

namespace WiredCurrentType {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::Clusters::PowerSource::WiredCurrentType& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::Clusters::PowerSource::WiredCurrentType& value);
} // namespace WiredCurrentType

namespace WiredAssessedCurrent {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<uint32_t>& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<uint32_t>& value);
} // namespace WiredAssessedCurrent

namespace WiredNominalVoltage {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value);
} // namespace WiredNominalVoltage

namespace WiredMaximumCurrent {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value);
} // namespace WiredMaximumCurrent

namespace WiredPresent {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, bool& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const bool& value);
} // namespace WiredPresent

namespace BatVoltage {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<uint32_t>& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<uint32_t>& value);
} // namespace BatVoltage

namespace BatPercentRemaining {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<uint8_t>& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<uint8_t>& value);
} // namespace BatPercentRemaining

namespace BatTimeRemaining {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<uint32_t>& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<uint32_t>& value);
} // namespace BatTimeRemaining

namespace BatChargeLevel {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::Clusters::PowerSource::BatChargeLevel& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::Clusters::PowerSource::BatChargeLevel& value);
} // namespace BatChargeLevel

namespace BatReplacementNeeded {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, bool& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const bool& value);
} // namespace BatReplacementNeeded

namespace BatReplaceability {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::Clusters::PowerSource::BatReplaceability& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::Clusters::PowerSource::BatReplaceability& value);
} // namespace BatReplaceability

namespace BatPresent {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, bool& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const bool& value);
} // namespace BatPresent

namespace BatReplacementDescription {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::CharSpan& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::CharSpan& value);
} // namespace BatReplacementDescription

namespace BatCommonDesignation {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value);
} // namespace BatCommonDesignation

namespace BatANSIDesignation {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::CharSpan& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::CharSpan& value);
} // namespace BatANSIDesignation

namespace BatIECDesignation {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::CharSpan& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::CharSpan& value);
} // namespace BatIECDesignation

namespace BatApprovedChemistry {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value);
} // namespace BatApprovedChemistry

namespace BatCapacity {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value);
} // namespace BatCapacity

namespace BatQuantity {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value);
} // namespace BatQuantity

namespace BatChargeState {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::Clusters::PowerSource::BatChargeState& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::Clusters::PowerSource::BatChargeState& value);
} // namespace BatChargeState

namespace BatTimeToFullCharge {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<uint32_t>& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<uint32_t>& value);
} // namespace BatTimeToFullCharge

namespace BatFunctionalWhileCharging {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, bool& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const bool& value);
} // namespace BatFunctionalWhileCharging

namespace BatChargingCurrent {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<uint32_t>& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<uint32_t>& value);
} // namespace BatChargingCurrent

namespace FeatureMap {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value);
} // namespace FeatureMap

namespace ClusterRevision {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value);
} // namespace ClusterRevision

} // namespace Attributes
} // PowerSource

namespace GeneralCommissioning {
namespace Attributes {

namespace Breadcrumb {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint64_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint64_t& value);
} // namespace Breadcrumb

namespace RegulatoryConfig {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::Clusters::GeneralCommissioning::RegulatoryLocationType& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::Clusters::GeneralCommissioning::RegulatoryLocationType& value);
} // namespace RegulatoryConfig

namespace LocationCapability {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::Clusters::GeneralCommissioning::RegulatoryLocationType& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::Clusters::GeneralCommissioning::RegulatoryLocationType& value);
} // namespace LocationCapability

namespace SupportsConcurrentConnection {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, bool& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const bool& value);
} // namespace SupportsConcurrentConnection

namespace FeatureMap {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value);
} // namespace FeatureMap

namespace ClusterRevision {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value);
} // namespace ClusterRevision

} // namespace Attributes
} // GeneralCommissioning

namespace NetworkCommissioning {
namespace Attributes {

namespace MaxNetworks {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value);
} // namespace MaxNetworks

namespace ScanMaxTimeSeconds {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value);
} // namespace ScanMaxTimeSeconds

namespace ConnectMaxTimeSeconds {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value);
} // namespace ConnectMaxTimeSeconds

namespace InterfaceEnabled {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, bool& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const bool& value);
} // namespace InterfaceEnabled

namespace LastNetworkingStatus {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<chip::app::Clusters::NetworkCommissioning::NetworkCommissioningStatus>& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<chip::app::Clusters::NetworkCommissioning::NetworkCommissioningStatus>& value);
} // namespace LastNetworkingStatus

namespace LastNetworkID {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<chip::ByteSpan>& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<chip::ByteSpan>& value);
} // namespace LastNetworkID

namespace LastConnectErrorValue {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<int32_t>& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<int32_t>& value);
} // namespace LastConnectErrorValue

namespace FeatureMap {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value);
} // namespace FeatureMap

namespace ClusterRevision {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value);
} // namespace ClusterRevision

} // namespace Attributes
} // NetworkCommissioning

namespace DiagnosticLogs {
namespace Attributes {

namespace FeatureMap {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value);
} // namespace FeatureMap

namespace ClusterRevision {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value);
} // namespace ClusterRevision

} // namespace Attributes
} // DiagnosticLogs

namespace GeneralDiagnostics {
namespace Attributes {

namespace RebootCount {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value);
} // namespace RebootCount

namespace UpTime {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint64_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint64_t& value);
} // namespace UpTime

namespace TotalOperationalHours {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value);
} // namespace TotalOperationalHours

namespace BootReasons {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value);
} // namespace BootReasons

namespace TestEventTriggersEnabled {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, bool& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const bool& value);
} // namespace TestEventTriggersEnabled

namespace FeatureMap {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value);
} // namespace FeatureMap

namespace ClusterRevision {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value);
} // namespace ClusterRevision

} // namespace Attributes
} // GeneralDiagnostics

namespace SoftwareDiagnostics {
namespace Attributes {

namespace CurrentHeapFree {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint64_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint64_t& value);
} // namespace CurrentHeapFree

namespace CurrentHeapUsed {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint64_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint64_t& value);
} // namespace CurrentHeapUsed

namespace CurrentHeapHighWatermark {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint64_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint64_t& value);
} // namespace CurrentHeapHighWatermark

namespace FeatureMap {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value);
} // namespace FeatureMap

namespace ClusterRevision {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value);
} // namespace ClusterRevision

} // namespace Attributes
} // SoftwareDiagnostics

namespace ThreadNetworkDiagnostics {
namespace Attributes {

namespace Channel {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<uint16_t>& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<uint16_t>& value);
} // namespace Channel

namespace RoutingRole {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<chip::app::Clusters::ThreadNetworkDiagnostics::RoutingRole>& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<chip::app::Clusters::ThreadNetworkDiagnostics::RoutingRole>& value);
} // namespace RoutingRole

namespace NetworkName {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<chip::CharSpan>& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<chip::CharSpan>& value);
} // namespace NetworkName

namespace PanId {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<uint16_t>& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<uint16_t>& value);
} // namespace PanId

namespace ExtendedPanId {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<uint64_t>& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<uint64_t>& value);
} // namespace ExtendedPanId

namespace MeshLocalPrefix {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<chip::ByteSpan>& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<chip::ByteSpan>& value);
} // namespace MeshLocalPrefix

namespace OverrunCount {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint64_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint64_t& value);
} // namespace OverrunCount

namespace PartitionId {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<uint32_t>& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<uint32_t>& value);
} // namespace PartitionId

namespace Weighting {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<uint8_t>& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<uint8_t>& value);
} // namespace Weighting

namespace DataVersion {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<uint8_t>& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<uint8_t>& value);
} // namespace DataVersion

namespace StableDataVersion {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<uint8_t>& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<uint8_t>& value);
} // namespace StableDataVersion

namespace LeaderRouterId {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<uint8_t>& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<uint8_t>& value);
} // namespace LeaderRouterId

namespace DetachedRoleCount {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value);
} // namespace DetachedRoleCount

namespace ChildRoleCount {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value);
} // namespace ChildRoleCount

namespace RouterRoleCount {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value);
} // namespace RouterRoleCount

namespace LeaderRoleCount {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value);
} // namespace LeaderRoleCount

namespace AttachAttemptCount {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value);
} // namespace AttachAttemptCount

namespace PartitionIdChangeCount {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value);
} // namespace PartitionIdChangeCount

namespace BetterPartitionAttachAttemptCount {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value);
} // namespace BetterPartitionAttachAttemptCount

namespace ParentChangeCount {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value);
} // namespace ParentChangeCount

namespace TxTotalCount {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value);
} // namespace TxTotalCount

namespace TxUnicastCount {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value);
} // namespace TxUnicastCount

namespace TxBroadcastCount {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value);
} // namespace TxBroadcastCount

namespace TxAckRequestedCount {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value);
} // namespace TxAckRequestedCount

namespace TxAckedCount {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value);
} // namespace TxAckedCount

namespace TxNoAckRequestedCount {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value);
} // namespace TxNoAckRequestedCount

namespace TxDataCount {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value);
} // namespace TxDataCount

namespace TxDataPollCount {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value);
} // namespace TxDataPollCount

namespace TxBeaconCount {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value);
} // namespace TxBeaconCount

namespace TxBeaconRequestCount {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value);
} // namespace TxBeaconRequestCount

namespace TxOtherCount {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value);
} // namespace TxOtherCount

namespace TxRetryCount {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value);
} // namespace TxRetryCount

namespace TxDirectMaxRetryExpiryCount {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value);
} // namespace TxDirectMaxRetryExpiryCount

namespace TxIndirectMaxRetryExpiryCount {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value);
} // namespace TxIndirectMaxRetryExpiryCount

namespace TxErrCcaCount {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value);
} // namespace TxErrCcaCount

namespace TxErrAbortCount {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value);
} // namespace TxErrAbortCount

namespace TxErrBusyChannelCount {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value);
} // namespace TxErrBusyChannelCount

namespace RxTotalCount {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value);
} // namespace RxTotalCount

namespace RxUnicastCount {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value);
} // namespace RxUnicastCount

namespace RxBroadcastCount {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value);
} // namespace RxBroadcastCount

namespace RxDataCount {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value);
} // namespace RxDataCount

namespace RxDataPollCount {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value);
} // namespace RxDataPollCount

namespace RxBeaconCount {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value);
} // namespace RxBeaconCount

namespace RxBeaconRequestCount {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value);
} // namespace RxBeaconRequestCount

namespace RxOtherCount {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value);
} // namespace RxOtherCount

namespace RxAddressFilteredCount {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value);
} // namespace RxAddressFilteredCount

namespace RxDestAddrFilteredCount {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value);
} // namespace RxDestAddrFilteredCount

namespace RxDuplicatedCount {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value);
} // namespace RxDuplicatedCount

namespace RxErrNoFrameCount {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value);
} // namespace RxErrNoFrameCount

namespace RxErrUnknownNeighborCount {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value);
} // namespace RxErrUnknownNeighborCount

namespace RxErrInvalidSrcAddrCount {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value);
} // namespace RxErrInvalidSrcAddrCount

namespace RxErrSecCount {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value);
} // namespace RxErrSecCount

namespace RxErrFcsCount {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value);
} // namespace RxErrFcsCount

namespace RxErrOtherCount {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value);
} // namespace RxErrOtherCount

namespace ActiveTimestamp {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<uint64_t>& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<uint64_t>& value);
} // namespace ActiveTimestamp

namespace PendingTimestamp {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<uint64_t>& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<uint64_t>& value);
} // namespace PendingTimestamp

namespace Delay {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<uint32_t>& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<uint32_t>& value);
} // namespace Delay

namespace ChannelPage0Mask {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<chip::ByteSpan>& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<chip::ByteSpan>& value);
} // namespace ChannelPage0Mask

namespace FeatureMap {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value);
} // namespace FeatureMap

namespace ClusterRevision {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value);
} // namespace ClusterRevision

} // namespace Attributes
} // ThreadNetworkDiagnostics

namespace WiFiNetworkDiagnostics {
namespace Attributes {

namespace Bssid {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<chip::ByteSpan>& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<chip::ByteSpan>& value);
} // namespace Bssid

namespace SecurityType {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<chip::app::Clusters::WiFiNetworkDiagnostics::SecurityType>& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<chip::app::Clusters::WiFiNetworkDiagnostics::SecurityType>& value);
} // namespace SecurityType

namespace WiFiVersion {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<chip::app::Clusters::WiFiNetworkDiagnostics::WiFiVersionType>& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<chip::app::Clusters::WiFiNetworkDiagnostics::WiFiVersionType>& value);
} // namespace WiFiVersion

namespace ChannelNumber {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<uint16_t>& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<uint16_t>& value);
} // namespace ChannelNumber

namespace Rssi {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<int8_t>& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<int8_t>& value);
} // namespace Rssi

namespace BeaconLostCount {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<uint32_t>& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<uint32_t>& value);
} // namespace BeaconLostCount

namespace BeaconRxCount {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<uint32_t>& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<uint32_t>& value);
} // namespace BeaconRxCount

namespace PacketMulticastRxCount {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<uint32_t>& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<uint32_t>& value);
} // namespace PacketMulticastRxCount

namespace PacketMulticastTxCount {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<uint32_t>& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<uint32_t>& value);
} // namespace PacketMulticastTxCount

namespace PacketUnicastRxCount {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<uint32_t>& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<uint32_t>& value);
} // namespace PacketUnicastRxCount

namespace PacketUnicastTxCount {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<uint32_t>& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<uint32_t>& value);
} // namespace PacketUnicastTxCount

namespace CurrentMaxRate {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<uint64_t>& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<uint64_t>& value);
} // namespace CurrentMaxRate

namespace OverrunCount {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<uint64_t>& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<uint64_t>& value);
} // namespace OverrunCount

namespace FeatureMap {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value);
} // namespace FeatureMap

namespace ClusterRevision {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value);
} // namespace ClusterRevision

} // namespace Attributes
} // WiFiNetworkDiagnostics

namespace EthernetNetworkDiagnostics {
namespace Attributes {

namespace PHYRate {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<chip::app::Clusters::EthernetNetworkDiagnostics::PHYRateType>& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<chip::app::Clusters::EthernetNetworkDiagnostics::PHYRateType>& value);
} // namespace PHYRate

namespace FullDuplex {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<bool>& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<bool>& value);
} // namespace FullDuplex

namespace PacketRxCount {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint64_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint64_t& value);
} // namespace PacketRxCount

namespace PacketTxCount {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint64_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint64_t& value);
} // namespace PacketTxCount

namespace TxErrCount {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint64_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint64_t& value);
} // namespace TxErrCount

namespace CollisionCount {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint64_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint64_t& value);
} // namespace CollisionCount

namespace OverrunCount {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint64_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint64_t& value);
} // namespace OverrunCount

namespace CarrierDetect {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<bool>& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<bool>& value);
} // namespace CarrierDetect

namespace TimeSinceReset {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint64_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint64_t& value);
} // namespace TimeSinceReset

namespace FeatureMap {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value);
} // namespace FeatureMap

namespace ClusterRevision {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value);
} // namespace ClusterRevision

} // namespace Attributes
} // EthernetNetworkDiagnostics

namespace TimeSynchronization {
namespace Attributes {

namespace UTCTime {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<uint64_t>& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<uint64_t>& value);
} // namespace UTCTime

namespace Granularity {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::Clusters::TimeSynchronization::GranularityEnum& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::Clusters::TimeSynchronization::GranularityEnum& value);
} // namespace Granularity

namespace TimeSource {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::Clusters::TimeSynchronization::TimeSourceEnum& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::Clusters::TimeSynchronization::TimeSourceEnum& value);
} // namespace TimeSource

namespace TrustedTimeNodeId {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<chip::NodeId>& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<chip::NodeId>& value);
} // namespace TrustedTimeNodeId

namespace DefaultNtp {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<chip::CharSpan>& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<chip::CharSpan>& value);
} // namespace DefaultNtp

namespace LocalTime {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<uint64_t>& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<uint64_t>& value);
} // namespace LocalTime

namespace TimeZoneDatabase {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, bool& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const bool& value);
} // namespace TimeZoneDatabase

namespace NtpServerPort {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<uint16_t>& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<uint16_t>& value);
} // namespace NtpServerPort

namespace FeatureMap {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value);
} // namespace FeatureMap

namespace ClusterRevision {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value);
} // namespace ClusterRevision

} // namespace Attributes
} // TimeSynchronization

namespace BridgedDeviceBasic {
namespace Attributes {

namespace VendorName {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::CharSpan& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::CharSpan& value);
} // namespace VendorName

namespace VendorID {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::VendorId& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::VendorId& value);
} // namespace VendorID

namespace ProductName {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::CharSpan& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::CharSpan& value);
} // namespace ProductName

namespace NodeLabel {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::CharSpan& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::CharSpan& value);
} // namespace NodeLabel

namespace HardwareVersion {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value);
} // namespace HardwareVersion

namespace HardwareVersionString {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::CharSpan& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::CharSpan& value);
} // namespace HardwareVersionString

namespace SoftwareVersion {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value);
} // namespace SoftwareVersion

namespace SoftwareVersionString {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::CharSpan& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::CharSpan& value);
} // namespace SoftwareVersionString

namespace ManufacturingDate {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::CharSpan& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::CharSpan& value);
} // namespace ManufacturingDate

namespace PartNumber {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::CharSpan& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::CharSpan& value);
} // namespace PartNumber

namespace ProductURL {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::CharSpan& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::CharSpan& value);
} // namespace ProductURL

namespace ProductLabel {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::CharSpan& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::CharSpan& value);
} // namespace ProductLabel

namespace SerialNumber {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::CharSpan& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::CharSpan& value);
} // namespace SerialNumber

namespace Reachable {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, bool& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const bool& value);
} // namespace Reachable

namespace UniqueID {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::CharSpan& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::CharSpan& value);
} // namespace UniqueID

namespace FeatureMap {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value);
} // namespace FeatureMap

namespace ClusterRevision {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value);
} // namespace ClusterRevision

} // namespace Attributes
} // BridgedDeviceBasic

namespace Switch {
namespace Attributes {

namespace NumberOfPositions {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value);
} // namespace NumberOfPositions

namespace CurrentPosition {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value);
} // namespace CurrentPosition

namespace MultiPressMax {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value);
} // namespace MultiPressMax

namespace FeatureMap {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value);
} // namespace FeatureMap

namespace ClusterRevision {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value);
} // namespace ClusterRevision

} // namespace Attributes
} // Switch

namespace AdministratorCommissioning {
namespace Attributes {

namespace WindowStatus {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::Clusters::AdministratorCommissioning::CommissioningWindowStatus& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::Clusters::AdministratorCommissioning::CommissioningWindowStatus& value);
} // namespace WindowStatus

namespace AdminFabricIndex {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<chip::FabricIndex>& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<chip::FabricIndex>& value);
} // namespace AdminFabricIndex

namespace AdminVendorId {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<uint16_t>& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<uint16_t>& value);
} // namespace AdminVendorId

namespace FeatureMap {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value);
} // namespace FeatureMap

namespace ClusterRevision {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value);
} // namespace ClusterRevision

} // namespace Attributes
} // AdministratorCommissioning

namespace OperationalCredentials {
namespace Attributes {

namespace SupportedFabrics {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value);
} // namespace SupportedFabrics

namespace CommissionedFabrics {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value);
} // namespace CommissionedFabrics

namespace CurrentFabricIndex {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value);
} // namespace CurrentFabricIndex

namespace FeatureMap {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value);
} // namespace FeatureMap

namespace ClusterRevision {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value);
} // namespace ClusterRevision

} // namespace Attributes
} // OperationalCredentials

namespace GroupKeyManagement {
namespace Attributes {

namespace MaxGroupsPerFabric {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value);
} // namespace MaxGroupsPerFabric

namespace MaxGroupKeysPerFabric {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value);
} // namespace MaxGroupKeysPerFabric

namespace FeatureMap {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value);
} // namespace FeatureMap

namespace ClusterRevision {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value);
} // namespace ClusterRevision

} // namespace Attributes
} // GroupKeyManagement

namespace FixedLabel {
namespace Attributes {

namespace FeatureMap {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value);
} // namespace FeatureMap

namespace ClusterRevision {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value);
} // namespace ClusterRevision

} // namespace Attributes
} // FixedLabel

namespace UserLabel {
namespace Attributes {

namespace FeatureMap {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value);
} // namespace FeatureMap

namespace ClusterRevision {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value);
} // namespace ClusterRevision

} // namespace Attributes
} // UserLabel

namespace ProxyConfiguration {
namespace Attributes {

namespace FeatureMap {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value);
} // namespace FeatureMap

namespace ClusterRevision {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value);
} // namespace ClusterRevision

} // namespace Attributes
} // ProxyConfiguration

namespace ProxyDiscovery {
namespace Attributes {

namespace FeatureMap {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value);
} // namespace FeatureMap

namespace ClusterRevision {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value);
} // namespace ClusterRevision

} // namespace Attributes
} // ProxyDiscovery

namespace ProxyValid {
namespace Attributes {

namespace FeatureMap {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value);
} // namespace FeatureMap

namespace ClusterRevision {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value);
} // namespace ClusterRevision

} // namespace Attributes
} // ProxyValid

namespace BooleanState {
namespace Attributes {

namespace StateValue {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, bool& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const bool& value);
} // namespace StateValue

namespace FeatureMap {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value);
} // namespace FeatureMap

namespace ClusterRevision {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value);
} // namespace ClusterRevision

} // namespace Attributes
} // BooleanState

namespace ModeSelect {
namespace Attributes {

namespace Description {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::CharSpan& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::CharSpan& value);
} // namespace Description

namespace StandardNamespace {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<uint16_t>& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<uint16_t>& value);
} // namespace StandardNamespace

namespace CurrentMode {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value);
} // namespace CurrentMode

namespace StartUpMode {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<uint8_t>& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<uint8_t>& value);
} // namespace StartUpMode

namespace OnMode {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<uint8_t>& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<uint8_t>& value);
} // namespace OnMode

namespace FeatureMap {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value);
} // namespace FeatureMap

namespace ClusterRevision {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value);
} // namespace ClusterRevision

} // namespace Attributes
} // ModeSelect

namespace DoorLock {
namespace Attributes {

namespace LockState {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<chip::app::Clusters::DoorLock::DlLockState>& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<chip::app::Clusters::DoorLock::DlLockState>& value);
} // namespace LockState

namespace LockType {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::Clusters::DoorLock::DlLockType& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::Clusters::DoorLock::DlLockType& value);
} // namespace LockType

namespace ActuatorEnabled {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, bool& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const bool& value);
} // namespace ActuatorEnabled

namespace DoorState {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<chip::app::Clusters::DoorLock::DlDoorState>& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<chip::app::Clusters::DoorLock::DlDoorState>& value);
} // namespace DoorState

namespace DoorOpenEvents {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value);
} // namespace DoorOpenEvents

namespace DoorClosedEvents {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value);
} // namespace DoorClosedEvents

namespace OpenPeriod {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value);
} // namespace OpenPeriod

namespace NumberOfTotalUsersSupported {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value);
} // namespace NumberOfTotalUsersSupported

namespace NumberOfPINUsersSupported {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value);
} // namespace NumberOfPINUsersSupported

namespace NumberOfRFIDUsersSupported {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value);
} // namespace NumberOfRFIDUsersSupported

namespace NumberOfWeekDaySchedulesSupportedPerUser {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value);
} // namespace NumberOfWeekDaySchedulesSupportedPerUser

namespace NumberOfYearDaySchedulesSupportedPerUser {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value);
} // namespace NumberOfYearDaySchedulesSupportedPerUser

namespace NumberOfHolidaySchedulesSupported {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value);
} // namespace NumberOfHolidaySchedulesSupported

namespace MaxPINCodeLength {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value);
} // namespace MaxPINCodeLength

namespace MinPINCodeLength {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value);
} // namespace MinPINCodeLength

namespace MaxRFIDCodeLength {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value);
} // namespace MaxRFIDCodeLength

namespace MinRFIDCodeLength {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value);
} // namespace MinRFIDCodeLength

namespace CredentialRulesSupport {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::BitMask<chip::app::Clusters::DoorLock::DlCredentialRuleMask>& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::BitMask<chip::app::Clusters::DoorLock::DlCredentialRuleMask>& value);
} // namespace CredentialRulesSupport

namespace NumberOfCredentialsSupportedPerUser {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value);
} // namespace NumberOfCredentialsSupportedPerUser

namespace Language {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::CharSpan& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::CharSpan& value);
} // namespace Language

namespace LEDSettings {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value);
} // namespace LEDSettings

namespace AutoRelockTime {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value);
} // namespace AutoRelockTime

namespace SoundVolume {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value);
} // namespace SoundVolume

namespace OperatingMode {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::Clusters::DoorLock::DlOperatingMode& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::Clusters::DoorLock::DlOperatingMode& value);
} // namespace OperatingMode

namespace SupportedOperatingModes {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::BitMask<chip::app::Clusters::DoorLock::DlSupportedOperatingModes>& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::BitMask<chip::app::Clusters::DoorLock::DlSupportedOperatingModes>& value);
} // namespace SupportedOperatingModes

namespace DefaultConfigurationRegister {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::BitMask<chip::app::Clusters::DoorLock::DlDefaultConfigurationRegister>& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::BitMask<chip::app::Clusters::DoorLock::DlDefaultConfigurationRegister>& value);
} // namespace DefaultConfigurationRegister

namespace EnableLocalProgramming {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, bool& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const bool& value);
} // namespace EnableLocalProgramming

namespace EnableOneTouchLocking {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, bool& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const bool& value);
} // namespace EnableOneTouchLocking

namespace EnableInsideStatusLED {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, bool& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const bool& value);
} // namespace EnableInsideStatusLED

namespace EnablePrivacyModeButton {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, bool& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const bool& value);
} // namespace EnablePrivacyModeButton

namespace LocalProgrammingFeatures {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::BitMask<chip::app::Clusters::DoorLock::DlLocalProgrammingFeatures>& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::BitMask<chip::app::Clusters::DoorLock::DlLocalProgrammingFeatures>& value);
} // namespace LocalProgrammingFeatures

namespace WrongCodeEntryLimit {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value);
} // namespace WrongCodeEntryLimit

namespace UserCodeTemporaryDisableTime {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value);
} // namespace UserCodeTemporaryDisableTime

namespace SendPINOverTheAir {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, bool& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const bool& value);
} // namespace SendPINOverTheAir

namespace RequirePINforRemoteOperation {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, bool& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const bool& value);
} // namespace RequirePINforRemoteOperation

namespace ExpiringUserTimeout {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value);
} // namespace ExpiringUserTimeout

namespace FeatureMap {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value);
} // namespace FeatureMap

namespace ClusterRevision {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value);
} // namespace ClusterRevision

} // namespace Attributes
} // DoorLock

namespace WindowCovering {
namespace Attributes {

namespace Type {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::Clusters::WindowCovering::Type& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::Clusters::WindowCovering::Type& value);
} // namespace Type

namespace PhysicalClosedLimitLift {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value);
} // namespace PhysicalClosedLimitLift

namespace PhysicalClosedLimitTilt {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value);
} // namespace PhysicalClosedLimitTilt

namespace CurrentPositionLift {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<uint16_t>& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<uint16_t>& value);
} // namespace CurrentPositionLift

namespace CurrentPositionTilt {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<uint16_t>& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<uint16_t>& value);
} // namespace CurrentPositionTilt

namespace NumberOfActuationsLift {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value);
} // namespace NumberOfActuationsLift

namespace NumberOfActuationsTilt {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value);
} // namespace NumberOfActuationsTilt

namespace ConfigStatus {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::BitMask<chip::app::Clusters::WindowCovering::ConfigStatus>& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::BitMask<chip::app::Clusters::WindowCovering::ConfigStatus>& value);
} // namespace ConfigStatus

namespace CurrentPositionLiftPercentage {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<chip::Percent>& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<chip::Percent>& value);
} // namespace CurrentPositionLiftPercentage

namespace CurrentPositionTiltPercentage {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<chip::Percent>& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<chip::Percent>& value);
} // namespace CurrentPositionTiltPercentage

namespace OperationalStatus {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::BitMask<chip::app::Clusters::WindowCovering::OperationalStatus>& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::BitMask<chip::app::Clusters::WindowCovering::OperationalStatus>& value);
} // namespace OperationalStatus

namespace TargetPositionLiftPercent100ths {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<chip::Percent100ths>& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<chip::Percent100ths>& value);
} // namespace TargetPositionLiftPercent100ths

namespace TargetPositionTiltPercent100ths {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<chip::Percent100ths>& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<chip::Percent100ths>& value);
} // namespace TargetPositionTiltPercent100ths

namespace EndProductType {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::Clusters::WindowCovering::EndProductType& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::Clusters::WindowCovering::EndProductType& value);
} // namespace EndProductType

namespace CurrentPositionLiftPercent100ths {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<chip::Percent100ths>& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<chip::Percent100ths>& value);
} // namespace CurrentPositionLiftPercent100ths

namespace CurrentPositionTiltPercent100ths {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<chip::Percent100ths>& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<chip::Percent100ths>& value);
} // namespace CurrentPositionTiltPercent100ths

namespace InstalledOpenLimitLift {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value);
} // namespace InstalledOpenLimitLift

namespace InstalledClosedLimitLift {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value);
} // namespace InstalledClosedLimitLift

namespace InstalledOpenLimitTilt {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value);
} // namespace InstalledOpenLimitTilt

namespace InstalledClosedLimitTilt {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value);
} // namespace InstalledClosedLimitTilt

namespace Mode {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::BitMask<chip::app::Clusters::WindowCovering::Mode>& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::BitMask<chip::app::Clusters::WindowCovering::Mode>& value);
} // namespace Mode

namespace SafetyStatus {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::BitMask<chip::app::Clusters::WindowCovering::SafetyStatus>& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::BitMask<chip::app::Clusters::WindowCovering::SafetyStatus>& value);
} // namespace SafetyStatus

namespace FeatureMap {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value);
} // namespace FeatureMap

namespace ClusterRevision {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value);
} // namespace ClusterRevision

} // namespace Attributes
} // WindowCovering

namespace BarrierControl {
namespace Attributes {

namespace BarrierMovingState {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value);
} // namespace BarrierMovingState

namespace BarrierSafetyStatus {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value);
} // namespace BarrierSafetyStatus

namespace BarrierCapabilities {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value);
} // namespace BarrierCapabilities

namespace BarrierOpenEvents {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value);
} // namespace BarrierOpenEvents

namespace BarrierCloseEvents {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value);
} // namespace BarrierCloseEvents

namespace BarrierCommandOpenEvents {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value);
} // namespace BarrierCommandOpenEvents

namespace BarrierCommandCloseEvents {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value);
} // namespace BarrierCommandCloseEvents

namespace BarrierOpenPeriod {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value);
} // namespace BarrierOpenPeriod

namespace BarrierClosePeriod {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value);
} // namespace BarrierClosePeriod

namespace BarrierPosition {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value);
} // namespace BarrierPosition

namespace FeatureMap {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value);
} // namespace FeatureMap

namespace ClusterRevision {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value);
} // namespace ClusterRevision

} // namespace Attributes
} // BarrierControl

namespace PumpConfigurationAndControl {
namespace Attributes {

namespace MaxPressure {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<int16_t>& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<int16_t>& value);
} // namespace MaxPressure

namespace MaxSpeed {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<uint16_t>& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<uint16_t>& value);
} // namespace MaxSpeed

namespace MaxFlow {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<uint16_t>& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<uint16_t>& value);
} // namespace MaxFlow

namespace MinConstPressure {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<int16_t>& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<int16_t>& value);
} // namespace MinConstPressure

namespace MaxConstPressure {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<int16_t>& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<int16_t>& value);
} // namespace MaxConstPressure

namespace MinCompPressure {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<int16_t>& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<int16_t>& value);
} // namespace MinCompPressure

namespace MaxCompPressure {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<int16_t>& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<int16_t>& value);
} // namespace MaxCompPressure

namespace MinConstSpeed {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<uint16_t>& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<uint16_t>& value);
} // namespace MinConstSpeed

namespace MaxConstSpeed {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<uint16_t>& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<uint16_t>& value);
} // namespace MaxConstSpeed

namespace MinConstFlow {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<uint16_t>& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<uint16_t>& value);
} // namespace MinConstFlow

namespace MaxConstFlow {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<uint16_t>& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<uint16_t>& value);
} // namespace MaxConstFlow

namespace MinConstTemp {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<int16_t>& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<int16_t>& value);
} // namespace MinConstTemp

namespace MaxConstTemp {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<int16_t>& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<int16_t>& value);
} // namespace MaxConstTemp

namespace PumpStatus {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::BitMask<chip::app::Clusters::PumpConfigurationAndControl::PumpStatus>& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::BitMask<chip::app::Clusters::PumpConfigurationAndControl::PumpStatus>& value);
} // namespace PumpStatus

namespace EffectiveOperationMode {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::Clusters::PumpConfigurationAndControl::PumpOperationMode& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::Clusters::PumpConfigurationAndControl::PumpOperationMode& value);
} // namespace EffectiveOperationMode

namespace EffectiveControlMode {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::Clusters::PumpConfigurationAndControl::PumpControlMode& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::Clusters::PumpConfigurationAndControl::PumpControlMode& value);
} // namespace EffectiveControlMode

namespace Capacity {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<int16_t>& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<int16_t>& value);
} // namespace Capacity

namespace Speed {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<uint16_t>& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<uint16_t>& value);
} // namespace Speed

namespace LifetimeRunningHours {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<uint32_t>& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<uint32_t>& value);
} // namespace LifetimeRunningHours

namespace Power {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<uint32_t>& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<uint32_t>& value);
} // namespace Power

namespace LifetimeEnergyConsumed {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<uint32_t>& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<uint32_t>& value);
} // namespace LifetimeEnergyConsumed

namespace OperationMode {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::Clusters::PumpConfigurationAndControl::PumpOperationMode& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::Clusters::PumpConfigurationAndControl::PumpOperationMode& value);
} // namespace OperationMode

namespace ControlMode {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::Clusters::PumpConfigurationAndControl::PumpControlMode& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::Clusters::PumpConfigurationAndControl::PumpControlMode& value);
} // namespace ControlMode

namespace FeatureMap {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value);
} // namespace FeatureMap

namespace ClusterRevision {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value);
} // namespace ClusterRevision

} // namespace Attributes
} // PumpConfigurationAndControl

namespace Thermostat {
namespace Attributes {

namespace LocalTemperature {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<int16_t>& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<int16_t>& value);
} // namespace LocalTemperature

namespace OutdoorTemperature {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<int16_t>& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<int16_t>& value);
} // namespace OutdoorTemperature

namespace Occupancy {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value);
} // namespace Occupancy

namespace AbsMinHeatSetpointLimit {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, int16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const int16_t& value);
} // namespace AbsMinHeatSetpointLimit

namespace AbsMaxHeatSetpointLimit {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, int16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const int16_t& value);
} // namespace AbsMaxHeatSetpointLimit

namespace AbsMinCoolSetpointLimit {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, int16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const int16_t& value);
} // namespace AbsMinCoolSetpointLimit

namespace AbsMaxCoolSetpointLimit {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, int16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const int16_t& value);
} // namespace AbsMaxCoolSetpointLimit

namespace PICoolingDemand {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value);
} // namespace PICoolingDemand

namespace PIHeatingDemand {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value);
} // namespace PIHeatingDemand

namespace HVACSystemTypeConfiguration {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value);
} // namespace HVACSystemTypeConfiguration

namespace LocalTemperatureCalibration {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, int8_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const int8_t& value);
} // namespace LocalTemperatureCalibration

namespace OccupiedCoolingSetpoint {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, int16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const int16_t& value);
} // namespace OccupiedCoolingSetpoint

namespace OccupiedHeatingSetpoint {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, int16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const int16_t& value);
} // namespace OccupiedHeatingSetpoint

namespace UnoccupiedCoolingSetpoint {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, int16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const int16_t& value);
} // namespace UnoccupiedCoolingSetpoint

namespace UnoccupiedHeatingSetpoint {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, int16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const int16_t& value);
} // namespace UnoccupiedHeatingSetpoint

namespace MinHeatSetpointLimit {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, int16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const int16_t& value);
} // namespace MinHeatSetpointLimit

namespace MaxHeatSetpointLimit {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, int16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const int16_t& value);
} // namespace MaxHeatSetpointLimit

namespace MinCoolSetpointLimit {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, int16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const int16_t& value);
} // namespace MinCoolSetpointLimit

namespace MaxCoolSetpointLimit {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, int16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const int16_t& value);
} // namespace MaxCoolSetpointLimit

namespace MinSetpointDeadBand {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, int8_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const int8_t& value);
} // namespace MinSetpointDeadBand

namespace RemoteSensing {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value);
} // namespace RemoteSensing

namespace ControlSequenceOfOperation {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::Clusters::Thermostat::ThermostatControlSequence& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::Clusters::Thermostat::ThermostatControlSequence& value);
} // namespace ControlSequenceOfOperation

namespace SystemMode {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value);
} // namespace SystemMode

namespace ThermostatRunningMode {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value);
} // namespace ThermostatRunningMode

namespace StartOfWeek {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value);
} // namespace StartOfWeek

namespace NumberOfWeeklyTransitions {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value);
} // namespace NumberOfWeeklyTransitions

namespace NumberOfDailyTransitions {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value);
} // namespace NumberOfDailyTransitions

namespace TemperatureSetpointHold {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value);
} // namespace TemperatureSetpointHold

namespace TemperatureSetpointHoldDuration {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<uint16_t>& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<uint16_t>& value);
} // namespace TemperatureSetpointHoldDuration

namespace ThermostatProgrammingOperationMode {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value);
} // namespace ThermostatProgrammingOperationMode

namespace ThermostatRunningState {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value);
} // namespace ThermostatRunningState

namespace SetpointChangeSource {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value);
} // namespace SetpointChangeSource

namespace SetpointChangeAmount {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<int16_t>& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<int16_t>& value);
} // namespace SetpointChangeAmount

namespace SetpointChangeSourceTimestamp {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value);
} // namespace SetpointChangeSourceTimestamp

namespace OccupiedSetback {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<uint8_t>& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<uint8_t>& value);
} // namespace OccupiedSetback

namespace OccupiedSetbackMin {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<uint8_t>& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<uint8_t>& value);
} // namespace OccupiedSetbackMin

namespace OccupiedSetbackMax {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<uint8_t>& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<uint8_t>& value);
} // namespace OccupiedSetbackMax

namespace UnoccupiedSetback {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<uint8_t>& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<uint8_t>& value);
} // namespace UnoccupiedSetback

namespace UnoccupiedSetbackMin {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<uint8_t>& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<uint8_t>& value);
} // namespace UnoccupiedSetbackMin

namespace UnoccupiedSetbackMax {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<uint8_t>& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<uint8_t>& value);
} // namespace UnoccupiedSetbackMax

namespace EmergencyHeatDelta {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value);
} // namespace EmergencyHeatDelta

namespace ACType {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value);
} // namespace ACType

namespace ACCapacity {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value);
} // namespace ACCapacity

namespace ACRefrigerantType {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value);
} // namespace ACRefrigerantType

namespace ACCompressorType {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value);
} // namespace ACCompressorType

namespace ACErrorCode {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value);
} // namespace ACErrorCode

namespace ACLouverPosition {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value);
} // namespace ACLouverPosition

namespace ACCoilTemperature {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<int16_t>& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<int16_t>& value);
} // namespace ACCoilTemperature

namespace ACCapacityformat {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value);
} // namespace ACCapacityformat

namespace FeatureMap {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value);
} // namespace FeatureMap

namespace ClusterRevision {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value);
} // namespace ClusterRevision

} // namespace Attributes
} // Thermostat

namespace FanControl {
namespace Attributes {

namespace FanMode {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::Clusters::FanControl::FanModeType& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::Clusters::FanControl::FanModeType& value);
} // namespace FanMode

namespace FanModeSequence {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::Clusters::FanControl::FanModeSequenceType& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::Clusters::FanControl::FanModeSequenceType& value);
} // namespace FanModeSequence

namespace PercentSetting {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<uint8_t>& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<uint8_t>& value);
} // namespace PercentSetting

namespace PercentCurrent {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value);
} // namespace PercentCurrent

namespace SpeedMax {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value);
} // namespace SpeedMax

namespace SpeedSetting {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<uint8_t>& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<uint8_t>& value);
} // namespace SpeedSetting

namespace SpeedCurrent {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value);
} // namespace SpeedCurrent

namespace RockSupport {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value);
} // namespace RockSupport

namespace RockSetting {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value);
} // namespace RockSetting

namespace WindSupport {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value);
} // namespace WindSupport

namespace WindSetting {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value);
} // namespace WindSetting

namespace FeatureMap {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value);
} // namespace FeatureMap

namespace ClusterRevision {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value);
} // namespace ClusterRevision

} // namespace Attributes
} // FanControl

namespace ThermostatUserInterfaceConfiguration {
namespace Attributes {

namespace TemperatureDisplayMode {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value);
} // namespace TemperatureDisplayMode

namespace KeypadLockout {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value);
} // namespace KeypadLockout

namespace ScheduleProgrammingVisibility {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value);
} // namespace ScheduleProgrammingVisibility

namespace FeatureMap {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value);
} // namespace FeatureMap

namespace ClusterRevision {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value);
} // namespace ClusterRevision

} // namespace Attributes
} // ThermostatUserInterfaceConfiguration

namespace ColorControl {
namespace Attributes {

namespace CurrentHue {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value);
} // namespace CurrentHue

namespace CurrentSaturation {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value);
} // namespace CurrentSaturation

namespace RemainingTime {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value);
} // namespace RemainingTime

namespace CurrentX {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value);
} // namespace CurrentX

namespace CurrentY {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value);
} // namespace CurrentY

namespace DriftCompensation {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value);
} // namespace DriftCompensation

namespace CompensationText {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::CharSpan& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::CharSpan& value);
} // namespace CompensationText

namespace ColorTemperatureMireds {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value);
} // namespace ColorTemperatureMireds

namespace ColorMode {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value);
} // namespace ColorMode

namespace Options {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value);
} // namespace Options

namespace NumberOfPrimaries {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<uint8_t>& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<uint8_t>& value);
} // namespace NumberOfPrimaries

namespace Primary1X {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value);
} // namespace Primary1X

namespace Primary1Y {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value);
} // namespace Primary1Y

namespace Primary1Intensity {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<uint8_t>& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<uint8_t>& value);
} // namespace Primary1Intensity

namespace Primary2X {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value);
} // namespace Primary2X

namespace Primary2Y {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value);
} // namespace Primary2Y

namespace Primary2Intensity {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<uint8_t>& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<uint8_t>& value);
} // namespace Primary2Intensity

namespace Primary3X {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value);
} // namespace Primary3X

namespace Primary3Y {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value);
} // namespace Primary3Y

namespace Primary3Intensity {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<uint8_t>& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<uint8_t>& value);
} // namespace Primary3Intensity

namespace Primary4X {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value);
} // namespace Primary4X

namespace Primary4Y {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value);
} // namespace Primary4Y

namespace Primary4Intensity {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<uint8_t>& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<uint8_t>& value);
} // namespace Primary4Intensity

namespace Primary5X {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value);
} // namespace Primary5X

namespace Primary5Y {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value);
} // namespace Primary5Y

namespace Primary5Intensity {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<uint8_t>& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<uint8_t>& value);
} // namespace Primary5Intensity

namespace Primary6X {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value);
} // namespace Primary6X

namespace Primary6Y {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value);
} // namespace Primary6Y

namespace Primary6Intensity {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<uint8_t>& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<uint8_t>& value);
} // namespace Primary6Intensity

namespace WhitePointX {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value);
} // namespace WhitePointX

namespace WhitePointY {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value);
} // namespace WhitePointY

namespace ColorPointRX {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value);
} // namespace ColorPointRX

namespace ColorPointRY {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value);
} // namespace ColorPointRY

namespace ColorPointRIntensity {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<uint8_t>& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<uint8_t>& value);
} // namespace ColorPointRIntensity

namespace ColorPointGX {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value);
} // namespace ColorPointGX

namespace ColorPointGY {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value);
} // namespace ColorPointGY

namespace ColorPointGIntensity {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<uint8_t>& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<uint8_t>& value);
} // namespace ColorPointGIntensity

namespace ColorPointBX {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value);
} // namespace ColorPointBX

namespace ColorPointBY {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value);
} // namespace ColorPointBY

namespace ColorPointBIntensity {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<uint8_t>& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<uint8_t>& value);
} // namespace ColorPointBIntensity

namespace EnhancedCurrentHue {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value);
} // namespace EnhancedCurrentHue

namespace EnhancedColorMode {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value);
} // namespace EnhancedColorMode

namespace ColorLoopActive {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value);
} // namespace ColorLoopActive

namespace ColorLoopDirection {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value);
} // namespace ColorLoopDirection

namespace ColorLoopTime {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value);
} // namespace ColorLoopTime

namespace ColorLoopStartEnhancedHue {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value);
} // namespace ColorLoopStartEnhancedHue

namespace ColorLoopStoredEnhancedHue {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value);
} // namespace ColorLoopStoredEnhancedHue

namespace ColorCapabilities {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value);
} // namespace ColorCapabilities

namespace ColorTempPhysicalMinMireds {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value);
} // namespace ColorTempPhysicalMinMireds

namespace ColorTempPhysicalMaxMireds {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value);
} // namespace ColorTempPhysicalMaxMireds

namespace CoupleColorTempToLevelMinMireds {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value);
} // namespace CoupleColorTempToLevelMinMireds

namespace StartUpColorTemperatureMireds {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<uint16_t>& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<uint16_t>& value);
} // namespace StartUpColorTemperatureMireds

namespace FeatureMap {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value);
} // namespace FeatureMap

namespace ClusterRevision {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value);
} // namespace ClusterRevision

} // namespace Attributes
} // ColorControl

namespace BallastConfiguration {
namespace Attributes {

namespace PhysicalMinLevel {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value);
} // namespace PhysicalMinLevel

namespace PhysicalMaxLevel {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value);
} // namespace PhysicalMaxLevel

namespace BallastStatus {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value);
} // namespace BallastStatus

namespace MinLevel {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value);
} // namespace MinLevel

namespace MaxLevel {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value);
} // namespace MaxLevel

namespace IntrinsicBalanceFactor {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<uint8_t>& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<uint8_t>& value);
} // namespace IntrinsicBalanceFactor

namespace BallastFactorAdjustment {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<uint8_t>& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<uint8_t>& value);
} // namespace BallastFactorAdjustment

namespace LampQuantity {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value);
} // namespace LampQuantity

namespace LampType {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::CharSpan& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::CharSpan& value);
} // namespace LampType

namespace LampManufacturer {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::CharSpan& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::CharSpan& value);
} // namespace LampManufacturer

namespace LampRatedHours {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<uint32_t>& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<uint32_t>& value);
} // namespace LampRatedHours

namespace LampBurnHours {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<uint32_t>& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<uint32_t>& value);
} // namespace LampBurnHours

namespace LampAlarmMode {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value);
} // namespace LampAlarmMode

namespace LampBurnHoursTripPoint {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<uint32_t>& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<uint32_t>& value);
} // namespace LampBurnHoursTripPoint

namespace FeatureMap {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value);
} // namespace FeatureMap

namespace ClusterRevision {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value);
} // namespace ClusterRevision

} // namespace Attributes
} // BallastConfiguration

namespace IlluminanceMeasurement {
namespace Attributes {

namespace MeasuredValue {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<uint16_t>& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<uint16_t>& value);
} // namespace MeasuredValue

namespace MinMeasuredValue {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<uint16_t>& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<uint16_t>& value);
} // namespace MinMeasuredValue

namespace MaxMeasuredValue {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<uint16_t>& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<uint16_t>& value);
} // namespace MaxMeasuredValue

namespace Tolerance {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value);
} // namespace Tolerance

namespace LightSensorType {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<uint8_t>& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<uint8_t>& value);
} // namespace LightSensorType

namespace FeatureMap {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value);
} // namespace FeatureMap

namespace ClusterRevision {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value);
} // namespace ClusterRevision

} // namespace Attributes
} // IlluminanceMeasurement

namespace TemperatureMeasurement {
namespace Attributes {

namespace MeasuredValue {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<int16_t>& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<int16_t>& value);
} // namespace MeasuredValue

namespace MinMeasuredValue {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<int16_t>& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<int16_t>& value);
} // namespace MinMeasuredValue

namespace MaxMeasuredValue {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<int16_t>& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<int16_t>& value);
} // namespace MaxMeasuredValue

namespace Tolerance {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value);
} // namespace Tolerance

namespace FeatureMap {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value);
} // namespace FeatureMap

namespace ClusterRevision {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value);
} // namespace ClusterRevision

} // namespace Attributes
} // TemperatureMeasurement

namespace PressureMeasurement {
namespace Attributes {

namespace MeasuredValue {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<int16_t>& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<int16_t>& value);
} // namespace MeasuredValue

namespace MinMeasuredValue {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<int16_t>& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<int16_t>& value);
} // namespace MinMeasuredValue

namespace MaxMeasuredValue {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<int16_t>& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<int16_t>& value);
} // namespace MaxMeasuredValue

namespace Tolerance {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value);
} // namespace Tolerance

namespace ScaledValue {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<int16_t>& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<int16_t>& value);
} // namespace ScaledValue

namespace MinScaledValue {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<int16_t>& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<int16_t>& value);
} // namespace MinScaledValue

namespace MaxScaledValue {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<int16_t>& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<int16_t>& value);
} // namespace MaxScaledValue

namespace ScaledTolerance {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value);
} // namespace ScaledTolerance

namespace Scale {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, int8_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const int8_t& value);
} // namespace Scale

namespace FeatureMap {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value);
} // namespace FeatureMap

namespace ClusterRevision {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value);
} // namespace ClusterRevision

} // namespace Attributes
} // PressureMeasurement

namespace FlowMeasurement {
namespace Attributes {

namespace MeasuredValue {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<uint16_t>& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<uint16_t>& value);
} // namespace MeasuredValue

namespace MinMeasuredValue {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<uint16_t>& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<uint16_t>& value);
} // namespace MinMeasuredValue

namespace MaxMeasuredValue {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<uint16_t>& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<uint16_t>& value);
} // namespace MaxMeasuredValue

namespace Tolerance {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value);
} // namespace Tolerance

namespace FeatureMap {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value);
} // namespace FeatureMap

namespace ClusterRevision {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value);
} // namespace ClusterRevision

} // namespace Attributes
} // FlowMeasurement

namespace RelativeHumidityMeasurement {
namespace Attributes {

namespace MeasuredValue {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<uint16_t>& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<uint16_t>& value);
} // namespace MeasuredValue

namespace MinMeasuredValue {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<uint16_t>& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<uint16_t>& value);
} // namespace MinMeasuredValue

namespace MaxMeasuredValue {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<uint16_t>& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<uint16_t>& value);
} // namespace MaxMeasuredValue

namespace Tolerance {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value);
} // namespace Tolerance

namespace FeatureMap {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value);
} // namespace FeatureMap

namespace ClusterRevision {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value);
} // namespace ClusterRevision

} // namespace Attributes
} // RelativeHumidityMeasurement

namespace OccupancySensing {
namespace Attributes {

namespace Occupancy {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value);
} // namespace Occupancy

namespace OccupancySensorType {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value);
} // namespace OccupancySensorType

namespace OccupancySensorTypeBitmap {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value);
} // namespace OccupancySensorTypeBitmap

namespace PirOccupiedToUnoccupiedDelay {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value);
} // namespace PirOccupiedToUnoccupiedDelay

namespace PirUnoccupiedToOccupiedDelay {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value);
} // namespace PirUnoccupiedToOccupiedDelay

namespace PirUnoccupiedToOccupiedThreshold {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value);
} // namespace PirUnoccupiedToOccupiedThreshold

namespace UltrasonicOccupiedToUnoccupiedDelay {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value);
} // namespace UltrasonicOccupiedToUnoccupiedDelay

namespace UltrasonicUnoccupiedToOccupiedDelay {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value);
} // namespace UltrasonicUnoccupiedToOccupiedDelay

namespace UltrasonicUnoccupiedToOccupiedThreshold {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value);
} // namespace UltrasonicUnoccupiedToOccupiedThreshold

namespace PhysicalContactOccupiedToUnoccupiedDelay {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value);
} // namespace PhysicalContactOccupiedToUnoccupiedDelay

namespace PhysicalContactUnoccupiedToOccupiedDelay {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value);
} // namespace PhysicalContactUnoccupiedToOccupiedDelay

namespace PhysicalContactUnoccupiedToOccupiedThreshold {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value);
} // namespace PhysicalContactUnoccupiedToOccupiedThreshold

namespace FeatureMap {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value);
} // namespace FeatureMap

namespace ClusterRevision {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value);
} // namespace ClusterRevision

} // namespace Attributes
} // OccupancySensing

namespace WakeOnLan {
namespace Attributes {

namespace MACAddress {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::CharSpan& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::CharSpan& value);
} // namespace MACAddress

namespace FeatureMap {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value);
} // namespace FeatureMap

namespace ClusterRevision {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value);
} // namespace ClusterRevision

} // namespace Attributes
} // WakeOnLan

namespace Channel {
namespace Attributes {

namespace FeatureMap {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value);
} // namespace FeatureMap

namespace ClusterRevision {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value);
} // namespace ClusterRevision

} // namespace Attributes
} // Channel

namespace TargetNavigator {
namespace Attributes {

namespace CurrentTarget {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value);
} // namespace CurrentTarget

namespace FeatureMap {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value);
} // namespace FeatureMap

namespace ClusterRevision {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value);
} // namespace ClusterRevision

} // namespace Attributes
} // TargetNavigator

namespace MediaPlayback {
namespace Attributes {

namespace CurrentState {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::Clusters::MediaPlayback::PlaybackStateEnum& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::Clusters::MediaPlayback::PlaybackStateEnum& value);
} // namespace CurrentState

namespace StartTime {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<uint64_t>& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<uint64_t>& value);
} // namespace StartTime

namespace Duration {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<uint64_t>& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<uint64_t>& value);
} // namespace Duration

namespace PlaybackSpeed {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, float& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const float& value);
} // namespace PlaybackSpeed

namespace SeekRangeEnd {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<uint64_t>& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<uint64_t>& value);
} // namespace SeekRangeEnd

namespace SeekRangeStart {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<uint64_t>& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<uint64_t>& value);
} // namespace SeekRangeStart

namespace FeatureMap {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value);
} // namespace FeatureMap

namespace ClusterRevision {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value);
} // namespace ClusterRevision

} // namespace Attributes
} // MediaPlayback

namespace MediaInput {
namespace Attributes {

namespace CurrentInput {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value);
} // namespace CurrentInput

namespace FeatureMap {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value);
} // namespace FeatureMap

namespace ClusterRevision {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value);
} // namespace ClusterRevision

} // namespace Attributes
} // MediaInput

namespace LowPower {
namespace Attributes {

namespace FeatureMap {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value);
} // namespace FeatureMap

namespace ClusterRevision {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value);
} // namespace ClusterRevision

} // namespace Attributes
} // LowPower

namespace KeypadInput {
namespace Attributes {

namespace FeatureMap {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value);
} // namespace FeatureMap

namespace ClusterRevision {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value);
} // namespace ClusterRevision

} // namespace Attributes
} // KeypadInput

namespace ContentLauncher {
namespace Attributes {

namespace SupportedStreamingProtocols {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value);
} // namespace SupportedStreamingProtocols

namespace FeatureMap {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value);
} // namespace FeatureMap

namespace ClusterRevision {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value);
} // namespace ClusterRevision

} // namespace Attributes
} // ContentLauncher

namespace AudioOutput {
namespace Attributes {

namespace CurrentOutput {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value);
} // namespace CurrentOutput

namespace FeatureMap {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value);
} // namespace FeatureMap

namespace ClusterRevision {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value);
} // namespace ClusterRevision

} // namespace Attributes
} // AudioOutput

namespace ApplicationLauncher {
namespace Attributes {

namespace FeatureMap {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value);
} // namespace FeatureMap

namespace ClusterRevision {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value);
} // namespace ClusterRevision

} // namespace Attributes
} // ApplicationLauncher

namespace ApplicationBasic {
namespace Attributes {

namespace VendorName {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::CharSpan& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::CharSpan& value);
} // namespace VendorName

namespace VendorID {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::VendorId& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::VendorId& value);
} // namespace VendorID

namespace ApplicationName {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::CharSpan& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::CharSpan& value);
} // namespace ApplicationName

namespace ProductID {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value);
} // namespace ProductID

namespace Status {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::Clusters::ApplicationBasic::ApplicationStatusEnum& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::Clusters::ApplicationBasic::ApplicationStatusEnum& value);
} // namespace Status

namespace ApplicationVersion {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::CharSpan& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::CharSpan& value);
} // namespace ApplicationVersion

namespace FeatureMap {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value);
} // namespace FeatureMap

namespace ClusterRevision {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value);
} // namespace ClusterRevision

} // namespace Attributes
} // ApplicationBasic

namespace AccountLogin {
namespace Attributes {

namespace FeatureMap {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value);
} // namespace FeatureMap

namespace ClusterRevision {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value);
} // namespace ClusterRevision

} // namespace Attributes
} // AccountLogin

namespace ElectricalMeasurement {
namespace Attributes {

namespace MeasurementType {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value);
} // namespace MeasurementType

namespace DcVoltage {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, int16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const int16_t& value);
} // namespace DcVoltage

namespace DcVoltageMin {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, int16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const int16_t& value);
} // namespace DcVoltageMin

namespace DcVoltageMax {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, int16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const int16_t& value);
} // namespace DcVoltageMax

namespace DcCurrent {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, int16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const int16_t& value);
} // namespace DcCurrent

namespace DcCurrentMin {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, int16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const int16_t& value);
} // namespace DcCurrentMin

namespace DcCurrentMax {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, int16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const int16_t& value);
} // namespace DcCurrentMax

namespace DcPower {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, int16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const int16_t& value);
} // namespace DcPower

namespace DcPowerMin {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, int16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const int16_t& value);
} // namespace DcPowerMin

namespace DcPowerMax {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, int16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const int16_t& value);
} // namespace DcPowerMax

namespace DcVoltageMultiplier {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value);
} // namespace DcVoltageMultiplier

namespace DcVoltageDivisor {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value);
} // namespace DcVoltageDivisor

namespace DcCurrentMultiplier {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value);
} // namespace DcCurrentMultiplier

namespace DcCurrentDivisor {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value);
} // namespace DcCurrentDivisor

namespace DcPowerMultiplier {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value);
} // namespace DcPowerMultiplier

namespace DcPowerDivisor {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value);
} // namespace DcPowerDivisor

namespace AcFrequency {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value);
} // namespace AcFrequency

namespace AcFrequencyMin {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value);
} // namespace AcFrequencyMin

namespace AcFrequencyMax {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value);
} // namespace AcFrequencyMax

namespace NeutralCurrent {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value);
} // namespace NeutralCurrent

namespace TotalActivePower {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, int32_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const int32_t& value);
} // namespace TotalActivePower

namespace TotalReactivePower {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, int32_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const int32_t& value);
} // namespace TotalReactivePower

namespace TotalApparentPower {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value);
} // namespace TotalApparentPower

namespace Measured1stHarmonicCurrent {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, int16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const int16_t& value);
} // namespace Measured1stHarmonicCurrent

namespace Measured3rdHarmonicCurrent {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, int16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const int16_t& value);
} // namespace Measured3rdHarmonicCurrent

namespace Measured5thHarmonicCurrent {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, int16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const int16_t& value);
} // namespace Measured5thHarmonicCurrent

namespace Measured7thHarmonicCurrent {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, int16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const int16_t& value);
} // namespace Measured7thHarmonicCurrent

namespace Measured9thHarmonicCurrent {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, int16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const int16_t& value);
} // namespace Measured9thHarmonicCurrent

namespace Measured11thHarmonicCurrent {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, int16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const int16_t& value);
} // namespace Measured11thHarmonicCurrent

namespace MeasuredPhase1stHarmonicCurrent {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, int16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const int16_t& value);
} // namespace MeasuredPhase1stHarmonicCurrent

namespace MeasuredPhase3rdHarmonicCurrent {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, int16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const int16_t& value);
} // namespace MeasuredPhase3rdHarmonicCurrent

namespace MeasuredPhase5thHarmonicCurrent {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, int16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const int16_t& value);
} // namespace MeasuredPhase5thHarmonicCurrent

namespace MeasuredPhase7thHarmonicCurrent {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, int16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const int16_t& value);
} // namespace MeasuredPhase7thHarmonicCurrent

namespace MeasuredPhase9thHarmonicCurrent {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, int16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const int16_t& value);
} // namespace MeasuredPhase9thHarmonicCurrent

namespace MeasuredPhase11thHarmonicCurrent {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, int16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const int16_t& value);
} // namespace MeasuredPhase11thHarmonicCurrent

namespace AcFrequencyMultiplier {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value);
} // namespace AcFrequencyMultiplier

namespace AcFrequencyDivisor {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value);
} // namespace AcFrequencyDivisor

namespace PowerMultiplier {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value);
} // namespace PowerMultiplier

namespace PowerDivisor {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value);
} // namespace PowerDivisor

namespace HarmonicCurrentMultiplier {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, int8_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const int8_t& value);
} // namespace HarmonicCurrentMultiplier

namespace PhaseHarmonicCurrentMultiplier {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, int8_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const int8_t& value);
} // namespace PhaseHarmonicCurrentMultiplier

namespace InstantaneousVoltage {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, int16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const int16_t& value);
} // namespace InstantaneousVoltage

namespace InstantaneousLineCurrent {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value);
} // namespace InstantaneousLineCurrent

namespace InstantaneousActiveCurrent {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, int16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const int16_t& value);
} // namespace InstantaneousActiveCurrent

namespace InstantaneousReactiveCurrent {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, int16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const int16_t& value);
} // namespace InstantaneousReactiveCurrent

namespace InstantaneousPower {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, int16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const int16_t& value);
} // namespace InstantaneousPower

namespace RmsVoltage {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value);
} // namespace RmsVoltage

namespace RmsVoltageMin {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value);
} // namespace RmsVoltageMin

namespace RmsVoltageMax {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value);
} // namespace RmsVoltageMax

namespace RmsCurrent {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value);
} // namespace RmsCurrent

namespace RmsCurrentMin {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value);
} // namespace RmsCurrentMin

namespace RmsCurrentMax {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value);
} // namespace RmsCurrentMax

namespace ActivePower {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, int16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const int16_t& value);
} // namespace ActivePower

namespace ActivePowerMin {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, int16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const int16_t& value);
} // namespace ActivePowerMin

namespace ActivePowerMax {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, int16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const int16_t& value);
} // namespace ActivePowerMax

namespace ReactivePower {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, int16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const int16_t& value);
} // namespace ReactivePower

namespace ApparentPower {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value);
} // namespace ApparentPower

namespace PowerFactor {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, int8_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const int8_t& value);
} // namespace PowerFactor

namespace AverageRmsVoltageMeasurementPeriod {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value);
} // namespace AverageRmsVoltageMeasurementPeriod

namespace AverageRmsUnderVoltageCounter {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value);
} // namespace AverageRmsUnderVoltageCounter

namespace RmsExtremeOverVoltagePeriod {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value);
} // namespace RmsExtremeOverVoltagePeriod

namespace RmsExtremeUnderVoltagePeriod {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value);
} // namespace RmsExtremeUnderVoltagePeriod

namespace RmsVoltageSagPeriod {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value);
} // namespace RmsVoltageSagPeriod

namespace RmsVoltageSwellPeriod {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value);
} // namespace RmsVoltageSwellPeriod

namespace AcVoltageMultiplier {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value);
} // namespace AcVoltageMultiplier

namespace AcVoltageDivisor {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value);
} // namespace AcVoltageDivisor

namespace AcCurrentMultiplier {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value);
} // namespace AcCurrentMultiplier

namespace AcCurrentDivisor {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value);
} // namespace AcCurrentDivisor

namespace AcPowerMultiplier {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value);
} // namespace AcPowerMultiplier

namespace AcPowerDivisor {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value);
} // namespace AcPowerDivisor

namespace OverloadAlarmsMask {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value);
} // namespace OverloadAlarmsMask

namespace VoltageOverload {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, int16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const int16_t& value);
} // namespace VoltageOverload

namespace CurrentOverload {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, int16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const int16_t& value);
} // namespace CurrentOverload

namespace AcOverloadAlarmsMask {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value);
} // namespace AcOverloadAlarmsMask

namespace AcVoltageOverload {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, int16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const int16_t& value);
} // namespace AcVoltageOverload

namespace AcCurrentOverload {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, int16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const int16_t& value);
} // namespace AcCurrentOverload

namespace AcActivePowerOverload {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, int16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const int16_t& value);
} // namespace AcActivePowerOverload

namespace AcReactivePowerOverload {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, int16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const int16_t& value);
} // namespace AcReactivePowerOverload

namespace AverageRmsOverVoltage {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, int16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const int16_t& value);
} // namespace AverageRmsOverVoltage

namespace AverageRmsUnderVoltage {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, int16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const int16_t& value);
} // namespace AverageRmsUnderVoltage

namespace RmsExtremeOverVoltage {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, int16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const int16_t& value);
} // namespace RmsExtremeOverVoltage

namespace RmsExtremeUnderVoltage {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, int16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const int16_t& value);
} // namespace RmsExtremeUnderVoltage

namespace RmsVoltageSag {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, int16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const int16_t& value);
} // namespace RmsVoltageSag

namespace RmsVoltageSwell {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, int16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const int16_t& value);
} // namespace RmsVoltageSwell

namespace LineCurrentPhaseB {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value);
} // namespace LineCurrentPhaseB

namespace ActiveCurrentPhaseB {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, int16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const int16_t& value);
} // namespace ActiveCurrentPhaseB

namespace ReactiveCurrentPhaseB {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, int16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const int16_t& value);
} // namespace ReactiveCurrentPhaseB

namespace RmsVoltagePhaseB {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value);
} // namespace RmsVoltagePhaseB

namespace RmsVoltageMinPhaseB {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value);
} // namespace RmsVoltageMinPhaseB

namespace RmsVoltageMaxPhaseB {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value);
} // namespace RmsVoltageMaxPhaseB

namespace RmsCurrentPhaseB {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value);
} // namespace RmsCurrentPhaseB

namespace RmsCurrentMinPhaseB {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value);
} // namespace RmsCurrentMinPhaseB

namespace RmsCurrentMaxPhaseB {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value);
} // namespace RmsCurrentMaxPhaseB

namespace ActivePowerPhaseB {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, int16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const int16_t& value);
} // namespace ActivePowerPhaseB

namespace ActivePowerMinPhaseB {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, int16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const int16_t& value);
} // namespace ActivePowerMinPhaseB

namespace ActivePowerMaxPhaseB {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, int16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const int16_t& value);
} // namespace ActivePowerMaxPhaseB

namespace ReactivePowerPhaseB {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, int16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const int16_t& value);
} // namespace ReactivePowerPhaseB

namespace ApparentPowerPhaseB {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value);
} // namespace ApparentPowerPhaseB

namespace PowerFactorPhaseB {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, int8_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const int8_t& value);
} // namespace PowerFactorPhaseB

namespace AverageRmsVoltageMeasurementPeriodPhaseB {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value);
} // namespace AverageRmsVoltageMeasurementPeriodPhaseB

namespace AverageRmsOverVoltageCounterPhaseB {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value);
} // namespace AverageRmsOverVoltageCounterPhaseB

namespace AverageRmsUnderVoltageCounterPhaseB {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value);
} // namespace AverageRmsUnderVoltageCounterPhaseB

namespace RmsExtremeOverVoltagePeriodPhaseB {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value);
} // namespace RmsExtremeOverVoltagePeriodPhaseB

namespace RmsExtremeUnderVoltagePeriodPhaseB {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value);
} // namespace RmsExtremeUnderVoltagePeriodPhaseB

namespace RmsVoltageSagPeriodPhaseB {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value);
} // namespace RmsVoltageSagPeriodPhaseB

namespace RmsVoltageSwellPeriodPhaseB {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value);
} // namespace RmsVoltageSwellPeriodPhaseB

namespace LineCurrentPhaseC {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value);
} // namespace LineCurrentPhaseC

namespace ActiveCurrentPhaseC {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, int16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const int16_t& value);
} // namespace ActiveCurrentPhaseC

namespace ReactiveCurrentPhaseC {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, int16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const int16_t& value);
} // namespace ReactiveCurrentPhaseC

namespace RmsVoltagePhaseC {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value);
} // namespace RmsVoltagePhaseC

namespace RmsVoltageMinPhaseC {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value);
} // namespace RmsVoltageMinPhaseC

namespace RmsVoltageMaxPhaseC {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value);
} // namespace RmsVoltageMaxPhaseC

namespace RmsCurrentPhaseC {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value);
} // namespace RmsCurrentPhaseC

namespace RmsCurrentMinPhaseC {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value);
} // namespace RmsCurrentMinPhaseC

namespace RmsCurrentMaxPhaseC {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value);
} // namespace RmsCurrentMaxPhaseC

namespace ActivePowerPhaseC {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, int16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const int16_t& value);
} // namespace ActivePowerPhaseC

namespace ActivePowerMinPhaseC {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, int16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const int16_t& value);
} // namespace ActivePowerMinPhaseC

namespace ActivePowerMaxPhaseC {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, int16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const int16_t& value);
} // namespace ActivePowerMaxPhaseC

namespace ReactivePowerPhaseC {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, int16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const int16_t& value);
} // namespace ReactivePowerPhaseC

namespace ApparentPowerPhaseC {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value);
} // namespace ApparentPowerPhaseC

namespace PowerFactorPhaseC {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, int8_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const int8_t& value);
} // namespace PowerFactorPhaseC

namespace AverageRmsVoltageMeasurementPeriodPhaseC {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value);
} // namespace AverageRmsVoltageMeasurementPeriodPhaseC

namespace AverageRmsOverVoltageCounterPhaseC {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value);
} // namespace AverageRmsOverVoltageCounterPhaseC

namespace AverageRmsUnderVoltageCounterPhaseC {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value);
} // namespace AverageRmsUnderVoltageCounterPhaseC

namespace RmsExtremeOverVoltagePeriodPhaseC {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value);
} // namespace RmsExtremeOverVoltagePeriodPhaseC

namespace RmsExtremeUnderVoltagePeriodPhaseC {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value);
} // namespace RmsExtremeUnderVoltagePeriodPhaseC

namespace RmsVoltageSagPeriodPhaseC {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value);
} // namespace RmsVoltageSagPeriodPhaseC

namespace RmsVoltageSwellPeriodPhaseC {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value);
} // namespace RmsVoltageSwellPeriodPhaseC

namespace FeatureMap {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value);
} // namespace FeatureMap

namespace ClusterRevision {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value);
} // namespace ClusterRevision

} // namespace Attributes
} // ElectricalMeasurement

namespace TestCluster {
namespace Attributes {

namespace Boolean {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, bool& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const bool& value);
} // namespace Boolean

namespace Bitmap8 {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::BitMask<chip::app::Clusters::TestCluster::Bitmap8MaskMap>& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::BitMask<chip::app::Clusters::TestCluster::Bitmap8MaskMap>& value);
} // namespace Bitmap8

namespace Bitmap16 {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::BitMask<chip::app::Clusters::TestCluster::Bitmap16MaskMap>& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::BitMask<chip::app::Clusters::TestCluster::Bitmap16MaskMap>& value);
} // namespace Bitmap16

namespace Bitmap32 {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::BitMask<chip::app::Clusters::TestCluster::Bitmap32MaskMap>& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::BitMask<chip::app::Clusters::TestCluster::Bitmap32MaskMap>& value);
} // namespace Bitmap32

namespace Bitmap64 {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::BitMask<chip::app::Clusters::TestCluster::Bitmap64MaskMap>& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::BitMask<chip::app::Clusters::TestCluster::Bitmap64MaskMap>& value);
} // namespace Bitmap64

namespace Int8u {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value);
} // namespace Int8u

namespace Int16u {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value);
} // namespace Int16u

namespace Int24u {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value);
} // namespace Int24u

namespace Int32u {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value);
} // namespace Int32u

namespace Int40u {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint64_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint64_t& value);
} // namespace Int40u

namespace Int48u {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint64_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint64_t& value);
} // namespace Int48u

namespace Int56u {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint64_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint64_t& value);
} // namespace Int56u

namespace Int64u {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint64_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint64_t& value);
} // namespace Int64u

namespace Int8s {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, int8_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const int8_t& value);
} // namespace Int8s

namespace Int16s {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, int16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const int16_t& value);
} // namespace Int16s

namespace Int24s {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, int32_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const int32_t& value);
} // namespace Int24s

namespace Int32s {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, int32_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const int32_t& value);
} // namespace Int32s

namespace Int40s {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, int64_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const int64_t& value);
} // namespace Int40s

namespace Int48s {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, int64_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const int64_t& value);
} // namespace Int48s

namespace Int56s {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, int64_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const int64_t& value);
} // namespace Int56s

namespace Int64s {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, int64_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const int64_t& value);
} // namespace Int64s

namespace Enum8 {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value);
} // namespace Enum8

namespace Enum16 {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value);
} // namespace Enum16

namespace FloatSingle {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, float& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const float& value);
} // namespace FloatSingle

namespace FloatDouble {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, double& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const double& value);
} // namespace FloatDouble

namespace OctetString {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::ByteSpan& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::ByteSpan& value);
} // namespace OctetString

namespace LongOctetString {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::ByteSpan& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::ByteSpan& value);
} // namespace LongOctetString

namespace CharString {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::CharSpan& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::CharSpan& value);
} // namespace CharString

namespace LongCharString {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::CharSpan& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::CharSpan& value);
} // namespace LongCharString

namespace EpochUs {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint64_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint64_t& value);
} // namespace EpochUs

namespace EpochS {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value);
} // namespace EpochS

namespace VendorId {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::VendorId& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::VendorId& value);
} // namespace VendorId

namespace EnumAttr {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::Clusters::TestCluster::SimpleEnum& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::Clusters::TestCluster::SimpleEnum& value);
} // namespace EnumAttr

namespace RangeRestrictedInt8u {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value);
} // namespace RangeRestrictedInt8u

namespace RangeRestrictedInt8s {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, int8_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const int8_t& value);
} // namespace RangeRestrictedInt8s

namespace RangeRestrictedInt16u {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value);
} // namespace RangeRestrictedInt16u

namespace RangeRestrictedInt16s {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, int16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const int16_t& value);
} // namespace RangeRestrictedInt16s

namespace TimedWriteBoolean {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, bool& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const bool& value);
} // namespace TimedWriteBoolean

namespace GeneralErrorBoolean {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, bool& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const bool& value);
} // namespace GeneralErrorBoolean

namespace ClusterErrorBoolean {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, bool& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const bool& value);
} // namespace ClusterErrorBoolean

namespace Unsupported {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, bool& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const bool& value);
} // namespace Unsupported

namespace NullableBoolean {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<bool>& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<bool>& value);
} // namespace NullableBoolean

namespace NullableBitmap8 {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<chip::BitMask<chip::app::Clusters::TestCluster::Bitmap8MaskMap>>& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<chip::BitMask<chip::app::Clusters::TestCluster::Bitmap8MaskMap>>& value);
} // namespace NullableBitmap8

namespace NullableBitmap16 {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<chip::BitMask<chip::app::Clusters::TestCluster::Bitmap16MaskMap>>& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<chip::BitMask<chip::app::Clusters::TestCluster::Bitmap16MaskMap>>& value);
} // namespace NullableBitmap16

namespace NullableBitmap32 {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<chip::BitMask<chip::app::Clusters::TestCluster::Bitmap32MaskMap>>& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<chip::BitMask<chip::app::Clusters::TestCluster::Bitmap32MaskMap>>& value);
} // namespace NullableBitmap32

namespace NullableBitmap64 {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<chip::BitMask<chip::app::Clusters::TestCluster::Bitmap64MaskMap>>& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<chip::BitMask<chip::app::Clusters::TestCluster::Bitmap64MaskMap>>& value);
} // namespace NullableBitmap64

namespace NullableInt8u {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<uint8_t>& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<uint8_t>& value);
} // namespace NullableInt8u

namespace NullableInt16u {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<uint16_t>& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<uint16_t>& value);
} // namespace NullableInt16u

namespace NullableInt24u {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<uint32_t>& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<uint32_t>& value);
} // namespace NullableInt24u

namespace NullableInt32u {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<uint32_t>& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<uint32_t>& value);
} // namespace NullableInt32u

namespace NullableInt40u {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<uint64_t>& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<uint64_t>& value);
} // namespace NullableInt40u

namespace NullableInt48u {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<uint64_t>& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<uint64_t>& value);
} // namespace NullableInt48u

namespace NullableInt56u {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<uint64_t>& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<uint64_t>& value);
} // namespace NullableInt56u

namespace NullableInt64u {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<uint64_t>& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<uint64_t>& value);
} // namespace NullableInt64u

namespace NullableInt8s {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<int8_t>& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<int8_t>& value);
} // namespace NullableInt8s

namespace NullableInt16s {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<int16_t>& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<int16_t>& value);
} // namespace NullableInt16s

namespace NullableInt24s {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<int32_t>& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<int32_t>& value);
} // namespace NullableInt24s

namespace NullableInt32s {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<int32_t>& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<int32_t>& value);
} // namespace NullableInt32s

namespace NullableInt40s {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<int64_t>& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<int64_t>& value);
} // namespace NullableInt40s

namespace NullableInt48s {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<int64_t>& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<int64_t>& value);
} // namespace NullableInt48s

namespace NullableInt56s {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<int64_t>& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<int64_t>& value);
} // namespace NullableInt56s

namespace NullableInt64s {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<int64_t>& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<int64_t>& value);
} // namespace NullableInt64s

namespace NullableEnum8 {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<uint8_t>& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<uint8_t>& value);
} // namespace NullableEnum8

namespace NullableEnum16 {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<uint16_t>& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<uint16_t>& value);
} // namespace NullableEnum16

namespace NullableFloatSingle {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<float>& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<float>& value);
} // namespace NullableFloatSingle

namespace NullableFloatDouble {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<double>& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<double>& value);
} // namespace NullableFloatDouble

namespace NullableOctetString {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<chip::ByteSpan>& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<chip::ByteSpan>& value);
} // namespace NullableOctetString

namespace NullableCharString {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<chip::CharSpan>& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<chip::CharSpan>& value);
} // namespace NullableCharString

namespace NullableEnumAttr {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<chip::app::Clusters::TestCluster::SimpleEnum>& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<chip::app::Clusters::TestCluster::SimpleEnum>& value);
} // namespace NullableEnumAttr

namespace NullableRangeRestrictedInt8u {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<uint8_t>& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<uint8_t>& value);
} // namespace NullableRangeRestrictedInt8u

namespace NullableRangeRestrictedInt8s {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<int8_t>& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<int8_t>& value);
} // namespace NullableRangeRestrictedInt8s

namespace NullableRangeRestrictedInt16u {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<uint16_t>& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<uint16_t>& value);
} // namespace NullableRangeRestrictedInt16u

namespace NullableRangeRestrictedInt16s {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<int16_t>& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<int16_t>& value);
} // namespace NullableRangeRestrictedInt16s

namespace WriteOnlyInt8u {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value);
} // namespace WriteOnlyInt8u

namespace FeatureMap {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value);
} // namespace FeatureMap

namespace ClusterRevision {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value);
} // namespace ClusterRevision

} // namespace Attributes
} // TestCluster

namespace FaultInjection {
namespace Attributes {

namespace FeatureMap {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value);
} // namespace FeatureMap

namespace ClusterRevision {
EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value);
EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value);
} // namespace ClusterRevision

} // namespace Attributes
} // FaultInjection


} // matter_bridge
} // unify
