# critical-message-queue.mak

# Add the name of the plugin.
PLUGIN_NAME := critical-message-queue

# Creating unique test name.
# Note: No need to change this line.
PLUGIN_TEST_NAME:= $(PLUGIN_PREFIX)$(PLUGIN_NAME)-test

# List the test files here.
# Must be relative to $ZNET root
$(PLUGIN_TEST_NAME)_FILES := \
  $(AFV2_TEST_FRAMEWORK_FILES) \
  app/framework/util/print-formatter.c   \
  app/framework/plugin/critical-message-queue/critical-message-queue.c \
  app/framework/plugin/critical-message-queue/critical-message-queue-test.c

# List plugin test steps here.
$(PLUGIN_TEST_NAME)_TEST_STEPS := \
  all-tests

# Needs to add this test to the global plugin test list.
# Note: No need to change this line.
PLUGIN_TEST_LIST += $(PLUGIN_TEST_NAME)

# Including dependencies
# Note: No need to change the following lines.
ifneq (($MAKECMDGOALS),clean)
  -include $($(PLUGIN_TEST_NAME)_FILES:.c=.d)
endif
