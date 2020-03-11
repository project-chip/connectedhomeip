SUB_DIRS=src

.PHONY: all clean run_tests

all clean run_tests:
	git submodule update --init
	$(foreach dir,$(SUB_DIRS), $(MAKE) $@ -C $(dir))
