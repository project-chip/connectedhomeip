from __future__ import print_function
from __future__ import absolute_import
from builtins import input
from builtins import range
import os
import socket
import netifaces
import shutil
import psutil
import subprocess
import logging
from time import sleep

log = logging.getLogger(__name__)

hostapd_config = '''
#sets the wifi interface to use, is wlan0 in most cases
interface={2}
#driver to use, nl80211 works in most cases
driver=nl80211
#sets the ssid of the virtual wifi access point
ssid={0}
#sets the mode of wifi, depends upon the devices you will be using. It can be a,b,g,n. Setting to g ensures backward compatiblity.
hw_mode=g
#sets the channel for your wifi
channel=11
#####Sets WPA and WPA2 authentication#####
#wpa option sets which wpa implementation to use
#1 - wpa only
#2 - wpa2 only
#3 - both
wpa=2
#sets wpa passphrase required by the clients to authenticate themselves on the network
wpa_passphrase={1}
#sets wpa key management
wpa_key_mgmt=WPA-PSK
#sets encryption used by WPA
wpa_pairwise=CCMP
#sets encryption used by WPA2
rsn_pairwise=CCMP
#maximum lifetime for PTK in seconds.
wpa_ptk_rekey=600
# 802.11n support
ieee80211n=1
# QoS support, also required for full speed on 802.11n/ac/ax
wmm_enabled=1 
'''

dhcpd_config = '''
option domain-name-servers 8.8.8.8, 8.8.4.4;
option subnet-mask 255.255.255.0;
option routers {0};
subnet {1}0 netmask 255.255.255.0 {{
  range {1}100 {1}200;
}}
'''


class AccessPoint:
    def __init__(self, wlan='wlan0', ip='192.168.4.1', netmask='255.255.255.0', ssid='CHIPnet',
                 password='CHIPnet123'):
        self.wlan = wlan
        self.ip = ip
        self.netmask = netmask
        self.ssid = ssid
        self.password = password
        self.root_directory = "/etc/accesspoint/"
        self.hostapd_config_path = os.path.join(self.root_directory, "hostapd.conf")
        self.dhcpd_config_path = os.path.join(self.root_directory, "dhcpd.conf")

        if not os.path.exists(self.root_directory):
            os.makedirs(self.root_directory)

    def _check_parameters(self):
        interfaces = netifaces.interfaces()

        if self.wlan not in interfaces:
            log.error("Wlan {} interface was not found".format(self.wlan))
            return False

        if not self._validate_ip(self.ip):
            log.error("Wrong ip {}".format(self.ip))
            return False

        if self.ssid is None:
            log.error("SSID must not be None")
            return False

        self.ssid = str(self.ssid)

        if self.password is None:
            log.error("Password must not be None")
            return False

        self.password = str(self.password)

        return True

    def _write_hostapd_config(self):
        with open(self.hostapd_config_path, 'w') as hostapd_config_file:
            hostapd_config_file.write(hostapd_config.format(self.ssid, self.password, self.wlan))

        log.debug("Hostapd config saved to %s", self.hostapd_config_path)

    def _write_dhcpd_config(self):
        with open(self.dhcpd_config_path, 'w') as dhcpd_config_file:
            dhcpd_config_file.write(dhcpd_config.format(self.ip, self.ip[:self.ip.rfind('.')+1]))

        log.debug("Dhcpd config saved to %s", self.hostapd_config_path)

    def _validate_ip(self, addr):
        try:
            socket.inet_aton(addr)
            return True  # legal
        except socket.error:
            log.error("Wrong ip %s", str(addr))
            return False  # Not legal

    def _check_dependencies(self):
        check = True

        if shutil.which('hostapd') is None:
            log.error('hostapd executable not found. Make sure you have installed hostapd.')
            check = False

        if shutil.which('dhcpd') is None:
            log.error('dhcpd executable not found. Make sure you have installed dhcpd.')
            check = False

        return check

    def _pre_start(self):
        try:
            self._execute_shell('rfkill unblock wlan')
            self._execute_shell('sleep 1')
        except:
            pass

    def _start_router(self):
        self._pre_start()

        # Toggle interface and restart DHCP service
        log.debug("Toggle interface and restart DHCP service")
        ret = self._execute_shell('ip link set {} down'.format(self.wlan))
        log.debug(ret)
        ret = self._execute_shell('ip link set {} up'.format(self.wlan))
        log.debug(ret)
        ret = self._execute_shell('ip addr flush dev {}'.format(self.wlan))
        log.debug(ret)
        ret = self._execute_shell('ip addr add {}/24 dev {}'.format(self.ip ,self.wlan))
        log.debug(ret)

        # NAT settings
        for file_name in ["ip_dynaddr", "ip_forward"]:
            with open(os.path.join("/proc/sys/net/ipv4", file_name), 'w') as proc_file:
                proc_file.write("1")

        # Setting iptables
        log.debug("Setting iptables")
        ret = self._execute_shell('iptables -t nat -D POSTROUTING -s {}0/24 -j MASQUERADE >/dev/null 2>&1 || true'.format(self.ip[:self.ip.rfind('.')+1]))
        log.debug(ret)
        ret = self._execute_shell('iptables -t nat -A POSTROUTING -s {}0/24 -j MASQUERADE'.format(self.ip[:self.ip.rfind('.')+1]))
        log.debug(ret)

        ret = self._execute_shell('iptables -D FORWARD -o {} -m state --state RELATED,ESTABLISHED -j ACCEPT >/dev/null 2>&1 || true'.format(self.wlan))
        log.debug(ret)
        ret = self._execute_shell('iptables -A FORWARD -o {} -m state --state RELATED,ESTABLISHED -j ACCEPT'.format(self.wlan))
        log.debug(ret)

        ret = self._execute_shell('iptables -D FORWARD -i {} -j ACCEPT >/dev/null 2>&1 || true'.format(self.wlan))
        log.debug(ret)
        ret = self._execute_shell('iptables -A FORWARD -i {} -j ACCEPT'.format(self.wlan))
        log.debug(ret)

        # Start dhcpd
        cmd = 'dhcpd -cf {} {}'.format(self.hostapd_config_path, self.wlan)
        log.debug('Starting HostAP daemon...')
        ret = self._execute_shell(cmd)
        log.debug(ret)
        
        sleep(2)

        # Start hostapd
        cmd = 'hostapd -B {}'.format(self.hostapd_config_path)
        log.debug('Starting HostAP daemon...')
        ret = self._execute_shell(cmd)
        log.debug(ret)
        log.debug('AP is running.')

        return True

    def _stop_router(self):

        # Stop hostapd
        log.debug('Stop hostapd')
        self._execute_shell('pkill hostapd')

        # Toggle interface and restart DHCP service
        log.debug("Toggle interface and restart DHCP service")
        ret = self._execute_shell('ip link set {} down'.format(self.wlan))
        log.debug(ret)
        ret = self._execute_shell('ip link set {} up'.format(self.wlan))
        log.debug(ret)
        ret = self._execute_shell('ip addr flush dev {}'.format(self.wlan))
        log.debug(ret)

        # Disable forwarding in iptables.
        log.debug('Disabling forward rules in iptables.')
        self._execute_shell('iptables -P FORWARD DROP')

        # Delete iptables rules that were added for wlan traffic.
        self._execute_shell('iptables --table nat --delete-chain')
        self._execute_shell('iptables --table nat -F')
        self._execute_shell('iptables --table nat -X')

        log.debug('AP has stopped.')
        return True

    def is_running(self):
        proceses = [proc.name() for proc in psutil.process_iter()]
        return 'hostapd' in proceses

    def stop(self):
        if not self._check_parameters():
            return False

        if not self.is_running():
            log.debug("Not running")
            return True

        return self._stop_router()

    def start(self):
        if not self._check_dependencies():
            return False

        if not self._check_parameters():
            return False

        if self.is_running():
            log.debug("Already started")
            return True

        self._write_hostapd_config()
        self._write_dhcpd_config()

        return self._start_router()

    def _execute_shell(self, command_string):
        p = subprocess.Popen(command_string, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        p.wait()
        result = p.communicate()

        return result[0].decode()

    def get_network_param(self):
        return dict(zip(('ssid','password','ip'), (self.ssid, self.password,self.ip)))
