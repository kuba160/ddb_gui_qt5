all:
ifeq ($(OS),Windowts_NT)
	# windows
	$(eval QMAKE_PREFIX=/mingw64/qt5-static/bin/)
	pacman --needed -S mingw-w64-x86_64-qt5-static
else
	$(eval UNAME_S:=$(shell uname -s))
ifeq ($(UNAME_S),Darwin)
	# macos
	echo $(UNAME_S)
	brew install qt@5
else
	# linux
	echo $(UNAME_S)
	apt-get install -y qt5-default
endif
endif
	$(QMAKE_PREFIX)qmake -o Makefile_qmake
	$(MAKE) -f Makefile_qmake
	cp release/ddb_gui_qt5.* .