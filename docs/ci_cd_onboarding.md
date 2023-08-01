# Building Matter
## Checkout
```
git clone --recurse-submodules git@github.com:project-chip/connectedhomeip.git
```
Requires SSH keys to be set up. Using HTTPS for the time being.

Checkout fails on Windows machines due to file length issues.

## Prerequisites
Make sure to sudo apt-get update first before doing installs.

## Temporary: What is this fail?
https://github.com/project-chip/connectedhomeip/actions/runs/5718699957/job/15495048973#step:7:4124
ERROR   20:14:14.938 - TEST OUT  : [1690834454937] [12649:60904] [DMG] Subscription Liveness timeout with SubscriptionID = 0x2fd2d3a7, Peer = 01:0000000012344321

## General Questions
  What is ZAP?
