from rich import print
from rich.pretty import pprint
from rich import pretty
from rich import inspect
from rich.console import Console
import logging
from chip import ChipDeviceCtrl
import chip.clusters as Clusters
import coloredlogs
import chip.logging
import argparse
import builtins


def ReplInit():
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
    logging.getLogger().setLevel(logging.ERROR)


def matterhelp(classOrObj=None):
    if (classOrObj == None):
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
