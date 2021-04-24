#!/bin/sh
QT5_STATIC_URL="https://github.com/kuba160/qt5_static_build/releases/download/5.12.10/qt_5.12.10_static_linux.zip"
QT5_STATIC_LOC=/opt
QMAKE=/opt/qt5_static/bin/qmake
BUILD_PWD=`pwd`

cd $QT5_STATIC_LOC
wget -q $QT5_STATIC_URL
unzip -qq *.zip

cd $BUILD_PWD
$QMAKE
make

mkdir -p plugins
mv ddb_gui_qt5.so plugins/
zip -r ddb_gui_qt5_linux-static.zip plugins/ddb_gui_qt5.so
