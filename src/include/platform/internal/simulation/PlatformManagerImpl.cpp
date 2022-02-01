/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

/**
 *    @file
 *          Provides an generic implementation of PlatformManager features
 *          for use on Simulated platforms.
 */

#pragma once

#include <platform/DeviceControlServer.h>
#include <platform/PlatformManager.h>
#include <platform/internal/CHIPDeviceLayerInternal.h>

#include <signal.h>
#include <unistd.h>

using namespace ::chip::app::Clusters;

namespace chip {
namespace DeviceLayer {

namespace {
void SignalHandler(int signum)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    ChipLogDetail(DeviceLayer, "Caught signal %d", signum);

    // The BootReason attribute SHALL indicate the reason for the Node’s most recent boot, the real usecase
    // for this attribute is embedded system. In simulation, we use different signals to tell the current
    // running process to terminate with different reasons.
    switch (signum)
    {
    case SIGINT:
        ConfigurationMgr().StoreBootReason(DiagnosticDataProvider::BootReasonType::SoftwareReset);
        err = CHIP_ERROR_REBOOT_SIGNAL_RECEIVED;
        break;
    case SIGHUP:
        ConfigurationMgr().StoreBootReason(DiagnosticDataProvider::BootReasonType::BrownOutReset);
        err = CHIP_ERROR_REBOOT_SIGNAL_RECEIVED;
        break;
    case SIGTERM:
        ConfigurationMgr().StoreBootReason(DiagnosticDataProvider::BootReasonType::PowerOnReboot);
        err = CHIP_ERROR_REBOOT_SIGNAL_RECEIVED;
        break;
    case SIGUSR1:
        ConfigurationMgr().StoreBootReason(DiagnosticDataProvider::BootReasonType::HardwareWatchdogReset);
        err = CHIP_ERROR_REBOOT_SIGNAL_RECEIVED;
        break;
    case SIGUSR2:
        ConfigurationMgr().StoreBootReason(DiagnosticDataProvider::BootReasonType::SoftwareWatchdogReset);
        err = CHIP_ERROR_REBOOT_SIGNAL_RECEIVED;
        break;
    case SIGTSTP:
        ConfigurationMgr().StoreBootReason(DiagnosticDataProvider::BootReasonType::SoftwareUpdateCompleted);
        err = CHIP_ERROR_REBOOT_SIGNAL_RECEIVED;
        break;
    case SIGTRAP:
        PlatformMgrImpl().HandleSoftwareFault(SoftwareDiagnostics::Events::SoftwareFault::Id);
        break;
    case SIGILL:
        PlatformMgrImpl().HandleGeneralFault(GeneralDiagnostics::Events::HardwareFaultChange::Id);
        break;
    case SIGALRM:
        PlatformMgrImpl().HandleGeneralFault(GeneralDiagnostics::Events::RadioFaultChange::Id);
        break;
    case SIGVTALRM:
        PlatformMgrImpl().HandleGeneralFault(GeneralDiagnostics::Events::NetworkFaultChange::Id);
        break;
    case SIGIO:
        PlatformMgrImpl().HandleSwitchEvent(Switch::Events::SwitchLatched::Id);
        break;
    default:
        break;
    }

    if (err == CHIP_ERROR_REBOOT_SIGNAL_RECEIVED)
    {
        PlatformMgr().Shutdown();
        exit(EXIT_FAILURE);
    }
}
} // namespace

void PlatformManagerImpl::SetupSignalHandlers()
{
    struct sigaction action;

    memset(&action, 0, sizeof(action));
    action.sa_handler = SignalHandler;
    sigaction(SIGINT, &action, NULL);
    sigaction(SIGHUP, &action, NULL);
    sigaction(SIGTERM, &action, NULL);
    sigaction(SIGUSR1, &action, NULL);
    sigaction(SIGUSR2, &action, NULL);
    sigaction(SIGTSTP, &action, NULL);
}

CHIP_ERROR PlatformManagerImpl::_SetUserLabelList(
    EndpointId endpoint, AttributeList<app::Clusters::UserLabel::Structs::LabelStruct::Type, kMaxUserLabels> & labelList)
{
    // TODO:: store the user labelList, and read back stored user labelList if it has been set. Add yaml test to verify this.
    return CHIP_NO_ERROR;
}

CHIP_ERROR
PlatformManagerImpl::_GetUserLabelList(
    EndpointId endpoint, AttributeList<app::Clusters::UserLabel::Structs::LabelStruct::Type, kMaxUserLabels> & labelList)
{
    // In simulation, return following hardcoded labelList on all endpoints.
    UserLabel::Structs::LabelStruct::Type room;
    UserLabel::Structs::LabelStruct::Type orientation;
    UserLabel::Structs::LabelStruct::Type floor;
    UserLabel::Structs::LabelStruct::Type direction;

    room.label = CharSpan::fromCharString("room");
    room.value = CharSpan::fromCharString("bedroom 2");

    orientation.label = CharSpan::fromCharString("orientation");
    orientation.value = CharSpan::fromCharString("North");

    floor.label = CharSpan::fromCharString("floor");
    floor.value = CharSpan::fromCharString("2");

    direction.label = CharSpan::fromCharString("direction");
    direction.value = CharSpan::fromCharString("up");

    labelList.add(room);
    labelList.add(orientation);
    labelList.add(floor);
    labelList.add(direction);

    return CHIP_NO_ERROR;
}

CHIP_ERROR PlatformManagerImpl::_GetFixedLabelList(
    EndpointId endpoint, AttributeList<app::Clusters::FixedLabel::Structs::LabelStruct::Type, kMaxFixedLabels> & labelList)
{
    // In simulation, return following hardcoded labelList on all endpoints.
    FixedLabel::Structs::LabelStruct::Type room;
    FixedLabel::Structs::LabelStruct::Type orientation;
    FixedLabel::Structs::LabelStruct::Type floor;
    FixedLabel::Structs::LabelStruct::Type direction;

    room.label = CharSpan::fromCharString("room");
    room.value = CharSpan::fromCharString("bedroom 2");

    orientation.label = CharSpan::fromCharString("orientation");
    orientation.value = CharSpan::fromCharString("North");

    floor.label = CharSpan::fromCharString("floor");
    floor.value = CharSpan::fromCharString("2");

    direction.label = CharSpan::fromCharString("direction");
    direction.value = CharSpan::fromCharString("up");

    labelList.add(room);
    labelList.add(orientation);
    labelList.add(floor);
    labelList.add(direction);

    return CHIP_NO_ERROR;
}

CHIP_ERROR PlatformManagerImpl::_GetSupportedLocales(AttributeList<chip::CharSpan, kMaxLanguageTags> & supportedLocales)
{
    supportedLocales.add(CharSpan::fromCharString("en-US"));
    supportedLocales.add(CharSpan::fromCharString("de-DE"));
    supportedLocales.add(CharSpan::fromCharString("fr-FR"));
    supportedLocales.add(CharSpan::fromCharString("en-GB"));
    supportedLocales.add(CharSpan::fromCharString("es-ES"));
    supportedLocales.add(CharSpan::fromCharString("zh-CN"));
    supportedLocales.add(CharSpan::fromCharString("it-IT"));
    supportedLocales.add(CharSpan::fromCharString("ja-JP"));

    return CHIP_NO_ERROR;
}

CHIP_ERROR PlatformManagerImpl::_GetSupportedCalendarTypes(
    AttributeList<app::Clusters::TimeFormatLocalization::CalendarType, kMaxCalendarTypes> & supportedCalendarTypes)
{
    supportedCalendarTypes.add(app::Clusters::TimeFormatLocalization::CalendarType::kBuddhist);
    supportedCalendarTypes.add(app::Clusters::TimeFormatLocalization::CalendarType::kChinese);
    supportedCalendarTypes.add(app::Clusters::TimeFormatLocalization::CalendarType::kCoptic);
    supportedCalendarTypes.add(app::Clusters::TimeFormatLocalization::CalendarType::kEthiopian);
    supportedCalendarTypes.add(app::Clusters::TimeFormatLocalization::CalendarType::kGregorian);
    supportedCalendarTypes.add(app::Clusters::TimeFormatLocalization::CalendarType::kHebrew);
    supportedCalendarTypes.add(app::Clusters::TimeFormatLocalization::CalendarType::kIndian);
    supportedCalendarTypes.add(app::Clusters::TimeFormatLocalization::CalendarType::kIslamic);
    supportedCalendarTypes.add(app::Clusters::TimeFormatLocalization::CalendarType::kJapanese);
    supportedCalendarTypes.add(app::Clusters::TimeFormatLocalization::CalendarType::kKorean);
    supportedCalendarTypes.add(app::Clusters::TimeFormatLocalization::CalendarType::kPersian);
    supportedCalendarTypes.add(app::Clusters::TimeFormatLocalization::CalendarType::kTaiwanese);

    return CHIP_NO_ERROR;
}

void PlatformManagerImpl::HandleSoftwareFault(uint32_t EventId)
{
    SoftwareDiagnosticsDelegate * delegate = GetDiagnosticDataProvider().GetSoftwareDiagnosticsDelegate();

    if (delegate != nullptr)
    {
        SoftwareDiagnostics::Structs::SoftwareFaultStruct::Type softwareFault;
        char threadName[kMaxThreadNameLength + 1];

#ifdef __APPLE__
        uint64_t ktid;
        pthread_threadid_np(NULL, &ktid);
        softwareFault.id = ktid;
#else
        softwareFault.id = gettid();
#endif
        snprintf(threadName, kMaxThreadNameLength, "0x%016" PRIx64, softwareFault.id);
        softwareFault.name           = CharSpan::fromCharString(threadName);
        softwareFault.faultRecording = ByteSpan(Uint8::from_const_char("FaultRecording"), strlen("FaultRecording"));

        delegate->OnSoftwareFaultDetected(softwareFault);
    }
}

void PlatformManagerImpl::HandleSwitchEvent(uint32_t EventId)
{
    SwitchDeviceControlDelegate * delegate = DeviceControlServer::DeviceControlSvr().GetSwitchDelegate();

    if (delegate == nullptr)
    {
        ChipLogError(DeviceLayer, "No delegate registered to handle Switch event");
        return;
    }

    if (EventId == Switch::Events::SwitchLatched::Id)
    {
        uint8_t newPosition = 0;

#if CHIP_CONFIG_TEST
        newPosition = 100;
#endif
        delegate->OnSwitchLatched(newPosition);
    }
    else if (EventId == Switch::Events::InitialPress::Id)
    {
        uint8_t newPosition = 0;

#if CHIP_CONFIG_TEST
        newPosition = 100;
#endif
        delegate->OnInitialPressed(newPosition);
    }
    else if (EventId == Switch::Events::LongPress::Id)
    {
        uint8_t newPosition = 0;

#if CHIP_CONFIG_TEST
        newPosition = 100;
#endif
        delegate->OnLongPressed(newPosition);
    }
    else if (EventId == Switch::Events::ShortRelease::Id)
    {
        uint8_t previousPosition = 0;

#if CHIP_CONFIG_TEST
        previousPosition = 50;
#endif
        delegate->OnShortReleased(previousPosition);
    }
    else if (EventId == Switch::Events::LongRelease::Id)
    {
        uint8_t previousPosition = 0;

#if CHIP_CONFIG_TEST
        previousPosition = 50;
#endif
        delegate->OnLongReleased(previousPosition);
    }
    else if (EventId == Switch::Events::MultiPressOngoing::Id)
    {
        uint8_t newPosition                   = 0;
        uint8_t currentNumberOfPressesCounted = 0;

#if CHIP_CONFIG_TEST
        newPosition                   = 10;
        currentNumberOfPressesCounted = 5;
#endif
        delegate->OnMultiPressOngoing(newPosition, currentNumberOfPressesCounted);
    }
    else if (EventId == Switch::Events::MultiPressComplete::Id)
    {
        uint8_t newPosition                 = 0;
        uint8_t totalNumberOfPressesCounted = 0;

#if CHIP_CONFIG_TEST
        newPosition                 = 10;
        totalNumberOfPressesCounted = 5;
#endif
        delegate->OnMultiPressComplete(newPosition, totalNumberOfPressesCounted);
    }
    else
    {
        ChipLogError(DeviceLayer, "Unknow event ID:%d", EventId);
    }
}

void PlatformManagerImpl::HandleGeneralFault(uint32_t EventId)
{
    GeneralDiagnosticsDelegate * delegate = GetDiagnosticDataProvider().GetGeneralDiagnosticsDelegate();

    if (delegate == nullptr)
    {
        ChipLogError(DeviceLayer, "No delegate registered to handle General Diagnostics event");
        return;
    }

    if (EventId == GeneralDiagnostics::Events::HardwareFaultChange::Id)
    {
        GeneralFaults<kMaxHardwareFaults> previous;
        GeneralFaults<kMaxHardwareFaults> current;

#if CHIP_CONFIG_TEST
        // On Simulation, set following hardware faults statically.
        ReturnOnFailure(previous.add(EMBER_ZCL_HARDWARE_FAULT_TYPE_RADIO));
        ReturnOnFailure(previous.add(EMBER_ZCL_HARDWARE_FAULT_TYPE_POWER_SOURCE));

        ReturnOnFailure(current.add(EMBER_ZCL_HARDWARE_FAULT_TYPE_RADIO));
        ReturnOnFailure(current.add(EMBER_ZCL_HARDWARE_FAULT_TYPE_SENSOR));
        ReturnOnFailure(current.add(EMBER_ZCL_HARDWARE_FAULT_TYPE_POWER_SOURCE));
        ReturnOnFailure(current.add(EMBER_ZCL_HARDWARE_FAULT_TYPE_USER_INTERFACE_FAULT));
#endif
        delegate->OnHardwareFaultsDetected(previous, current);
    }
    else if (EventId == GeneralDiagnostics::Events::RadioFaultChange::Id)
    {
        GeneralFaults<kMaxRadioFaults> previous;
        GeneralFaults<kMaxRadioFaults> current;

#if CHIP_CONFIG_TEST
        // On Simulation, set following radio faults statically.
        ReturnOnFailure(previous.add(EMBER_ZCL_RADIO_FAULT_TYPE_WI_FI_FAULT));
        ReturnOnFailure(previous.add(EMBER_ZCL_RADIO_FAULT_TYPE_THREAD_FAULT));

        ReturnOnFailure(current.add(EMBER_ZCL_RADIO_FAULT_TYPE_WI_FI_FAULT));
        ReturnOnFailure(current.add(EMBER_ZCL_RADIO_FAULT_TYPE_CELLULAR_FAULT));
        ReturnOnFailure(current.add(EMBER_ZCL_RADIO_FAULT_TYPE_THREAD_FAULT));
        ReturnOnFailure(current.add(EMBER_ZCL_RADIO_FAULT_TYPE_NFC_FAULT));
#endif
        delegate->OnRadioFaultsDetected(previous, current);
    }
    else if (EventId == GeneralDiagnostics::Events::NetworkFaultChange::Id)
    {
        GeneralFaults<kMaxNetworkFaults> previous;
        GeneralFaults<kMaxNetworkFaults> current;

#if CHIP_CONFIG_TEST
        // On Simulation, set following radio faults statically.
        ReturnOnFailure(previous.add(EMBER_ZCL_NETWORK_FAULT_TYPE_HARDWARE_FAILURE));
        ReturnOnFailure(previous.add(EMBER_ZCL_NETWORK_FAULT_TYPE_NETWORK_JAMMED));

        ReturnOnFailure(current.add(EMBER_ZCL_NETWORK_FAULT_TYPE_HARDWARE_FAILURE));
        ReturnOnFailure(current.add(EMBER_ZCL_NETWORK_FAULT_TYPE_NETWORK_JAMMED));
        ReturnOnFailure(current.add(EMBER_ZCL_NETWORK_FAULT_TYPE_CONNECTION_FAILED));
#endif
        delegate->OnNetworkFaultsDetected(previous, current);
    }
    else
    {
        ChipLogError(DeviceLayer, "Unknow event ID:%d", EventId);
    }
}

void PlatformManagerImpl::OnDeviceRebooted(intptr_t arg)
{
    PlatformManagerDelegate * platformManagerDelegate       = PlatformMgr().GetDelegate();
    GeneralDiagnosticsDelegate * generalDiagnosticsDelegate = GetDiagnosticDataProvider().GetGeneralDiagnosticsDelegate();

    if (generalDiagnosticsDelegate != nullptr)
    {
        generalDiagnosticsDelegate->OnDeviceRebooted();
    }

    // The StartUp event SHALL be emitted by a Node after completing a boot or reboot process
    if (platformManagerDelegate != nullptr)
    {
        uint32_t softwareVersion;

        ReturnOnFailure(ConfigurationMgr().GetSoftwareVersion(softwareVersion));
        platformManagerDelegate->OnStartUp(softwareVersion);
    }
}

void PlatformManagerImpl::OnDeviceShutdown()
{
    PlatformManagerDelegate * platformManagerDelegate = PlatformMgr().GetDelegate();
    uint64_t upTime                                   = 0;

    // The ShutDown event SHOULD be emitted by a Node prior to any orderly shutdown sequence.
    if (platformManagerDelegate != nullptr)
    {
        platformManagerDelegate->OnShutDown();
    }

    if (GetDiagnosticDataProvider().GetUpTime(upTime) == CHIP_NO_ERROR)
    {
        uint32_t totalOperationalHours = 0;

        if (ConfigurationMgr().GetTotalOperationalHours(totalOperationalHours) == CHIP_NO_ERROR)
        {
            ConfigurationMgr().StoreTotalOperationalHours(totalOperationalHours + static_cast<uint32_t>(upTime / 3600));
        }
        else
        {
            ChipLogError(DeviceLayer, "Failed to get total operational hours of the Node");
        }
    }
    else
    {
        ChipLogError(DeviceLayer, "Failed to get current uptime since the Node’s last reboot");
    }
}

} // namespace DeviceLayer
} // namespace chip
