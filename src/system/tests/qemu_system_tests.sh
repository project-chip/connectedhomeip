#!/usr/bin/env bash
#
#
#    Copyright (c) 2020 Project CHIP Authors
#
#    Licensed under the Apache License, Version 2.0 (the "License");
#    you may not use this file except in compliance with the License.
#    You may obtain a copy of the License at
#
#        http://www.apache.org/licenses/LICENSE-2.0
#
#    Unless required by applicable law or agreed to in writing, software
#    distributed under the License is distributed on an "AS IS" BASIS,
#    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#    See the License for the specific language governing permissions and
#    limitations under the License.
#
#
#    Description:
#      Execute system tests by setting the correct environment
#

abs_top_srcdir=$1 abs_top_builddir=$2 QEMU_TEST_TARGET=libSystemLayerTests.a $(abs_top_srcdir)/scripts/tools/qemu_run_test.sh
