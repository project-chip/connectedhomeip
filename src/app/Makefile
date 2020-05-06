subsystem:
	cd plugin/basic-server && $(MAKE)
	cd plugin/on-off-server && $(MAKE)
	cd plugin/level-control-server && $(MAKE)

clean:
	cd plugin/basic-server && $(MAKE) clean
	cd plugin/on-off-server && $(MAKE) clean
	cd plugin/level-control-server && $(MAKE) clean

test:
	cd plugin/basic-server && $(MAKE) test
	cd plugin/on-off-server && $(MAKE) test
	cd plugin/level-control-server && $(MAKE) test
