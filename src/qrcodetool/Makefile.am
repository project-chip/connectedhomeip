include $(abs_top_nlbuild_autotools_dir)/automake/pre.am

if CHIP_BUILD_TOOLS

bin_PROGRAMS                       = qrcodetool
qrcodetool_SOURCES                 = qrcodetool.cpp setup_payload_commands.cpp setup_payload_commands.h qrcodetool_command_manager.h

qrcodetool_CPPFLAGS                                       = \
    -I$(top_srcdir)/src                                     \
    -I$(top_srcdir)/src/lib                                 \
    -I$(top_srcdir)/src/lib/core                            \
    -I$(top_srcdir)/src/system                              \
    -I$(top_srcdir)/src/include                             \
    $(NLASSERT_CPPFLAGS)                                    \
    $(NLFAULTINJECTION_CPPFLAGS)                            \
    $(NLIO_CPPFLAGS)                                        \
    $(NULL)

qrcodetool_LDADD = \
    $(COMMON_LDFLAGS)                                       \
    $(top_builddir)/src/setup_payload/libSetupPayload.a     \
    $(top_builddir)/src/lib/libCHIP.a                       \
    $(NLFAULTINJECTION_LDFLAGS) $(NLFAULTINJECTION_LIBS)    \
    $(NULL)

if CHIP_WITH_ANDROID
qrcodetool_LDADD                                         += \
    -llog                                                   \
    $(NULL)
endif #CHIP_WITH_ANDROID

NLFOREIGN_FILE_DEPENDENCIES = \
      $(top_builddir)/src/setup_payload/libSetupPayload.a   \
      $(top_builddir)/src/lib/libCHIP.a                     \
      $(NULL)

NLFOREIGN_SUBDIR_DEPENDENCIES = \
      $(NLFAULTINJECTION_FOREIGN_SUBDIR_DEPENDENCY)         \
      $(NULL)

endif

include $(abs_top_nlbuild_autotools_dir)/automake/post.am
