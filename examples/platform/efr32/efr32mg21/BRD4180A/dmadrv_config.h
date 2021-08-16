#ifndef DMADRV_CONFIG_H
#define DMADRV_CONFIG_H

// <<< Use Configuration Wizard in Context Menu >>>

// <o EMDRV_DMADRV_DMA_IRQ_PRIORITY> DMA interrupt priority <0-15>
// <i> Priority of the DMA interrupt. Smaller number equals higher priority.
// <i> Default: 8
#define EMDRV_DMADRV_DMA_IRQ_PRIORITY 8

// <o EMDRV_DMADRV_DMA_CH_COUNT> Number of available channels <1-8>
// <i> Number of DMA channels supported by the driver. A lower channel count
// <i> will reduce RAM memory footprint. The default is to support all channels
// <i> on the device.
// <i> Default: 8
#define EMDRV_DMADRV_DMA_CH_COUNT 8

// <o EMDRV_DMADRV_DMA_CH_PRIORITY> Number of fixed priority channels
// <i> This will configure channels [0, CH_PRIORITY - 1] as fixed priority,
// <i> and channels [CH_PRIORITY, CH_COUNT] as round-robin.
// <i> Default: 0
#define EMDRV_DMADRV_DMA_CH_PRIORITY 0

// <<< end of configuration section >>>

#endif // DMADRV_CONFIG_H
