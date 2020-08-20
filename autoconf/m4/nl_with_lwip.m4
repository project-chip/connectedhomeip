#
#    Copyright (c) 2020 Project CHIP Authors
#    Copyright (c) 2015-2017 Nest Labs, Inc.
#
#    Licensed under the Apache License, Version 2.0 (the "License");
#    you may not use this file except in compliance with the License.
#    You may obtain a copy of the License at
#
#        http://www.apache.org/licenses/LICENSE-2.0
#
#    Unless required by applicable law or agreed to in writing, software
#    distributed under the License is distributed on an "AS IS" BASIS,
#    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#    See the License for the specific language governing permissions and
#    limitations under the License.
#

#
#    Description:
#      This file defines a GNU autoconf M4-style macro for the presence
#      of the LwIP package.
#

#
# NL_WITH_LWIP(LWIP_ENABLED)
#
# ----------------------------------------------------------------------------
AC_DEFUN([NL_WITH_LWIP],
[
    # If LwIP support is enabled...
    if test "$1" = 1; then

        # Influential external variables for the package support
    
        AC_ARG_VAR(LWIP_CPPFLAGS, [LwIP C preprocessor flags])
        AC_ARG_VAR(LWIP_LDFLAGS,  [LwIP linker flags])
        AC_ARG_VAR(LWIP_LIBS,     [LwIP linker libraries])
    
        # Allow the user to specify both external headers and libraries
        # together (or internal).
    
        AC_ARG_WITH(lwip,
            AS_HELP_STRING([--with-lwip=DIR],
                [Specify location of the LwIP headers and libraries, or "internal" to build using CHIP's internal copy of LwIP @<:@default=internal@:>@.]),
            [
                if test "${withval}" = "no"; then
                    AC_MSG_ERROR([${PACKAGE_NAME} requires the LwIP package])
                elif test "${withval}" = "internal"; then
                    lwip_dir=${withval}
                    nl_with_lwip=${withval}
                else
                    lwip_dir=${withval}
                    nl_with_lwip=external
                fi
            ],
            [lwip_dir=; nl_with_lwip=internal])
    
        # Allow users to specify external headers and libraries independently.
    
        AC_ARG_WITH(lwip-includes,
            AS_HELP_STRING([--with-lwip-includes=DIR],
                [Specify location of LwIP headers @<:@default=internal@:>@.]),
            [
                if test "${withval}" = "no"; then
                    AC_MSG_ERROR([${PACKAGE_NAME} requires the LwIP package])
                fi
    
                if test "x${lwip_dir}" != "x"; then
                    AC_MSG_WARN([overriding --with-lwip=${lwip_dir}])
                fi
    
                if test "${withval}" = "internal"; then
                    lwip_header_dir=${withval}
                    nl_with_lwip=${withval}
                else
                    lwip_header_dir=${withval}
                    nl_with_lwip=external
                fi
            ],
            [
                lwip_header_dir=;
                if test "x${nl_with_lwip}" = "x"; then
                    nl_with_lwip=internal
                fi
            ])
    
        AC_ARG_WITH(lwip-libs,
            AS_HELP_STRING([--with-lwip-libs=DIR],
                [Specify location of LwIP libraries @<:@default=internal@:>@.]),
            [
                if test "${withval}" = "no"; then
                    AC_MSG_ERROR([${PACKAGE_NAME} requires the LwIP package])
                fi
    
                if test "x${lwip_dir}" != "x"; then
                    AC_MSG_WARN([overriding --with-lwip=${lwip_dir}])
                fi
    
                if test "${withval}" = "internal"; then
                    lwip_library_dir=${withval}
                    nl_with_lwip=${withval}
                else
                    lwip_library_dir=${withval}
                    nl_with_lwip=external
                fi
            ],
            [
                lwip_library_dir=;
                if test "x${nl_with_lwip}" = "x"; then
                    nl_with_lwip=internal
                fi
            ])
    
        AC_ARG_WITH(lwip-target,
            AS_HELP_STRING([--with-lwip-target=target],
                [Specify the target environment for which LwIP will be built.  Choose one of: standalone, nrf5, efr32, k32w, none @<:@default=standalone@:>@.]),
            [
                if test "${nl_with_lwip}" != "internal"; then
                    AC_MSG_ERROR([--with-lwip-target can only be used when --with-lwip=internal is selected])
                else
                    case "${withval}" in
                    standalone|nrf5|efr32|k32w|none)
                        nl_with_lwip_target=${withval}
                        ;;
                    *)
                        AC_MSG_ERROR([Invalid value ${withval} for --with-lwip-target])
                        ;;
                    esac
                fi
            ],
            [
                nl_with_lwip_target=standalone
            ])
    
        AC_MSG_CHECKING([source of the LwIP package])
        AC_MSG_RESULT([${nl_with_lwip}])
    
        # If the user has selected or has defaulted into the internal LwIP
        # package, set the values appropriately. Otherwise, run through the
        # usual routine.
    
        if test "${nl_with_lwip}" = "internal"; then
    
            LWIP_CPPFLAGS="-I${ac_abs_confdir}/third_party/lwip/repo/lwip/src/include -I${ac_abs_confdir}/third_party/lwip/repo/lwip/src/include/ipv4 -I${ac_abs_confdir}/third_party/lwip/repo/lwip/src/include/ipv6 -I${ac_abs_confdir}/src/lwip"
            LWIP_LDFLAGS="-L${ac_pwd}/src/lwip"
            LWIP_LIBS="-llwip"
    
            # On Darwin, "common" symbols are not added to the table of contents
            # by ranlib and so can cause link time errors. This option
            # offers a workaround.
            case "${target_os}" in
                *darwin*)
                    LWIP_CPPFLAGS="${LWIP_CPPFLAGS} -fno-common"
                    ;;
            esac
            
            # The LwIP source code depends on a set of environment-specific headers and source
            # files to configure it for use on a particular target platform.  When an external
            # LwIP implementation is chosen, the user is expected to pass the appropriate
            # CPPFLAGS/LDFLAGS/LIBS values to configure to allow the build to find these files.
            #
            # When the "internal" LwIP package is selected, the user can use the --with-lwip-target
            # option to chose among a set of pre-configured files for specific target platforms.
            # The platform-specific files are located within subdirectories of src/lwip that are
            # named after the platform.
            # 
            # The default choice for --with-lwip-target is "standalone", which adapts LwIP for
            # use on a desktop linux system.  This is typically used for testing purposes.
            #
            # The target "none" directs the build to not include any environment-specific files.
            # This allows the user to build with the internal LwIP sources but supplied their
            # own environment-specific files using CPP/CFLAGS/LDFLAGS values.
    
            case "${nl_with_lwip_target}" in
            standalone)
                LWIP_CPPFLAGS="${LWIP_CPPFLAGS} -I${ac_abs_confdir}/src/lwip/standalone"
                LWIP_LIBS="${LWIP_LIBS} -lpthread"
                ;;
            nrf5)
                LWIP_CPPFLAGS="${LWIP_CPPFLAGS} -I${ac_abs_confdir}/src/lwip/nrf5 -I${ac_abs_confdir}/src/lwip/freertos"
                ;;
            efr32)
                LWIP_CPPFLAGS="${LWIP_CPPFLAGS} -I${ac_abs_confdir}/src/lwip/efr32 -I${ac_abs_confdir}/src/lwip/freertos"
                ;;
            k32w)
                LWIP_CPPFLAGS="${LWIP_CPPFLAGS} -I${ac_abs_confdir}/src/lwip/k32w -I${ac_abs_confdir}/src/lwip/freertos"
                ;;
            *)
                ;;
            esac

            AC_MSG_CHECKING([LwIP target environment])
            AC_MSG_RESULT([${nl_with_lwip_target}])

        else
            # We always prefer checking the values of the various '--with-lwip-...'
            # options first to using pkg-config because the former might be used
            # in a cross-compilation environment on a system that also contains
            # pkg-config. In such a case, the user wants what he/she specified
            # rather than what pkg-config indicates.
    
            if test "x${lwip_dir}" != "x" -o "x${lwip_header_dir}" != "x" -o "x${lwip_library_dir}" != "x"; then
                if test "x${lwip_dir}" != "x"; then
                    if test -d "${lwip_dir}"; then
                        if test -d "${lwip_dir}/include"; then
                            LWIP_CPPFLAGS="-I${lwip_dir}/include -I${lwip_dir}/include/ipv4 -I${lwip_dir}/include/ipv6"
                        else
                            LWIP_CPPFLAGS="-I${lwip_dir} -I${lwip_dir}/ipv4 -I${lwip_dir}/ipv6"
                        fi
    
                        if test -d "${lwip_dir}/lib"; then
                            LWIP_LDFLAGS="-L${lwip_dir}/lib"
                        else
                            LWIP_LDFLAGS="-L${lwip_dir}"
                        fi
                    else
                        AC_MSG_ERROR([No such directory ${lwip_dir}])
                    fi
                fi
    
                if test "x${lwip_header_dir}" != "x"; then
                    if test -d "${lwip_header_dir}"; then
                        LWIP_CPPFLAGS="-I${lwip_header_dir} -I${lwip_header_dir}/ipv4 -I${lwip_header_dir}/ipv6"
                    else
                        AC_MSG_ERROR([No such directory ${lwip_header_dir}])
                    fi
                fi
    
                if test "x${lwip_library_dir}" != "x"; then
                    if test -d "${lwip_library_dir}"; then
                        LWIP_LDFLAGS="-L${lwip_library_dir}"
                    else
                        AC_MSG_ERROR([No such directory ${lwip_library_dir}])
                    fi
                fi
    
                LWIP_LIBS="${LWIP_LDFLAGS} -llwip"
    
            elif test "x${PKG_CONFIG}" != "x" -a "${PKG_CONFIG} --exists lwip"; then
                LWIP_CPPFLAGS="`${PKG_CONFIG} --cflags lwip`"
                LWIP_LDFLAGS="`${PKG_CONFIG} --libs-only-L lwip`"
                LWIP_LIBS="`${PKG_CONFIG} --libs-only-l lwip`"
    
            else
                AC_MSG_ERROR([Cannot find the LwIP package.])
            fi
        fi
    
        AC_SUBST(LWIP_CPPFLAGS)
        AC_SUBST(LWIP_LDFLAGS)
        AC_SUBST(LWIP_LIBS)
    
    
        nl_saved_CPPFLAGS="${CPPFLAGS}"
        nl_saved_LDFLAGS="${LDFLAGS}"
        nl_saved_LIBS="${LIBS}"
    
        CPPFLAGS="${CPPFLAGS} ${LWIP_CPPFLAGS}"
        LDFLAGS="${LDFLAGS} ${LWIP_LDFLAGS}"
        LIBS="${LIBS} ${LWIP_LIBS}"
    
        # LwIP does a miserable job of making its headers work
        # standalone. Check the two below, standalone, then check the
        # rest.
    
        AC_CHECK_HEADERS([lwip/debug.h] [lwip/err.h],
            [],
            [
                AC_MSG_ERROR(The LwIP header "$ac_header" is required but cannot be found.)
            ])
    
        AC_CHECK_HEADERS([lwip/dns.h] [lwip/ethip6.h] [lwip/init.h] [lwip/ip_addr.h] [lwip/ip.h] [lwip/mem.h] [lwip/netif.h] [lwip/opt.h] [lwip/pbuf.h] [lwip/raw.h] [lwip/snmp.h] [lwip/stats.h] [lwip/sys.h] [lwip/tcp.h] [lwip/tcpip.h] [lwip/udp.h],
            [],
            [
                AC_MSG_ERROR(The LwIP header "$ac_header" is required but cannot be found.)
            ],
            [#include <lwip/err.h>
             #include <lwip/ip4_addr.h>
            ])

        if test "${ac_no_link}" != "yes"; then
            # If we are building against LwIP, check whether or not the
            # version of LwIP has the new, standard {raw,udp}_bind_netif APIs
            # that can be used rather than the bespoke CHIP-specific LwIP
            # intf_filter PCB fields.
            #
            # Assert -Werror=implicit-function-declaration to ensure we do not
            # get a false positive due to C's usually optimistic and loose
            # "we'll figure it out at link time" approach to symbols.
    
            nl_with_lwip_saved_CPPFLAGS="${CPPFLAGS}"
    
            CPPFLAGS="${CPPFLAGS} -Werror=implicit-function-declaration"
    
            AC_MSG_CHECKING([whether LwIP has raw_bind_netif()])
            AC_TRY_COMPILE([#include <lwip/init.h>
                #include <lwip/netif.h>
                #include <lwip/raw.h>
                ],[struct raw_pcb *pcb = NULL;
                const struct netif *netif = NULL;
                raw_bind_netif(pcb, netif);],[
                AC_MSG_RESULT([yes])
                AC_DEFINE(HAVE_LWIP_RAW_BIND_NETIF, 1, [Define to 1 if LwIP has the raw_bind_netif() interface])],[
                AC_MSG_RESULT([no])])
    
            AC_MSG_CHECKING([whether LwIP has udp_bind_netif()])
            AC_TRY_COMPILE([#include <lwip/init.h>
                #include <lwip/netif.h>
                #include <lwip/udp.h>
                ],[struct udp_pcb *pcb = NULL;
                const struct netif *netif = NULL;
                udp_bind_netif(pcb, netif);],[
                AC_MSG_RESULT([yes])
                AC_DEFINE(HAVE_LWIP_UDP_BIND_NETIF, 1, [Define to 1 if LwIP has the udp_bind_netif() interface])],[
                AC_MSG_RESULT([no])])
    
            CPPFLAGS="${nl_with_lwip_saved_CPPFLAGS}"
        fi
    
        CPPFLAGS="${nl_saved_CPPFLAGS}"
        LDFLAGS="${nl_saved_LDFLAGS}"
        LIBS="${nl_saved_LIBS}"
    
    fi
    
    # Define automake conditionals for building CHIP with LwIP
    AM_CONDITIONAL([CHIP_WITH_LWIP], [test "${1}" == 1])
    AM_CONDITIONAL([CHIP_WITH_LWIP_INTERNAL], [test "${nl_with_lwip}" = "internal"])
    AM_CONDITIONAL([CHIP_LWIP_TARGET_STANDALONE], [test "${nl_with_lwip}" = "internal" -a "${nl_with_lwip_target}" = "standalone" ])
    AM_CONDITIONAL([CHIP_LWIP_TARGET_NRF5], [test "${nl_with_lwip}" = "internal" -a "${nl_with_lwip_target}" = "nrf5" ])
    AM_CONDITIONAL([CHIP_LWIP_TARGET_EFR32], [test "${nl_with_lwip}" = "internal" -a "${nl_with_lwip_target}" = "efr32" ])
    AM_CONDITIONAL([CHIP_LWIP_TARGET_K32W], [test "${nl_with_lwip}" = "internal" -a "${nl_with_lwip_target}" = "k32w" ])
])
