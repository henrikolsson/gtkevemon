INSTALL_BIN = /usr/local/bin

all:
	$(MAKE) -C src

debug:
	$(MAKE) -C src debug

clean:
	$(MAKE) -C src clean

install:
	install -Dm 755 src/gtkevemon ${INSTALL_BIN}/gtkevemon
	$(MAKE) -C icon

uninstall:
	${RM} ${INSTALL_BIN}/gtkevemon
	$(MAKE) -C icon uninstall
