set -x

case "$OSTYPE" in
  darwin*)
	brew install qt@5
	;; 
  linux*)
	apt-get install -y qt5-default
	;;
  msys*)
	export QMAKE_PREFIX=/mingw64/qt5-static/bin/
	pacman --needed -S mingw-w64-x86_64-qt5-static
	;;
esac

"$QMAKE_PREFIX"qmake -o Makefile_qmake
make -f Makefile_qmake
cp release/ddb_gui_qt5.* .