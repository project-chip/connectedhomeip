PLUGINS = basic-server on-off-server level-control-server identify-server zcl-data-model message-dispatch
SAMPLES = utest

subsystem:
	for p in $(PLUGINS); do cd plugin/$$p && $(MAKE); cd ../..; done
	cd utest && $(MAKE)

clean:
	for p in $(PLUGINS); do cd plugin/$$p && $(MAKE) clean; cd ../..; done
	cd utest && $(MAKE) clean

test:
	for p in $(PLUGINS); do cd plugin/$$p && $(MAKE) test; cd ../..; done
	cd utest && $(MAKE) test
