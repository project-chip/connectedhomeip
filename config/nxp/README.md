# NXP Zephyr build and configuration files

This directory contains build scripts and common configuration files used by
CHIP NXP Zephyr applications. It is structured as follows:

| File/Folder | Contents                                                                                                                            |
| ----------- | ----------------------------------------------------------------------------------------------------------------------------------- |
| chip-gn     | GN project used to build selected CHIP libraries with the _nxp_zephyr_ platform integration layer                                   |
| chip-module | CMake wrapper for the GN project defined in `chip-gn` directory, and other components that allow one to use CHIP as a Zephyr module |
| app         | Common and optional Kconfig configuration files that can be used in NXP Zephyr applications                                         |
