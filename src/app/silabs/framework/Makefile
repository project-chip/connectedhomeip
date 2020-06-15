# This makefile here is for convenience.
# I (Timotej) spend endless amount of time asking what is the
# "official" way to regenerate everything under afv2, and what
# is the target in main Makefile to run all unit tests for afv2.
# Well, here they are. This is the way how to do regen and 
# how to do test from right here in app/framework/
#
#

SAMPLE_APPS_REGENERATE_SCRIPT=sample-apps-regenerate.pl

.PHONY: regen
regen: regen-sample-apps regen-scenarios regen-doc

regen-doc:
	cd test/headers; ./regenerate-documentation.pl em35x

# We do the regen command both with and without -r because -r only refreshes the
# ISC files, it does not generate the project files
regen-%:
	cd $(patsubst regen-%,%,$@) && ./$(SAMPLE_APPS_REGENERATE_SCRIPT) -a && ./$(SAMPLE_APPS_REGENERATE_SCRIPT) -a -r

.PHONY: test
test:
	cd ../..; make afv2-tests

.PHONY: clean
clean:
	cd ../..; make clean
