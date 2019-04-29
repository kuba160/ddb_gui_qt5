# ddb_gui_qt5

This plugin provides a Qt interface for [deadbeef](http://deadbeef.sourceforge.net/). This plugin is continuation of [deadbeef-qt plugin](https://github.com/redpunk231/deadbeef-qt.git) ported to Qt5

## Install
For installation requires Qt 5 (specific version unknown) or greater and DeaDBeeF 0.7. For debian based systems install `qt5-default`.

Get ddb_gui_qt5 sources from repository:
```bash
$ git clone git://github.com/kuba160/ddb_gui_qt5
```
Create temporary directory (for example </path/to/sources>/build) and change into it:
```bash
$ cd ddb_gui_qt/
$ mkdir build
$ cd build
```
Run cmake (`CMAKE_INSTALL_PREFIX` default is /usr/local):
```bash
$ cmake -D CMAKE_INSTALL_PREFIX=<DEADBEEF_INSTALL_PREFIX> ../
```

Then run make:
```bash
$ make
# make install
```
Then choose the QT gui plugin in the preference and restart DeaDBeeF. Another possibility is to start deadbeef with `--gui qt5`	 parameter.
