#!/usr/bin/env bash
DIR=/dev/disk/by-id
DEVICE_CONFIG_FILE=/etc/fstab

echo "Add new device start..."
echo "Connect your device via USB"

res=$(inotifywait "$DIR")
if [[ ${res} != *"CREATE"* ]]; then
    echo "Create new device in /dev/disk/by-id directory error"
    exit 1
fi

new_device_id=$(echo "$res" | awk '{printf $NF}')

if sudo cat "$DEVICE_CONFIG_FILE" | grep -q "$new_device_id"; then
    echo "$new_device_id device is already set"
    exit 0
fi

device_index=1

while :; do
    dir_name="MBED_$device_index"
    if [ ! -d /media/"$dir_name" ]; then
        break
    fi

    ((device_index++))
done

sudo mkdir -p /media/MBED_"$device_index"
line="/dev/disk/by-id/$new_device_id /media/MBED_$device_index vfat rw,user,sync,nofail,umask=007,gid=20 0 0"
echo "$line" | sudo tee -a "$DEVICE_CONFIG_FILE"

echo "Finished"
