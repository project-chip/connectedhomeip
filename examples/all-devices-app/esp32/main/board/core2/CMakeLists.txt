#
#    Copyright (c) 2026 Project CHIP Authors
#    All rights reserved.
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
#    Description:
#      Empty component that exists only to host idf_component.yml: the top-level
#      CMakeLists.txt adds this directory to EXTRA_COMPONENT_DIRS only when
#      building for the Core2, which is what makes its espressif/m5stack_core_2
#      dependency conditional on the device type rather than just the chip.
#
idf_component_register()
