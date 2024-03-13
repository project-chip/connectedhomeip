#! /usr/bin/python3
import modules.arguments as _args
import modules.manager as _man
import modules.util as _util
import sys
import os

def main(argv):
    # try:
        # Paths
        paths = _util.Paths(os.path.dirname(__file__))
        # Versions
        args = _args.ArgumentList(paths)
        ver = args.compile()
        # Manager
        man = _man.ProvisionManager(ver)
        man.execute(paths, args)
    # except Exception as e:
    #     _util.fail(e)

main(sys.argv[1:])
