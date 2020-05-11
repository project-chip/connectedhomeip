PLUGINS = cluster-server-basic cluster-server-on-off cluster-server-level-control cluster-server-identify core-data-model core-message-dispatch test-on-off

subsystem:
	for p in $(PLUGINS); do cd plugin/$$p && $(MAKE); cd ../..; done

clean:
	for p in $(PLUGINS); do cd plugin/$$p && $(MAKE) clean; cd ../..; done

test:
	for p in $(PLUGINS); do cd plugin/$$p && $(MAKE) test; cd ../..; done
