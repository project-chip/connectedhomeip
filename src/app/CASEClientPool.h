/*
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
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
