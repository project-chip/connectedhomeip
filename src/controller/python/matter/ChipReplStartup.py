#
#    Copyright (c) 2021 Project CHIP Authors
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

import argparse
import atexit
import builtins
import logging
import os
import pathlib

import coloredlogs
from rich import inspect, pretty
from rich.console import Console

import matter.CertificateAuthority
import matter.clusters as Clusters  # noqa: F401
import matter.FabricAdmin
import matter.logging
import matter.native
from matter.ChipStack import ChipStack
from matter.storage import PersistentStorageINI, PersistentStorageJSON


def ReplInit(debug):
    #
    # Install the pretty printer that rich provides to replace the existing
    # printer.
    #
    pretty.install(indent_guides=True, expand_all=True)

    console = Console()

    console.rule('Matter REPL')
    console.print('''
            [bold blue]

            Welcome to the Matter Python REPL!

            For help, please type [/][bold green]matterhelp()[/][bold blue]

            To get more information on a particular object/class, you can pass
            that into [bold green]matterhelp()[/][bold blue] as well.

            ''')
    console.rule()

    coloredlogs.install(level='DEBUG')
    matter.logging.RedirectToPythonLogging()

    if debug:
        logging.getLogger().setLevel(logging.DEBUG)
    else:
        logging.getLogger().setLevel(logging.WARN)


certificateAuthorityManager = None


@atexit.register
def StackShutdown():
    if not certificateAuthorityManager:
        return
    certificateAuthorityManager.Shutdown()
    builtins.chipStack.Shutdown()


def matterhelp(classOrObj=None):
    if (classOrObj is None):
        inspect(builtins.devCtrl, methods=True, help=True, private=False)
        inspect(mattersetlog)
        inspect(mattersetdebug)
    else:
        inspect(classOrObj, methods=True, help=True, private=False)


def mattersetlog(level):
    logging.getLogger().setLevel(level)


def mattersetdebug(enableDebugMode: bool = True):
    ''' Enables debug mode that is utilized by some Matter modules
        to better facilitate debugging of failures (e.g throwing exceptions instead
        of returning well-formatted results).
    '''
    builtins.enableDebugMode = enableDebugMode


def main():
    console = Console()

    parser = argparse.ArgumentParser(formatter_class=argparse.ArgumentDefaultsHelpFormatter)
    parser.add_argument(
        "-d", "--debug", help="set default logging level to debug", action="store_true")
    parser.add_argument(
        "-s", "--storage-path", metavar="PATH", default="/tmp/repl-storage.json",
        help="path to persistent storage configuration file")
    parser.add_argument(
        "--chip-tool-common-storage-path", metavar="PATH",
        help="path to chip-tool common persistent storage configuration file (INI format)")
    parser.add_argument(
        "--chip-tool-fabric-storage-path", metavar="PATH",
        help="path to chip-tool fabric persistent storage configuration file (INI format)")
    parser.add_argument(
        "-t", "--trust-store", metavar="PATH", default="credentials/development/paa-root-certs",
        help="path to the PAA trust store")
    parser.add_argument(
        "-b", "--ble-controller", metavar="INDEX", type=int, default=0,
        help="BLE controller selector (see example or platform docs for details)")
    parser.add_argument(
        "-i", "--server-interactions", action="store_true",
        help="enable server interactions")
    args = parser.parse_args()

    if args.chip_tool_common_storage_path or args.chip_tool_fabric_storage_path:
        if not (args.chip_tool_common_storage_path and args.chip_tool_fabric_storage_path):
            console.print('''
[bold red]Error: [/][bold]One must specify both chip-tool common and chip-tool fabric storage paths[/]
            ''')
            return

    if not os.path.exists(args.trust_store):
        # there is a chance that the script is being run from a sub-path of a checkout.
        # try to adjust for convenience
        store_path = pathlib.Path(args.trust_store)
        if not store_path.is_absolute():
            prefix = pathlib.Path().absolute()
            while prefix != prefix.parent:
                if prefix.joinpath(store_path).exists():
                    oldpath = args.trust_store
                    args.trust_store = prefix.joinpath(store_path).as_posix()
                    console.print(f'''
[bold] Replacing [/] store path {oldpath} with {args.trust_store}
Note that you are still running from {os.getcwd()} so other relative paths may be off.
                    ''')
                    break
                prefix = prefix.parent

    if not os.path.exists(args.trust_store):
        console.print(f'''
[bold red] Missing directory:     [/]{args.trust_store}
[bold] Your current directory: [/]{os.getcwd()}

Please add a valid `--trust-store` argument to your script. If using jupyterlab,
the command should look like:

%run {{module.path}} --trust-store /chip/root/credentials/development/paa-root-certs

or run `os.chdir` to the root of your CHIP repository checkout.
        ''')
        # nothing we can do ... things will NOT work
        return

    matter.native.Init(bluetoothAdapter=args.ble_controller)

    global certificateAuthorityManager
    global chipStack
    global caList
    global devCtrl

    ReplInit(args.debug)

    if args.chip_tool_common_storage_path and args.chip_tool_fabric_storage_path:
        storage = PersistentStorageINI(args.chip_tool_common_storage_path, args.chip_tool_fabric_storage_path)
    else:
        storage = PersistentStorageJSON(args.storage_path)

    chipStack = ChipStack(storage, enableServerInteractions=args.server_interactions)
    certificateAuthorityManager = matter.CertificateAuthority.CertificateAuthorityManager(chipStack, chipStack.GetStorageManager())

    certificateAuthorityManager.LoadAuthoritiesFromStorage()

    if not certificateAuthorityManager.activeCaList:
        ca = certificateAuthorityManager.NewCertificateAuthority()
        ca.NewFabricAdmin(vendorId=0xFFF1, fabricId=1)
    elif not certificateAuthorityManager.activeCaList[0].adminList:
        certificateAuthorityManager.activeCaList[0].NewFabricAdmin(vendorId=0xFFF1, fabricId=1)

    caList = certificateAuthorityManager.activeCaList

    devCtrl = caList[0].adminList[0].NewController(paaTrustStorePath=args.trust_store)
    builtins.devCtrl = devCtrl

    console.print(
        '\n\n[blue]The following objects have been created:')

    console.print(
        '''\t[red]certificateAuthorityManager[blue]:\tManages a list of CertificateAuthority instances.
    \t[red]caList[blue]:\t\t\t\tThe list of CertificateAuthority instances.
    \t[red]caList\\[n].adminList\\[m][blue]:\t\tA specific FabricAdmin object at index m for the nth CertificateAuthority instance.''')

    console.print(
        f'\n\n[blue]Default CHIP Device Controller (NodeId: {devCtrl.nodeId}): '
        f'has been initialized to manage [bold red]caList[0].adminList[0][blue] (FabricId = {caList[0].adminList[0].fabricId}), '
        'and is available as [bold red]devCtrl')


if __name__ == "__main__":
    main()
