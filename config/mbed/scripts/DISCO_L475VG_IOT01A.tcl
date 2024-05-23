source [find interface/stlink.cfg]
transport select hla_swd
set CHIPNAME STM32L475
source [find target/stm32l4x.cfg]
reset_config srst_only srst_nogate