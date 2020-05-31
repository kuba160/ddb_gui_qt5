TEMPLATE = app
QT = 
SOURCES += test_libintl.c

win32 {
	LIBS += -lintl
}
