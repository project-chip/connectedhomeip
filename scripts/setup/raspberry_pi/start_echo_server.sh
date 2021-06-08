#!/bin/bash

if ! netstat -a | less | grep -q "echo"; then
    #Setting TCP and UDP echo server
    echo "Setting TCP and UDP echo server"

    echo 'echo            stream  tcp     nowait  root    internal' | sudo tee -a /etc/inetd.conf
    echo 'echo            dgram   udp     wait    root    internal' | sudo tee -a /etc/inetd.conf
    sudo service openbsd-inetd restart

    netstat -a | less | grep "echo"
fi