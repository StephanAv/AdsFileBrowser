QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17


# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    adsfileinfonode.cpp \
    adsfilesystemmodel.cpp \
    adsfiletree.cpp \
    main.cpp \
    mainwindow.cpp

HEADERS += \
    adsfileinfonode.h \
    adsfilesystemmodel.h \
    adsfiletree.h \
    mainwindow.h

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

unix:!macx: LIBS += -L$$PWD/DeviceManager_ADS_Samples/build/ -lDeviceManager

INCLUDEPATH += $$PWD/DeviceManager_ADS_Samples/Areas
INCLUDEPATH += $$PWD/DeviceManager_ADS_Samples/ADS
INCLUDEPATH += $$PWD/DeviceManager_ADS_Samples/build/_deps/beckhoff_ads-src/AdsLib

#DEPENDPATH += $$PWD/DeviceManager_ADS_Samples/Areas

unix:!macx: PRE_TARGETDEPS += $$PWD/DeviceManager_ADS_Samples/build/libDeviceManager.a

unix:!macx: LIBS += -L$$PWD/DeviceManager_ADS_Samples/build/_deps/beckhoff_ads-build/ -lads

# Linux ADS Client

INCLUDEPATH += $$PWD/DeviceManager_ADS_Samples/build/_deps/beckhoff_ads-build
DEPENDPATH += $$PWD/DeviceManager_ADS_Samples/build/_deps/beckhoff_ads-build
unix:!macx: PRE_TARGETDEPS += $$PWD/DeviceManager_ADS_Samples/build/_deps/beckhoff_ads-build/libads.a

# Windows ADS Client
win32: DEFINES += USE_TWINCAT_ROUTER
win32: INCLUDEPATH += C:\TwinCAT\AdsApi\TcAdsDll\Include

DEPENDPATH += C:/TwinCAT/AdsApi/TcAdsDll/x64
win32: LIBS += -L$$PWD/ -lTcAdsDll

# Windows Device Manager build dependency

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/DeviceManager_ADS_Samples/build/release/ -lDeviceManager
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/DeviceManager_ADS_Samples/build/debug/ -lDeviceManager

DEPENDPATH += $$PWD/DeviceManager_ADS_Samples/build/Debug



#win32: LIBS += -L$$PWD/DeviceManager_ADS_Samples/build/debug/ -lDeviceManager
#win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../../../../TwinCAT/AdsApi/TcAdsDll/x64/lib/ -lTcAdsDll
#else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../../../../TwinCAT/AdsApi/TcAdsDll/x64/lib/ -lTcAdsDlld


