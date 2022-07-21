[<< Back to TOC](../README.md)

# How to find your Raspberry Pi on the Network

## Finding the IP address of your Raspberry Pi

Sometimes it can be difficult to find your Raspberry Pi on the network. One way of interacting with the Raspberry Pi is connecting a keyboard, mouse and monitor to it. The preferred method, however, is over SSH. For this, you will need to know the IP address of your Raspberry Pi.

[This](https://raspberryexpert.com/find-raspberry-pi-ip-address/) is a good tutorial on how to find the IP address.


| Platform | Strategy |
| -------- | -------- | 
| Mac / Linux | ***Nmap*** <br> &emsp; The use of nmap on the Mac may require a software download. <br> &emsp; Use nmap with the following command: <br> &emsp;&emsp; `sudo nmap -sn <subnet>.0/24` <br><br> &emsp; Example: `sudo nmap -sn 1-.4.148.0/24` <br><br> &emsp; Among other returned values, you will see: <br> &emsp;&emsp;&emsp; `Nmap scan report for ubuntu.silabs.com (10.4.148.44)` <br> &emsp;&emsp;&emsp; `Host is up (0.00025s latency).` <br> &emsp;&emsp;&emsp; `MAC Address: E4:5F:01:7B:CD:12 (Raspberry Pi Trading)` <br><br> &emsp; And this is the Raspberry Pi at 10.4.148.44 <br><br> ***Arp*** <br> &emsp; Alternatively, use Arp with the following command: <br> &emsp;&emsp; `arp -a \| grep -i "b8:27:eb\|dc:a6:32"` |
| Windows | In the command prompt, use `nslookup` to fnd your Raspberry Pi. <br> &emsp; Example: `nslookup ubuntu` |

<br>  

## Connecting to your Raspberry Pi over SSH

| Platform | Strategy |
| -------- | -------- | 
| Mac / Linux / Windows | Once you have found your Raspberry Pi's IP address, you can use Secure Shell (SSH) to connect to it over the command line with the following command: <br> &emsp; `ssh <raspberry pi's username>@<raspberry pi's IP address>` <br><br> Example: <br> &emsp; `ssh ubuntu@10.4.148.44` <br> &emsp;`password: raspberrypi` <br><br> When prompted provide the raspberry pi's password, in the case of the Silicon Labs Matter Hub image the username is ***ubuntu*** and the password is ***raspberrypi*** | 
