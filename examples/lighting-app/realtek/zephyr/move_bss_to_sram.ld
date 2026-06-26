/*
 * move_bss_to_sram.ld
 *
 * Move large BSS variables from DTCM (RAM) to SRAM to fix RAM overflow.
 * These variables don't need DTCM's fast tightly-coupled memory speed.
 *
 * This file is included via zephyr_linker_sources(RAM_SECTIONS ...)
 * in CMakeLists.txt, before the .bss section, so these input sections
 * are matched here FIRST and assigned to .sram_large_bss before .bss
 * can claim them.
 *
 * Note: KEEP() is needed because --gc-sections is enabled; without it
 * the collector might discard these sections from the new output section.
 */

.sram_large_bss (NOLOAD) : ALIGN(4)
{
    /*
     * OpenThread instance (~35 KB)
     * Symbol: ot::gInstanceRaw
     */
    KEEP(*(.bss._ZN2ot12gInstanceRawE))

    /*
     * Matter PacketBuffer pool (~24 KB)
     * Symbol: chip::System::PacketBuffer::sBufferPool
     */
    KEEP(*(.bss._ZN4chip6System12PacketBuffer11sBufferPoolE))

    /*
     * Matter InteractionModel engine (~9.7 KB)
     * Symbol: chip::app::sInteractionModelEngine
     */
    KEEP(*(.bss._ZN4chip3app23sInteractionModelEngineE))

    /*
     * Matter Server instance (~11.5 KB)
     * Symbol: chip::Server::sServer
     */
    KEEP(*(.bss._ZN4chip6Server7sServerE))

    /*
     * CHIP sysheap (~8 KB)
     * Symbol: (anonymous namespace)::sHeapMemory
     */
    KEEP(*(.bss._ZN12_GLOBAL__N_111sHeapMemoryE))

    /*
     * mbedtls heap (~8 KB)
     * Symbol: _mbedtls_heap
     */
    KEEP(*(.bss._mbedtls_heap))

    /*
     * mbedtls AES lookup tables (~8 KB total)
     */
    KEEP(*(.bss.RT0))
    KEEP(*(.bss.RT1))
    KEEP(*(.bss.RT2))
    KEEP(*(.bss.RT3))
    KEEP(*(.bss.FT0))
    KEEP(*(.bss.FT1))
    KEEP(*(.bss.FT2))
    KEEP(*(.bss.FT3))
    KEEP(*(.bss.FSb))
    KEEP(*(.bss.RSb))

    /*
     * Thread stack manager instance (~3.9 KB)
     * Symbol: chip::DeviceLayer::ThreadStackManagerImpl::sInstance
     */
    KEEP(*(.bss._ZN4chip11DeviceLayer22ThreadStackManagerImpl9sInstanceE))

    /*
     * Group peer table (~2 KB)
     * Symbol: (anonymous namespace)::gGroupPeerTable
     */
    KEEP(*(.bss._ZN4chip12_GLOBAL__N_115gGroupPeerTableE))

    /*
     * Matter event buffers (~2 KB total)
     */
    KEEP(*(.bss._ZN4chipL16sCritEventBufferE))
    KEEP(*(.bss._ZN4chipL17sDebugEventBufferE))
    KEEP(*(.bss._ZN4chipL16sInfoEventBufferE))

    /*
     * AppTask initParams (~1.6 KB)
     */
    KEEP(*(.bss._ZZN7AppTask4InitEvE10initParams))
} > SRAM
