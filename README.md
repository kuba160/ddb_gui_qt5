#deadbeef-qt
This plugin provides a Qt interface for [deadbeef](http://deadbeef.sourceforge.net/)

##Install
For installation requires Qt 4.4 or greater and DeaDBeeF 0.5.6.

Get deadbeef-qt sources from repository:
```bash
$ git clone git://github.com/redpunk231/deadbeef-qt.git
```
Create temporary directory (for example </path/to/sources>/build) and change into it:
```bash
$ cd deadbeef-qt/
$ mkdir build
$ cd build
```
Run cmake:
```bash
$ cmake -D CMAKE_INSTALL_PREFIX=<DEADBEEF_INSTALL_PREFIX> ../
```
where the variable \<DEADBEEF_INSTALL_PREFIX\> is typically /usr

Then run make:
```bash
$ make
# make install
```
Then choose the QT gui plugin in the preference and restart DeaDBeeF.

##Arch Linux
PKGBUILD from [AUR](https://aur.archlinux.org/packages/deadbeef-qt-git/)
