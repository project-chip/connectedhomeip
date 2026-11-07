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
#include <lib/dnssd/minimal_mdns/AddressPolicy_FilteredImpl.h>

#include <lib/support/logging/CHIPLogging.h>

#include <cstring>
#include <utility>

namespace mdns {
namespace Minimal {
namespace {

/// Forwards an underlying ListenIterator, dropping every interface that the
/// predicate rejects.
class FilteringListenIterator : public ListenIterator
{
public:
    FilteringListenIterator(chip::Platform::UniquePtr<ListenIterator> underlying, const InterfacePredicate & predicate) :
        mUnderlying(std::move(underlying)), mPredicate(predicate)
    {}

    bool Next(chip::Inet::InterfaceId * id, chip::Inet::IPAddressType * type) override
    {
        while (mUnderlying->Next(id, type))
        {
            if (IsAllowed(*id))
            {
                return true;
            }
        }
        return false;
    }

private:
    bool IsAllowed(chip::Inet::InterfaceId id) const
    {
        // A null interface id is not a specific interface: minimal mDNS uses it
        // to listen for IPv4 on every interface at once. There is no name to
        // match against, so it is always kept.
        if (!id.IsPresent())
        {
            return true;
        }

        char name[chip::Inet::InterfaceId::kMaxIfNameLength];
        if (id.GetInterfaceName(name, sizeof(name)) != CHIP_NO_ERROR)
        {
            ChipLogError(Discovery, "Failed to get interface name; interface not used for mDNS.");
            return false;
        }

        if (!mPredicate.IsAllowed(name))
        {
            ChipLogDetail(Discovery, "Interface %s not used for mDNS: rejected by the configured policy.", name);
            return false;
        }

        return true;
    }

    chip::Platform::UniquePtr<ListenIterator> mUnderlying;
    const InterfacePredicate & mPredicate;
};

} // namespace

bool InterfaceNameAllowList::IsAllowed(const char * name) const
{
    if (name == nullptr)
    {
        return false;
    }

    for (const char * allowed : mNames)
    {
        if ((allowed != nullptr) && (strcmp(allowed, name) == 0))
        {
            return true;
        }
    }

    return false;
}

chip::Platform::UniquePtr<ListenIterator> FilteredAddressPolicy::GetListenEndpoints()
{
    auto underlying = mUnderlying.GetListenEndpoints();
    if (!underlying)
    {
        return underlying;
    }

    return chip::Platform::UniquePtr<ListenIterator>(
        chip::Platform::New<FilteringListenIterator>(std::move(underlying), mPredicate));
}

chip::Platform::UniquePtr<IpAddressIterator> FilteredAddressPolicy::GetIpAddressesForEndpoint(chip::Inet::InterfaceId interfaceId,
                                                                                              chip::Inet::IPAddressType type)
{
    // Endpoints reach this point only after passing the predicate in
    // GetListenEndpoints, so their addresses need no further filtering.
    return mUnderlying.GetIpAddressesForEndpoint(interfaceId, type);
}

} // namespace Minimal
} // namespace mdns
