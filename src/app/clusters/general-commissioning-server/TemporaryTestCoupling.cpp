/*
 *    Copyright (c) 2025 Project CHIP Authors
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
#include <clusters/GeneralCommissioning/Attributes.h>

// TODO: we have a dependency on GeneralCommissioning: setting a breadcrumb
// via `GeneralCommissioning::SetBreadcrumb`. This should be decoupled when
// GeneralCommissioning is turned into code-driven
//
// See https://github.com/project-chip/connectedhomeip/issues/39412
namespace chip::app::Clusters::GeneralCommissioning {
void SetBreadcrumb(Attributes::Breadcrumb::TypeInfo::Type breadcrumb)
{
    // NOTHING, this is a test
}
} // namespace chip::app::Clusters::GeneralCommissioning
