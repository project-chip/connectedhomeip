/*
 *
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <app-common/zap-generated/attributes/Accessors.h>

EmberAfStatus OtaRequestorServerSetUpdateState(chip::app::Clusters::OtaSoftwareUpdateRequestor::OTAUpdateStateEnum value);
EmberAfStatus OtaRequestorServerGetUpdateState(chip::EndpointId endpointId,
                                               chip::app::Clusters::OtaSoftwareUpdateRequestor::OTAUpdateStateEnum & value);
EmberAfStatus OtaRequestorServerSetUpdateStateProgress(chip::app::DataModel::Nullable<uint8_t> value);
EmberAfStatus OtaRequestorServerGetUpdateStateProgress(chip::EndpointId endpointId,
                                                       chip::app::DataModel::Nullable<uint8_t> & value);

void OtaRequestorServerOnStateTransition(chip::app::Clusters::OtaSoftwareUpdateRequestor::OTAUpdateStateEnum previousState,
                                         chip::app::Clusters::OtaSoftwareUpdateRequestor::OTAUpdateStateEnum newState,
                                         chip::app::Clusters::OtaSoftwareUpdateRequestor::OTAChangeReasonEnum reason,
                                         chip::app::DataModel::Nullable<uint32_t> const & targetSoftwareVersion);
void OtaRequestorServerOnVersionApplied(uint32_t softwareVersion, uint16_t productId);
void OtaRequestorServerOnDownloadError(uint32_t softwareVersion, uint64_t bytesDownloaded,
                                       chip::app::DataModel::Nullable<uint8_t> progressPercent,
                                       chip::app::DataModel::Nullable<int64_t> platformCode);
