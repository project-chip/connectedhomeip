---
orphan: true
---

# NXP OTA image tool

## Overview

This tool can generate an OTA image in the `|OTA standard header|TLV1|...|TLVn|`
format. The payload contains data in standard TLV format (not Matter TLV format.
During OTA transfer, these TLV can span across multiple BDX blocks, thus the
`OTAImageProcessorImpl` instance should take this into account.

Each TLV will be processed by its associated processor, pre-registered in
`OTAImageProcessorImpl` and identified by the TLV tag. If a processor cannot be
found for current decoded tag, the OTA transfer will be canceled.

An application is able to define its own processors, thus enabling extending the
default OTA functionality. The application can also opt to disable the default
processors (application, bootloader and factory data) by setting
`chip_enable_ota_default_processors=0`.

## Usage

TODO: add more options

Example:

```
python3 ./scripts/tools/nxp/ota/ota_image_tool.py create -v 0xDEAD -p 0xBEEF -vn 50000 -vs "1.0" -da sha256 -fd --cert_declaration $FACTORY_DATA_DEST/Chip-Test-CD-$VID-$PID.der --dac_cert $FACTORY_DATA_DEST/Chip-DAC-NXP-$VID-$PID-Cert.der --dac_key $FACTORY_DATA_DEST/Chip-DAC-NXP-$VID-$PID-Key.der --pai_cert $FACTORY_DATA_DEST/Chip-PAI-NXP-$VID-$PID-Cert.der -app ~/binaries/ota_update/chip-k32w0x-light-example-50000.bin --app-version 50000 --app-version-str "50000_test" --app-build-date "$DATE" ~/binaries/ota_update/chip-k32w0x-light-example-50000.bin $FACTORY_DATA_DEST/chip-k32w0x-light-example-50000.ota
```

Example (only factory data update):

```
python3 ./scripts/tools/nxp/ota/ota_image_tool.py create -v 0xDEAD -p 0xBEEF -vn 50000 -vs "1.0" -da sha256 -fd --cert_declaration $FACTORY_DATA_DEST/Chip-Test-CD-$VID-$PID.der --dac_cert $FACTORY_DATA_DEST/Chip-DAC-NXP-$VID-$PID-Cert.der --dac_key $FACTORY_DATA_DEST/Chip-DAC-NXP-$VID-$PID-Key.der --pai_cert $FACTORY_DATA_DEST/Chip-PAI-NXP-$VID-$PID-Cert.der $FACTORY_DATA_DEST/chip-k32w0x-light-example-50000.ota
```

## Default processors

The default processors for K32W0 are already implemented in:

-   `OTAApplicationProcessor` for application update.
-   TODO: `OTABootloaderProcessor` for SSBL update.
-   `OTAFactoryDataProcessor` for factory data update.

## Implementing custom processors

A custom processor should implement the interface defined by the
`OTATlvProcessor` abstract interface (simplified version; see `OTATlvHeader.h`
for full version):

```
class OTATlvProcessor
{
public:
    virtual CHIP_ERROR Init() = 0;
    virtual CHIP_ERROR Clear() = 0;
    virtual CHIP_ERROR ApplyAction() = 0;
    virtual CHIP_ERROR AbortAction() = 0;

    CHIP_ERROR Process(ByteSpan & block);
protected:
    virtual CHIP_ERROR ProcessInternal(ByteSpan & block) = 0;
};

```

Note that `ProcessInternal` should return:

-   `CHIP_NO_ERROR` if block was processed successfully.
-   `CHIP_ERROR_BUFFER_TOO_SMALL` if current block doesn't contain all necessary
    data. This can happen when a TLV value field has a header, but it is split
    across two blocks.
-   `CHIP_OTA_FETCH_ALREADY_SCHEDULED` if block was processed successfully and
    the fetching is already scheduled by the processor. This happens in the
    default application processor, because the next data fetching is scheduled
    through a callback (called when enough external flash was erased).

`Process` is the public API that is used inside `OTAImageProcessorImpl` for data
processing. This is a wrapper over `ProcessInternal`, which can return
`CHIP_OTA_CHANGE_PROCESSOR` to notify a new processor should be selected for the
remaining data.

Furthermore, a processor can use an instance of `OTADataAccumulator` to to
accumulate data until a given threshold. This is useful when a custom payload
contains metadata that need parsing: accumulate data until the threshold is
reached or return `CHIP_ERROR_BUFFER_TOO_SMALL` to signal
`OTAImageProcessorImpl` more data is needed.

```
/**
 * This class can be used to accumulate data until a given threshold.
 * Should be used by OTATlvProcessor derived classes if they need
 * metadata accumulation (e.g. for custom header decoding).
 */
class OTADataAccumulator
{
public:
    void Init(uint32_t threshold);
    void Clear();
    CHIP_ERROR Accumulate(ByteSpan & block);

    inline uint8_t* data() { return mBuffer.Get(); }

private:
    uint32_t mThreshold;
    uint32_t mBufferOffset;
    Platform::ScopedMemoryBuffer<uint8_t> mBuffer;
};
```

## Factory data update

`DAC`, `PAI` and `CD` can be updated at a later time by creating a factory data
update OTA image. If the `PAA` changes, make sure to generate the new
certificates using the new `PAA` (which is only used by the controller, e.g.
`chip-tool`). Please see the
[manufacturing flow guide](../../../../examples/platform/nxp/doc/manufacturing_flow.md)
for generating new certificates.

Example of OTA image generation with factory data and application update (using
env variables set in the prerequisites of manufacturing flow):

```
python3 ./scripts/tools/nxp/ota/ota_image_tool.py create -v 0xDEAD -p 0xBEEF -vn 50000 -vs "1.0" -da sha256 -fd --cert_declaration $FACTORY_DATA_DEST/Chip-Test-CD-$VID-$PID.der --dac_cert $FACTORY_DATA_DEST/Chip-DAC-NXP-$VID-$PID-Cert.der --dac_key $FACTORY_DATA_DEST/Chip-DAC-NXP-$VID-$PID-Key.der --pai_cert $FACTORY_DATA_DEST/Chip-PAI-NXP-$VID-$PID-Cert.der -app $FACTORY_DATA_DEST/chip-k32w0x-light-example-50000.bin --app-version 50000 --app-version-str "50000_test" --app-build-date "$DATE" $FACTORY_DATA_DEST/chip-k32w0x-light-example-50000.bin $FACTORY_DATA_DEST/chip-k32w0x-light-example-50000.ota
```
