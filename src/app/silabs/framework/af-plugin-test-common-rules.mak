# af-plugin-test-common-rules.mak

# This makefile creates rules for each app framework plugin that is included
# in the master makefile prior to the inclusion of this file. Also, it creates
# rules per each test step defined by each included plugin.

# The intention here is that only the plugin specific makefile needs to be 
# configured and stored and included in the master makefile. This makefile
# is going to create all the necessary rules to create rules for and to run 
# each and every test step

# Creating and immediate variable to store framework plugin makefile targets
FRAMEWORK_PLUGIN_MAKEFILE_TARGETS :=

# Template to create rules for all specific test steps defined by the plugin
define PLUGIN_TEST_STEP_template
$$(1)-$(2):
	@if [ ! -f "$$($(1)_BINARY)" ];then \
		@$(MAKE) $$($(1)_BINARY); \
	fi
	@$$($(1)_BINARY) $(2)
endef

# Template for general plugin test rules
define PLUGIN_RULES_template
.PHONY: announce-$(1)
announce-$(1):
	@echo "\ntarget: run-$(1)"

$(1)_OBJECTS := $($(1)_FILES:.c=.o)
$(1)_BINARY := bin/$(1)
$(1)_TEST_STEP_LIST := $(addprefix $(1)-,$($(1)_TEST_STEPS))

.PHONY: run-$(1)
run-$(1): announce-$(1) $$($(1)_BINARY) $$($(1)_TEST_STEP_LIST)

$$($(1)_BINARY): $$($(1)_OBJECTS)
	$(CC) $(OPTIONS) $$^ -o $$@

# Create unique rule per each test steps defined by the plugin
$(foreach plugin_test_step,$($(1)_TEST_STEPS),$(eval $(call PLUGIN_TEST_STEP_template,$(1),$(plugin_test_step),$$($(1)_BINARY))))

FRAMEWORK_PLUGIN_MAKEFILE_TARGETS += run-$(1) \

endef

# Create unique rules for each plugin that is added to the PLUGIN_TEST_LIST
$(foreach plugin_test_name,$(PLUGIN_TEST_LIST),$(eval $(call PLUGIN_RULES_template,$(plugin_test_name))))
