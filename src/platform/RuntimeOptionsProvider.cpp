/*
 *
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include <platform/RuntimeOptionsProvider.h>

namespace chip {
namespace app {
namespace {
RuntimeOptionsProvider sRuntimeOptionsProvider;
} // namespace
RuntimeOptionsProvider & RuntimeOptionsProvider::Instance()
{
    return sRuntimeOptionsProvider;
}
} // namespace app
} // namespace chip
