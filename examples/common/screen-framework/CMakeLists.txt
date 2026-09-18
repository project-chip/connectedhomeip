# The display-only dependencies are keyed on the IDF version, not on
# CONFIG_HAVE_DISPLAY: component requirements are collected before Kconfig is
# available, so CONFIG_* is always empty at this point. The `tft` component comes
# from the m5stack-tft library, which builds only against the legacy `driver`
# component that ESP-IDF 6.0 dissolved into per-peripheral components, so on 6.x the
# library is not part of the build at all (see the applications' CMakeLists.txt).
set(require_list chip)

if(IDF_VERSION_MAJOR LESS 6)
    list(APPEND require_list driver)
    if(${IDF_TARGET} STREQUAL "esp32")
        list(APPEND require_list tft)
    endif()
endif()

idf_component_register(SRCS "Display.cpp"
                            "ScreenManager.cpp"
                            "Screen.cpp"
                            "ListScreen.cpp"
                       INCLUDE_DIRS "include"
                       REQUIRES ${require_list})

# Everything here compiles to nothing unless CONFIG_HAVE_DISPLAY is set, and on 6.x it
# cannot be satisfied. Fail with an explanation rather than a missing-header error deep
# inside the m5stack-tft library. (Kconfig is available past idf_component_register,
# which returns from the file during the requirements pass.)
if(CONFIG_HAVE_DISPLAY AND IDF_VERSION_MAJOR GREATER_EQUAL 6)
    message(FATAL_ERROR
        "The display depends on the m5stack-tft library, which does not support ESP-IDF 6.x. "
        "Select a device type without a display, or use ESP-IDF 5.x.")
endif()

# m5stack-tft declares most of its font tables without `const`, so they land in .data and
# cost DRAM (~11 kB for the three unused ones) rather than living in flash like
# tft_Dejavu24 does.  The library is unmaintained, so rather than forking it, cut the only
# reference: TFT_setFont() names every font, which is what keeps all of them alive through
# --gc-sections. No application here selects COMIC24/MINYA24/TOONEY32, so point those
# branches at tft_DefaultFont, which is what TFT_setFont() falls back to for an
# unrecognized font anyway. The three tables become unreferenced and are dropped by the
# linker.
if(tft IN_LIST BUILD_COMPONENTS)
    idf_component_get_property(tft_dir tft COMPONENT_DIR)
    set_source_files_properties("${tft_dir}/tft.c"
        TARGET_DIRECTORY idf::tft
        PROPERTIES COMPILE_DEFINITIONS
            "tft_Comic24=tft_DefaultFont;tft_minya24=tft_DefaultFont;tft_tooney32=tft_DefaultFont")
endif()
