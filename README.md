# ddb_gui_qt5

This plugin provides a Qt interface for [deadbeef](http://deadbeef.sourceforge.net/). This plugin is continuation of [deadbeef-qt plugin](https://github.com/redpunk231/deadbeef-qt.git) ported to Qt5

![image](https://user-images.githubusercontent.com/6359901/56922057-a6252880-6ac7-11e9-807f-3dc7b49ad502.png)

## Install
For installation requires Qt 5 (>=5.9.0) or greater and DeaDBeeF (>=1.8.0). For debian based systems install `qt5-default`.

Get ddb_gui_qt5 sources from repository:
```bash
$ git clone git://github.com/kuba160/ddb_gui_qt5
```
Run qmake:
```bash
$ qmake ddb_gui_qt5.pro
$ make
```

The default install path on linux is `~/.local/lib/deadbeef`, on other systems copy `ddb_gui_qt5.so` / `ddb_gui_qt5.dll` to your plugins directory.
```bash
$ make install
```

Then choose the QT gui plugin in the preferences and restart DeaDBeeF. Another possibility is to start deadbeef with `--gui qt5` parameter.
