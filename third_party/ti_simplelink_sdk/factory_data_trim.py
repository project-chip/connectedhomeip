#!/usr/bin/env python

# Copyright (c) 2022 Project CHIP Authors
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

"""Script to extract the factory data content from the matter application + factory data image 

This script will trim the factory data content out of the matter application image. It will also get the length of the factory data from the matter application map file. The output is the Matter image without the dummy factory d.

Run with:
    python factory_data_trim.py <matter image> <matter image map file> <factory data image> <device family>
"""

import itertools
import re
import sys

import intelhex

matter_app_file = sys.argv[1]
matter_app_map_file = sys.argv[2]
matter_image_without_factory_data_hex = sys.argv[3]
device_family = sys.argv[4]

# extract factory data length from map file
with open(matter_app_map_file, "r") as map_file:
    if device_family == 'cc13x2_26x2':
        pattern = ".factory_data   0x000ac000"
    else:
        pattern = ".factory_data   0x000fe800"
    for line in map_file:
        if re.search(pattern, line):
            factory_data_num_bytes = line
            break

# this is the length of the factory data in hexadecmial form
factory_data_num_bytes = factory_data_num_bytes[32:]

# convert hex image to dictionary
matter_image = intelhex.IntelHex()
matter_image.fromfile(matter_app_file, format='hex')
matter_image_dict = matter_image.todict()

if device_family == 'cc13x2_26x2':
    # 704512 is 0xAC000 - start of factory data
    start_index = list(matter_image_dict.keys()).index(704512)
else:
    # 1042432 is 0xFE800 - start of factory data
    start_index = list(matter_image_dict.keys()).index(1042432)
# convert length of factory data into a decimal value
end_index = start_index + int(factory_data_num_bytes, 16)

# slice dictionary to remove factory data elements
matter_image_dict_first_half = dict(itertools.islice(matter_image_dict.items(), 0, start_index))
matter_image_dict_second_half = dict(itertools.islice(matter_image_dict.items(), end_index, len(matter_image_dict)))

# convert sliced dictionary to back to hex
matter_image_without_factory_data_first_half = intelhex.IntelHex()
matter_image_without_factory_data_second_half = intelhex.IntelHex()

matter_image_without_factory_data_first_half.fromdict(matter_image_dict_first_half)
matter_image_without_factory_data_second_half.fromdict(matter_image_dict_second_half)

matter_image_without_factory_data_first_half.merge(matter_image_without_factory_data_second_half, overlap='error')

matter_image_without_factory_data_first_half.tofile(matter_image_without_factory_data_hex, format='hex')
