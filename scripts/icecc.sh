if [ "$1" = "start" ]; then
    IFS='.' read -ra PARTS <<<"$HOSTNAME"
    iceccd -d -m 0 -N "devcontainer-${PARTS[0]}"
fi

if [ "$1" = "stop" ]; then
    pgrep icecc >/dev/null 2>&1
    if [ $? -eq 0 ]; then
        pgrep icecc | xargs kill -9
    fi
fi
