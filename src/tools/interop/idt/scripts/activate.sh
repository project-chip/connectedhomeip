sudo docker run \
     -it \
     --mount source=$(pwd)/idt,target=/idt,type=bind \
     --privileged \
     -v /dev/bus/usb:/dev/bus/usb \
     -v /var/run/dbus:/var/run/dbus \
     --net=host \
     idt
