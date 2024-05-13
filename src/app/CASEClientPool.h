/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include <app/CASEClient.h>
#include <lib/support/Pool.h>

namespace chip {

class CASEClientPoolDelegate
{
public:
    virtual CASEClient * Allocate() = 0;

    virtual void Release(CASEClient * client) = 0;

    virtual ~CASEClientPoolDelegate() {}
};

template <size_t N>
class CASEClientPool : public CASEClientPoolDelegate
{
public:
    ~CASEClientPool() override { mClientPool.ReleaseAll(); }

    CASEClient * Allocate() override { return mClientPool.CreateObject(); }

    void Release(CASEClient * client) override { mClientPool.ReleaseObject(client); }

private:
    ObjectPool<CASEClient, N> mClientPool;
};

}; // namespace chip
