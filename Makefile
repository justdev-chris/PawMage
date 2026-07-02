# Top-level Makefile (in pawmage/)
all:
	$(MAKE) -C libpawm
	$(MAKE) -C pawm-convert

clean:
	$(MAKE) -C libpawm clean
	$(MAKE) -C pawm-convert clean

install:
	$(MAKE) -C libpawm install
	$(MAKE) -C pawm-convert install

uninstall:
	$(MAKE) -C libpawm uninstall
	$(MAKE) -C pawm-convert uninstall

.PHONY: all clean install uninstall
