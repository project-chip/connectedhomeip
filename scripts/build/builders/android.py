# Copyright (c) 2021 Project CHIP Authors
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import os
import shlex

from enum import Enum, auto

from .builder import Builder


class AndroidBoard(Enum):
    ARM = auto()
    ARM64 = auto()
    X64 = auto()

    def TargetCpuName(self):
        if self == AndroidBoard.ARM:
            return 'arm'
        elif self == AndroidBoard.ARM64:
            return 'arm64'
        elif self == AndroidBoard.X64:
            return 'x64'
        else:
            raise Exception('Unknown board type: %r' % self)

    def AbiName(self):
        if self == AndroidBoard.ARM:
            return 'armeabi-v7a'
        elif self == AndroidBoard.ARM64:
            return 'arm64-v8a'
        elif self == AndroidBoard.X64:
            return 'x86_64'
        else:
            raise Exception('Unknown board type: %r' % self)


class AndroidBuilder(Builder):

    def __init__(self, root, runner, output_prefix: str, board: AndroidBoard):
        super(AndroidBuilder, self).__init__(root, runner, output_prefix)
        self.board = board

    def validate_build_environment(self):
        for k in ['ANDROID_NDK_HOME', 'ANDROID_HOME']:
            if k not in os.environ:
                raise Exception(
                    'Environment %s missing, cannot build android libraries' % k)

        # SDK manager must be runnable to 'accept licenses'
        sdk_manager = os.path.join(os.environ['ANDROID_HOME'], 'tools', 'bin',
                                   'sdkmanager')
        if not (os.path.isfile(sdk_manager) and os.access(sdk_manager, os.X_OK)):
            raise Exception("'%s' is not executable by the current user" %
                            sdk_manager)

        # In order to accept a license, the licenses folder is updated with the hash of the
        # accepted license
        android_home = os.environ['ANDROID_HOME']
        licenses = os.path.join(android_home, 'licenses')
        if not os.path.exists(licenses):
            # Initial install may not have licenses at all
            if not os.access(android_home, os.W_OK):
                raise Exception(
                    "'%s' is NOT writable by the current user (needed to create licenses folder for accept)"
                    % android_home)

        elif not os.access(licenses, os.W_OK):
            raise Exception(
                "'%s' is NOT writable by the current user (needed to accept licenses)"
                % licenses)

    def generate(self):
        if not os.path.exists(self.output_dir):
            # NRF does a in-place update  of SDK tools
            if not self._runner.dry_run:
                self.validate_build_environment()

            gn_args = {}
            gn_args['target_os'] = 'android'
            gn_args['target_cpu'] = self.board.TargetCpuName()
            gn_args['android_ndk_root'] = os.environ['ANDROID_NDK_HOME']
            gn_args['android_sdk_root'] = os.environ['ANDROID_HOME']

            args = '--args=%s' % (' '.join([
                '%s="%s"' % (key, shlex.quote(value))
                for key, value in gn_args.items()
            ]))

            self._Execute([
                'gn', 'gen', '--check', '--fail-on-unused-args', self.output_dir, args
            ],
                title='Generating ' + self.identifier)

            self._Execute([
                'bash', '-c',
                'yes | %s/tools/bin/sdkmanager --licenses >/dev/null' %
                os.environ['ANDROID_HOME']
            ],
                title='Accepting NDK licenses')

    def _build(self):
        self._Execute(['ninja', '-C', self.output_dir],
                      title='Building JNI ' + self.identifier)

        # NOTE: the following IDE-specific build instructions are NOT used:
        #  - "rsync -a out/"android_$TARGET_CPU"/lib/*.jar src/android/CHIPTool/app/libs"
        #    => using the 'ninjaOutputDir' project property instead to take the jar files directly
        #       from the output

        # JNILibs will be copied as long as they reside in src/main/jniLibs/ABI:
        #    https://developer.android.com/studio/projects/gradle-external-native-builds#jniLibs
        # to avoid redefined in IDE mode, copy to another place and add that path in build.gradle

        # We do NOT use python builtins for copy, so that the 'execution commands' are available
        # when using dry run.
        jnilibs_dir = os.path.join(
            self.root, 'src/android/CHIPTool/app/libs/jniLibs', self.board.AbiName())
        self._Execute(['mkdir', '-p', jnilibs_dir],
                      title='Prepare Native libs ' + self.identifier)

        # TODO: Runtime dependencies should be computed by the build system rather than hardcoded
        # GN supports getting these dependencies like:
        #   gn desc out/android-x64-chip_tool/ //src/controller/java runtime_deps
        #   gn desc out/android-x64-chip_tool/ //src/setup_payload/java runtime_deps
        # However  this assumes that the output folder has been populated, which will not be
        # the case for `dry-run` executions. Hence this harcoding here.
        #
        #   If we unify the JNI libraries, libc++_shared.so may not be needed anymore, which could
        # be another path of resolving this inconsistency.
        for libName in ['libSetupPayloadParser.so', 'libCHIPController.so', 'libc++_shared.so']:
            self._Execute(['cp', os.path.join(self.output_dir, 'lib', 'jni', self.board.AbiName(
            ), libName), os.path.join(jnilibs_dir, libName)])

        # App compilation
        self._Execute([
            '%s/src/android/CHIPTool/gradlew' % self.root, '-p',
            '%s/src/android/CHIPTool' % self.root,
            '-PchipSdkJarDir=%s' % os.path.join(self.output_dir, 'lib'),
            '-PbuildDir=%s' % self.output_dir, 'build'
        ],
            title='Building APP ' + self.identifier)

    def build_outputs(self):
        outputs = {
            'CHIPController.jar':
                os.path.join(self.output_dir, 'lib', 'CHIPController.jar'),
            'SetupPayloadParser.jar':
                os.path.join(self.output_dir, 'lib', 'SetupPayloadParser.jar'),
            'ChipTool-debug.apk':
                os.path.join(self.output_dir, 'outputs', 'apk', 'debug',
                             'app-debug.apk'),
            'ChipTool-release-unsigned.apk':
                os.path.join(self.output_dir, 'outputs', 'apk', 'release',
                             'app-release-unsigned.apk'),

            'jni/%s/libSetupPayloadParser.so' % self.board.AbiName():
                os.path.join(self.output_dir, 'lib', 'jni',
                             self.board.AbiName(), 'libSetupPayloadParser.so'),
            'jni/%s/libCHIPController.so' % self.board.AbiName():
                os.path.join(self.output_dir, 'lib', 'jni',
                             self.board.AbiName(), 'libCHIPController.so'),
            'jni/%s/libc++_shared.so' % self.board.AbiName():
                os.path.join(self.output_dir, 'lib', 'jni',
                             self.board.AbiName(), 'libc++_shared.so'),
        }

        return outputs
