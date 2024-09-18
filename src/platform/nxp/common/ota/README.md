# Multi-image OTA

The OTA processing is now delegated to instances of `OTATlvProcessor` derived
classes. These instances are registered with the `OTAImageProcessorImpl`
instance, which manages the selection of processors that should process the next
blocks, until a full TLV block was transferred.

The application is able to define its own processors, thus extending the default
OTA functionality. The application can also opt to disable the default
processors (application, SSBL and factory data).

Please note that if an OTA image containing multiple TLV is transferred, then
the action for each TLV is applied sequentially, If one of the actions fails,
the remaining actions will not be applied and OTA abort is called. TBD: should
all actions be applied only if there is no error? Or should each action be
applied separately?

## Default processors

The default processors for K32W0 are already implemented in:

-   `OTAFirmwareProcessor` for application/SSBL update. Enabled by default.
-   `OTAFactoryDataProcessor` for factory data update. Disabled by default, user
    has to specify `chip_ota_enable_factory_data_processor=1` in the build args.

Some SDK OTA module flags are defined to support additional features:

-   `gOTAAllowCustomStartAddress=1` - enable `EEPROM` offset value. Used
    internally by SDK OTA module.
-   `gOTAUseCustomOtaEntry=1` - support custom OTA entry for multi-image.
-   `gOTACustomOtaEntryMemory=1` - K32W0 uses `OTACustomStorage_ExtFlash` (1) by
    default.

## Implementing custom processors

A custom processor should implement the abstract interface defined in
`OTATlvProcessor.h`. Below is a compact version:

```
class OTATlvProcessor
{
public:
    virtual CHIP_ERROR Init() = 0;
    virtual CHIP_ERROR Clear() = 0;
    virtual CHIP_ERROR ApplyAction() = 0;
    virtual CHIP_ERROR AbortAction() = 0;
    virtual CHIP_ERROR ExitAction();

    CHIP_ERROR Process(ByteSpan & block);
    void RegisterDescriptorCallback(ProcessDescriptor callback);
protected:
    virtual CHIP_ERROR ProcessInternal(ByteSpan & block) = 0;
};

```

Some details regarding the interface:

-   `Init` will be called whenever the processor is selected.
-   `Clear` will be called when abort occurs or after the apply action takes
    place.
-   `ApplyAction` will be called in `OTAImageProcessorImpl::HandleApply`, before
    the board is reset.
-   `AbortAction` will be called in `OTAImageProcessorImpl::HandleAbort`.
    Processors should reset state here.
-   `ExitAction` is optional and should be implemented by the processors that
    want to execute an action after all data has been transferred, but before
    `HandleApply` is called. It's called before the new processor selection
    takes place. This is useful in the context of multiple TLV transferred in a
    single OTA process.
-   `Process` is the public API used inside `OTAImageProcessorImpl` for data
    processing. This is a wrapper over `ProcessInternal`, which can return
    `CHIP_OTA_CHANGE_PROCESSOR` to notify a new processor should be selected for
    the remaining data.
-   `RegisterDescriptorCallback` can be used to register a callback for
    processing the descriptor. It's optional.
-   `ProcessInternal` should return: _ `CHIP_NO_ERROR` if block was processed
    successfully. _ `CHIP_ERROR_BUFFER_TOO_SMALL` if current block doesn't
    contain all necessary data. This can happen when a TLV value field has a
    header, but it is split across two blocks. \*
    `CHIP_OTA_FETCH_ALREADY_SCHEDULED` if block was processed successfully and
    the fetching is already scheduled by the processor. This happens in the
    default application processor, because the next data fetching is scheduled
    through a callback (called when enough external flash was erased).

Furthermore, a processor can use an instance of `OTADataAccumulator` to
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

## SSBL max entries example

`CONFIG_CHIP_K32W0_MAX_ENTRIES_TEST` can be set to 1 to enable max entries test.
There will be 8 additional processors registered in default `OtaHooks`
implementation. The OTA image should be generated with the
`create_ota_images.sh` script from `./scripts/tools/nxp/ota/examples`.

## Factory data restore mechanism

Prior to factory data update, the old factory data is backed up in external
flash. If anything interrupts the update (e.g. power loss), there is a slight
chance the internal flash factory data section is erased and has to be restored
at next boot. The `FactoryDataProvider` offers a default restore mechanism and
support for registering additional restore mechanisms or overwriting the default
one.

Prior to factory data update, the old factory data is backed up in external
flash. If anything interrupts the update (e.g. power loss), there is a slight
chance the internal flash factory data section is erased and has to be restored
at next boot. The `FactoryDataProvider` offers a default restore mechanism and
support for registering additional restore mechanisms or overwriting the default
one.

Restore mechanisms are just functions that have this signature:
`CHIP_ERROR (*)(void)`. Any such function can be registered through
`FactoryDataProvider::RegisterRestoreMechanism`.

The default restore mechanism is implemented as a weak function:
`FactoryDataDefaultRestoreMechanism`. It is registered in
`FactoryDataProvider::Init`, before factory data validation, and it can be
overwritten at application level. When doing the actual restore, the mechanisms
are called in the order they were registered.

Please note that the restore mechanisms registration order matters. Once a
restore mechanism is successful (`CHIP_NO_ERROR` is returned), the restore
process has finished and subsequent restore mechanisms will not be called.
