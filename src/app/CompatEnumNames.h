/*
 *    Copyright (c) 2023 Project CHIP Authors
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
 * This file contains backwards-compatibility enum name definitions.  This file
 * is meant to be included at the end of cluster-enums.h, after all the normal
 * enum definitions are available.
 */

#pragma once

namespace chip {
namespace app {
namespace Clusters {

namespace OtaSoftwareUpdateProvider {
// https://github.com/project-chip/connectedhomeip/pull/29459 renamed these
// enums to follow the spec names.
using OTAApplyUpdateAction = ApplyUpdateActionEnum;
using OTADownloadProtocol  = DownloadProtocolEnum;
using OTAQueryStatus       = StatusEnum;
} // namespace OtaSoftwareUpdateProvider

namespace OtaSoftwareUpdateRequestor {
// https://github.com/project-chip/connectedhomeip/pull/29459 renamed these
// enums to follow the spec names.
using OTAAnnouncementReason = AnnouncementReasonEnum;
using OTAChangeReasonEnum   = ChangeReasonEnum;
using OTAUpdateStateEnum    = UpdateStateEnum;
} // namespace OtaSoftwareUpdateRequestor

} // namespace Clusters
} // namespace app
} // namespace chip
