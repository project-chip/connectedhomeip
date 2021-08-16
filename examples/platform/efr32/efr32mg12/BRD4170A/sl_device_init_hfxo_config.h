#ifndef SL_DEVICE_INIT_HFXO_CONFIG_H
#define SL_DEVICE_INIT_HFXO_CONFIG_H

// <<< Use Configuration Wizard in Context Menu >>>

// <o SL_DEVICE_INIT_HFXO_MODE> Mode
// <i>
// <cmuOscMode_Crystal=> Crystal oscillator
// <cmuOscMode_External=> External digital clock
// <i> Default: cmuOscMode_Crystal
#define SL_DEVICE_INIT_HFXO_MODE           cmuOscMode_Crystal

// <o SL_DEVICE_INIT_HFXO_FREQ> Frequency <38000000-40000000>
// <i> Default: 38400000
#define SL_DEVICE_INIT_HFXO_FREQ           38400000

// <o SL_DEVICE_INIT_HFXO_CTUNE> CTUNE <0-511>
// <i> Default: 360
#define SL_DEVICE_INIT_HFXO_CTUNE          340

// <o SL_DEVICE_INIT_HFXO_AUTOSTART> Auto-start HFXO
// <true=> True
// <false=> False
// <i> Default: false
#define SL_DEVICE_INIT_HFXO_AUTOSTART      false

// <o SL_DEVICE_INIT_HFXO_AUTOSELECT> Auto-select HFXO
// <true=> True
// <false=> False
// <i> Default: false
#define SL_DEVICE_INIT_HFXO_AUTOSELECT      false

// <<< end of configuration section >>>

#endif // SL_DEVICE_INIT_HFXO_CONFIG_H
