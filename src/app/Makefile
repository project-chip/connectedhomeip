PLUGINS = test-on-off test-unit

subsystem:
	for p in $(PLUGINS); do cd plugin/$$p && $(MAKE); cd ../..; done

clean:
	for p in $(PLUGINS); do cd plugin/$$p && $(MAKE) clean; cd ../..; done
	find . -name '*.o' -exec rm -f {} \;
	find . -name '*.out' -exec rm -f {} \;

test:
	for p in $(PLUGINS); do cd plugin/$$p && $(MAKE) test; cd ../..; done
