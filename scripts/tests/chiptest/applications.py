#
#    Copyright (c) 2025 Project CHIP Authors
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

import logging
import os
import shutil
import stat
import subprocess
import tempfile
import threading
import time

from .application_paths import ApplicationPaths

TEST_NODE_ID = '0x12344321'


def copy_with_fsync(src, dst):
    with open(src, 'rb') as fsrc, open(dst, 'wb') as fdst:
        while chunk := fsrc.read(4096):
            fdst.write(chunk)
        fdst.flush()
        os.fsync(fdst.fileno())


class CommissioneeApp:
    def __init__(self, app_id, runner, command):
        self.id = app_id
        self.process = None
        self.outpipe = None
        self.runner = runner
        self.command = command
        self.cv_stopped = threading.Condition()
        self.stopped = True
        self.lastLogIndex = 0
        self.kvsPathSet = {f"/tmp/chip_kvs_{app_id}"}
        self.snapshot_dir = os.path.join(tempfile.gettempdir(), f"chip_snapshot_{app_id}")
        self.options = None
        self.killed = False
        self.isPaired = False

    def start(self, options=None):
        if not self.process:
            # Cache command line options to be used for reboots
            if options:
                self.options = options
            # Make sure to assign self.process before we do any operations that
            # might fail, so attempts to kill us on failure actually work.
            self.process, self.outpipe, errpipe = self.__startServer(
                self.runner, self.command)
            self.waitForAnyAdvertisement()
            self.__updateSetUpCode()
            with self.cv_stopped:
                self.stopped = False
                self.cv_stopped.notify()
            return True
        return False

    def stop(self):
        if self.process:
            with self.cv_stopped:
                self.stopped = True
                self.cv_stopped.notify()
            self.__terminateProcess()
            return True
        return False

    def factoryReset(self):
        wasRunning = (not self.killed) and self.stop()

        for kvs in self.kvsPathSet:
            if os.path.exists(kvs):
                os.unlink(kvs)

        if wasRunning:
            return self.start()

        return True

    def factoryResetOrRestoreState(self):
        if not self.restoreState():
            self.factoryReset()

    def saveState(self):
        os.makedirs(self.snapshot_dir, exist_ok=True)

        for kvs in self.kvsPathSet:
            if os.path.exists(kvs):
                snapshot_file = os.path.join(self.snapshot_dir, os.path.basename(kvs))
                copy_with_fsync(kvs, snapshot_file)

    def restoreState(self):
        restored = False

        for kvs in self.kvsPathSet:
            snapshot_file = os.path.join(self.snapshot_dir, os.path.basename(kvs))
            if os.path.exists(snapshot_file):
                copy_with_fsync(snapshot_file, kvs)
                os.chmod(kvs, stat.S_IRUSR | stat.S_IWUSR | stat.S_IRGRP | stat.S_IWGRP | stat.S_IROTH | stat.S_IWOTH)
                restored = True

        self.isPaired = restored
        return restored

    def waitForAnyAdvertisement(self):
        self.__waitFor("mDNS service published:", self.process, self.outpipe)

    def waitForMessage(self, message, timeoutInSeconds=10):
        self.__waitFor(message, self.process, self.outpipe, timeoutInSeconds)
        return True

    def kill(self):
        self.__terminateProcess()
        self.killed = True

    def wait(self, timeout=None):
        while True:
            # If the App was never started, AND was killed, exit immediately
            if self.killed:
                return 0
            # If the App was never started, wait cannot be called on the process
            if self.process is None:
                time.sleep(0.1)
                continue
            code = self.process.wait(timeout)
            with self.cv_stopped:
                if not self.stopped:
                    return code
                # When the server is manually stopped, process waiting is
                # overridden so the other processes that depends on the
                # accessory beeing alive does not stop.
                while self.stopped:
                    self.cv_stopped.wait()

    def __startServer(self, runner, command):
        app_cmd = command + ['--interface-id', str(-1)]

        if not self.options:
            logging.debug('Executing application under test with default args')
            app_cmd = app_cmd + ['--KVS', next(iter(self.kvsPathSet))]
        else:
            logging.debug('Executing application under test with the following args:')
            for key, value in self.options.items():
                logging.debug('   %s: %s' % (key, value))
                app_cmd = app_cmd + [key, value]
                if key == '--KVS':
                    self.kvsPathSet.add(value)
        return runner.RunSubprocess(app_cmd, name='APP ', wait=False)

    def __waitFor(self, waitForString, server_process, outpipe, timeoutInSeconds=10):
        logging.debug('Waiting for %s' % waitForString)

        start_time = time.monotonic()
        ready, self.lastLogIndex = outpipe.CapturedLogContains(
            waitForString, self.lastLogIndex)
        if ready:
            self.lastLogIndex += 1

        while not ready:
            if server_process.poll() is not None:
                died_str = ('Server died while waiting for %s, returncode %d' %
                            (waitForString, server_process.returncode))
                logging.error(died_str)
                raise Exception(died_str)
            if time.monotonic() - start_time > timeoutInSeconds:
                raise Exception('Timeout while waiting for %s' % waitForString)
            time.sleep(0.1)
            ready, self.lastLogIndex = outpipe.CapturedLogContains(
                waitForString, self.lastLogIndex)
            if ready:
                self.lastLogIndex += 1

        logging.debug('Success waiting for: %s' % waitForString)

    def __updateSetUpCode(self):
        qrLine = self.outpipe.FindLastMatchingLine('.*SetupQRCode: *\\[(.*)]')
        if not qrLine:
            raise Exception("Unable to find QR code")
        self.setupCode = qrLine.group(1)

    def __terminateProcess(self):
        if self.process:
            self.process.terminate()  # sends SIGTERM
            try:
                exit_code = self.process.wait(10)
                if exit_code:
                    raise Exception('Subprocess failed with exit code: %d' % exit_code)
            except subprocess.TimeoutExpired:
                logging.debug('Subprocess did not terminate on SIGTERM, killing it now')
                self.process.kill()
                # The exit code when using Python subprocess will be the signal used to kill it.
                # Ideally, we would recover the original exit code, but the process was already
                # ignoring SIGTERM, indicating something was already wrong.
                self.process.wait(10)
            self.process = None
            self.outpipe = None


class CommissionerApp():
    def __init__(self, runner, apps_register, paths: ApplicationPaths, dry_run):
        self.runner = runner
        self.apps_register = apps_register
        self.paths = paths
        self.dry_run = dry_run

    def pair(self, setup_code, extra_args):
        pass

    def test(self, yaml_file, pics_file, timeout_seconds, extra_args):
        pass

    def run(self, cmd, name, timeout_seconds=None):
        if self.dry_run:
            # Some of our command arguments have spaces in them, so if we are
            # trying to log commands people can run we should quote those.
            def quoter(arg): return f"'{arg}'" if ' ' in arg else arg
            logging.info(" ".join(map(quoter, cmd)))
            return

        self.runner.RunSubprocess(cmd, name=name,
                                  dependencies=[self.apps_register], timeout_seconds=timeout_seconds)

    def factoryReset(self):
        pass


class ChipReplCommissionerApp(CommissionerApp):
    def __init__(self, runner, apps_register, paths: ApplicationPaths, dry_run):
        super().__init__(runner, apps_register, paths, dry_run)

    def pair(self, setup_code, extra_args):
        self.setup_code = setup_code

    def test(self, yaml_file, pics_file, timeout_seconds, extra_args):
        cmd = self.paths.chip_repl_yaml_tester_cmd.copy()
        cmd += ['--setup-code', self.setup_code]
        cmd += ['--yaml-path', yaml_file]
        cmd += ["--pics-file", pics_file]

        self.run(cmd, 'CHIP_REPL_YAML_TESTER', timeout_seconds)


class ChipToolCommissionerApp(CommissionerApp):
    def __init__(self, runner, apps_register, paths: ApplicationPaths, dry_run):
        super().__init__(runner, apps_register, paths, dry_run)

        app_id = os.path.basename(self.paths.chip_tool_with_python_cmd[-1])
        self.__storage_dir = tempfile.mkdtemp()
        self.__snapshot_dir = os.path.join(tempfile.gettempdir(), f"chip_snapshot_{app_id}")

    def pair(self, setup_code, extra_args):
        cmd = self.paths.chip_tool_with_python_cmd.copy()
        cmd += ['pairing', 'code', TEST_NODE_ID, setup_code]
        cmd += extra_args
        cmd += ['--server_path', self.paths.chip_tool[-1]]
        cmd += ['--server_arguments', f'interactive server --storage-directory {self.__storage_dir}']

        if self.dry_run:
            self.run(cmd, 'PAIR')
        else:
            # If the app is already paired, restore previous state
            app = self.apps_register.get('default')
            if app.isPaired:
                self.restoreState()
            else:
                self.run(cmd, 'PAIR')
                app.saveState()
                self.saveState()

    def test(self, yaml_file, pics_file, timeout_seconds, extra_args):
        cmd = self.paths.chip_tool_with_python_cmd.copy()
        cmd += ['tests', yaml_file]
        cmd += ['--PICS', pics_file]
        cmd += extra_args
        cmd += ['--server_path', self.paths.chip_tool[-1]]
        cmd += ['--server_arguments', f'interactive server --storage-directory {self.__storage_dir}']

        self.run(cmd, 'TEST', timeout_seconds)

    def factoryReset(self):
        shutil.rmtree(self.__storage_dir, ignore_errors=True)

    def saveState(self):
        os.makedirs(self.__snapshot_dir, exist_ok=True)
        shutil.copytree(self.__storage_dir, self.__snapshot_dir, dirs_exist_ok=True)

    def restoreState(self):
        if not os.path.exists(self.__snapshot_dir):
            return False

        shutil.copytree(self.__snapshot_dir, self.__storage_dir, dirs_exist_ok=True)
        return True
