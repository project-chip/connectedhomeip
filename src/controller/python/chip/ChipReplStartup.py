import argparse
import atexit
import builtins
import logging
import os
import pathlib

import chip.CertificateAuthority
import chip.clusters as Clusters  # noqa: F401
import chip.FabricAdmin
import chip.logging
import chip.native
import coloredlogs
from chip.ChipStack import ChipStack
from rich import inspect, pretty
from rich.console import Console

_fabricAdmins = None


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
    chip.logging.RedirectToPythonLogging()

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

    parser = argparse.ArgumentParser()
    parser.add_argument(
        "-p", "--storagepath",
        help="Path to persistent storage configuration file (default: /tmp/repl-storage.json)",
        action="store",
        default="/tmp/repl-storage.json")
    parser.add_argument(
        "-d", "--debug", help="Set default logging level to debug.", action="store_true")
    parser.add_argument(
        "-t", "--trust-store", help="Path to the PAA trust store.", action="store", default="./credentials/development/paa-root-certs")
    parser.add_argument(
        "-b", "--ble-adapter", help="Set the Bluetooth adapter index.", type=int, default=None)
    parser.add_argument(
        "-s", "--server-interactions", help="Enable server interactions.", action="store_true")
    args = parser.parse_args()

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

    chip.native.Init(bluetoothAdapter=args.ble_adapter)

    global certificateAuthorityManager
    global chipStack
    global caList
    global devCtrl

    ReplInit(args.debug)

    chipStack = ChipStack(persistentStoragePath=args.storagepath, enableServerInteractions=args.server_interactions)
    certificateAuthorityManager = chip.CertificateAuthority.CertificateAuthorityManager(chipStack, chipStack.GetStorageManager())

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
    \t[red]caList\[n].adminList\[m][blue]:\t\tA specific FabricAdmin object at index m for the nth CertificateAuthority instance.''')

    console.print(
        f'\n\n[blue]Default CHIP Device Controller (NodeId: {devCtrl.nodeId}): '
        f'has been initialized to manage [bold red]caList[0].adminList[0][blue] (FabricId = {caList[0].adminList[0].fabricId}), '
        'and is available as [bold red]devCtrl')


if __name__ == "__main__":
    main()
