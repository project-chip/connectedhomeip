TOPTARGETS := all clean check

SUBDIRS = lwip system lib/core lib/support ble inet qrcode

$(TOPTARGETS): $(SUBDIRS)
$(SUBDIRS):
		$(MAKE) -C $@ $(MAKECMDGOALS)

.PHONY: $(TOPTARGETS) $(SUBDIRS)
