import argparse

from .. import __version__


def toolkit(descr):
    parser = argparse.ArgumentParser(description=descr)
    parser.add_argument('-v', '--version', action='version',
                        version=f'alive_progress {__version__}')

    def run(f):
        try:
            f(**parser.parse_args().__dict__)
        except KeyboardInterrupt:
            pass

    return parser, run
