/*
 *
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

#include "AddressPolicy_SingleInterface.h"
#include <lib/dnssd/minimal_mdns/AddressPolicy.h>
#include <lib/support/logging/CHIPLogging.h>

namespace mdns {
namespace Minimal {
namespace {
class SingleInterface : public mdns::Minimal::ListenIterator
{
public:
    SingleInterface(chip::Platform::UniquePtr<ListenIterator> parent, chip::Inet::InterfaceId & selectedIface) :
        mParent(std::move(parent)), mSelectedIface(selectedIface)
    {}

    bool Next(chip::Inet::InterfaceId * out_id, chip::Inet::IPAddressType * out_type) override
    {
        chip::Inet::InterfaceId id;
        chip::Inet::IPAddressType type;

        if (!mParent->Next(&id, &type))
        {
            return false;
        }

        while (id != mSelectedIface)
        {
            if (!mParent->Next(&id, &type))
            {
                return false;
            }
        }

        *out_id   = id;
        *out_type = type;
        return true;
    }

private:
    chip::Platform::UniquePtr<ListenIterator> mParent;
    chip::Inet::InterfaceId & mSelectedIface;
};

} // namespace

SingleInterfaceAddressPolicy::SingleInterfaceAddressPolicy(AddressPolicy * parentPolicy, chip::Inet::InterfaceId & selectedIface) :
    mParentPolicy(parentPolicy), mSelectedIface(&selectedIface)
{}

chip::Platform::UniquePtr<ListenIterator> SingleInterfaceAddressPolicy::GetListenEndpoints()
{
    VerifyOrDie(mParentPolicy != nullptr);
    return chip::Platform::UniquePtr<ListenIterator>(
        chip::Platform::New<SingleInterface>(mParentPolicy->GetListenEndpoints(), *mSelectedIface));
}

chip::Platform::UniquePtr<IpAddressIterator>
SingleInterfaceAddressPolicy::GetIpAddressesForEndpoint(chip::Inet::InterfaceId interfaceId, chip::Inet::IPAddressType type)
{
    return mParentPolicy->GetIpAddressesForEndpoint(interfaceId, type);
}

} // namespace Minimal
} // namespace mdns
