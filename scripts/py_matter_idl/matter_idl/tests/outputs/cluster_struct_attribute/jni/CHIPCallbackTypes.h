/*
 *
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <app-common/zap-generated/cluster-objects.h>
#include <app/util/af-enums.h>

typedef void (*CHIPDefaultSuccessCallbackType)(void *, const chip::app::DataModel::NullObjectType &);
typedef void (*CHIPDefaultWriteSuccessCallbackType)(void *);
typedef void (*CHIPDefaultFailureCallbackType)(void *, CHIP_ERROR);


typedef void (*CHIPDemoClusterClusterSingleFailSafeAttributeCallbackType)(void *, chip::app::Clusters::DemoCluster::Attributes::SingleFailSafe::TypeInfo::DecodableArgType);
typedef void (*CHIPDemoClusterClusterArmFailsafesAttributeCallbackType)(void *, const chip::app::Clusters::DemoCluster::Attributes::ArmFailsafes::TypeInfo::DecodableType &);

