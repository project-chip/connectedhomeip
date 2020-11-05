#include <cstdio>

#include <mdns/minimal/DnsHeader.h>
#include <support/CHIPMem.h>

int main(int argc, char ** args)
{
    if (chip::Platform::MemoryInit() != CHIP_NO_ERROR)
    {
        printf("FAILED to initialize memory");
        return 1;
    }

    printf("Running...\n");

    // TODO:
    //   - send mDNS requests
    //   - receive mDNS replies

    printf("Done...\n");
    return 0;
}