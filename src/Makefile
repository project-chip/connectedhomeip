TOPTARGETS := all clean run_tests

SUBDIRS = lwip system lib/core lib/support inet

$(TOPTARGETS): $(SUBDIRS)
$(SUBDIRS):
		$(MAKE) -C $@ $(MAKECMDGOALS)

.PHONY: $(TOPTARGETS) $(SUBDIRS)
