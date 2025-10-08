# Debugging tips

When debugging problems with Matter on Linux there are multiple system services
involved. Many of those services provide detailed logs and tracing information
which is not enabled by default but can be useful for troubleshooting. Most
notably these include:

## The HCI Interface

The interface between the Bluetooth Host and the Bluetooth Controller is based
on a standardized serial protocol. The information exchanged via this protocol
can be captured into a file by the `hcidump` command. The data can be recorded
by running the command with the `--save-dump` commandline switch:

```bash
hcidump --save-dump=dump.pcap
```

The resulting capture file `dump.pcap` can be viewed with tools like
`wireshark`.

Package required on Debian/Ubuntu systems: `bluez-hcidump`.

## D-Bus

D-Bus connects Matter to other system services. The communication happening over
D-Bus can be logged to a `pcap` file using the following command:

```bash
dbus-monitor --system --pcap > dbus.pcap
```

As with the HCI interface the resulting `pcap` file can be viewed with tools
like `wireshark`.

Package required on Debian/Ubuntu systems: `dbus-bin`.

## The Bluez Bluetooth stack

Linux systems use the Bluez stack to manage Bluetooth devices. The stack is
implemented in the `bluetoothd` system daemon. When the daemon is started with
the `--debug` commandline switch it produces detailed a log of operations. In
systems where `systemd` is used to manage daemons the following command opens an
editor where the `--debug` switch can be added:

```bash
systemctl edit bluetooth.service
```

In the editor window the following lines need to be typed:

```ini
[Service]
ExecStart=
ExecStart=/usr/libexec/bluetooth/bluetoothd --experimental --debug
```

After saving the file and closing the editor the service needs to be restarted
with the `systemd restart bluetooth.service`. The
`systemctl status bluetooth.service` command can then be used to verify if the
daemon is running with the expected commandline options:

```bash
● bluetooth.service - Bluetooth service
     Loaded: loaded (/usr/lib/systemd/system/bluetooth.service; enabled; preset: enabled)
    Drop-In: /etc/systemd/system/bluetooth.service.d
             └─override.conf
     Active: active (running) since Fri 2025-05-16 12:01:40 UTC; 2 days ago
       Docs: man:bluetoothd(8)
   Main PID: 44013 (bluetoothd)
     Status: "Running"
      Tasks: 1 (limit: 3853)
     Memory: 1004.0K (peak: 1.7M)
        CPU: 878ms
     CGroup: /system.slice/bluetooth.service
             └─44013 /usr/libexec/bluetooth/bluetoothd --experimental --debug

May 19 08:09:55 bones bluetoothd[44013]: src/adapter.c:dev_disconnected() Device 00:1A:7D:DA:71:13 disconnected, reason 2
May 19 08:09:55 bones bluetoothd[44013]: src/adapter.c:adapter_remove_connection()
May 19 08:09:55 bones bluetoothd[44013]: plugins/policy.c:disconnect_cb() reason 2
May 19 08:09:55 bones bluetoothd[44013]: src/adapter.c:bonding_attempt_complete() hci0 bdaddr 00:1A:7D:DA:71:13 type 1 status 0xe
May 19 08:09:55 bones bluetoothd[44013]: src/device.c:device_bonding_complete() bonding (nil) status 0x0e
May 19 08:09:55 bones bluetoothd[44013]: src/device.c:device_bonding_failed() status 14
May 19 08:09:55 bones bluetoothd[44013]: src/adapter.c:resume_discovery()
May 19 08:10:25 bones bluetoothd[44013]: src/device.c:device_remove() Removing device /org/bluez/hci0/dev_00_1A_7D_DA_71_13
May 19 08:10:25 bones bluetoothd[44013]: src/device.c:btd_device_unref() Freeing device /org/bluez/hci0/dev_00_1A_7D_DA_71_13
May 19 08:10:25 bones bluetoothd[44013]: src/device.c:device_free() 0xaaaaff1b68a0
```

## wpa-supplicant

The `wpa_supplicant` daemon manages WiFi interfaces and networks. The amount of
logging can be changed at runtime by sending D-Bus calls to the daemon. In order
to enable detailed debug logging run:

```bash
gdbus call --system --dest fi.w1.wpa_supplicant1 --object-path /fi/w1/wpa_supplicant1 --method org.freedesktop.DBus.Properties.Set fi.w1.wpa_supplicant1 DebugLevel '<string "debug">'
```

and to reset the logging level back to normal run:

```bash
gdbus call --system --dest fi.w1.wpa_supplicant1 --object-path /fi/w1/wpa_supplicant1 --method org.freedesktop.DBus.Properties.Set fi.w1.wpa_supplicant1 DebugLevel '<string "info">'
```

## System Journal

In modern Linux systems all of the logs from the kernel and system services are
handled by the `systemd-journald` service. In order to read those log messages
the `journalctl` command needs to be used. A typical case would be to extract
all journal entries for a specific time period based on the timestamps of test
runs. This can be done by a command like below:

```bash
journalctl --since=2025-05-16T09:14:00Z --until=2025-05-16T09:16:30Z -o short-iso-precise

```
