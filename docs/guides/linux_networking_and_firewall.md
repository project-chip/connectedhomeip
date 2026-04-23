# Matter Linux Networking and Firewall Configuration

This document provides instructions on how to configure the network and firewall on a Linux device running Matter applications.

## Network Discovery and Firewall

If you encounter network discovery issues (especially when pairing with controllers that require mDNS), you may need to configure the firewall on the device running the Matter application to allow incoming connections.

Instead of disabling the firewall entirely, it is recommended to allow only the specific ports required for Matter and mDNS. If using `ufw` (Uncomplicated Firewall), you can allow them with:

```bash
sudo ufw allow 5353/udp
sudo ufw allow 5540/udp
```

*Note: Port 5353 is used for mDNS (multicast DNS) and port 5540 is the default port for secure device messages in Matter.*

## Same Wi-Fi Network

Ensure your device running the Matter application (laptop or workstation) is on the same local network (Wi-Fi) as the controller. This is critical for local discovery and communication.
