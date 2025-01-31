QT = core network serialport websockets sql

CONFIG += c++17 cmdline serialport

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
        Database.cpp \
        Device.cpp \
        Pollings.cpp \
        Tests.cpp \
        UartPort.cpp \
        UdpSocket.cpp \
        WebServer.cpp \
        Wireless.cpp \
        aux_functions.cpp \
        file_handler.cpp \
        frametype0_functions.cpp \
        frametype1_functions.cpp \
        frametype3_functions.cpp \
        frametype4_functions.cpp \
        global_variables.cpp \
        main.cpp \
        process_eth_data.cpp \
        process_uart_data.cpp \
        process_webserver_data.cpp \
        time_functions.cpp

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

HEADERS += \
    Database.h \
    Device.h \
    Pollings.h \
    Tests.h \
    UartPort.h \
    UdpSocket.h \
    WebServer.h \
    Wireless.h \
    aux_functions.h \
    dali_headers.h \
    eth_frames.h \
    file_handler.h \
    frametype0_functions.h \
    frametype1_functions.h \
    frametype3_functions.h \
    frametype4_functions.h \
    global_variables.h \
    process_eth_data.h \
    process_uart_data.h \
    process_webserver_data.h \
    structures.h \
    time_functions.h \
    version_config.h

QMAKE_CXXFLAGS += -O3

