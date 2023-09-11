#
# SPDX-FileCopyrightText: 2022 Project CHIP Authors
#
# SPDX-License-Identifier: Apache-2.0
#

# Copy the merged.hex file if it exists, otherwise copy zephyr.hex

set(HEX_NAME merged.hex)

if(EXISTS "zephyr/${HEX_NAME}")
  configure_file("zephyr/${HEX_NAME}" "${FLASHBUNDLE_FIRMWARE}" COPYONLY)
else()
  configure_file("zephyr/${KERNEL_HEX_NAME}" "${FLASHBUNDLE_FIRMWARE}" COPYONLY)
endif()
