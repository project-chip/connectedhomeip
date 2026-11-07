/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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

#include <lib/dnssd/minimal_mdns/AddressPolicy.h>
#include <lib/support/Span.h>

namespace mdns {
namespace Minimal {

/// Decides whether minimal mDNS should make use of a given network interface.
///
/// This is the extension point for selection rules beyond an exact list of
/// names, such as pattern matching or an exclusion list.
class InterfacePredicate
{
public:
    virtual ~InterfacePredicate() = default;

    /// `name` is the operating system interface name, e.g. "eth0".
    virtual bool IsAllowed(const char * name) const = 0;
};

/// Accepts only interfaces whose name appears in the given list.
///
/// Names match exactly.
///
/// Neither the span nor the strings it points to are copied: the caller retains
/// ownership and must keep both alive for as long as this object is in use.
class InterfaceNameAllowList : public InterfacePredicate
{
public:
    explicit InterfaceNameAllowList(chip::Span<const char * const> names) : mNames(names) {}

    bool IsAllowed(const char * name) const override;

private:
    const chip::Span<const char * const> mNames;
};

/// An AddressPolicy that defers to another policy while hiding every interface
/// rejected by the given predicate.
///
/// Hidden interfaces never reach ServerBase::Listen, so they do not consume one
/// of its limited endpoints. This matters on hosts carrying a large number of
/// interfaces that can never usefully carry mDNS traffic, such as the virtual
/// interface created for every container on a Docker or Kubernetes host.
///
/// Both the underlying policy and the predicate are borrowed and must outlive
/// this object.
///
/// The policy to wrap is whichever one is installed at the time, so this must
/// be set up after the platform default is in place and before the server
/// starts listening:
///
///     GlobalMinimalMdnsServer::Instance();  // installs the platform default
///     static InterfaceNameAllowList allowed(names);
///     static FilteredAddressPolicy filtered(*GetAddressPolicy(), allowed);
///     SetAddressPolicy(&filtered);
class FilteredAddressPolicy : public AddressPolicy
{
public:
    FilteredAddressPolicy(AddressPolicy & underlying, const InterfacePredicate & predicate) :
        mUnderlying(underlying), mPredicate(predicate)
    {}

    chip::Platform::UniquePtr<ListenIterator> GetListenEndpoints() override;

    chip::Platform::UniquePtr<IpAddressIterator> GetIpAddressesForEndpoint(chip::Inet::InterfaceId interfaceId,
                                                                           chip::Inet::IPAddressType type) override;

private:
    AddressPolicy & mUnderlying;
    const InterfacePredicate & mPredicate;
};

} // namespace Minimal
} // namespace mdns
