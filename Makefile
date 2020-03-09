SUB_DIRS=src

.PHONY: all clean test

all test clean:
	git submodule update --init
	$(foreach dir,$(SUB_DIRS), $(MAKE) $@ -C $(dir))
