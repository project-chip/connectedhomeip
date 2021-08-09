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

import logging
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


class AndroidBuilder(Builder):

  def __init__(self,
               root,
               runner,
               output_prefix: str,
               board: AndroidBoard):
    super(AndroidBuilder, self).__init__(root, runner, output_prefix)
    self.board = board

  def validate_build_environment(self):
    for k in ['ANDROID_NDK_HOME', 'ANDROID_HOME']:
      if k not in os.environ:
        raise Exception('Environment %s missing, cannot build android libraries' % k)

    # SDK manager must be runnable to 'accept licenses'
    sdk_manager = os.path.join(os.environ['ANDROID_HOME'], 'tools', 'bin', 'sdkmanager')
    if not (os.path.isfile(sdk_manager) and os.access(sdk_manager, os.X_OK)):
      raise Exception("'%s' is not executable by the current user" % sdk_manager)

    # In order to accept a license, the licenses folder is updated with the hash of the
    # accepted license
    licenses = os.path.join(os.environ['ANDROID_HOME'], 'licenses')
    if not os.access(licenses, os.W_OK):
      raise Exception("'%s' is writable by the current user (needed to accept licenses)" % licenses)


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

        args = '--args=%s' % (' '.join(['%s="%s"' % (key, shlex.quote(value)) for key,value in gn_args.items()]))
        
        self._Execute(['gn', 'gen', '--check', '--fail-on-unused-args', self.output_dir, args], title='Generating ' + self.identifier)

        self._Execute(['bash', '-c', 'yes | %s/tools/bin/sdkmanager --licenses >/dev/null' % os.environ['ANDROID_HOME']],
          title='Accepting NDK licenses')


  def build(self):
    self._Execute(['ninja', '-C', self.output_dir], title='Building JNI ' + self.identifier)

    # NOTE: the following IDE-specific build instructions are NOT used:
    #  - "rsync -a out/"android_$TARGET_CPU"/lib/*.jar src/android/CHIPTool/app/libs"
    #    => using the 'ninjaOutputDir' project property instead to take the jar files directly
    #       from the output
    #  - "rsync -a out/"android_$TARGET_CPU"/lib/jni/* src/android/CHIPTool/app/src/main/jniLib"
    #    => JNI libraries not used by the build taks (TODO: is this true? APK?)

    self._Execute(['src/android/CHIPTool/gradlew', '-p', 'src/android/CHIPTool', '-PchipSdkJarDir=%s' % os.path.join(self.output_dir, 'lib'),
    '-PbuildDir=%s' % self.output_dir, 'build'], title='Building APP ' + self.identifier)

  def jni_output_libs(self):
    """Get a dictionary of JNI-required files."""
    items = {}

    scan_root = os.path.join(self.output_dir, 'lib', 'jni')
    for root, dirs, files in os.walk(scan_root):
      dir_name = root[len(scan_root) + 1:]
      for file_name in files:
        items[os.path.join(dir_name, file_name)] = os.path.join(root, file_name)

    return items


  def outputs(self):
    outputs ={
      'CHIPController.jar': os.path.join(self.output_dir, 'lib', 'CHIPController.jar'),
      'SetupPayloadParser.jar': os.path.join(self.output_dir, 'lib', 'SetupPayloadParser.jar'),
    }

    outputs.update(self.jni_output_libs())

    return outputs
