# SPAKE2P Parameters Tool

## Introduction

spake2p tool provides command line interface (CLI) utility used for generating
spake parameters (PIN code and verifier) for device manufacturing provisioning.

## Usage Examples

Specify 'help' option for the detailed 'spake2p' tool usage instructions:

```
./spake2p help
```

Specify '--help' option for detail instructions on command usage:

```
./spake2p gen-verifier --help
```

Example command that generates spake2p verifier for a given PIN code:

```
./spake2p gen-verifier --pin-code 45502684 --iteration-count 1000 --salt "U1BBS0UyUCBLZXkgU2FsdA==" --out spake2p-provisioning-data.csv
```

Example command that generates 100 sets of spake2p parameters (random PIN Codes,
random Salts and corresponding Verifiers):

```
./spake2p gen-verifier --count 100 --iteration-count 15000 --salt-len 32 --out spake2p-provisioning-data.csv
```

Example command that generates 100 sets of spake2p parameters (Specific PIN
Codes, random Salts and corresponding Verifiers):

```
./spake2p gen-verifier --count 100 --pin-code-file pincodes.csv --iteration-count 15000 --salt-len 32 --out spake2p-provisioning-data.csv
```

Notes: Each line of the `pincodes.csv` should be a valid PIN code. You can use
`spake2p --help` to get the example content of the file.
