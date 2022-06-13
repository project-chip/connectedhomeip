/*
 * default_bd.cpp
 *
 *  Created on: Jul 30, 2020
 *      Author: gdbeckstein
 */

#include "BlockDevice.h"

#include "SlicingBlockDevice.h"

#if COMPONENT_SPIF
#include "SPIFBlockDevice.h"
#endif

#if COMPONENT_QSPIF
#include "QSPIFBlockDevice.h"
#endif

#if COMPONENT_DATAFLASH
#include "DataFlashBlockDevice.h"
#endif

#if COMPONENT_SD
#include "SDBlockDevice.h"

#if (STATIC_PINMAP_READY)
const spi_pinmap_t static_spi_pinmap = get_spi_pinmap(MBED_CONF_SD_SPI_MOSI, MBED_CONF_SD_SPI_MISO, MBED_CONF_SD_SPI_CLK, NC);
#endif
#endif

BlockDevice * BlockDevice::get_default_instance()
{
#if COMPONENT_SPIF

    static SPIFBlockDevice default_bd;

    return &default_bd;

#elif COMPONENT_QSPIF

    static QSPIFBlockDevice default_bd;

    return &default_bd;

#elif COMPONENT_DATAFLASH

    static DataFlashBlockDevice default_bd;

    return &default_bd;

#elif COMPONENT_SD

#if (STATIC_PINMAP_READY)
    static SDBlockDevice default_bd(static_spi_pinmap, MBED_CONF_SD_SPI_CS);
#else
    static SDBlockDevice default_bd;
#endif

    return &default_bd;

#else

    return NULL;

#endif
}

/**
 * You can override this function to suit your hardware/memory configuration
 * By default it simply returns what is returned by BlockDevice::get_default_instance();
 */
mbed::BlockDevice * get_secondary_bd(void)
{
    mbed::BlockDevice * default_bd = mbed::BlockDevice::get_default_instance();
    static mbed::SlicingBlockDevice sliced_bd(default_bd, 0x0, MCUBOOT_SLOT_SIZE);
    return &sliced_bd;
}
