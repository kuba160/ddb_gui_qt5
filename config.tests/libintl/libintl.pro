TEMPLATE = app
QT = 
SOURCES += test_libintl.cpp

win32 {
	LIBS += -lintl
}
