PLUGINS = cluster-server-basic cluster-server-on-off cluster-server-level-control cluster-server-identify core-data-model core-message-dispatch
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
