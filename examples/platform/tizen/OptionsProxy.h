/*
 *
 * SPDX-FileCopyrightText: 2021-2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <string>
#include <vector>

#include <service_app.h>

class OptionsProxy
{
public:
    /**
     * @brief Extract Matter options from Tizen application
     *
     * @param argv0       - String used as the first argument for argv array.
     * @param app_control - Tizen application framework control handle.
     */
    void Parse(const char * argv0, app_control_h app_control);

    size_t argc() { return mArgs.size(); }
    const char ** argv() { return mArgv.data(); }

private:
    std::vector<std::string> mArgs;
    // C-style array based on mArgs C++ strings
    std::vector<const char *> mArgv;
};
