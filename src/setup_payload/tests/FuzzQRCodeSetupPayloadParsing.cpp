#include <cstdint>
#include <iostream>

#include "setup_payload/Base38Decode.h"
#include "setup_payload/QRCodeSetupPayloadParser.h"

using namespace chip;

extern "C" int LLVMFuzzerTestOneInput(const uint8_t * data, size_t len)
{
    chip::Platform::MemoryInit();

    std::string s((const char *) data, len);
    SetupPayload payload;
    QRCodeSetupPayloadParser(s).populatePayload(payload);

    return 0;
}
