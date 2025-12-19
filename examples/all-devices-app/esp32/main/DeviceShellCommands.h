/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
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

 #include <lib/shell/Commands.h>
 #include <lib/shell/Engine.h>
 #include <lib/shell/commands/Help.h>
 #include <lib/support/CodeUtils.h>
 #include <lib/support/logging/CHIPLogging.h>
 #include <string>

 namespace chip {
 namespace Shell {

 class DeviceCommands
 {
 public:
     // delete the copy constructor
     DeviceCommands(const DeviceCommands &) = delete;
     // delete the move constructor
     DeviceCommands(DeviceCommands &&) = delete;
     // delete the assignment operator
     DeviceCommands & operator=(const DeviceCommands &) = delete;

     static DeviceCommands & GetInstance()
     {
         static DeviceCommands instance;
         return instance;
     }

     // Register the Device commands
     void Register();

     // API to get the stored device type
     const std::string & GetStoredDeviceType() const { return mStoredDeviceType; }

 private:
     DeviceCommands() {}

     // Simple storage for device type input
     static std::string mStoredDeviceType;

     static CHIP_ERROR DeviceHandler(int argc, char ** argv)
     {
         if (argc == 0)
         {
             sSubShell.ForEachCommand(PrintCommandHelp, nullptr);
             return CHIP_NO_ERROR;
         }

         CHIP_ERROR error = sSubShell.ExecCommand(argc, argv);

         if (error != CHIP_NO_ERROR)
         {
             streamer_printf(streamer_get(), "Error: %" CHIP_ERROR_FORMAT "\r\n", error.Format());
         }

         return error;
     }

    static CHIP_ERROR SetDeviceTypeHandler(int argc, char ** argv);

    static Shell::Engine sSubShell;
 };

 } // namespace Shell
 } // namespace chip

 // C-style API for accessing stored device type
 extern "C" {
     const char * GetStoredDeviceType();
 }
