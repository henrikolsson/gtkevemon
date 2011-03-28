INSTALL_BIN = /usr/local/bin

all:
	$(MAKE) -C src

clean:
	$(MAKE) -C src clean

install:
	mkdir -p ${INSTALL_BIN}
	cp src/gtkevemon ${INSTALL_BIN}
	$(MAKE) -C icon

uninstall:
	rm -f ${INSTALL_BIN}/gtkevemon
	rmdir --ignore-fail-on-non-empty ${INSTALL_BIN}
	$(MAKE) -C icon uninstall
