---
orphan: true
---

## Manufacturing data

By default, the example application is configured to use generic test
certificates and provisioning data embedded with the application code. It is
possible for a final stage application to generate its own manufacturing data:

-   Generate new certificates

    _PAI_VID_ and _PAI_PID_ variables must be changed accordingly inside
    generate_cert.sh script

    _generate_cert.sh_ script needs as input parameter the path to chip-cert
    tool (compile it from ./src/tools/chip-cert). The output of the script is:
    the DAC, PAI and PAA certificates. The DAC and PAI certificates will be
    written in a special section of the internal flash, while the PAA will be
    used on the chip-tool side as trust anchor. Please note that for _real
    production manufacturing_ the "production PAA" is trusted via the DCL rather
    than thorough PAA certificate generated along with DAC and PAI. The PAI cert
    may also have a different lifecycle.

    ```
    user@ubuntu:~/Desktop/git/connectedhomeip$ ./examples/platform/nxp/k32w/k32w0/scripts/generate_cert.sh  ./src/tools/chip-cert/out/chip-cert
    ```

-   Generate new provisioning data and convert all the data to a binary
    (unencrypted data):

    ```
    user@ubuntu:~/Desktop/git/connectedhomeip$  python3 ./scripts/tools/nxp/factory_data_generator/generate.py -i 10000 -s UXKLzwHdN3DZZLBaL2iVGhQi/OoQwIwJRQV4rpEalbA= -p 14014 -d 1000 --vid 0x1037 --pid 0xa220 --vendor_name "NXP Semiconductors" --product_name "Lighting app" --serial_num "SN:12345678" --date "2022-10-21" --hw_version 1 --hw_version_str "1.0" --cert_declaration /home/ubuntu/manufacturing/Chip-Test-CD-1037-a220.der --dac_cert /home/ubuntu/manufacturing/Chip-DAC-NXP-Cert.der --dac_key /home/ubuntu/manufacturing/Chip-DAC-NXP-Key.der --pai_cert /home/ubuntu/manufacturing/Chip-PAI-NXP-Cert.der --spake2p_path ./src/tools/spake2p/out/spake2p --out out.bin
    ```

-   Same example as above, but with an already generated verifier passed as
    input:

    ```
    user@ubuntu:~/Desktop/git/connectedhomeip$  python3 ./scripts/tools/nxp/factory_data_generator/generate.py -i 10000 -s UXKLzwHdN3DZZLBaL2iVGhQi/OoQwIwJRQV4rpEalbA= -p 14014 -d 1000 --vid 0x1037 --pid 0xa220 --vendor_name "NXP Semiconductors" --product_name "Lighting app" --serial_num "SN:12345678" --date "2022-10-21" --hw_version 1 --hw_version_str "1.0" --cert_declaration /home/ubuntu/manufacturing/Chip-Test-CD-1037-a220.der --dac_cert /home/ubuntu/manufacturing/Chip-DAC-NXP-Cert.der --dac_key /home/ubuntu/manufacturing/Chip-DAC-NXP-Key.der --pai_cert /home/ubuntu/manufacturing/Chip-PAI-NXP-Cert.der --spake2p_path ./src/tools/spake2p/out/spake2p --spake2p_verifier ivD5n3L2t5+zeFt6SjW7BhHRF30gFXWZVvvXgDxgCNcE+BGuTA5AUaVm3qDZBcMMKn1a6CakI4SxyPUnJr0CpJ4pwpr0DvpTlkQKqaRvkOQfAQ1XDyf55DuavM5KVGdDrg== --out out.bin
    ```

-   Generate new provisioning data and convert all the data to a binary
    (encrypted data with the AES key):

    ```
    user@ubuntu:~/Desktop/git/connectedhomeip$  python3 ./scripts/tools/nxp/factory_data_generator/generate.py -i 10000 -s UXKLzwHdN3DZZLBaL2iVGhQi/OoQwIwJRQV4rpEalbA= -p 14014 -d 1000 --vid 0x1037 --pid 0xa220 --vendor_name "NXP Semiconductors" --product_name "Lighting app" --serial_num "SN:12345678" --date "2022-10-21" --hw_version 1 --hw_version_str "1.0" --cert_declaration /home/ubuntu/manufacturing/Chip-Test-CD-1037-a220.der --dac_cert /home/ubuntu/manufacturing/Chip-DAC-NXP-Cert.der --dac_key /home/ubuntu/manufacturing/Chip-DAC-NXP-Key.der --pai_cert /home/ubuntu/manufacturing/Chip-PAI-NXP-Cert.der --spake2p_path ./src/tools/spake2p/out/spake2p --out outEncrypted.bin --aes128_key 2B7E151628AED2A6ABF7158809CF4F3C
    ```

    Here is the interpretation of the parameters:

    ```
    -i                 -> SPAKE2+ iteration
    -s                 -> SPAKE2+ salt (passed as base64 encoded string)
    -p                 -> SPAKE2+ passcode
    -d                 -> discriminator
    --vid              -> Vendor ID
    --pid              -> Product ID
    --vendor_name      -> Vendor Name
    --product_name     -> Product Name
    --serial_num       -> Serial Number
    --date             -> Manufacturing Date (YYYY-MM-DD format)
    --hw_version       -> Hardware Version as number
    --hw_version_str   -> Hardware Version as string
    --cert_declaration -> path to the Certification Declaration (der format) location
    --dac_cert         -> path to the DAC (der format) location
    --dac_key          -> path to the DAC key (der format) location
    --pai_cert         -> path to the PAI (der format) location
    --spake2p_path     -> path to the spake2p tool (compile it from ./src/tools/spake2p)
    --out              -> name of the binary that will be used for storing all the generated data
    --aes128_key       -> 128 bits AES key used to encrypt the whole dataset
    --spake2p_verifier -> SPAKE2+ verifier (passed as base64 encoded string). If this option is set,
                          all SPAKE2+ inputs will be encoded in the final binary. The spake2p tool
                          will not be used to generate a new verifier on the fly.
    ```

-   Write out.bin to the \$platform:

    For the K32W0x1 platform, the binary needs to be written in the internal
    flash at location 0x9D600 using DK6Programmer:

    ```
    DK6Programmer.exe -Y -V2 -s <COM_PORT> -P 1000000 -Y -p FLASH@0x9D600="out.bin"
    ```

-   Generate a new CD (certification declaration):

    Inside _gen-test-cds.sh_, the parameters _vids_, _pid0_, _device_type_id_
    must be changed accordingly. Use _Chip-Test-CD-Signing-\*_ key and
    certificate already available in
    _./credentials/test/certification-declaration/_ which acts as CSA
    Certificate. This CSA certificate is also hard-coded as Trust Anchor in the
    current chip-tool version. To use this certificate and avoid generating a
    new one, lines 69-70 must be commented in the _gen-test-cds.sh_ script (the
    ones that are generating a new CD signing authority).

    ```
    user@ubuntu:~/Desktop/git/connectedhomeip$ ./credentials/test/gen-test-cds.sh ./src/tools/chip-cert/out/chip-cert
    ```

-   Set the correct VID/PID and CD in the
    examples/$APP_NAME/nxp/$platform/ChipProjectConfig.h file VID and PID values
    should correspond to the ones used for DAC. CD bytes should be the ones
    obtained at the step above:

    ```
    user@ubuntu:~/manufacturing hexdump -ve '1/1 "0x%.2x, "' Chip-Test-CD-1037-A220.der
    ```

-   Use _chip_with_factory_data=1_ gn compilation argument

    This is needed in order to load the data from the special flash section.
    Build and flash the application.

-   Run chip-tool with a new PAA:

    ```
    ./chip-tool pairing ble-thread 2 hex: $hex_value 14014 1000 --paa-trust-store-path /home/ubuntu/certs/paa
    ```

    Here is the interpretation of the parameters:

    ```
    --paa-trust-store-path -> path to the generated PAA (der format)
    ```

    _paa-trust-store-path_ must contain only the PAA certificate. Avoid placing
    other certificates in the same location as this may confuse chip-tool.

    PAA certificate can be copied to the chip-tool machine using SCP for
    example.

    This is needed for testing self-generated DACs, but likely not required for
    "true production" with production PAI issued DACs.

-   Useful information/Known issues

        Implementation of manufacturing data provisioning has been validated using test certificates generated by OpenSSL 1.1.1l.

        Also, demo DAC, PAI and PAA certificates needed in case _chip_with_factory_data=1_ is used can be found in examples/platform/nxp/k32w/k32w0/scripts/demo_generated_certs.

        dut1/dut2 folders contains different DACs/Private Keys and can be used for testing topologies with 2 DUTS.

        out_dut1.bin/out2_dut2.bin contains the corresponding DACs/PAIs generated using generate_nxp_chip_factory_bin.py script. The discriminator is 14014 and the passcode is 1000.

        These demo certificates are working with the CDs installed in CHIPProjectConfig.h.
