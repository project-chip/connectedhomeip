#include "app/clusters/ota-requestor/ota-image-processor.h"

class LinuxOTAImageProcessor : public OTAImageProcessor {

    // Virtuial functions from OTAImageProcessor -- start
    // Open file, find block of space in persistent memory, or allocate a buffer, etc.
    CHIP_ERROR PrepareDownload() { return CHIP_NO_ERROR;}

    // Must not be a blocking call to support cases that require IO to elements such as // external peripherals/radios
     CHIP_ERROR ProcessBlock(chip::ByteSpan & data) { return CHIP_NO_ERROR;}

    // Close file, close persistent storage, etc
    CHIP_ERROR Finalize() { return CHIP_NO_ERROR;}

    chip::Optional<uint8_t> PercentComplete() { return chip::Optional<uint8_t>(0);}

    // Clean up the download which could mean erasing everything that was written,
    // releasing buffers, etc.
    CHIP_ERROR Abort() { return CHIP_NO_ERROR;}

    // Virtuial functions from OTAImageProcessor -- end
};
