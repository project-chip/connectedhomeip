/**
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "account-login-delegate.h"

namespace chip {
namespace app {
namespace Clusters {
namespace AccountLogin {

void SetDefaultDelegate(EndpointId endpoint, Delegate * delegate);

} // namespace AccountLogin
} // namespace Clusters
} // namespace app
} // namespace chip
