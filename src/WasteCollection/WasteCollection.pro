QT       += core gui sql charts network printsupport location positioning quick quickwidgets serialport
QML_IMPORT_PATH += C:/Qt/6.7.3/mingw_64/qml
QML_DESIGNER_IMPORT_PATH += C:/Qt/6.7.3/mingw_64/qml

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
QT += charts
QT += texttospeech
CONFIG += c++17
RC_ICONS = logo_untitled.ico

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    arduino.cpp \
    connection.cpp \
    main.cpp \
    gwastecollection.cpp \
    employee.cpp \
    municipality.cpp \
    sewer.cpp \
    collectionteam.cpp \
    recyclingcenter.cpp

HEADERS += \
    arduino.h \
    connection.h \
    gwastecollection.h \
    employee.h \
    municipality.h \
    sewer.h \
    collectionteam.h \
    recyclingcenter.h

FORMS += \
    gwastecollection.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    resources.qrc

DISTFILES +=
CONFIG -= qml_debug
