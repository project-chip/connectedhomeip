#!/bin/bash

#####################################################################
### Default parameters
hostapd_process_id=$(pidof hostapd)

true ${INTERFACE:=wlan0}
true ${AP_SSID:=CHIPnet}
true ${AP_PASSWORD:=CHIPnet123}
true ${AP_GATEWAY:=192.168.4.1}

#####################################################################
### Declare ap_stop function
function ap_stop() {
    if [[ ! -z $hostapd_process_id ]]; then
        echo "Stopping access point!"
        kill $hostapd_process_id
	    ip link set ${INTERFACE} down
        ip link set ${INTERFACE} up
        ip addr flush dev ${INTERFACE}
    fi
}

#####################################################################
### Declare ap_start function
function ap_start() {
    if [[ ! -z $hostapd_process_id ]]; then
        echo "Access point already running - restart!"
        ap_stop
    fi    

    # Create hostapd configuration file
    cat > "/etc/hostapd.conf" << EOF
interface=${INTERFACE}
driver=nl80211
ssid=${AP_SSID}
hw_mode=g
channel=11
wpa=2
wpa_passphrase=${AP_PASSWORD}
wpa_key_mgmt=WPA-PSK
wpa_pairwise=CCMP
rsn_pairwise=CCMP
wpa_ptk_rekey=600
ieee80211n=1
wmm_enabled=1 
EOF
 
    ### unblock wlan
    rfkill unblock wlan
    
    echo "Setting interface ${INTERFACE}"
    
    ### Toggle interface and restart DHCP service 
    ip link set ${INTERFACE} down
    ip link set ${INTERFACE} up
    ip addr flush dev ${INTERFACE}
    ip addr add ${AP_GATEWAY}/24 dev ${INTERFACE}
    
    ### NAT settings
    echo "NAT settings ip_dynaddr, ip_forward"
    
    for i in ip_dynaddr ip_forward ; do 
        if [ $(cat /proc/sys/net/ipv4/$i) ]; then
            echo $i already 1 
        else
            echo "1" > /proc/sys/net/ipv4/$i
        fi
    done
    
    cat /proc/sys/net/ipv4/ip_dynaddr 
    cat /proc/sys/net/ipv4/ip_forward
    
    echo "Setting iptables..."
    iptables -t nat -D POSTROUTING -s ${AP_GATEWAY::-1}0/24 -j MASQUERADE > /dev/null 2>&1 || true
    iptables -t nat -A POSTROUTING -s ${AP_GATEWAY::-1}0/24 -j MASQUERADE
    
    iptables -D FORWARD -o ${INTERFACE} -m state --state RELATED,ESTABLISHED -j ACCEPT > /dev/null 2>&1 || true
    iptables -A FORWARD -o ${INTERFACE} -m state --state RELATED,ESTABLISHED -j ACCEPT
    
    iptables -D FORWARD -i ${INTERFACE} -j ACCEPT > /dev/null 2>&1 || true
    iptables -A FORWARD -i ${INTERFACE} -j ACCEPT
    
    echo "Configuring DHCP server .."
    
    cat > "/etc/dhcp/dhcpd.conf" << EOF
option domain-name-servers 8.8.8.8, 8.8.4.4;
option subnet-mask 255.255.255.0;
option routers ${AP_GATEWAY};
subnet ${AP_GATEWAY::-1}0 netmask 255.255.255.0 {
  range ${AP_GATEWAY::-1}100 ${AP_GATEWAY::-1}200;
}
EOF
   
    echo "Starting DHCP server .."
    dhcpd ${INTERFACE}
    
    echo "Starting HostAP daemon ..."
    hostapd -B /etc/hostapd.conf
}

#####################################################################
### Check if using supported commands [start/stop]

SUPPORTED_COMMAND=(start stop)
COMMAND=$1

if [[ ! " ${SUPPORTED_COMMAND[@]} " =~ " ${COMMAND} " ]]; then
    echo "ERROR: Command $COMMAND not supported"
    exit 1
fi

#####################################################################
### Handle start command

if [[ "$COMMAND" == *"start"* ]]; then
    ### Parse start command arguments
    # Remove $1 from argument list
    shift
    
    for i in "$@"; do
        case $i in
        --interface=*)
            INTERFACE="${i#*=}"
            shift
            ;;
        --ap_gateway=*)
            AP_GATEWAY="${i#*=}"
            shift
            ;;
        --ap_ssid=*)
            AP_SSID="${i#*=}"
            shift
            ;;
        --ap_pswd=*)
            AP_PASSWORD="${i#*=}"
            shift
            ;;

        *)
        # unknown option
            ;;
        esac
    done
    ap_start
fi

#####################################################################
### Handle stop command

if [[ "$COMMAND" == *"stop"* ]]; then
    if [[ ! -z $hostapd_process_id ]]; then
        ap_stop
    else
        echo "WiFi Access Point is not running - nothing to stop!"
    fi	
fi
