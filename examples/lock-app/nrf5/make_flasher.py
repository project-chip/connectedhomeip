#!/usr/bin/env python

import argparse
import datetime
import os
import stat
import sys
import textwrap

def main(argv):
    parser = argparse.ArgumentParser(description='Flashing script generator.')

    parser.add_argument("--output", metavar='FILENAME', required=True,
                        help='flashing script name')
    parser.add_argument("--image", metavar='FILENAME', required=True,
                        help='image to be flashed')


    args = parser.parse_args(argv)

    script = """
      import os
      import subprocess
      import sys

      nrfjprog = ["nrfjprog", "-f", "nrf52"]

      subprocess.check_call(nrfjprog + [
        "--program",
        os.path.join(os.path.dirname(sys.argv[0]), "{image}"),
        "--sectorerase"])

      subprocess.check_call(nrfjprog + ["--reset"])
    """

    content = ('#!/usr/bin/env python\n'
               + '# Generated at {:%Y-%m-%d %H:%M:%S}'.format(
                   datetime.datetime.now())
               + textwrap.dedent(script).format(image=args.image))

    with open(args.output, "w") as f:
        f.write(content)
    os.chmod(args.output, (stat.S_IXUSR | stat.S_IRUSR | stat.S_IWUSR
                         | stat.S_IXGRP | stat.S_IRGRP
                         | stat.S_IXOTH | stat.S_IROTH))


if __name__ == '__main__':
    sys.exit(main(sys.argv[1:]))
