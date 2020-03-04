SUB_DIRS=src

.PHONY: all clean test

all test clean:
	$(foreach dir,$(SUB_DIRS), $(MAKE) $@ -C $(dir))
