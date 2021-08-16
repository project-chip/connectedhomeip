#ifndef SL_DEVICE_INIT_LFXO_CONFIG_H
#define SL_DEVICE_INIT_LFXO_CONFIG_H

// <<< Use Configuration Wizard in Context Menu >>>

// <o SL_DEVICE_INIT_LFXO_MODE> Mode
// <i>
// <cmuLfxoOscMode_Crystal=> Crystal oscillator
// <cmuLfxoOscMode_AcCoupledSine=> AC-coupled buffer
// <cmuLfxoOscMode_External=> External digital clock
// <i> Default: cmuLfxoOscMode_Crystal
#define SL_DEVICE_INIT_LFXO_MODE           cmuLfxoOscMode_Crystal

// <o SL_DEVICE_INIT_LFXO_CTUNE> CTUNE <0-127>
// <i> Default: 63
#define SL_DEVICE_INIT_LFXO_CTUNE          37

// <o SL_DEVICE_INIT_LFXO_PRECISION> LFXO precision in PPM <0-65535>
// <i> Default: 500
#define SL_DEVICE_INIT_LFXO_PRECISION      100

// <o SL_DEVICE_INIT_LFXO_TIMEOUT> Startup Timeout Delay
// <i>
// <cmuLfxoStartupDelay_2Cycles=> 2 cycles
// <cmuLfxoStartupDelay_256Cycles=> 256 cycles
// <cmuLfxoStartupDelay_1KCycles=> 1K cycles
// <cmuLfxoStartupDelay_2KCycles=> 2K cycles
// <cmuLfxoStartupDelay_4KCycles=> 4K cycles
// <cmuLfxoStartupDelay_8KCycles=> 8K cycles
// <cmuLfxoStartupDelay_16KCycles=> 16K cycles
// <cmuLfxoStartupDelay_32KCycles=> 32K cycles
// <i> Default: cmuLfxoStartupDelay_4KCycles
#define SL_DEVICE_INIT_LFXO_TIMEOUT           cmuLfxoStartupDelay_4KCycles
// <<< end of configuration section >>>

#endif // SL_DEVICE_INIT_LFXO_CONFIG_H
