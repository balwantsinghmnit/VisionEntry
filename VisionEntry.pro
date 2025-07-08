QT += core gui widgets sql concurrent charts multimedia multimediawidgets

CONFIG += c++17

# === OpenCV Config ===
OPENCV_DIR = C:/opencv/build
INCLUDEPATH += $$OPENCV_DIR/include
LIBS += -L$$OPENCV_DIR/x64/vc15/lib
LIBS += -lopencv_world455d  # or opencv_world455 for release

LIBS += -lshell32 -lkernel32 -luuid

SOURCES += \
    backuprestoredialog.cpp \
    camerathread.cpp \
    dbmanager.cpp \
    facelogindialog.cpp \
    logger.cpp \
    loginwindow.cpp \
    main.cpp \
    mainwindow.cpp \
    registerdialog.cpp \
    thememanager.cpp \
    updateprofiledialog.cpp

HEADERS += \
    backuprestoredialog.h \
    camerathread.h \
    dbmanager.h \
    facelogindialog.h \
    logger.h \
    loginwindow.h \
    mainwindow.h \
    registerdialog.h \
    thememanager.h \
    updateprofiledialog.h

FORMS += \
    backuprestoredialog.ui \
    facelogindialog.ui \
    loginwindow.ui \
    mainwindow.ui \
    registerdialog.ui \
    updateprofiledialog.ui
