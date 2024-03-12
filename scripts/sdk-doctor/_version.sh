#!/bin/bash

#
# Copyright (c) 2024 Project CHIP Authors
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

ROOT_DIR=$(realpath "$(dirname "$0")"/../..)
cd "$ROOT_DIR"

SPEC_VERSION=$(head -n 1 "$ROOT_DIR"/SPECIFICATION_VERSION)
SPEC_SHA=$(head -n 1 "$ROOT_DIR"/data_model/spec_sha)
SCRAPPER_VERSION=$(head -n 1 "$ROOT_DIR"/data_model/scraper_version)

echo 'SPEC VERSION:' "$SPEC_VERSION"
echo 'SPEC SHA:' "$SPEC_SHA"
echo 'SCRAPER VERSION:' "$SCRAPPER_VERSION"
