# Troubleshooting Avahi

## Resetting the cache

Avahi keeps a cache of old results. To reset the cache, kill the daemon. It will
auto-restart.

`sudo avahi-daemon --kill`

## Stopping the daemon

If you really want to stop the daemon, killing it is not sufficient because it
will just restart. To stop it completely:

```
sudo systemctl mask avahi-daemon.socket
sudo systemctl disable avahi-daemon
sudo systemctl stop avahi-daemon
```

## Problem: Failed to create avahi group: Not permitted

Avahi is not set up to publish records by default. This has to be explicitly
allowed in the config file. In /etc/avahi/avahi-daemon.conf, add the following
lines:

```
[publish]
disable-user-service-publishing=no
```

Then restart the daemon

```
sudo systemctl restart avahi
```
