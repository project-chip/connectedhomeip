include $(abs_top_nlbuild_autotools_dir)/automake/pre.am

if CHIP_BUILD_TOOLS

bin_PROGRAMS                       = qrcodetool
qrcodetool_SOURCES                   = qrcodetool.cpp setup_payload_commands.cpp setup_payload_commands.h qrcodetool_command_manager.h

qrcodetool_CPPFLAGS                   = \
    $(NLASSERT_CPPFLAGS)              \
    -I$(top_srcdir)/src               \
    -I$(top_srcdir)/src/lib           \
    -I$(top_srcdir)/src/system        \
    -I$(top_srcdir)/src/include/      \
    $(NULL)

qrcodetool_LDADD = \
    $(top_builddir)/src/lib/libCHIP.a \
    $(top_builddir)/src/setup_payload/libSetupPayload.a \
    $(NULL)

NLFOREIGN_FILE_DEPENDENCIES = \
      $(qrcodetool_LDADD) \
      $(NULL)

endif

include $(abs_top_nlbuild_autotools_dir)/automake/post.am
