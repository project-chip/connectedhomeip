#!/bin/bash

# Execute an executable inside an FVP and communicate with its telnetterminal0 using netcat

[[ -z $TELNET ]] && TELNET=0
[[ -z $TIMEOUT ]] && TIMEOUT=30

function show_usage {
    cat <<EOF
Usage: $BASH_SOURCE <fvp> [fvp options] -- <executable> [executable options]

Environment:
 * TIMEOUT: Set the maximum time to wait for the FVP's telnet port.
   The current value is $TIMEOUT.
 * TELNET: Set the telnet terminal to connect to (0..5).
   The current value is $TELNET.
EOF
}

NAME=$(basename "$BASH_SOURCE")

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
        dashdash=1
    elif [[ $dashdash -eq 0 ]]; then
        fvp_args+=("$1")
    elif [[ $exe = "" ]]; then
        exe=$1
    else
        exe_args+=("$1")
    fi
    shift
done

if [[ $dashdash -eq 0 || -z "$exe" ]]; then
    show_usage >&2
    echo "Did not get -- <executable>" >&2
    exit 1
fi

# Create temporary file to capture FVP output.
tmpfile=$(mktemp)
trap "rm -rf $tmpfile;" EXIT

# Spawn the FVP.
echo "$NAME: Launching FVP" >&2
$fvp "${fvp_args[@]}" "$exe" >"$tmpfile" 2>&1 &
fvp_pid=$!
if ! kill -s 0 $fvp_pid 2>/dev/null; then
    cat "$tmpfile" >&2
    echo "$NAME: $fvp: Failed to launch" >&2
    exit 1
fi
trap "kill -9 $fvp_pid 2>/dev/null" EXIT

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
    done <"$tmpfile"

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
