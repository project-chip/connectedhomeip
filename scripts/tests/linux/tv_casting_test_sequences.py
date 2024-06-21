#!/usr/bin/env -S python3 -B

# Copyright (c) 2024 Project CHIP Authors
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

from linux.tv_casting_test_sequence_utils import App, Sequence, Step

"""
In this file, we define the test sequences with the relevant steps that will be used in the `scripts/tests/run_tv_casting_test.py` 
for validating the casting experience between the Linux tv-casting-app and the Linux tv-app. 

At the beginning of each test sequence we need to indicate the start up of the tv-app using the `START_APP` string as the `input_cmd`
followed by the same for the tv-casting-app. On the other hand, at the end of each test sequence we need to ensure that each app will 
be stopped by providing the `STOP_APP` string as the `input_cmd`. As noted in the example below of `example_test_sequence`, the first
four steps pertain to starting the apps while the last two are for signaling stopping the apps. 

Note: `START_APP` and `STOP_APP` are reserved for signaling the starting and stopping of apps.

Example:
     test_sequences = [
          Sequence(
               name='example_test_sequence',
               step=[
                    # Signal to start the tv-app.
                    Step(app=App.TV_APP, input_cmd=START_APP),

                    # Validate that the tv-app is up and running.
                    Step(app=App.TV_APP, timeout_sec=APP_MAX_START_WAIT_SEC, output_msg=['Started commissioner']),

                    # Signal to start the tv-casting-app.
                    Step(app=App.TV_CASTING_APP, input_cmd=START_APP),

                    # Validate that the server is properly initialized in the tv-casting-app output.
                    Step(app=App.TV_CASTING_APP, timeout_sec=APP_MAX_START_WAIT_SEC, output_msg=['Server initialization complete']),

                    # Additional steps for testing the casting experience.

                    # Signal to stop the tv-casting-app as we finished validation.
                    Step(app=App.TV_CASTING_APP, input_cmd=STOP_APP),

                    # Signal to stop the tv-app as we finished validation.
                    Step(app=App.TV_APP, input_cmd=STOP_APP)
               ]
          )
     ]
"""

# Signal to start the app.
START_APP = 'START'

# Signal to stop the app.
STOP_APP = 'STOP'

# The maximum amount of time to wait for the Linux tv-app or Linux tv-casting-app to start before timeout.
APP_MAX_START_WAIT_SEC = 2

# Values that identify the Linux tv-app and are noted in the 'Device Configuration' in the Linux tv-app output
# as well as under the 'Discovered Commissioner' details in the Linux tv-casting-app output.
VENDOR_ID = 0xFFF1   # Spec 7.20.2.1 MEI code: test vendor IDs are 0xFFF1 to 0xFFF4
PRODUCT_ID = 0x8001  # Test product id
DEVICE_TYPE_CASTING_VIDEO_PLAYER = 0x23    # Device type library 10.3: Casting Video Player

TEST_TV_CASTING_APP_DEVICE_NAME = 'Test TV casting app'  # Test device name for identifying the tv-casting-app

# Values to verify the subscription state against from the `ReportDataMessage` in the Linux tv-casting-app output.
CLUSTER_MEDIA_PLAYBACK = '0x506'  # Application Cluster Spec 6.10.3 Cluster ID: Media Playback
ATTRIBUTE_CURRENT_PLAYBACK_STATE = '0x0000_0000'  # Application Cluster Spec 6.10.6 Attribute ID: Current State of Playback

test_sequences = [
    Sequence(
        name='commissionee_generated_passcode_test',
        steps=[
             # Signal to start the tv-app.
            Step(app=App.TV_APP, input_cmd=START_APP),

            # Validate that the tv-app is up and running.
            Step(app=App.TV_APP, timeout_sec=APP_MAX_START_WAIT_SEC, output_msg=['Started commissioner']),

            # Signal to start the tv-casting-app.
            Step(app=App.TV_CASTING_APP, input_cmd=START_APP),

            # Validate that the server is properly initialized in the tv-casting-app output.
            Step(app=App.TV_CASTING_APP, timeout_sec=APP_MAX_START_WAIT_SEC, output_msg=['Server initialization complete']),

            # Validate that there is a valid discovered commissioner with {VENDOR_ID}, {PRODUCT_ID}, and {DEVICE_TYPE_CASTING_VIDEO_PLAYER} in the tv-casting-app output.
            Step(app=App.TV_CASTING_APP, output_msg=['Discovered Commissioner #0', f'Vendor ID: {VENDOR_ID}', f'Product ID: {PRODUCT_ID}',
                                                     f'Device Type: {DEVICE_TYPE_CASTING_VIDEO_PLAYER}', 'Supports Commissioner Generated Passcode: true']),

            # Validate that we are ready to send `cast request` command to the tv-casting-app subprocess.
            Step(app=App.TV_CASTING_APP, output_msg=['Example: cast request 0']),

            # Send `cast request {valid_discovered_commissioner_number}\n` command to the tv-casting-app subprocess.
            Step(app=App.TV_CASTING_APP, input_cmd='cast request 0\n'),

            # Validate that the `Identification Declaration` message block in the tv-casting-app output has the expected values for `device Name`, `vendor id`, and `product id`.
            Step(app=App.TV_CASTING_APP, output_msg=['Identification Declaration Start', f'device Name: {TEST_TV_CASTING_APP_DEVICE_NAME}',
                                                     f'vendor id: {VENDOR_ID}', f'product id: {PRODUCT_ID}', 'Identification Declaration End']),

            # Validate that the `Identification Declaration` message block in the tv-app output has the expected values for `device Name`, `vendor id`, and `product id`.
            Step(app=App.TV_APP, output_msg=['Identification Declaration Start', f'device Name: {TEST_TV_CASTING_APP_DEVICE_NAME}',
                                             f'vendor id: {VENDOR_ID}', f'product id: {PRODUCT_ID}', 'Identification Declaration End']),

            # Validate that we received the cast request from the tv-casting-app on the tv-app output.
            Step(app=App.TV_APP,
                 output_msg=['PROMPT USER: Test TV casting app is requesting permission to cast to this TV, approve?']),

            # Validate that we received the instructions on the tv-app output for sending the `controller ux ok` command.
            Step(app=App.TV_APP, output_msg=['Via Shell Enter: controller ux ok|cancel']),

            # Send `controller ux ok` command to the tv-app subprocess.
            Step(app=App.TV_APP, input_cmd='controller ux ok\n'),

            # Validate that pairing succeeded between the tv-casting-app and the tv-app.
            Step(app=App.TV_APP, output_msg=['Secure Pairing Success']),

            # Validate that commissioning succeeded in the tv-casting-app output.
            Step(app=App.TV_CASTING_APP, output_msg=['Commissioning completed successfully']),

            # Validate that commissioning succeeded in the tv-app output.
            Step(app=App.TV_APP, output_msg=['------PROMPT USER: commissioning success']),

            # Validate the subscription state by looking at the `Cluster` and `Attribute` values in the `ReportDataMessage` block in the tv-casting-app output.
            Step(app=App.TV_CASTING_APP, output_msg=[
                'ReportDataMessage =', f'Cluster = {CLUSTER_MEDIA_PLAYBACK}', f'Attribute = {ATTRIBUTE_CURRENT_PLAYBACK_STATE}', 'InteractionModelRevision =', '}']),

            # Validate the LaunchURL in the tv-app output.
            Step(app=App.TV_APP,
                 output_msg=['ContentLauncherManager::HandleLaunchUrl TEST CASE ContentURL=https://www.test.com/videoid DisplayString=Test video']),

            # Validate the LaunchURL in the tv-casting-app output.
            Step(app=App.TV_CASTING_APP, output_msg=['InvokeResponseMessage =',
                                                     'exampleData', 'InteractionModelRevision =', '},']),

            # Signal to stop the tv-casting-app as we finished validation.
            Step(app=App.TV_CASTING_APP, input_cmd=STOP_APP),

            # Signal to stop the tv-app as we finished validation.
            Step(app=App.TV_APP, input_cmd=STOP_APP)
        ]
    )
]
