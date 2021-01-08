# nRF Connect build and configuration files

This directory contains build scripts and common configuration files used by
CHIP nRF Connect applications. It is structured as follows:

| File/Folder   | Contents                                                                                                                                                       |
| ------------- | -------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| chip-gn       | GN project used to build selected CHIP libraries with the _nrfconnect_ platform integration layer                                                              |
| chip-module   | CMake wrapper for the GN project defined in `chip-gn` directory, and other components that allow one to use CHIP as a Zephyr module                            |
| app/app.cmake | Helper CMake module to be used by nRF Connect applications in this repository to inject `chip-module` into the Zephyr module list and load Zephyr build system |
| app/\*.conf   | Common and optional Kconfig configuration files that can be used in nRF Connect applications                                                                   |
