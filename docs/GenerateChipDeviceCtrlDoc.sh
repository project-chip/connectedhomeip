#
# Copyright (c) 2024 Project CHIP Authors
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

if [[ $# -ne 1 ]]; then
    echo 'usage: GenerateChipDeviceCtrlDoc.sh build_directory'
    exit 1
fi

pydoc-markdown -m chip.ChipDeviceCtrl '{
    renderer: {
      type: markdown,
      descriptive_class_title: false,
      render_toc: true,
      render_toc_title: "ChipDeviceCtrl.py API",
      insert_header_anchors: true,
      add_full_prefix: true,
    }
  }' >"$1"/ChipDeviceCtrlAPI.md
