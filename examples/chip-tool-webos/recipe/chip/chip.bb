# Copyright (c) 2021 LG Electronics, Inc.

DESCRIPTION = "CHIP (Connected Home over IP) framework and SDK sponsored by matter"
AUTHOR = "Sungmok Shin <sungmok.shin@lge.com>"
LICENSE = "Apache-2.0"
LIC_FILES_CHKSUM = " \
    file://LICENSE;md5=86d3f3a95c324c9479bd8986968f4327 \
"
require gn-utils.inc
inherit webos_public_repo
inherit pkgconfig webos_filesystem_paths

# Fork of project-chip/connectedhomeip
SRC_URI = " \
    git://github.com/cabin15/connectedhomeip.git;branch=master;protocol=https;name=main \
"

DEPENDS = "openssl avahi glib-2.0 glib-2.0-native libpbnjson pmloglib luna-service2"

S = "${WORKDIR}/git"
SRCREV_main = "f07886c4a54830c8c135766210c85084dfb7d9a1"
B = "${WORKDIR}/build"
PR = "r0"

BUILD_TYPE = "Release"
OUT_DIR = "${B}/${BUILD_TYPE}"
OUT_IMAGE_DIR = "${OUT_DIR}/image"
TARGET_CPU = "${@gn_arch_name(TARGET_ARCH)}"

GN_ARGS = "\
    is_debug=${@oe.utils.conditional('WEBOS_DISTRO_PRERELEASE', 'devel', 'true', 'false', d)}\
    target_os=\"webos\"\
    target_cpu=\"${TARGET_CPU}\"\
    chip_enable_python_modules=false\
    ar_webos=\"${AR}\"\
    cc_webos=\"${CC}\"\
    cxx_webos=\"${CXX}\"\
    sysroot_webos=\"${PKG_CONFIG_SYSROOT_DIR}\"\
    chip_build_tests=false\
    enable_syslog=true \
    chip_config_network_layer_ble=false \
    treat_warnings_as_errors=false \
"

do_configure() {
    GN_ARGS="${GN_ARGS}"
    echo GN_ARGS is "${GN_ARGS}"
    echo BUILD_TARGETS are "${TARGET}"
    cd ${S}
    rm -rf ${S}/third_party/pigweed
    git checkout ${S}/third_party
    bash -c "source ${S}/scripts/activate.sh"
    ${S}/.environment/cipd/pigweed/gn gen ${OUT_DIR} --args="${GN_ARGS}"
}

do_compile() {
    if [ ! -f ${OUT_DIR}/build.ninja ]; then
        do_configure
    fi
    ${S}/.environment/cipd/pigweed/ninja ${PARALLEL_MAKE} -C ${OUT_DIR} ${TARGET}
}

SYSROOT_DIRS += "/connectedhomeip"

copy_file_recursive() {
    cd $1 && find . -type d -exec install -d $2/"{}" \;
    cd $1 && find . -type f -exec install -c -m 444 "{}" $2/"{}" \;
}

# test_event_7 + com.webos.service.matter
do_install() {
    install -d ${D}${libdir}
    install -c -m 555 ${OUT_DIR}/lib/libCHIP.a ${D}${libdir}
    install -c -m 555 ${OUT_DIR}/lib/libmbedtls.a ${D}${libdir}

    install -d ${D}${datadir}/luna-service2/roles.d
    install -d ${D}${datadir}/luna-service2/services.d
    install -d ${D}${datadir}/luna-service2/client-permissions.d
    install -d ${D}${datadir}/luna-service2/api-permissions.d
    install -d ${D}${datadir}/luna-service2/manifests.d
    install -d ${D}${datadir}/luna-service2/groups.d

    install -c -m 555 ${S}/examples/chip-tool-webos/webos/files/sysbus/com.webos.service.matter.service ${D}${datadir}/luna-service2/services.d
    install -c -m 555 ${S}/examples/chip-tool-webos/webos/files/sysbus/com.webos.service.matter.role.json ${D}${datadir}/luna-service2/roles.d
    install -c -m 555 ${S}/examples/chip-tool-webos/webos/files/sysbus/com.webos.service.matter.perm.json ${D}${datadir}/luna-service2/client-permissions.d
    install -c -m 555 ${S}/examples/chip-tool-webos/webos/files/sysbus/com.webos.service.matter.api.json ${D}${datadir}/luna-service2/api-permissions.d
    install -c -m 555 ${S}/examples/chip-tool-webos/webos/files/sysbus/com.webos.service.matter.manifest.json ${D}${datadir}/luna-service2/manifests.d
    install -c -m 555 ${S}/examples/chip-tool-webos/webos/files/sysbus/com.webos.service.matter.groups.json ${D}${datadir}/luna-service2/groups.d

    rm -rf ${S}/third_party/efr32_sdk
    rm -rf ${S}/third_party/p6
    rm -rf ${S}/third_party/mbed-os
    rm -rf ${S}/third_party/qpg_sdk
    rm -rf ${S}/third_party/pigweed/repo/pw_tokenizer
    rm -rf ${S}/third_party/out/host/obj/src/qrcodetool/bin

    install -d ${D}/connectedhomeip/build
    install -d ${D}/connectedhomeip/config
    install -d ${D}/connectedhomeip/examples
    install -d ${D}/connectedhomeip/out/host/obj
    install -d ${D}/connectedhomeip/out/host/lib
    install -d ${D}/connectedhomeip/out/host/gen
    install -d ${D}/connectedhomeip/src
    install -d ${D}/connectedhomeip/build_overrides
    install -d ${D}/connectedhomeip/third_party
    install -d ${D}/connectedhomeip/zzz_generated
    copy_file_recursive ${S}/build ${D}/connectedhomeip/build
    copy_file_recursive ${S}/config ${D}/connectedhomeip/config
    copy_file_recursive ${S}/examples ${D}/connectedhomeip/examples
    copy_file_recursive ${OUT_DIR}/obj ${D}/connectedhomeip/out/host/obj
    copy_file_recursive ${OUT_DIR}/lib ${D}/connectedhomeip/out/host/lib
    copy_file_recursive ${OUT_DIR}/gen ${D}/connectedhomeip/out/host/gen
    install -c -m 555 ${S}/BUILD.gn ${D}/connectedhomeip
    copy_file_recursive ${S}/src ${D}/connectedhomeip/src
    copy_file_recursive ${S}/build_overrides ${D}/connectedhomeip/build_overrides
    copy_file_recursive ${S}/third_party ${D}/connectedhomeip/third_party
    copy_file_recursive ${S}/zzz_generated ${D}/connectedhomeip/zzz_generated

    install -d ${D}${sbindir}
    mv ${OUT_DIR}/chip-tool-webos ${OUT_DIR}/chip-service
    install -c -m 555 ${OUT_DIR}/chip-service ${D}${sbindir}
}

sysroot_stage_all_append() {
    sysroot_stage_dir ${D}/connectedhomeip ${SYSROOT_DESTDIR}${base_prefix}/connectedhomeip
}

INSANE_SKIP_${PN} = "installed-vs-shipped ldflags"

FILES_${PN} += " \
    ${datadir}/luna-service2 \
"
