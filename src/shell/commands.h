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
 *      Source implementation for a generic shell API for CHIP examples.
 */

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

int cmd_echo(int argc, char ** argv);
int cmd_exit(int argc, char ** argv);
int cmd_help(int argc, char ** argv);
int cmd_version(int argc, char ** argv);

void chip_shell_cmd_init();

#ifdef __cplusplus
}
#endif
