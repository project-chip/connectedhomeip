Like pcap runner, we should add a thread runner that periodically dumps the following to a log.
Maybe also thread pcap?

```
ot-ctl log level 5
ot-ctl netdata show
ot-ctl srp server state
ot-ctl srp server service
ot-ctl srp server host
ot-ctl dataset active
ot-ctl leaderdata
ot-ctl neighbor table
ot-ctl router table
ot-ctl eidcache
ot-ctl counters mac
ot-ctl counters mle
ot-ctl counters ip
ifconfig
ip -6 addr list
ip -6 neigh
ip -6 route list table all
```

This would require hooking 52840 RCP to the Pi and having it join the active thread net before capture.   
otbr is already cloned in the docker image.
