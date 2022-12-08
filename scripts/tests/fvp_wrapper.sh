#!/bin/bash

# Execute an executable inside an FVP and communicate with its telnetterminal0 using netcat

[[ -z $TELNET ]] && TELNET=0
[[ -z $TIMEOUT ]] && TIMEOUT=30

NAME=$(basename "$BASH_SOURCE")
OBJDUMP="/opt/gcc-arm-none-eabi-10.3-2021.10/bin/arm-none-eabi-objdump"
CMDLINE_SYMBOL=__cmdline__
CMDLINE_MAGIC="cmdline"

declare -a tmp_files pids

function cleanup {
    if [[ ${#tmp_files[@]} -gt 0 ]]; then
        rm -f "${tmp_files[@]}"
    fi

    if [[ ${#pids[@]} -gt 0 ]]; then
        kill ${pids[@]} 2>/dev/null
    fi
}

trap cleanup EXIT

function show_usage {
    cat <<EOF
Usage: $BASH_SOURCE <fvp> [fvp options] -- <executable> [executable options]

Environment:
 * TIMEOUT: Set the maximum time to wait for the FVP's telnet port.
   The current value is $TIMEOUT.
 * TELNET: Set the telnet terminal to connect to (0..5).
   The current value is $TELNET.
EOF

    [[ $# -gt 0 ]] && echo "$@"
}

# Parse command-line
if [[ $# -lt 2 ]]; then
    show_usage >&2
    exit 1
fi

fvp=$1
exe=
dashdash=0
shift
declare -a fvp_args
declare -a exe_args
while [[ $# -gt 0 ]]; do
    if [[ $1 = -- ]]; then
        if [[ $dashdash -eq 1 ]]; then
            show_usage "-- should be passed exactly once" >&2
            exit 1
        fi
        dashdash=1
    elif [[ $dashdash -eq 0 ]]; then
        fvp_args+=("$1")
    elif [[ $exe = "" ]]; then
        if [[ -z "$1" ]]; then
            show_usage "Got blank argument" >&2
            exit 1
        fi
        exe=$1
    else
        exe_args+=("$1")
    fi
    shift
done

if [[ $dashdash -eq 0 || -z "$exe" ]]; then
    show_usage "Did not get -- <executable>" >&2
    exit 1
fi

# Create temporary file to capture FVP output.
tmpout=$(mktemp)
tmp_files+=($tmpout)

# Spawn the FVP.
echo "$NAME: Launching FVP" >&2

# Inject executable args.
if [[ ${#exe_args[@]} -gt 0 ]]; then
    # Get the virtual address of the command-line.
    nsfile=${exe%.elf}_ns.elf
    vaddr=$($OBJDUMP -t "$nsfile" | grep $CMDLINE_SYMBOL | awk '{print $1}')

    # Write arguments to a temporary file, separated by NUL and ending with two NULs.
    tmpargs=$(mktemp)
    tmp_files+=($tmpargs)

    python3 <<EOF
with open('$tmpargs', 'wb') as fp:
    fp.write(b'$CMDLINE_MAGIC\0')
    fp.write(b'\0'.join(b'${exe_args[@]}'.split(b' ')))
    fp.write(b'\0\0')
EOF

    # Add a data file loaded at the symbol's virtual address to the FVP args.
    fvp_args+=("--data")
    fvp_args+=("$tmpargs@0x$vaddr")

    echo "$NAME: Wrote ${#exe_args[@]} argument(s) ($(wc -c <$tmpargs) byte(s)) at 0x$vaddr" >&2
fi

$fvp "${fvp_args[@]}" -a "$exe" >"$tmpout" 2>&1 &
fvp_pid=$!
pids+=($fvp_pid)

if ! kill -s 0 $fvp_pid 2>/dev/null; then
    cat "$tmpout" >&2
    echo "$NAME: $fvp: Failed to launch" >&2
    exit 1
fi

# Wait for FVP to echo its telnet port
echo "$NAME: Waiting for telnet port" >&2
port=
while [[ $TIMEOUT -gt 0 && -z $port ]]; do
    start_time=$(date +%s)

    while read line; do
        if [[ "$line" =~ "telnetterminal$TELNET:" ]]; then
            port=$(echo $line | awk '{print $8}')
            break
        fi

        echo "$NAME: $line" >&2
    done <"$tmpout"

    [[ -z "$port" ]] || break

    delta_time=$(($(date +%s) - start_time))

    TIMEOUT=$((TIMEOUT - $delta_time))

    if ! kill -s 0 $fvp_pid 2>/dev/null; then
        echo "$NAME: $fvp: Failed" >&2
        exit 1
    fi
done

if [[ -z $port ]]; then
    echo "$NAME: Did not get port from FVP" >&2
    exit 2
fi

# Bind stdio to the telnet port
echo "$NAME: Establishing connection to port $port" >&2
netcat 127.0.0.1 $port
