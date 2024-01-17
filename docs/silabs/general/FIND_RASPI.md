# How to find your Raspberry Pi on the Network

## Finding the IP address of your Raspberry Pi

Sometimes it can be difficult to find your Raspberry Pi on the network. One way of interacting with the Raspberry Pi is to connect a keyboard, mouse, and monitor to it. The preferred method, however, is over SSH. For this, you will need to know the IP address of your Raspberry Pi.

This is a [good tutorial](https://raspberryexpert.com/find-raspberry-pi-ip-address/) on how to find the IP address.

### Mac / Linux

***Nmap***

The use of nmap on the Mac may require a software download.
Use nmap with the following command:

```shell
    $ sudo nmap -sn <subnet>.0/24`
```

Example: `sudo nmap -sn 10.4.148.0/24`, Among other returned values, you will see something: 

```shell
   $ Nmap scan report for ubuntu.silabs.com (10.4.148.44)
   $ Host is up (0.00025s latency).
   $ MAC Address: AA:BB:CC:11:22:33 (Raspberry Pi Trading)
```

And this is the Raspberry Pi at 10.4.148.44.

***Arp***

Alternatively, use Arp with the following command:

```shell
   $ arp -a \| grep -i "CC:BB:22\|DC:a6:32"`
```

### Windows

In the command prompt, use `nslookup` to fnd your Raspberry Pi.

Example: `nslookup ubuntu`

## Connecting to your Raspberry Pi over SSH

### Mac / Linux / Windows

Once you have found your Raspberry Pi's IP address, you can use Secure Shell (SSH) to connect to it over the command line with the following command:

```shell
    $ ssh <raspberry pi's username>@<raspberry pi's IP address>
```

Example:

```shell
    $ ssh ubuntu@10.4.148.44
```

`password: raspberrypi or ubuntu`  When prompted provide the raspberry pi's password, in the case of the Silicon Labs Matter Hub image the username is `ubuntu` and the password is either `ubuntu` or `<a user set password>`. (Note that if you are logging into your Raspberry Pi for the first time you may be asked to change the default password to a password of your choosing.)