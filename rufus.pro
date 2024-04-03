QT += core

CONFIG += c++17 console
CONFIG -= app_bundle

# include zlib
!win32 {
    LIBS += -lz
} else {
    LIBS += -L"C:/Program Files (x86)/zlib/lib" -lzlibstatic
    INCLUDEPATH += zlib
}

SOURCES += \
    src/core/scene.cpp \
    src/core/ff7text.cpp \
    src/core/gzip.cpp \
    src/core/csvfile.cpp \
    src/arguments.cpp \
    src/main.cpp

HEADERS += \
    src/core/scene.h \
    src/core/ff7text.h \
    src/core/gzip.h \
    src/core/csvfile.h \
    src/arguments.h

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DEFINES += PROG_VERSION=\"\\\"1.0.0\\\"\" RC_PRODUCT_VERSION=1,0,0,0 PROG_NAME=\"\\\"Rufus\\\"\" RC_COMPANY_NAME_STR=\"\\\"myst6re\\\"\"

win32 {
    RC_FILE = Resources.rc
    LIBS += -ladvapi32 -lMsacm32 -lWinmm
}

OTHER_FILES += Resources.rc \
    README.md \
    deploy.bat
