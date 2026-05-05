/*
 *    Copyright (c) 2026 Project CHIP Authors
 *
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
#pragma once

#include <app/ConcreteAttributePath.h>

namespace chip::app::DataModel {

/// Describes the level of an attribute change. The existence of this is to
/// allow listeners to cater to two types of consumers:
///
/// 1. Applications generally care about ALL changes to a value, since they
///    may actuate hardware.
///
/// 2. Matter Interaction model identifies things that are reported (i.e.
///    generate traffic on the network and is visible to subscribers) and
///    quiet changes (i.e. not reported).
///
///    Examples of quiet changes are things that fluctuate (like a voltage
///    measurement) where we only report "large changes" or things that
///    continuously change (e.g. a current time would change, or a network
///    packet count would increase as soon as a network packet is sent, and
///    a network packet would be sent reporting the packet count)
///
/// As such we will have constants that say:
///   - kReportable: important/large change, reported to subscribers by Matter IM
///   - kQuiet: value changed, would be visible on a `ReadAttribute` however it
///     is not reported to subscribers.
///
enum class AttributeChangeType
{
    kReportable, // Change should be reported to subscribers
    kQuiet       // Change is minor or configured not to be reported
};

/// Help listeners for endpoint changes to make a quick decision about
/// what happened to an endpoint. This is because this is often cheaper
/// to have at call time compared to querying a DataModel::Provider for example
enum class EndpointChangeType
{
    kAdded,
    kRemoved,
};

/// Interface for components wishing to be notified of attribute changes.
///
/// Implement this interface to receive callbacks when attributes are modified
/// within a DataModel::Provider. Listeners are registered with a specific
/// DataModel::Provider instance.
///
/// Notifications are:
/// - Called *after* the attribute state has been updated in the provider.
/// - Triggered for all attribute changes, regardless of whether they are
///   IM-reportable (i.e., includes kQuiet changes).
/// - Primarily used by the application layer to synchronize external state
///   (e.g., hardware) with the new attribute value.
/// - Synchronous: Called inline during the attribute update process.
///   Implementations should be wary of re-entrancy or recursive calls if
///   they modify cluster state within the callback.
///     * you may also choose to defer cluster modifications to a separate
///       scheduled task. That would not cause recursive calls, however
///       not entering loops has to still be checked for.
/// - Fire-and-forget: There is no mechanism to report failure back to the
///   caller. If an action taken in the callback fails (e.g., hardware
///   actuation), the listener is responsible for any corrective measures,
///   such as reverting the attribute state in the DataModel::Provider.
class AttributeChangeListener
{
public:
    virtual ~AttributeChangeListener() = default;

    /// Called after an attribute's value has changed.
    virtual void OnAttributeChanged(const ConcreteAttributePath & path, AttributeChangeType type) {}

    /// Called when an endpoint's structure or composition changes
    /// (e.g., clusters added/removed, or for bridged device changes).
    virtual void OnEndpointChanged(EndpointId endpointId, EndpointChangeType type) {}

    AttributeChangeListener * GetNextAttributeChangeListener() const { return mNextAttributeChange; }
    void SetNextAttributeChangeListener(AttributeChangeListener * next) { mNextAttributeChange = next; }

private:
    /// NOTE: single linked list to minimize resource overhead.
    ///
    /// This prioritizes low flash/ram overhead over extra functionality or safety (i.e. we cannot
    /// have a single listener registered to multiple providers, there is no specific loop
    /// detection or prevention).
    AttributeChangeListener * mNextAttributeChange = nullptr;
};

} // namespace chip::app::DataModel
