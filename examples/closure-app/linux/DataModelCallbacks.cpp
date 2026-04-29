/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
 *    All rights reserved.
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

#include <app-common/zap-generated/callback.h>
#include <app/ConcreteAttributePath.h>
#include <app/data-model-provider/AttributeChangeListener.h>
#include <lib/support/logging/CHIPLogging.h>

using namespace chip;
using namespace chip::app;

void MatterCodegenPostAttributeChangeCallback(const chip::app::ConcreteAttributePath & path,
                                              chip::app::DataModel::AttributeChangeType type)
{
    // Stub for now, logic remains in MatterPostAttributeChangeCallback for Ember clusters.
}

/* Forwards all attributes changes */
void MatterClosureControlClusterServerAttributeChangedCallback(const app::ConcreteAttributePath & attributePath)
{
    // TODO: Consider moving this logic to MatterCodegenPostAttributeChangeCallback to support code-driven cluster compatibility.
    ChipLogProgress(Zcl, "Closure Control cluster ID: " ChipLogFormatMEI, ChipLogValueMEI(attributePath.mAttributeId));
}

void MatterClosureDimensionClusterServerAttributeChangedCallback(const app::ConcreteAttributePath & attributePath)
{
    // TODO: This callback is no longer called by Ember.
    // Migrate to MatterCodegenPostAttributeChangeCallback.
    ChipLogProgress(Zcl, "Closure Dimension cluster ID: " ChipLogFormatMEI, ChipLogValueMEI(attributePath.mAttributeId));
}
