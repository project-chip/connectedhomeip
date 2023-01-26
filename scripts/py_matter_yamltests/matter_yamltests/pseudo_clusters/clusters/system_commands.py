#
#    Copyright (c) 2023 Project CHIP Authors
#
#    Licensed under the Apache License, Version 2.0 (the 'License');
#    you may not use this file except in compliance with the License.
#    You may obtain a copy of the License at
#
#        http://www.apache.org/licenses/LICENSE-2.0
#
#    Unless required by applicable law or agreed to in writing, software
#    distributed under the License is distributed on an 'AS IS' BASIS,
#    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#    See the License for the specific language governing permissions and
#    limitations under the License.

import sys
import xmlrpc.client

from ..pseudo_cluster import PseudoCluster

DEFAULT_KEY = 'default'
IP = '127.0.0.1'
PORT = 9000

if sys.platform == 'linux':
    IP = '10.10.10.5'


def make_url():
    return 'http://' + IP + ':' + str(PORT) + '/'


def get_register_key(request):
    if request.arguments:
        values = request.arguments['values']
        for item in values:
            name = item['name']
            value = item['value']
            if name == 'registerKey':
                return value

    return DEFAULT_KEY


def get_options(request):
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


class SystemCommands(PseudoCluster):
    name = 'SystemCommands'

    async def Start(self, request):
        register_key = get_register_key(request)
        options = get_options(request)

        with xmlrpc.client.ServerProxy(make_url(), allow_none=True) as proxy:
            proxy.start(register_key, options)

    async def Stop(self, request):
        register_key = get_register_key(request)

        with xmlrpc.client.ServerProxy(make_url(), allow_none=True) as proxy:
            proxy.stop(register_key)

    async def Reboot(self, request):
        register_key = get_register_key(request)

        with xmlrpc.client.ServerProxy(make_url(), allow_none=True) as proxy:
            proxy.reboot(register_key)

    async def FactoryReset(self, request):
        register_key = get_register_key(request)

        with xmlrpc.client.ServerProxy(make_url(), allow_none=True) as proxy:
            proxy.factoryReset(register_key)
