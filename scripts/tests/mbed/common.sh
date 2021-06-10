#!/bin/bash

# Returns information about a single baord. Every time it's called it returns the next one in the list from mbedls.
# This will set:
# BOARD_NUMBER - how many boards mbedls is reporting
# BOARD_INDEX - index in mbedls, 1 based
# TARGET - target name like NRF52840_DK
# SERIAL - serial it uses /dev/ttyACM0
# TARGET_ID - unique id from mbedls

get_board() {
    mbedls
    set +x
    # this will create the var if not already set
    let "BOARD_INDEX += 1"

    BOARD_NUMBER=$(expr $(mbedls | wc -l) - 2)
    if [[ "$BOARD_NUMBER" -lt 1 ]]; then
        echo "No boards connected" && exit 1
    fi

    if [[ "$BOARD_INDEX" -gt $BOARD_NUMBER ]]; then
        BOARD_INDEX=$BOARD_NUMBER
    fi

    BOARD_LINE=$(mbedls | head -n $(expr $BOARD_INDEX + 2) | tail -n 1)

    TARGET=$(echo "$BOARD_LINE" | cut -d \| -f 2 | xargs)
    MOUNTPOINT=$(echo "$BOARD_LINE" | cut -d \| -f 4 | xargs)
    SERIAL=$(echo "$BOARD_LINE" | cut -d \| -f 5 | xargs)
    TARGET_ID=$(echo "$BOARD_LINE" | cut -d \| -f 6 | xargs)
    echo "TARGET $TARGET MOUNTPOINT $MOUNTPOINT SERIAL $SERIAL TARGET_ID $TARGET_ID BOARD_INDEX $BOARD_INDEX BOARD_NUMBER $BOARD_NUMBER"
    set -x
}

# Clones repo into given directory, CWD will be inside the repo after running
# Takes two arguments, but at least one:
# $1 - repo URL
# $2 - name of directory to clone to, eg.:  mbed-os-bluetooth-integration-testsuite
# $3 - optional SHA otherwise tip of main or master will be used

clone_repo() {
    if [ $# -lt 2 ]; then
        echo "clone_repo requires at least 2 arguments" && exit 1
    fi

    local REPO_URL="$1"
    local REPO_DIR="$2"
    local SHA="$3"

    if [ ! -d "$REPO_DIR" ]; then
        git clone "$REPO_URL" "$REPO_DIR"
    fi

    cd "$REPO_DIR"
    # this avoid the need for credentials
    git fetch

    # we need to temporarily allow commands to fail
    set +e
    # check if it's using main instead of master, returns 0 if exists
    git show-ref --verify --quiet refs/heads/main
    # we need to remember the return since the set below will override it
    MAIN_EXISTS_CHECK=$?
    set -e
    # if sha dir given, override
    if [ -n "$SHA" ]; then
        git checkout "$SHA"
    else
        if [ $MAIN_EXISTS_CHECK -eq 0 ]; then
            git reset --hard origin/main
        else
            git reset --hard origin/master
        fi
    fi
}

# Deploy the project in current dir and install python requirements

deploy_project() {
    mbed deploy

    # install requirements
    if [ -d "mbed-os" ]; then
        pip3 install -r mbed-os/requirements.txt
    elif [ -f "requirements.txt" ]; then
        pip3 install -r requirements.txt
    fi

    # there is a conflict with pyocd requirements
    pip3 install pyyaml --upgrade
}

# Sets TIMESTAMP variable to filename friendly timestamp like: 2021-04-21-16-32-17

get_timestamp() {
    TIMESTAMP=$(date +"%Y-%m-%d-%H-%M-%S")
}

# enable bluetooth on RPi

enable_bluetooth() {
    service dbus start
    bluetoothd &
}

download_artifacts() {
    if [ $# -lt 4 ]; then
        echo "download_artifacts requires at least 4 arguments" && exit 1
    fi

    GITHUB_REPOSITORY="$1"
    GITHUB_ARTIFACT_NAME="$2"
    GITHUB_TOKEN="$3"
    OUTPUT_NAME="$4"

    URL="https://api.github.com/repos/${GITHUB_REPOSITORY}/actions/artifacts"
    JQ_QUERY=".artifacts | map(select(.name==\""${GITHUB_ARTIFACT_NAME}"\")) | .[0].archive_download_url"

    jq --version
    curl ${URL} --output artifacts.json
    cat artifacts.json

    DOWNLOAD_URL="$(curl -s ${URL} | jq -r "${JQ_QUERY}")"
    AUTHORIZATION_HEADER="Authorization: token "${GITHUB_TOKEN}

    curl -s -H "$AUTHORIZATION_HEADER" $DOWNLOAD_URL -L --output $OUTPUT_NAME
}

download_artifacts_gh() {
    if [ $# -lt 4 ]; then
        echo "download_artifacts_gh requires at least 4 arguments" && exit 1
    fi

    GITHUB_REPOSITORY="$1"
    GITHUB_ACTION_ID="$2"
    GITHUB_TOKEN="$3"
    OUTPUT_NAME="$4"

    echo "${GITHUB_TOKEN}" >token.txt
    gh auth login --with-token <token.txt
    rm token.txt

    gh run download --repo ${GITHUB_REPOSITORY} ${GITHUB_ACTION_ID} --dir ${OUTPUT_NAME}
}

mount_mbed_device() {
    device_index=1
    for file in /dev/disk/by-id/*; do
        if [[ $file == *"MBED"* ]]; then
            mnt_name="MBED_${device_index}"
            mkdir -p /mnt/"${mnt_name}"
            mount "${file}" /mnt/"${mnt_name}"
            ((device_index++))
        fi
    done
}

flash_image_to_device() {
    if [ $# -lt 3 ]; then
        echo "flash_image_to_device requires at least 3 arguments. Please define image and target name and images directory" && exit 1
    fi

    image_name=$1
    target_name=$2
    images_dir=$3

    image=$(find $images_dir -name '${image_name}_${target_name}*')

    echo "$image"

    if [ -z "$image" ]; then
        echo "Image does not exists"
        exit 1
    fi

    targer_number=$(cat devices.json | jq length)

    for index in {1 .. $targer_number}; do
        platform_name = $(cat devices.json | jq '.[${index}] .platform_name')
        if [ "$target_name" = "$platform_name" ]; then
            target_id = $(cat devices.json | jq '.[${index}] .target_id')
        fi
    done

    echo "$target_id"

    if [ -z "$target_id" ]; then
        echo "Target not available"
        exit 1
    fi

    # Flash the binary
    mbedflash flash -i $image --tid $target_id
}
