#
#   Copyright (c) 2024 Project CHIP Authors
#
#   Licensed under the Apache License, Version 2.0 (the "License");
#   you may not use this file except in compliance with the License.
#   You may obtain a copy of the License at
#
#       http://www.apache.org/licenses/LICENSE-2.0
#
#   Unless required by applicable law or agreed to in writing, software
#   distributed under the License is distributed on an "AS IS" BASIS,
#   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#   See the License for the specific language governing permissions and
#   limitations under the License.
#

# This script checks is sysbuild is used to build the target. If not, it prints the fatal error
# message, as the nRF Connect examples do not support deprecated child-image approach anymore.

if (NOT SYSBUILD) 
    message(FATAL_ERROR " ###################################################################################\n"
                        " # This example does not support child-image approach anymore.                     #\n"
                        " # The nRF Connect SDK platform marked child-image approach as deprecated          #\n"
                        " # from v2.7.0 and it is recommended to use the new sysbuild solution.             #\n"
                        " #                                                                                 #\n"
                        " # To build this application with sysbuild support enabled,                        #\n"
                        " # you have to add --sysbuild flag to the build command, for example:              #\n"
                        " #                                                                                 #\n"
                        " # west build -b <your_board_name> --sysbuild                                      #\n"
                        " ###################################################################################\n")
endif()
