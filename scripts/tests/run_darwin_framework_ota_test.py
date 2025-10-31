#! /usr/bin/env -S python3 -B

import io
import json
import logging
import time
from pathlib import Path
from subprocess import PIPE

import click
from chiptest.accessories import AppsRegister
from chiptest.runner import Subprocess, Runner
from chiptest.test_definition import App, ExecutionCapture
from chipyaml.paths_finder import PathsFinder

TEST_NODE_ID = '0x12344321'
TEST_VID = '0xFFF1'
TEST_PID = '0x8001'


class DarwinToolRunner:
    def __init__(self, runner, application):
        self.process = None
        self.outpipe = None
        self.runner = runner
        self.lastLogIndex = 0
        self.application = application
        self.stdin = None

    def start(self):
        self.process, self.outpipe, errpipe = self.runner.RunSubprocess(self.application,
                                                                        name='DARWIN-TOOL',
                                                                        wait=False,
                                                                        stdin=PIPE)
        self.stdin = io.TextIOWrapper(self.process.stdin, line_buffering=True)

    def stop(self):
        if self.process:
            self.process.kill()

    def waitForMessage(self, message):
        logging.debug('Waiting for %s' % message)

        start_time = time.monotonic()
        ready, self.lastLogIndex = self.outpipe.CapturedLogContains(
            message, self.lastLogIndex)
        while not ready:
            if self.process.poll() is not None:
                died_str = ('Process died while waiting for %s, returncode %d' %
                            (message, self.process.returncode))
                logging.error(died_str)
                raise Exception(died_str)
            if time.monotonic() - start_time > 10:
                raise Exception('Timeout while waiting for %s' % message)
            time.sleep(0.1)
            ready, self.lastLogIndex = self.outpipe.CapturedLogContains(
                message, self.lastLogIndex)

        logging.debug('Success waiting for: %s' % message)


class InteractiveDarwinTool(DarwinToolRunner):
    def __init__(self, runner, prompt, application):
        self.prompt = prompt
        super().__init__(runner, application)

    def waitForPrompt(self):
        self.waitForMessage(self.prompt)

    def sendCommand(self, command):
        logging.debug('Sending command %s' % command)
        print(command, file=self.stdin)
        self.waitForPrompt()


@click.group(chain=True)
@click.pass_context
def main(context):
    pass


@main.command(
    'run', help='Execute the test')
@click.option(
    '--darwin-framework-tool',
    help="what darwin-framework-tool to use")
@click.option(
    '--ota-requestor-app',
    help='what ota requestor app to use')
@click.option(
    '--ota-data-file',
    required=True,
    help='The file to use to store our OTA data.  This file does not need to exist.')
@click.option(
    '--ota-image-file',
    required=True,
    help='The file to use to store the OTA image we plan to send.  This file does not need to exist.')
@click.option(
    '--ota-destination-file',
    required=True,
    help='The destination file to use for the requestor\'s download.  This file does not need to exist.')
@click.option(
    '--ota-candidate-file',
    required=True,
    help='The file to use for our OTA candidate JSON.  This file does not need to exist.')
@click.pass_context
def cmd_run(context, darwin_framework_tool, ota_requestor_app, ota_data_file, ota_image_file, ota_destination_file, ota_candidate_file):
    paths_finder = PathsFinder()

    if darwin_framework_tool is None:
        darwin_framework_tool = paths_finder.get('darwin-framework-tool')
    if ota_requestor_app is None:
        ota_requestor_app = paths_finder.get('chip-ota-requestor-app')

    if darwin_framework_tool is not None:
        darwin_framework_tool = Subprocess(kind='tool', path=Path(darwin_framework_tool))

    if ota_requestor_app is not None:
        ota_requestor_app = Subprocess(kind='app', path=Path(ota_requestor_app)
                                        ).add_args(('--otaDownloadPath', ota_destination_file))

    runner = Runner()
    runner.capture_delegate = ExecutionCapture()

    apps_register = AppsRegister()
    apps_register.init()

    darwin_tool = None

    try:
        apps_register.createOtaImage(ota_image_file, ota_data_file, "This is some test OTA data", vid=TEST_VID, pid=TEST_PID)
        json_data = {
            "deviceSoftwareVersionModel": [{
                "vendorId": int(TEST_VID, 16),
                "productId": int(TEST_PID, 16),
                "softwareVersion": 2,
                "softwareVersionString": "2.0",
                "cDVersionNumber": 18,
                "softwareVersionValid": True,
                "minApplicableSoftwareVersion": 0,
                "maxApplicableSoftwareVersion": 100,
                "otaURL": ota_image_file
            }]
        }
        with open(ota_candidate_file, "w") as f:
            json.dump(json_data, f)

        requestor_app = App(runner, ota_requestor_app)
        apps_register.add('default', requestor_app)

        requestor_app.start()

        pairing_cmd = darwin_framework_tool.add_args(('pairing', 'code', TEST_NODE_ID, requestor_app.setupCode))
        runner.RunSubprocess(pairing_cmd, name='PAIR', dependencies=[apps_register])

        # pairing get-commissioner-node-id does not seem to work right in interactive mode for some reason
        darwin_tool = DarwinToolRunner(runner, darwin_framework_tool.add_args(('pairing', 'get-commissioner-node-id')))
        darwin_tool.start()
        darwin_tool.waitForMessage(": Commissioner Node Id")
        nodeIdLine = darwin_tool.outpipe.FindLastMatchingLine('.*: Commissioner Node Id (0x[0-9A-F]+)')
        if not nodeIdLine:
            raise Exception("Unable to find commissioner node id")
        commissionerNodeId = nodeIdLine.group(1)
        darwin_tool.stop()

        prompt = "WAITING FOR COMMANDS NOW"
        darwin_tool = InteractiveDarwinTool(runner, prompt, darwin_framework_tool.add_args(
            ("interactive", "start", "--additional-prompt", prompt)))
        darwin_tool.start()

        darwin_tool.waitForPrompt()

        darwin_tool.sendCommand("otasoftwareupdateapp candidate-file-path %s" % ota_candidate_file)
        darwin_tool.sendCommand("otasoftwareupdateapp set-reply-params --status 0")
        darwin_tool.sendCommand("otasoftwareupdaterequestor announce-otaprovider %s 0 0 0 %s 0" %
                                (commissionerNodeId, TEST_NODE_ID))

        # Now wait for the OTA download to finish.
        requestor_app.waitForMessage("OTA image downloaded to %s" % ota_destination_file)

        # Make sure the right thing was downloaded.
        apps_register.compareFiles(ota_data_file, ota_destination_file)

    except Exception:
        logging.error("!!!!!!!!!!!!!!!!!!!! ERROR !!!!!!!!!!!!!!!!!!!!!!")
        runner.capture_delegate.LogContents()
        raise
    finally:
        if darwin_tool is not None:
            darwin_tool.stop()
        apps_register.killAll()
        apps_register.factoryResetAll()
        apps_register.removeAll()
        apps_register.uninit()


if __name__ == '__main__':
    main(auto_envvar_prefix='CHIP')
