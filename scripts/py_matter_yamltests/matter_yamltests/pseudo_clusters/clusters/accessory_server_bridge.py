#
# SPDX-FileCopyrightText: 2023 Project CHIP Authors
#
# SPDX-License-Identifier: Apache-2.0

import sys
import xmlrpc.client

_DEFAULT_KEY = 'default'
_IP = '127.0.0.1'
_PORT = 9000

if sys.platform == 'linux':
    _IP = '10.10.10.5'


def _make_url():
    return f'http://{_IP}:{_PORT}/'


def _get_option(request, item_name: str, default_value=None):
    if request.arguments:
        values = request.arguments['values']
        for item in values:
            name = item['name']
            value = item['value']
            if name == item_name:
                return value

    return default_value


def _get_start_options(request):
    options = []

    if request.arguments:
        values = request.arguments['values']
        for item in values:
            name = item['name']
            value = item['value']

            if name == 'discriminator':
                options.append('--discriminator')
                options.append(str(value))
            elif name == 'port':
                options.append('--secured-device-port')
                options.append(str(value))
            elif name == 'kvs':
                options.append('--KVS')
                options.append(str(value))
            elif name == 'minCommissioningTimeout':
                options.append('--min_commissioning_timeout')
                options.append(str(value))
            elif name == 'filepath':
                options.append('--filepath')
                options.append(str(value))
            elif name == 'otaDownloadPath':
                options.append('--otaDownloadPath')
                options.append(str(value))
            elif name == 'registerKey':
                pass
            else:
                raise KeyError(f'Unknown key: {name}')

    return options


class AccessoryServerBridge():
    def start(request):
        register_key = _get_option(request, 'registerKey', _DEFAULT_KEY)
        options = _get_start_options(request)

        with xmlrpc.client.ServerProxy(_make_url(), allow_none=True) as proxy:
            proxy.start(register_key, options)

    def stop(request):
        register_key = _get_option(request, 'registerKey', _DEFAULT_KEY)

        with xmlrpc.client.ServerProxy(_make_url(), allow_none=True) as proxy:
            proxy.stop(register_key)

    def reboot(request):
        register_key = _get_option(request, 'registerKey', _DEFAULT_KEY)

        with xmlrpc.client.ServerProxy(_make_url(), allow_none=True) as proxy:
            proxy.reboot(register_key)

    def factoryReset(request):
        register_key = _get_option(request, 'registerKey', _DEFAULT_KEY)

        with xmlrpc.client.ServerProxy(_make_url(), allow_none=True) as proxy:
            proxy.factoryReset(register_key)

    def waitForMessage(request):
        register_key = _get_option(request, 'registerKey', _DEFAULT_KEY)
        message = _get_option(request, 'message')

        with xmlrpc.client.ServerProxy(_make_url(), allow_none=True) as proxy:
            proxy.waitForMessage(register_key, [message])

    def createOtaImage(request):
        otaImageFilePath = _get_option(request, 'otaImageFilePath')
        rawImageFilePath = _get_option(request, 'rawImageFilePath')
        rawImageContent = _get_option(request, 'rawImageContent')

        with xmlrpc.client.ServerProxy(_make_url(), allow_none=True) as proxy:
            proxy.createOtaImage(otaImageFilePath, rawImageFilePath, rawImageContent)

    def compareFiles(request):
        file1 = _get_option(request, 'file1')
        file2 = _get_option(request, 'file2')

        with xmlrpc.client.ServerProxy(_make_url(), allow_none=True) as proxy:
            proxy.compareFiles(file1, file2)
