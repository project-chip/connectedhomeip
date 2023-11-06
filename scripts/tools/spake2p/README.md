# SPAKE2+ Python Tool

SPAKE2+ Python Tool is a Python script for generating SPAKE2+ protocol
parameters (only Verifier as of today). SPAKE2+ protocol is used during Matter
commissioning to establish a secure session between the commissioner and the
commissionee.

## Usage Examples

To list all available subcommands:

```console
$ ./spake2p.py --help
usage: spake2p.py [-h] subcommand       ...

SPAKE2+ Python Tool

positional arguments:
  subcommand
    gen-verifier    Generate SPAKE2+ Verifier

options:
  -h, --help        show this help message and exit
```

To display parameters of the `gen-verifier` subcommand:

```console
$ ./spake2p.py gen-verifier --help
usage: spake2p.py gen-verifier [-h] -p PASSCODE -s SALT -i count

options:
  -h, --help            show this help message and exit
  -p PASSCODE, --passcode PASSCODE
                        8-digit passcode
  -s SALT, --salt SALT  Salt of length 16 to 32 octets encoded in Base64
  -i count, --iteration-count count
                        Iteration count between 1000 and 100000
```

To generate SPAKE2+ verifier for "SPAKE2P Key Salt" salt and 20202021 passcode,
using 1000 PBKDF2 iterations:

```console
./spake2p.py gen-verifier -p 20202021 -s U1BBS0UyUCBLZXkgU2FsdA== -i 1000
uWFwqugDNGiEck/po7KHwwMwwqZgN10XuyBajPGuyzUEV/iree4lOrao5GuwnlQ65CJzbeUB49s31EH+NEkg0JVI5MGCQGMMT/SRPFNRODm3wH/MBiehuFc6FJ/NH6Rmzw==
```
