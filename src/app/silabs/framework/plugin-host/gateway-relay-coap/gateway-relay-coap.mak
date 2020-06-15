# gateway-relay-coap.mak

# The Makefile variables at the top should be able to be generic.  The 
# intention is they are only used to construct unique variables that the main
# Makefile will pull in.

# The variables are made generic in their name so this file can be 
# copy/pasted by other plugins to replicate the behavior and changing
# only a few items.

# Add the name of the plugin.
PLUGIN_NAME := gateway-relay-coap

# Creating unique test name.
# Note: No need to change this line.
PLUGIN_TEST_NAME := $(PLUGIN_PREFIX)$(PLUGIN_NAME)-test

# List the test files here.
# Must be relative to $ZNET root
$(PLUGIN_TEST_NAME)_FILES := \
  app/framework/plugin-host/gateway-relay-coap/gateway-relay-coap-cbor.c \
  app/framework/plugin-host/gateway-relay-coap/gateway-relay-coap-handle-read-attr-resp.c \
  app/framework/plugin-host/gateway-relay-coap/gateway-relay-coap-test.c \
  app/framework/test/test-framework-base.c \
  stack/framework/byte-utilities.c \
  ../../platform/base/hal/plugin/serial/ember-printf-convert.c
  
  
#  $(AFV2_TEST_FRAMEWORK_FILES)


# List plugin test steps here.
$(PLUGIN_TEST_NAME)_TEST_STEPS := \
  grc-read-attributes-response-bool-test \
  grc-read-attributes-response-uns08-test \
  grc-read-attributes-response-uns16-test \
  grc-read-attributes-response-uns32-test \
  grc-read-attributes-response-uns64-test \
  grc-read-attributes-response-neg08-test \
  grc-read-attributes-response-neg16-test \
  grc-read-attributes-response-neg32-test \
  grc-read-attributes-response-neg64-test \
  grc-read-attributes-response-odd-sizes-test \
  grc-read-attributes-response-octet-string-test \
  grc-read-attributes-response-text-string-test \
  grc-read-attributes-response-float-test \
  grc-read-attributes-response-attr-error-test \
  grc-read-attributes-response-unknown-zcl-type-test


# Needs to add this test to the global plugin test list.
# Note: No need to change this line.
PLUGIN_TEST_LIST += $(PLUGIN_TEST_NAME)

# Including dependencies
# Note: No need to change the following lines.
ifneq (($MAKECMDGOALS),clean)
  -include $($(PLUGIN_TEST_NAME)_FILES:.c=.d)
endif
