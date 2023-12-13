#
#    Copyright (c) 2023 Project CHIP Authors
#    All rights reserved.
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

import logging
import os
import re

import chip.native
import pytest
from chip import exceptions
from chip.clusters.Objects import ApplicationLauncher, Channel, ContentLauncher, KeypadInput, MediaPlayback, TargetNavigator
from common.utils import (connect_device, disconnect_device, discover_device, get_log_messages_from_response, get_setup_payload,
                          get_shell_commands_from_help_response, read_zcl_attribute, send_zcl_command)

cecKeyCode = KeypadInput.Enums.CecKeyCode
log = logging.getLogger(__name__)


@pytest.fixture(scope="session")
def binaryPath(request, rootDir):
    if request.config.getoption('binaryPath'):
        return request.config.getoption('binaryPath')
    else:
        return os.path.join(rootDir, 'examples/tv-app/openiotsdk/build/chip-openiotsdk-tv-app-example.elf')


@pytest.fixture(scope="session")
def controllerConfig(request):
    config = {
        'vendorId': 0xFFF1,
        'fabricId': 1,
        'persistentStoragePath': '/tmp/openiotsdk-test-storage.json'
    }
    return config


@pytest.mark.smoketest
def test_smoke_test(device):
    ret = device.wait_for_output("Open IoT SDK tv-app example application start")
    assert ret is not None and len(ret) > 0
    ret = device.wait_for_output("Open IoT SDK tv-app example application run")
    assert ret is not None and len(ret) > 0


@pytest.mark.commissioningtest
def test_commissioning(device, controller):
    assert controller is not None
    devCtrl = controller

    ret = device.wait_for_output("Open IoT SDK tv-app example application start")
    assert ret is not None and len(ret) > 0

    setupPayload = get_setup_payload(device)
    assert setupPayload is not None

    commissionable_device = discover_device(devCtrl, setupPayload)
    assert commissionable_device is not None

    assert commissionable_device.vendorId == int(setupPayload.attributes['VendorID'])
    assert commissionable_device.productId == int(setupPayload.attributes['ProductID'])
    assert commissionable_device.addresses[0] is not None

    nodeId = connect_device(devCtrl, setupPayload, commissionable_device)
    assert nodeId is not None
    log.info("Device {} connected".format(commissionable_device.addresses[0]))

    ret = device.wait_for_output("Commissioning completed successfully")
    assert ret is not None and len(ret) > 0

    assert disconnect_device(devCtrl, nodeId)


SHELL_MAIN_COMMANDS_NAME = ["base64", "exit", "help", "version",
                            "config", "device", "onboardingcodes", "dns",
                            "app", "stat"]

SHELL_APP_COMMANDS_NAME = ["help", "add", "remove", "setpin",
                           "add-admin-vendor"]

TV_CTRL_TEST_APP_PID = 1
TV_CTRL_TEST_APP_PIN_CODE = "34567890"
TV_CTRL_TEST_APP_ADMIN_VENDOR_ID = 65521


@pytest.mark.ctrltest
def test_command_check(device):
    try:
        chip.native.Init()
    except exceptions.ChipStackException as ex:
        log.error("CHIP initialization failed {}".format(ex))
        assert False
    except Exception:
        log.error("CHIP initialization failed")
        assert False

    ret = device.wait_for_output("Open IoT SDK tv-app example application start")
    assert ret is not None and len(ret) > 0
    ret = device.wait_for_output("Open IoT SDK tv-app example application run")
    assert ret is not None and len(ret) > 0

    # Wait for printing prompt
    ret = device.wait_for_output("Enter command")
    assert ret is not None and len(ret) > 0

    # Help
    ret = device.send(command="help", expected_output="Done")
    assert ret is not None and len(ret) > 1
    main_commands = get_shell_commands_from_help_response(ret[1:-1])
    assert set(SHELL_MAIN_COMMANDS_NAME) == set(main_commands)

    # App help
    ret = device.wait_for_output("Enter command")
    assert ret is not None and len(ret) > 0
    ret = device.send(command="app help", expected_output="Done")
    assert ret is not None and len(ret) > 1
    app_commands = get_shell_commands_from_help_response(ret[1:-2])
    assert set(SHELL_APP_COMMANDS_NAME) == set(app_commands)

    # App add
    ret = device.wait_for_output("Enter command")
    assert ret is not None and len(ret) > 0
    ret = device.send(command="app add {}".format(TV_CTRL_TEST_APP_PID), expected_output="Done")
    assert ret is not None and len(ret) > 1
    log_messages = get_log_messages_from_response(ret[1:-1])
    assert log_messages[-1] == "added app"
    result = re.findall(r'\d+', log_messages[-2])
    application_id = int(result[0])
    endpoint = int(result[1])
    index = int(result[2])
    assert application_id == TV_CTRL_TEST_APP_PID

    # App set pin
    ret = device.wait_for_output("Enter command")
    assert ret is not None and len(ret) > 0
    ret = device.send(command="app setpin {} {}".format(endpoint, TV_CTRL_TEST_APP_PIN_CODE), expected_output="Done")
    assert ret is not None and len(ret) > 1
    log_messages = get_log_messages_from_response(ret[1:-1])
    assert log_messages[-1] == "set pin success"

    # App remove
    ret = device.wait_for_output("Enter command")
    assert ret is not None and len(ret) > 0
    ret = device.send(command="app remove {}".format(endpoint), expected_output="Done")
    assert ret is not None and len(ret) > 1
    log_messages = get_log_messages_from_response(ret[1:-1])
    assert log_messages[-2] == "Removed device {} from dynamic endpoint {} (index={})".format(application_id, endpoint, index)
    assert log_messages[-1] == "removed app"

    # App add admin vendor
    ret = device.wait_for_output("Enter command")
    assert ret is not None and len(ret) > 0
    ret = device.send(command="app add-admin-vendor {}".format(TV_CTRL_TEST_APP_ADMIN_VENDOR_ID), expected_output="Done")
    assert ret is not None and len(ret) > 1
    log_messages = "added admin-vendor"


TV_CTRL_TEST_APP_ENDPOINT_ID = 1
TV_CTRL_TEST_APP_VENDOR_NAME = "Vendor1"
TV_CTRL_TEST_APP_NAME = "exampleid"
TV_CTRL_TEST_APP_PRODUCT_ID = 11
TV_CTRL_TEST_APP_VERSION = "Version1"
TV_CTRL_TEST_APP_TMP_ACCOUNT_ID = "tempAccountId"
TV_CTRL_TEST_APP_CATALOG_LIST = [123, 456]
TV_CTRL_TEST_CONTENT_PARAMETER_TYPE_1 = ContentLauncher.Enums.ParameterEnum.kActor
TV_CTRL_TEST_CONTENT_PARAMETER_VALUE_1 = "Gaby sHoffman"
TV_CTRL_TEST_CONTENT_PARAMETER_TYPE_2 = ContentLauncher.Enums.ParameterEnum.kEvent
TV_CTRL_TEST_CONTENT_PARAMETER_VALUE_2 = "Football games"
TV_CTRL_TEST_CONTENT_URL = "https://testurl"
TV_CTRL_TEST_CONTENT_DISPLAY_STRING = "Test string"
TV_CTRL_TEST_CONTENT_PROVIDER_NAME = "Test provider name"
TV_CTRL_TEST_KEY_PAD_SUPPORTED_KEY_LIST = [cecKeyCode.kUp, cecKeyCode.kDown, cecKeyCode.kLeft, cecKeyCode.kRight,
                                           cecKeyCode.kSelect, cecKeyCode.kBackward, cecKeyCode.kExit, cecKeyCode.kRootMenu, cecKeyCode.kSetupMenu]
TV_CTRL_TEST_KEY_PAD_UNSUPPORTED_KEY = cecKeyCode.kDisplayInformation
TV_CTRL_TEST_NAME = "Test name"


@pytest.mark.ctrltest
def test_tv_ctrl(device, controller):
    assert controller is not None
    devCtrl = controller

    ret = device.wait_for_output("Open IoT SDK tv-app example application start")
    assert ret is not None and len(ret) > 0

    setupPayload = get_setup_payload(device)
    assert setupPayload is not None

    # App add
    ret = device.wait_for_output("Enter command")
    assert ret is not None and len(ret) > 0
    ret = device.send(command="app add {}".format(TV_CTRL_TEST_APP_PID), expected_output="Done")
    assert ret is not None and len(ret) > 1
    log_messages = get_log_messages_from_response(ret[1:-1])
    assert log_messages[-1] == "added app"
    result = re.findall(r'\d+', log_messages[-2])
    application_id = int(result[0])
    endpoint = int(result[1])

    # App set pin
    ret = device.wait_for_output("Enter command")
    assert ret is not None and len(ret) > 0
    ret = device.send(command="app setpin {} {}".format(endpoint, TV_CTRL_TEST_APP_PIN_CODE), expected_output="Done")
    assert ret is not None and len(ret) > 1
    log_messages = get_log_messages_from_response(ret[1:-1])
    assert log_messages[-1] == "set pin success"

    # App add admin vendor
    ret = device.wait_for_output("Enter command")
    assert ret is not None and len(ret) > 0
    ret = device.send(command="app add-admin-vendor {}".format(TV_CTRL_TEST_APP_ADMIN_VENDOR_ID), expected_output="Done")
    assert ret is not None and len(ret) > 1
    log_messages = "added admin-vendor"

    commissionable_device = discover_device(devCtrl, setupPayload)
    assert commissionable_device is not None

    nodeId = connect_device(devCtrl, setupPayload, commissionable_device)
    assert nodeId is not None

    ret = device.wait_for_output("Commissioning completed successfully")
    assert ret is not None and len(ret) > 0

    # ApplicationBasic
    err, res = read_zcl_attribute(devCtrl, "ApplicationBasic", "VendorName", nodeId, endpoint)
    assert err == 0
    assert res.value == TV_CTRL_TEST_APP_VENDOR_NAME

    err, res = read_zcl_attribute(devCtrl, "ApplicationBasic", "VendorID", nodeId, endpoint)
    assert err == 0
    assert res.value == TV_CTRL_TEST_APP_PID

    err, res = read_zcl_attribute(devCtrl, "ApplicationBasic", "ApplicationName", nodeId, endpoint)
    assert err == 0
    assert res.value == TV_CTRL_TEST_APP_NAME

    err, res = read_zcl_attribute(devCtrl, "ApplicationBasic", "ProductID", nodeId, endpoint)
    assert err == 0
    assert res.value == TV_CTRL_TEST_APP_PRODUCT_ID

    err, res = read_zcl_attribute(devCtrl, "ApplicationBasic", "ApplicationVersion", nodeId, endpoint)
    assert err == 0
    assert res.value == TV_CTRL_TEST_APP_VERSION

    # AccountLogin
    err, res = send_zcl_command(devCtrl, "AccountLogin", "GetSetupPIN", nodeId, endpoint,
                                dict(tempAccountIdentifier=TV_CTRL_TEST_APP_TMP_ACCOUNT_ID),
                                requestTimeoutMs=1000)
    assert err == 0
    assert res.setupPIN == TV_CTRL_TEST_APP_PIN_CODE

    err, res = send_zcl_command(devCtrl, "AccountLogin", "Login", nodeId, endpoint,
                                dict(tempAccountIdentifier=TV_CTRL_TEST_APP_TMP_ACCOUNT_ID, setupPIN=TV_CTRL_TEST_APP_PIN_CODE),
                                requestTimeoutMs=1000)
    assert err == 0
    ret = device.wait_for_output("AccountLoginManager::HandleLogin success")
    assert ret is not None and len(ret) > 0

    err, res = send_zcl_command(devCtrl, "AccountLogin", "Logout", nodeId, endpoint,
                                None,
                                requestTimeoutMs=1000)
    assert err == 0
    ret = device.wait_for_output("AccountLoginManager::HandleLogout success")
    assert ret is not None and len(ret) > 0

    # ApplicationLauncher
    err, res = send_zcl_command(devCtrl, "ApplicationLauncher", "LaunchApp", nodeId, endpoint,
                                dict(application=ApplicationLauncher.Structs.ApplicationStruct(
                                    catalogVendorID=TV_CTRL_TEST_APP_ADMIN_VENDOR_ID, applicationID=application_id)),
                                requestTimeoutMs=1000)
    assert err == 0
    assert res.status == ApplicationLauncher.Enums.StatusEnum.kSuccess
    ret = device.wait_for_output("ApplicationLauncherManager::HandleLaunchApp")
    assert ret is not None and len(ret) > 0

    err, res = send_zcl_command(devCtrl, "ApplicationLauncher", "StopApp", nodeId, endpoint,
                                dict(application=ApplicationLauncher.Structs.ApplicationStruct(
                                    catalogVendorID=TV_CTRL_TEST_APP_ADMIN_VENDOR_ID, applicationID=application_id)),
                                requestTimeoutMs=1000)
    assert err == 0
    assert res.status == ApplicationLauncher.Enums.StatusEnum.kSuccess
    ret = device.wait_for_output("ApplicationLauncherManager::HandleStopApp")
    assert ret is not None and len(ret) > 0

    err, res = send_zcl_command(devCtrl, "ApplicationLauncher", "HideApp", nodeId, endpoint,
                                dict(application=ApplicationLauncher.Structs.ApplicationStruct(
                                    catalogVendorID=TV_CTRL_TEST_APP_ADMIN_VENDOR_ID, applicationID=application_id)),
                                requestTimeoutMs=1000)
    assert err == 0
    assert res.status == ApplicationLauncher.Enums.StatusEnum.kSuccess
    ret = device.wait_for_output("ApplicationLauncherManager::HandleHideApp")
    assert ret is not None and len(ret) > 0

    err, res = read_zcl_attribute(devCtrl, "ApplicationLauncher", "CatalogList", nodeId, endpoint)
    assert err == 0
    assert res.value == TV_CTRL_TEST_APP_CATALOG_LIST

    # Channel
    err, res = read_zcl_attribute(devCtrl, "Channel", "ChannelList", nodeId, endpoint)
    assert err == 0
    assert res.status == 0
    channels = res.value

    err, res = read_zcl_attribute(devCtrl, "Channel", "Lineup", nodeId, endpoint)
    assert err == 0
    assert res.value.lineupInfoType == Channel.Enums.LineupInfoTypeEnum.kMso

    err, res = read_zcl_attribute(devCtrl, "Channel", "CurrentChannel", nodeId, endpoint)
    assert err == 0
    assert res.value == channels[0]

    err, res = send_zcl_command(devCtrl, "Channel", "ChangeChannel", nodeId, endpoint,
                                dict(match=channels[1].name),
                                requestTimeoutMs=1000)
    assert err == 0
    assert res.status == Channel.Enums.StatusEnum.kSuccess

    err, res = read_zcl_attribute(devCtrl, "Channel", "CurrentChannel", nodeId, endpoint)
    assert err == 0
    assert res.value == channels[1]

    err, res = send_zcl_command(devCtrl, "Channel", "ChangeChannelByNumber", nodeId, endpoint,
                                dict(majorNumber=channels[0].majorNumber, minorNumber=channels[0].minorNumber),
                                requestTimeoutMs=1000)
    assert err == 0

    err, res = read_zcl_attribute(devCtrl, "Channel", "CurrentChannel", nodeId, endpoint)
    assert err == 0
    assert res.value == channels[0]

    err, res = send_zcl_command(devCtrl, "Channel", "SkipChannel", nodeId, endpoint,
                                dict(count=1),
                                requestTimeoutMs=1000)
    assert err == 0

    err, res = read_zcl_attribute(devCtrl, "Channel", "CurrentChannel", nodeId, endpoint)
    assert err == 0
    assert res.value == channels[1]

    # ContentLauncher
    err, res = read_zcl_attribute(devCtrl, "ContentLauncher", "AcceptHeader", nodeId, endpoint)
    assert err == 0
    assert res.status == 0
    ret = device.wait_for_output("ContentLauncherManager::HandleGetAcceptHeaderList")
    assert ret is not None and len(ret) > 0

    err, res = read_zcl_attribute(devCtrl, "ContentLauncher", "SupportedStreamingProtocols", nodeId, endpoint)
    assert err == 0
    assert res.value == ContentLauncher.Bitmaps.SupportedProtocolsBitmap.kDash | ContentLauncher.Bitmaps.SupportedProtocolsBitmap.kHls

    err, res = send_zcl_command(devCtrl, "ContentLauncher", "LaunchContent", nodeId, endpoint,
                                dict(search=ContentLauncher.Structs.ContentSearchStruct(parameterList=[
                                    ContentLauncher.Structs.ParameterStruct(
                                        type=TV_CTRL_TEST_CONTENT_PARAMETER_TYPE_1,
                                        value=TV_CTRL_TEST_CONTENT_PARAMETER_VALUE_1),
                                    ContentLauncher.Structs.ParameterStruct(
                                        type=TV_CTRL_TEST_CONTENT_PARAMETER_TYPE_2,
                                        value=TV_CTRL_TEST_CONTENT_PARAMETER_VALUE_2)
                                ]),
                                    autoPlay=True, data=None),
                                requestTimeoutMs=1000)
    assert err == 0
    assert res.status == ContentLauncher.Enums.StatusEnum.kSuccess
    ret = device.wait_for_output("ContentLauncherManager::HandleLaunchContent")
    assert ret is not None and len(ret) > 0
    ret = device.wait_for_output("TEST CASE found match=TV Show Example type={}".format(TV_CTRL_TEST_CONTENT_PARAMETER_TYPE_1))
    assert ret is not None and len(ret) > 0
    ret = device.wait_for_output("TEST CASE found match=Sports Example type={}".format(TV_CTRL_TEST_CONTENT_PARAMETER_TYPE_2))
    assert ret is not None and len(ret) > 0

    err, res = send_zcl_command(devCtrl, "ContentLauncher", "LaunchURL", nodeId, endpoint,
                                dict(contentURL=TV_CTRL_TEST_CONTENT_URL,
                                     displayString=TV_CTRL_TEST_CONTENT_DISPLAY_STRING,
                                     brandingInformation=ContentLauncher.Structs.BrandingInformationStruct(
                                         providerName=TV_CTRL_TEST_CONTENT_PROVIDER_NAME,
                                         background=None,
                                         logo=None,
                                         progressBar=None,
                                         splash=None,
                                         waterMark=None
                                     )),
                                requestTimeoutMs=1000)
    assert err == 0
    assert res.status == ContentLauncher.Enums.StatusEnum.kSuccess
    ret = device.wait_for_output("ContentLauncherManager::HandleLaunchUrl")
    assert ret is not None and len(ret) > 0
    ret = device.wait_for_output("ContentLauncherManager::HandleLaunchUrl TEST CASE ContentURL={} DisplayString={} BrandingInformation.ProviderName={}".format(
        TV_CTRL_TEST_CONTENT_URL,
        TV_CTRL_TEST_CONTENT_DISPLAY_STRING,
        TV_CTRL_TEST_CONTENT_PROVIDER_NAME))
    assert ret is not None and len(ret) > 0

    # KeypadInput
    for keyCode in TV_CTRL_TEST_KEY_PAD_SUPPORTED_KEY_LIST:
        err, res = send_zcl_command(devCtrl, "KeypadInput", "SendKey", nodeId, endpoint,
                                    dict(keyCode=keyCode), requestTimeoutMs=1000)
        assert err == 0
        assert res.status == KeypadInput.Enums.StatusEnum.kSuccess

    err, res = send_zcl_command(devCtrl, "KeypadInput", "SendKey", nodeId, endpoint,
                                dict(keyCode=TV_CTRL_TEST_KEY_PAD_UNSUPPORTED_KEY),
                                requestTimeoutMs=1000)
    assert err == 0
    assert res.status == KeypadInput.Enums.StatusEnum.kUnsupportedKey

    # TargetNavigator
    err, res = read_zcl_attribute(devCtrl, "TargetNavigator", "TargetList", nodeId, endpoint)
    assert err == 0
    assert res.status == 0

    err, res = read_zcl_attribute(devCtrl, "TargetNavigator", "CurrentTarget", nodeId, endpoint)
    assert err == 0
    assert res.status == 0
    current_target_id = res.value

    current_target_id += 1
    err, res = send_zcl_command(devCtrl, "TargetNavigator", "NavigateTarget", nodeId, endpoint,
                                dict(target=current_target_id, data=None),
                                requestTimeoutMs=1000)
    assert err == 0
    assert res.status == TargetNavigator.Enums.StatusEnum.kSuccess

    err, res = read_zcl_attribute(devCtrl, "TargetNavigator", "CurrentTarget", nodeId, endpoint)
    assert err == 0
    assert res.status == 0
    assert res.value == current_target_id

    # AudioOutput
    err, res = read_zcl_attribute(devCtrl, "AudioOutput", "OutputList", nodeId, TV_CTRL_TEST_APP_ENDPOINT_ID)
    assert err == 0
    assert res.status == 0

    err, res = read_zcl_attribute(devCtrl, "AudioOutput", "CurrentOutput", nodeId, TV_CTRL_TEST_APP_ENDPOINT_ID)
    assert err == 0
    current_audio_output = res.value

    current_audio_output += 1
    err, res = send_zcl_command(devCtrl, "AudioOutput", "SelectOutput", nodeId, TV_CTRL_TEST_APP_ENDPOINT_ID,
                                dict(index=current_audio_output),
                                requestTimeoutMs=1000)
    assert err == 0

    err, res = read_zcl_attribute(devCtrl, "AudioOutput", "CurrentOutput", nodeId, TV_CTRL_TEST_APP_ENDPOINT_ID)
    assert err == 0
    assert res.value == current_audio_output

    err, res = send_zcl_command(devCtrl, "AudioOutput", "RenameOutput", nodeId, TV_CTRL_TEST_APP_ENDPOINT_ID,
                                dict(index=current_audio_output, name=TV_CTRL_TEST_NAME),
                                requestTimeoutMs=1000)
    assert err == 0

    err, res = read_zcl_attribute(devCtrl, "AudioOutput", "OutputList", nodeId, TV_CTRL_TEST_APP_ENDPOINT_ID)
    assert err == 0
    assert res.status == 0
    audio_output = next(item for item in res.value if item.index == current_audio_output)
    assert audio_output.name == TV_CTRL_TEST_NAME

    # MediaInput
    err, res = read_zcl_attribute(devCtrl, "MediaInput", "InputList", nodeId, TV_CTRL_TEST_APP_ENDPOINT_ID)
    assert err == 0
    assert res.status == 0
    media_inputs = res.value

    err, res = read_zcl_attribute(devCtrl, "MediaInput", "CurrentInput", nodeId, TV_CTRL_TEST_APP_ENDPOINT_ID)
    assert err == 0
    current_media_input = res.value

    current_media_input += 1
    err, res = send_zcl_command(devCtrl, "MediaInput", "SelectInput", nodeId, TV_CTRL_TEST_APP_ENDPOINT_ID,
                                dict(index=current_media_input),
                                requestTimeoutMs=1000)
    assert err == 0

    err, res = read_zcl_attribute(devCtrl, "MediaInput", "CurrentInput", nodeId, TV_CTRL_TEST_APP_ENDPOINT_ID)
    assert err == 0
    assert res.value == current_media_input

    err, res = send_zcl_command(devCtrl, "MediaInput", "ShowInputStatus", nodeId, TV_CTRL_TEST_APP_ENDPOINT_ID,
                                None,
                                requestTimeoutMs=1000)
    assert err == 0
    ret = device.wait_for_output("MediaInputManager::HandleShowInputStatus")
    assert ret is not None and len(ret) > 0
    for media_input in media_inputs:
        ret = device.wait_for_output("[{}] type={} selected={} name={} desc={}".format(
            media_input.index,
            media_input.inputType,
            1 if media_input.index == current_media_input else 0,
            media_input.name,
            media_input.description
        ))
        assert ret is not None and len(ret) > 0

    err, res = send_zcl_command(devCtrl, "MediaInput", "HideInputStatus", nodeId, TV_CTRL_TEST_APP_ENDPOINT_ID,
                                None,
                                requestTimeoutMs=1000)
    assert err == 0
    ret = device.wait_for_output("MediaInputManager::HandleHideInputStatus")
    assert ret is not None and len(ret) > 0

    err, res = send_zcl_command(devCtrl, "MediaInput", "RenameInput", nodeId, TV_CTRL_TEST_APP_ENDPOINT_ID,
                                dict(index=current_media_input, name=TV_CTRL_TEST_NAME),
                                requestTimeoutMs=1000)
    assert err == 0

    err, res = read_zcl_attribute(devCtrl, "MediaInput", "InputList", nodeId, TV_CTRL_TEST_APP_ENDPOINT_ID)
    assert err == 0
    assert res.status == 0
    media_input = next(item for item in res.value if item.index == current_media_input)
    assert media_input.name == TV_CTRL_TEST_NAME

    # MediaPlayback
    err, res = send_zcl_command(devCtrl, "MediaPlayback", "Pause", nodeId, TV_CTRL_TEST_APP_ENDPOINT_ID,
                                None,
                                requestTimeoutMs=1000)
    assert err == 0
    assert res.status == MediaPlayback.Enums.StatusEnum.kSuccess

    err, res = read_zcl_attribute(devCtrl, "MediaPlayback", "CurrentState", nodeId, TV_CTRL_TEST_APP_ENDPOINT_ID)
    assert err == 0
    assert res.value == MediaPlayback.Enums.PlaybackStateEnum.kPaused

    err, res = send_zcl_command(devCtrl, "MediaPlayback", "Stop", nodeId, TV_CTRL_TEST_APP_ENDPOINT_ID,
                                None,
                                requestTimeoutMs=1000)
    assert err == 0
    assert res.status == MediaPlayback.Enums.StatusEnum.kSuccess

    err, res = read_zcl_attribute(devCtrl, "MediaPlayback", "CurrentState", nodeId, TV_CTRL_TEST_APP_ENDPOINT_ID)
    assert err == 0
    assert res.value == MediaPlayback.Enums.PlaybackStateEnum.kNotPlaying

    err, res = send_zcl_command(devCtrl, "MediaPlayback", "Play", nodeId, TV_CTRL_TEST_APP_ENDPOINT_ID,
                                None,
                                requestTimeoutMs=1000)
    assert err == 0
    assert res.status == MediaPlayback.Enums.StatusEnum.kSuccess

    err, res = read_zcl_attribute(devCtrl, "MediaPlayback", "CurrentState", nodeId, TV_CTRL_TEST_APP_ENDPOINT_ID)
    assert err == 0
    assert res.value == MediaPlayback.Enums.PlaybackStateEnum.kPlaying

    # LowPower
    err, res = send_zcl_command(devCtrl, "LowPower", "Sleep", nodeId, TV_CTRL_TEST_APP_ENDPOINT_ID,
                                None,
                                requestTimeoutMs=1000)
    assert err == 0
    ret = device.wait_for_output("LowPowerManager::HandleSleep")
    assert ret is not None and len(ret) > 0

    # WakeOnLan
    err, res = read_zcl_attribute(devCtrl, "WakeOnLan", "MACAddress", nodeId, TV_CTRL_TEST_APP_ENDPOINT_ID)
    assert err == 0
    assert res.value == "000000000000"

    assert disconnect_device(devCtrl, nodeId)
