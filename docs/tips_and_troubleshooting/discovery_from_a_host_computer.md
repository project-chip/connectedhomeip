# Device discovery from a Host computer (mDNS Scanning)

Device discovery for commissioning over the operational network happens over
DNS-SD. DNS-SD is handled over mDNS on WiFi and using an SRP server on a boarder
router for thread. On a computer, you can use the built-in mDNS applications to
discover devices for the purposes of testing. Avahi is the standard mDNS program
for linux, dns-sd is used on macs.

-   Scanning for commissionable devices
    -   avahi-browse -d local \_matterc.\_udp --resolve
    -   dns-sd -B \_matterc.\_udp
-   Scanning for commissioned devices
    -   avahi-browse -d local \_matter.\_tcp --resolve
    -   dns-sd -B \_matter.\_tcp
