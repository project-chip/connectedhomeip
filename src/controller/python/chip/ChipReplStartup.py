from rich import print
from rich.pretty import pprint
from rich import pretty
from rich import inspect
from rich.console import Console
import logging
from chip import ChipDeviceCtrl
import chip.clusters as Clusters
from chip.ChipStack import *
import coloredlogs
import chip.logging
import argparse
import builtins
import chip.FabricAdmin
from chip.utils import CommissioningBuildingBlocks
import atexit

_fabricAdmins = None


def LoadFabricAdmins():
    global _fabricAdmins

    #
    # Shutdown any fabric admins we had before as well as active controllers. This ensures we
    # relinquish some resources if this is called multiple times (e.g in a Jupyter notebook)
    #
    chip.FabricAdmin.FabricAdmin.ShutdownAll()
    ChipDeviceCtrl.ChipDeviceController.ShutdownAll()

    _fabricAdmins = []
    storageMgr = builtins.chipStack.GetStorageManager()

    console = Console()

    try:
        adminList = storageMgr.GetReplKey('fabricAdmins')
    except KeyError:
        console.print(
            "\n[purple]No previous fabric admins discovered in persistent storage - creating a new one...")

        #
        # Initialite a FabricAdmin with a VendorID of TestVendor1 (0xfff1)
        #
        _fabricAdmins.append(chip.FabricAdmin.FabricAdmin(0XFFF1))
        return _fabricAdmins

    console.print('\n')

    for k in adminList:
        console.print(
            f"[purple]Restoring FabricAdmin from storage to manage FabricId {adminList[k]['fabricId']}, FabricIndex {k}...")
        _fabricAdmins.append(chip.FabricAdmin.FabricAdmin(vendorId=int(adminList[k]['vendorId']),
                                                          fabricId=adminList[k]['fabricId'], fabricIndex=int(k)))

    console.print(
        '\n[blue]Fabric Admins have been loaded and are available at [red]fabricAdmins')
    return _fabricAdmins


def CreateDefaultDeviceController():
    global _fabricAdmins

    if (len(_fabricAdmins) == 0):
        raise RuntimeError("Was called before calling LoadFabricAdmins()")

    console = Console()

    console.print('\n')
    console.print(
        f"[purple]Creating default device controller on fabric {_fabricAdmins[0]._fabricId}...")
    return _fabricAdmins[0].NewController()


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


def StackShutdown():
    chip.FabricAdmin.FabricAdmin.ShutdownAll()
    ChipDeviceCtrl.ChipDeviceController.ShutdownAll()
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


console = Console()

parser = argparse.ArgumentParser()
parser.add_argument(
    "-p", "--storagepath", help="Path to persistent storage configuration file (default: /tmp/repl-storage.json)", action="store", default="/tmp/repl-storage.json")
parser.add_argument(
    "-d", "--debug", help="Set default logging level to debug.", action="store_true")
args = parser.parse_args()

ReplInit(args.debug)
chipStack = ChipStack(persistentStoragePath=args.storagepath)
fabricAdmins = LoadFabricAdmins()
devCtrl = CreateDefaultDeviceController()

builtins.devCtrl = devCtrl

atexit.register(StackShutdown)

console.print(
    '\n\n[blue]Default CHIP Device Controller has been initialized to manage [bold red]fabricAdmins[0][blue], and is available as [bold red]devCtrl')
