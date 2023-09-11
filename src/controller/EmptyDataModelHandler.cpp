/*
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

// The controller library requires DataModelHandler functions which are usually
// provided by a specific controller application, since the functions depend on
// the code generated based on application-specific clusters. Implement empty
// defaults so that the controller library is self-sufficient and a controller
// which has not yet implemented ZCL handlers builds without linker errors.

#include <app/util/DataModelHandler.h>

__attribute__((weak)) void InitDataModelHandler() {}
