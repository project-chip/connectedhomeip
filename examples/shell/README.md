# Matter Shell Reference

The `chip-shell` firmware exposes configuration and management APIs via a
command line interface (CLI). Use the shell CLI to experiment with Matter
interactively, which can also be used with additional application code. The
Matter functional test scripts use the shell CLI to execute test cases.

## Separator and escaping characters

The whitespace character (`' '`) is used to delimit the command name and the
different arguments, together with tab (`'\t'`) and new line characters (`'\r'`,
`'\n'`).

Some arguments might require to accept whitespaces on them. For those cases the
backslash character (`'\'`) can be used to escape separators or the backslash
itself.

Example:

```bash
> networkname Test\ Network
Done
> networkname
Test Network
Done
>
```

## Matter Shell Command List

-   [base64](#base64-decode-b64_string)
-   [device](README_DEVICE.md)
-   [echo](#echo-string)
-   [exit](#exit)
-   [help](#help)
-   [otcli](README_OTCLI.md)
-   [ping](#ping)
-   [rand](#rand)
-   [server](README_SERVER.md)
-   [version](#version)

## Matter Shell Command Details

### help

Display a list of all top-level commands supported and a brief description.

```bash
> help
  echo            Echo back provided inputs
  log             Logging utilities
  rand            Random number utilities
  base64          Base64 encode / decode utilities
  device          Device Layer commands
  otcli           Dispatch OpenThread CLI command
  ping            Using Echo Protocol to measure packet loss across network paths
  exit            Exit the shell application
  help            List out all top level commands
  version         Output the software version
Done
```

### base64 decode \<b64_string\>

Decode the given base64 string into hex.

```bash
> base64 decode EjQ=
1234
Done
```

### base64 encode \<hex_string\>

Decode the given hex string into base64.

```bash
> base64 encode 1234
EjQ=
Done
```

### echo \<string\>

Echo back the provided string to the terminal.

```bash
> echo hello
hello
Done
```

### exit

Exit the shell terminal. On an embedded system this may trigger a watchdog
reset.

```bash
> exit
Goodbye
```

### rand

Output a single byte random number.

```bash
> rand
103
Done
```

### version

Output the version of the Matter stack.

```bash
> version
CHIP 0.0.g54591338-dirty
Done
```
