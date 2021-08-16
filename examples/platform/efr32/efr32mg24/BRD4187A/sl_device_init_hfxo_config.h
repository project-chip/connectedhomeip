#ifndef SL_DEVICE_INIT_HFXO_CONFIG_H
#define SL_DEVICE_INIT_HFXO_CONFIG_H

// <<< Use Configuration Wizard in Context Menu >>>

// <o SL_DEVICE_INIT_HFXO_MODE> Mode
// <i>
// <cmuHfxoOscMode_Crystal=> Crystal oscillator
// <cmuHfxoOscMode_ExternalSine=> External sine wave
// <i> Default: cmuHfxoOscMode_Crystal
#define SL_DEVICE_INIT_HFXO_MODE           cmuHfxoOscMode_Crystal

// <o SL_DEVICE_INIT_HFXO_FREQ> Frequency <38000000-40000000>
// <i> Default: 39000000
#define SL_DEVICE_INIT_HFXO_FREQ           39000000

// <o SL_DEVICE_INIT_HFXO_CTUNE> CTUNE <0-255>
// <i> Default: 140
#define SL_DEVICE_INIT_HFXO_CTUNE          140

// <<< end of configuration section >>>

#endif // SL_DEVICE_INIT_HFXO_CONFIG_H
