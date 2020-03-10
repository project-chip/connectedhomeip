TOPTARGETS := all clean run_tests

SUBDIRS = lwip system chip-core/setup-payload

$(TOPTARGETS): $(SUBDIRS)
$(SUBDIRS):
		$(MAKE) -C $@ $(MAKECMDGOALS)

.PHONY: $(TOPTARGETS) $(SUBDIRS)
