TOPTARGETS := all clean check

SUBDIRS = lwip system lib/core lib/support ble inet

$(TOPTARGETS): $(SUBDIRS)
$(SUBDIRS):
		$(MAKE) -C $@ $(MAKECMDGOALS)

.PHONY: $(TOPTARGETS) $(SUBDIRS)
