SUB_DIRS=src

.PHONY: all clean check

all clean check:
	git submodule update --init
	$(foreach dir,$(SUB_DIRS), $(MAKE) $@ -C $(dir))
