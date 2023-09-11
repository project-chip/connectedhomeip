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


typedef void (*CHIPFirstClusterSomeIntegerAttributeCallbackType)(void *, chip::app::Clusters::First::Attributes::SomeInteger::TypeInfo::DecodableArgType);
typedef void (*CHIPSecondClusterFabricsAttributeCallbackType)(void *, const chip::app::Clusters::Second::Attributes::Fabrics::TypeInfo::DecodableType &);
typedef void (*CHIPSecondClusterSomeBytesAttributeCallbackType)(void *, chip::app::Clusters::Second::Attributes::SomeBytes::TypeInfo::DecodableArgType);
typedef void (*CHIPThirdClusterSomeEnumAttributeCallbackType)(void *, chip::app::Clusters::Third::Attributes::SomeEnum::TypeInfo::DecodableArgType);
typedef void (*CHIPThirdClusterOptionsAttributeCallbackType)(void *, chip::app::Clusters::Third::Attributes::Options::TypeInfo::DecodableArgType);

