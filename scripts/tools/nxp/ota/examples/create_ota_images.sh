#!/usr/bin/bash

ROOT=../../../../../
CHIP_CERT=$ROOT/src/tools/chip-cert
SPAKE2P=$ROOT/src/tools/spake2p

# Prerequisites
if [ ! -d "$CHIP_CERT/out" ]; then
    printf "chip-cert is not available. Compile it.\n"
    cd "$CHIP_CERT"
    gn gen out
    ninja -C out
    cd -
else
    printf "chip-cert is available.\n"
fi

if [ ! -d "$SPAKE2P/out" ]; then
    printf "spake2p is not available. Compile it.\n"
    cd "$SPAKE2P"
    gn gen out
    ninja -C out
    cd -
else
    printf "spake2p is available.\n"
fi

VERSION=50000
export FACTORY_DATA_DEST=./out/factory_data
export DEVICE_TYPE=100
export DATE="2023-01-01"
export TIME="$(date +"%T")"
export LIFETIME="7305"
export VID="1037"
export PID="A220"
export PAA_CERT=../../demo_generated_certs/paa/Chip-PAA-NXP-Cert.pem
export PAA_KEY=../../demo_generated_certs/paa/Chip-PAA-NXP-Key.pem

if [ ! -d "./out" ]; then
    mkdir "./out"
fi

if [ ! -d "$FACTORY_DATA_DEST" ]; then
    mkdir "$FACTORY_DATA_DEST"
fi

printf "Generate new certificates based on the same PAA\n"
../../generate_cert.sh "$CHIP_CERT"/out/chip-cert

printf "Generate new Certification Declaration\n"
"$CHIP_CERT"/out/chip-cert gen-cd \
    --key "$ROOT"/credentials/test/certification-declaration/Chip-Test-CD-Signing-Key.pem \
    --cert "$ROOT"/credentials/test/certification-declaration/Chip-Test-CD-Signing-Cert.pem \
    --out "$FACTORY_DATA_DEST/Chip-Test-CD-$VID-$PID".der \
    --format-version 1 \
    --vendor-id "0x$VID" \
    --product-id "0x$PID" \
    --device-type-id "0x$DEVICE_TYPE" \
    --certificate-id "ZIG20142ZB330003-24" \
    --security-level 0 \
    --security-info 0 \
    --version-number 9876 \
    --certification-type 1

printf "\nExample: command without input option specified\n"
python3 ../ota_image_tool.py create -v 0xDEAD -p 0xBEEF -vn "$VERSION" -vs "1.0" -da sha256 \
    ./out/app-standard-example-50000.ota || printf "Command failed because no option was specified.\n"

printf "\nExample: generate app OTA image with default descriptor\n"
python3 ../ota_image_tool.py create -v 0xDEAD -p 0xBEEF -vn "$VERSION" -vs "1.0" -da sha256 \
    --app-input-file ./binaries/app_example.bin \
    ./out/app-standard-example-50000.ota

printf "\nExample: generate app OTA image with specified descriptor\n"
python3 ../ota_image_tool.py create -v 0xDEAD -p 0xBEEF -vn "$VERSION" -vs "1.0" -da sha256 \
    --app-input-file ./binaries/app_example.bin \
    --app-version 50000 \
    --app-version-str "50000-version" \
    --app-build-date "$(date +\"%F\")" \
    ./out/app-with-descriptor-example-50000.ota

printf "\nExample: generate factory data OTA image\n"
python3 ../ota_image_tool.py create -v 0xDEAD -p 0xBEEF -vn "$VERSION" -vs "1.0" -da sha256 \
    -fd \
    --cert_declaration "$FACTORY_DATA_DEST/Chip-Test-CD-$VID-$PID".der \
    --dac_cert "$FACTORY_DATA_DEST/Chip-DAC-NXP-$VID-$PID"-Cert.der \
    --dac_key "$FACTORY_DATA_DEST/Chip-DAC-NXP-$VID-$PID"-Key.der \
    --pai_cert "$FACTORY_DATA_DEST/Chip-PAI-NXP-$VID-$PID"-Cert.der \
    ./out/factory-data-example-50000.ota

printf "\nExample: generate SSBL OTA image with specified descriptor\n"
python3 ../ota_image_tool.py create -v 0xDEAD -p 0xBEEF -vn 1 -vs "1.0" -da sha256 \
    --bl-input-file ./binaries/ssbl_ram_ota_entry_example.bin \
    --bl-version 1 \
    --bl-version-str "SSBL-version-1" \
    --bl-build-date "$(date +\"%F\")" \
    ./out/ssbl-with-descriptor-example.ota

printf "\nExample: generate app + SSBL + factory data update OTA image\n"
python3 ../ota_image_tool.py create -v 0xDEAD -p 0xBEEF -vn 1 -vs "1.0" -da sha256 \
    --app-input-file ./binaries/app_example.bin \
    --app-version 50000 \
    --app-version-str "50000-version" \
    --app-build-date "$(date +\"%F\")" \
    --bl-input-file ./binaries/ssbl_ram_ota_entry_example.bin \
    --bl-version 1 \
    --bl-version-str "SSBL-version-1" \
    --bl-build-date "$(date +\"%F\")" \
    -fd \
    --cert_declaration "$FACTORY_DATA_DEST/Chip-Test-CD-$VID-$PID".der \
    --dac_cert "$FACTORY_DATA_DEST/Chip-DAC-NXP-$VID-$PID"-Cert.der \
    --dac_key "$FACTORY_DATA_DEST/Chip-DAC-NXP-$VID-$PID"-Key.der \
    --pai_cert "$FACTORY_DATA_DEST/Chip-PAI-NXP-$VID-$PID"-Cert.der \
    ./out/app-ssbl-factory-data-example.ota

printf "\nExample: generate OTA image for maximum number of entries (8) using JSON\n"
python3 ../ota_image_tool.py create -v 0xDEAD -p 0xBEEF -vn 50000 -vs "1.0" -da sha256 \
    --json ./ota_max_entries_example.json \
    ./out/max-entries-example.ota
