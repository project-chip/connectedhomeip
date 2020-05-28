/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

/**
 *    @file
 *          Storage interface, this is a generic interface for storage
 *
 */

#ifndef CHIP_STORAGE_IMPL_H
#define CHIP_STORAGE_IMPL_H

#include <platform/Linux/ChipStorage.h>
#include <string>

#ifndef SYSCONFDIR
#define SYSCONFDIR "/etc"
#endif

#ifndef LOCALSTATEDIR
#define LOCALSTATEDIR "/tmp"
#endif

#define CHIP_DEFAULT_CONFIG_PATH SYSCONFDIR "/" PACKAGE_TARNAME
#define CHIP_DEFAULT_DATA_PATH LOCALSTATEDIR "/db/" PACKAGE_TARNAME

namespace chip {
namespace DeviceLayer {
namespace Internal {

class PosixStorage
{
public:
    static void SetConfigPath(const char * path) { sConfigPath = path; }

    static std::string GetConfigPath()
    {
        if (sConfigPath.empty())
            return std::string(CHIP_DEFAULT_CONFIG_PATH);
        else
            return sConfigPath;
    }

    static void SetDataPath(const char * path) { sDataPath = path; }

    static std::string GetDataPath()
    {
        if (sDataPath.empty())
            return std::string(CHIP_DEFAULT_DATA_PATH);
        else
            return sDataPath;
    }

    static ChipStorage * GetFacotryStorage();
    static ChipMutableStorage * GetConfigStorage();
    static ChipMutableStorage * GetCountersStorage();

private:
    static std::string sConfigPath;
    static std::string sDataPath;
};

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip

#endif // CHIP_STORAGE_IMPL_H
