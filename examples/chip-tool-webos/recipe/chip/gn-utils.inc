# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.

def gn_arch_name(yocto_arch):
    """Translates between Yocto's architecture values and the corresponding
    ones used by GN."""
    translation_table = {
        'aarch64': 'arm64',
        'arm': 'arm',
        'i586': 'x86',
        'i686': 'x86',
        'x86_64': 'x64',
    }
    try:
        return translation_table[yocto_arch]
    except KeyError:
        bb.fatal('"%s" is not a supported architecture.' % yocto_arch)


def write_toolchain_file(d, file_path):
    """Creates a complete GN toolchain file in |file_path|."""
    import string
    gcc_toolchain_tmpl = string.Template(
        'gcc_toolchain("${toolchain_name}") {\n'
        '  cc = "${cc}"\n'
        '  cxx = "${cxx}"\n'
        '  ar = "${ar}"\n'
        '  ld = cxx  # GN expects a compiler, not a linker.\n'
        '  nm = "${nm}"\n'
        '  readelf = "${readelf}"\n'
        '  extra_cflags = "${extra_cflags}"\n'
        '  extra_cppflags = "${extra_cppflags}"\n'
        '  extra_cxxflags = "${extra_cxxflags}"\n'
        '  extra_ldflags = "${extra_ldflags}"\n'
        '  toolchain_args = {\n'
        '    current_cpu = "${current_cpu}"\n'
        '    current_os = "linux"\n'
        '    is_clang = false\n'
        '  }\n'
        '}\n'
    )
    clang_toolchain_tmpl = string.Template(
        'clang_toolchain("clang_${toolchain_name}") {\n'
        '  extra_cflags = "${extra_cflags}"\n'
        '  extra_cppflags = "${extra_cppflags}"\n'
        '  extra_cxxflags = "${extra_cxxflags}"\n'
        '  extra_ldflags = "${extra_ldflags}"\n'
        '  toolchain_args = {\n'
        '    current_cpu = "${current_cpu}"\n'
        '    current_os = "linux"\n'
        '    is_clang = true\n'
        '    use_gold = true\n'
        '  }\n'
        '}\n'
    )

    native_toolchain = {
        'toolchain_name': 'yocto_native',
        'current_cpu': gn_arch_name(d.getVar('BUILD_ARCH', True)),
        'cc': d.expand('${BUILD_CC}'),
        'cxx': d.expand('${BUILD_CXX}'),
        'ar': d.expand('${BUILD_AR}'),
        'nm': d.expand('${BUILD_NM}'),
        'readelf': d.expand('${BUILD_PREFIX}readelf'),
        'extra_cflags': d.expand('${BUILD_CFLAGS}'),
        'extra_cppflags': d.expand('${BUILD_CPPFLAGS}'),
        'extra_cxxflags': d.expand('${BUILD_CXXFLAGS}'),
        'extra_ldflags': d.expand('${BUILD_LDFLAGS}'),
    }
    target_toolchain = {
        'toolchain_name': 'yocto_target',
        'current_cpu': gn_arch_name(d.getVar('TUNE_ARCH', True)),
        'cc': d.expand('${CC}'),
        'cxx': d.expand('${CXX}'),
        'ar': d.expand('${AR}'),
        'nm': d.expand('${NM}'),
        'readelf': d.expand('${TARGET_PREFIX}readelf'),
        'extra_cflags': d.expand('${TARGET_CFLAGS}'),
        'extra_cppflags': d.expand('${TARGET_CPPFLAGS}'),
        'extra_cxxflags': d.expand('${TARGET_CXXFLAGS}'),
        'extra_ldflags': d.expand('${TARGET_LDFLAGS}'),
        'strip': '',
    }

    with open(file_path, 'w') as toolchain_file:
        toolchain_file.write(
            '# This file has been generated automatically.\n'
            '\n'
            'import("//build/config/sysroot.gni")\n'
            'import("//build/toolchain/gcc_toolchain.gni")\n'
            '\n'
        )
        toolchain_file.write(gcc_toolchain_tmpl.substitute(native_toolchain))
        toolchain_file.write(gcc_toolchain_tmpl.substitute(target_toolchain))
        toolchain_file.write(clang_toolchain_tmpl.substitute(native_toolchain))
        toolchain_file.write(clang_toolchain_tmpl.substitute(target_toolchain))
