#ifndef SL_DEVICE_INIT_LFXO_CONFIG_H
#define SL_DEVICE_INIT_LFXO_CONFIG_H

// <<< Use Configuration Wizard in Context Menu >>>

// <o SL_DEVICE_INIT_LFXO_MODE> Mode
// <i>
// <cmuOscMode_Crystal=> Crystal oscillator
// <cmuOscMode_AcCoupled=> AC-coupled buffer
// <cmuOscMode_External=> External digital clock
// <i> Default: cmuOscMode_Crystal
#define SL_DEVICE_INIT_LFXO_MODE           cmuOscMode_Crystal

// <o SL_DEVICE_INIT_LFXO_CTUNE> CTUNE <0-127>
// <i> Default: 63
#define SL_DEVICE_INIT_LFXO_CTUNE          32

// <o SL_DEVICE_INIT_LFXO_PRECISION> LFXO precision in PPM <0-65535>
// <i> Default: 500
#define SL_DEVICE_INIT_LFXO_PRECISION      100

// <o SL_DEVICE_INIT_LFXO_TIMEOUT> Startup Timeout Delay
// <i>
// <_CMU_LFXOCTRL_TIMEOUT_2CYCLES=> 2 cycles
// <_CMU_LFXOCTRL_TIMEOUT_256CYCLES=> 256 cycles
// <_CMU_LFXOCTRL_TIMEOUT_1KCYCLES=> 1K cycles
// <_CMU_LFXOCTRL_TIMEOUT_2KCYCLES=> 2K cycles
// <_CMU_LFXOCTRL_TIMEOUT_4KCYCLES=> 4K cycles
// <_CMU_LFXOCTRL_TIMEOUT_8KCYCLES=> 8K cycles
// <_CMU_LFXOCTRL_TIMEOUT_16KCYCLES=> 16K cycles
// <_CMU_LFXOCTRL_TIMEOUT_32KCYCLES=> 32K cycles
// <_CMU_LFXOCTRL_TIMEOUT_DEFAULT=> Default
// <i> Default: _CMU_LFXOCTRL_TIMEOUT_DEFAULT
#define SL_DEVICE_INIT_LFXO_TIMEOUT           _CMU_LFXOCTRL_TIMEOUT_DEFAULT
// <<< end of configuration section >>>

#endif // SL_DEVICE_INIT_LFXO_CONFIG_H
