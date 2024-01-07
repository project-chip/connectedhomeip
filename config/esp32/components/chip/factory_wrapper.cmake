# Function to set variable with default value if not defined


function(set_default_value VAR DEFAULT_VALUE)
    if (NOT DEFINED ${VAR})
        set(${VAR} ${DEFAULT_VALUE} CACHE STRING ${VAR})
    endif()
endfunction()


function(set_values)
    # Set variables with default values if not defined
    set_default_value(DEVICE_NAME "My bulb")
    set_default_value(VENDOR_NAME "Test-vendor")
    set_default_value(DISCRIMINATOR 3841)
    set_default_value(PASSCODE 20202020)
    set_default_value(VENDOR_ID 0xFFF2)
    set_default_value(PRODUCT_ID 0x8001)
    set_default_value(HARDWARE_VERSION 1)
    set_default_value(HARDWARE_VERSION_STR "Devkit")
    set_default_value(DAC_CERT "${CHIP_ROOT}/credentials/test/attestation/Chip-Test-DAC-FFF2-8001-0008-Cert.der")
    set_default_value(DAC_KEY  "${CHIP_ROOT}/credentials/test/attestation/Chip-Test-DAC-FFF2-8001-0008-Key.der")
    set_default_value(PAI_CERT  "${CHIP_ROOT}/credentials/test/attestation/Chip-Test-PAI-FFF2-8001-Cert.der")
    set_default_value(CERT_DCLRN "${CHIP_ROOT}/credentials/test/certification-declaration/Chip-Test-CD-FFF2-8001.der")
endfunction()
