include $(abs_top_nlbuild_autotools_dir)/automake/pre.am

bin_PROGRAMS                       = chipqrcodetool
chipqrcodetool_SOURCES                   = chipqrcodetool.cpp setup_payload_commands.cpp setup_payload_commands.h chipqrcodetool_command_manager.h

chipqrcodetool_CPPFLAGS                   = \
    $(NLASSERT_CPPFLAGS)              \
    -I$(top_srcdir)/src               \
    -I$(top_srcdir)/src/lib           \
    -I$(top_srcdir)/src/system        \
    -I$(top_srcdir)/src/include/      \
    $(NULL)

chipqrcodetool_LDADD = \
    $(top_builddir)/src/lib/libCHIP.a \
    $(top_builddir)/src/setup_payload/libSetupPayload.a \
    $(NULL)

include $(abs_top_nlbuild_autotools_dir)/automake/post.am
