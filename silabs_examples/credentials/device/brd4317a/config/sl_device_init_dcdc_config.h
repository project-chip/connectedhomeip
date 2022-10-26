#ifndef SL_DEVICE_INIT_DCDC_CONFIG_H
#define SL_DEVICE_INIT_DCDC_CONFIG_H

// <<< Use Configuration Wizard in Context Menu >>>

// <q SL_DEVICE_INIT_DCDC_ENABLE> Enable DC/DC Converter
// <i>
// <i> Default: 1
#define SL_DEVICE_INIT_DCDC_ENABLE         1

// <q SL_DEVICE_INIT_DCDC_BYPASS> Set DC/DC Converter in Bypass Mode
// <i>
// <i> Default: 0
#define SL_DEVICE_INIT_DCDC_BYPASS         0

// <q SL_DEVICE_INIT_DCDC_PFMX_IPKVAL_OVERRIDE> Override for DCDC PFMX Mode Peak Current Setting
// <i>
// <i> Default: 0
#define SL_DEVICE_INIT_DCDC_PFMX_IPKVAL_OVERRIDE  1

// <o SL_DEVICE_INIT_DCDC_PFMX_IPKVAL> DCDC PFMX Mode Peak Current Setting <0-15>
// <i>
// <i> Default: DCDC_PFMXCTRL_IPKVAL_DEFAULT
#define SL_DEVICE_INIT_DCDC_PFMX_IPKVAL    3

// <<< end of configuration section >>>

#endif // SL_DEVICE_INIT_DCDC_CONFIG_H
