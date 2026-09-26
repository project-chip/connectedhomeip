/*
 *    Copyright (c) 2026 Project CHIP Authors
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

#include <app/clusters/general-commissioning-server/BreadCrumbTracker.h>
#include <cstdint>

namespace chip::app {

/**
 * In-memory fallback implementation of BreadCrumbTracker for simulated devices.
 *
 * Matter Core Specification (Section 14.3.6.4.2 Breadcrumb Field):
 * "See Breadcrumb Attribute subclause of General Commissioning Cluster in Matter Core for usage."
 *
 * Per specification, breadcrumb updates belong to the General Commissioning Cluster (typically on
 * Endpoint 0 / Root Node) rather than per-device storage. In all-devices-app, dynamic endpoint
 * devices instantiated via DeviceFactory are decoupled from the root node. This standalone tracker
 * satisfies the BreadCrumbTracker interface required by ThreadBorderRouterManagementCluster.
 *
 * TODO: Plumb the root node's GeneralCommissioningCluster via DeviceFactory::Context or an endpoint-sharing
 * mechanism so that breadcrumbs are reflected in the General Commissioning cluster attribute.
 */
class SimpleBreadCrumbTracker : public Clusters::BreadCrumbTracker
{
public:
    void SetBreadCrumb(uint64_t value) override { mBreadCrumb = value; }
    uint64_t GetBreadCrumb() const { return mBreadCrumb; }

private:
    uint64_t mBreadCrumb = 0;
};

} // namespace chip::app
