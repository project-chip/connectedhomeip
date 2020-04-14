:

JLINK_GDB_SERVER=JLinkGDBServer
DEVICE_TYPE=NRF52840_XXAA
RTT_PORT=19021

which ${JLINK_GDB_SERVER} || {
    echo "ERROR: ${JLINK_GDB_SERVER} not found"
    echo "Please install SEGGER J-Link software package"
    exit 1
}

which nc || {
    echo "ERROR: nc command not found"
    exit 1
}

# Launch JLink GDB Server in background; redirect output thru sed to add prefix.
${JLINK_GDB_SERVER} -device ${DEVICE_TYPE} -if SWD -speed 4000 -rtos GDBServer/RTOSPlugin_FreeRTOS $* > >(exec sed -e 's/^/JLinkGDBServer: /') 2>&1 &
GDB_SERVER_PID=$!

# Repeatedly open a connection to the GDB server's RTT port until
# the user kills the GDB server with an interrupt character.
while true;
do
    # Wait for GDB server to begin listening on RTT port
    while ! lsof -nP -i4TCP:${RTT_PORT} -sTCP:LISTEN > /dev/null; 
    do
        # Quit if the GDB server exits.
        if ! (kill -0 ${GDB_SERVER_PID} >/dev/null 2>&1); then
            echo ""
            exit
        fi

        # Wait a bit.
        sleep 0.1

    done

    # Connect to RTT port.
    nc localhost ${RTT_PORT}

done

