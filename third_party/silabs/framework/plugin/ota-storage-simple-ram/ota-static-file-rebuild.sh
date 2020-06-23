#!/bin/bash

# A script that generates the sample OTA file that ships with Afv2, and
# it generates the C code that hard codes the sample OTA file for
# servers that have no hardware storage.

# This script must run from the root of the branch!

# NOTE:  This dumb check will not catch all cases of how the script can be run.
# It only catches running the script from the immediate parent directory that 
# contains it.
/bin/pwd | grep app/framework/cluster
STATUS=$?
if [ $STATUS == 0 ]; then
  echo "Error:  This script must be run from the root of the branch"
fi

OTA_FILE=app/framework/plugin/ota-storage-simple-ram/ota-static-sample.ota
OTA_HEADER_FILE=app/framework/plugin/ota-storage-simple-ram/ota-static-file-data.h 
OTA_HEADER_SCRIPT=app/framework/plugin/ota-storage-simple-ram/ota-static-file.pl

echo ""

if [ "$CYGWIN" = 1 ]; then 
  IMAGE_BUILDER_EXE=./build/image-builder/build.windows/image-builder-ecc.exe
else
  IMAGE_BUILDER_EXE=./build/image-builder/build.linux/image-builder-ecc
fi

VERSION=0x05
MANUF_ID=0x1002
IMAGE_ID=0x5678
STRING="The latest and greatest upgrade."

echo "Creating $OTA_FILE"
$IMAGE_BUILDER_EXE     \
  --create $OTA_FILE   \
  --version $VERSION   \
  --manuf-id $MANUF_ID \
  --image-type $IMAGE_ID \
  --string "$STRING"     \
  --tag-id 0xF000        \
  --tag-length 10        \
  --test-sign 1

STATUS=$?
if [ $STATUS != 0 ]; then
  echo "Error: Image-builder failed."
  exit $STATUS
fi

echo -e "Creating $OTA_HEADER_FILE"
$OTA_HEADER_SCRIPT \
  --input $OTA_FILE \
  --output $OTA_HEADER_FILE \
  --extra-comments $IMAGE_BUILDER_EXE

STATUS=$?
if [ $STATUS != 0 ]; then
  echo "Error: Header generation script failed."
  exit $STATUS
fi

echo -e "\nScript Success!"
exit 0

