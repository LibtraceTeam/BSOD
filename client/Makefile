all:
	$(MAKE) -C src
	$(MAKE) -C modules/classic

clean:
	$(MAKE) clean -C src
	$(MAKE) clean -C modules/classic
	$(MAKE) clean -C modules/earth
	
		
run-app:
	cd runtime && ./bsod2
