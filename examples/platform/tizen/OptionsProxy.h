/*
 *
 *    Copyright (c) 2021-2022 Project CHIP Authors
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
