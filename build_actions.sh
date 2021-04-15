set -x
echo $TRAVIS_OS_NAME
case "$TRAVIS_OS_NAME" in
  osx)
	brew install qt@5
	export PATH="/usr/local/opt/qt@5/bin:$PATH"
	export LDFLAGS="-L/usr/local/opt/qt@5/lib"
	export CPPFLAGS="-I/usr/local/opt/qt@5/include"
	export PKG_CONFIG_PATH="/usr/local/opt/qt@5/lib/pkgconfig"
	;; 
  linux)
	apt-get install -y qt5-default
	;;
  windows)
	export QMAKE_PREFIX=/mingw64/qt5-static/bin/
	pacman --needed --noconfirm -S mingw-w64-x86_64-qt5-static
	;;
esac

"$QMAKE_PREFIX"qmake -o Makefile_qmake
make -f Makefile_qmake
cp release/ddb_gui_qt5.* .