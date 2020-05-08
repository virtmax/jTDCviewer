#-------------------------------------------------
#
#
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets printsupport

TARGET = jTDCviewer
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
        external/MbsClient/mbsclient.cpp \
        external/MbsClient/GSI_MBS_API/MbsAPI/f_evt.c \
        external/MbsClient/GSI_MBS_API/MbsAPI/fLmd.c \
        external/MbsClient/GSI_MBS_API/MbsAPIbase/f_mbs_status.c \
        external/MbsClient/GSI_MBS_API/MbsAPIbase/f_ut_time.c \
        external/MbsClient/GSI_MBS_API/MbsAPIbase/f_ut_status.c \
        external/MbsClient/GSI_MBS_API/MbsAPI/f_ut_utime.c \
        external/MbsClient/GSI_MBS_API/MbsAPIbase/f_stccomm.c \
        external/MbsClient/GSI_MBS_API/MbsAPI/f_evcli.c \
        external/MbsClient/GSI_MBS_API/MbsAPIbase/f_swaplw.c \
        external/qcustomplotzoom/qcustomplotzoom.cpp \
        external/qcustomplotzoom/qcustomplot/qcustomplot.cpp \
        external/NucMath/src/datatable.cpp \
        external/NucMath/src/tablerow.cpp \
        external/NucMath/src/functions.cpp \
        external/NucMath/src/hist.cpp \
        external/NucMath/src/hist2.cpp \
        external/NucMath/src/integration.cpp \
        external/NucMath/src/interpolation.cpp \
        external/NucMath/src/interpolationdata.cpp \
        external/NucMath/src/interpolationsteffen.cpp \
        external/NucMath/src/minimizer.cpp \
        external/NucMath/src/minimizerdownhillsimplex.cpp \
        external/NucMath/src/optpoint.cpp \
        external/NucMath/src/utils.cpp \
        external/NucMath/src/regression.cpp \
        external/NucMath/src/signalprocessing.cpp \
        external/NucMath/src/signalreconstruction.cpp \
        external/NucMath/src/stringoperations.cpp \
        external/NucMath/src/timeseries.cpp \
        external/VmeModules/elb_vfb6_unpacker.cpp \

HEADERS  += mainwindow.h \
        external/qcustomplotzoom/qcustomplotzoom.h \
        external/qcustomplotzoom/qcustomplot/qcustomplot.h \
        external/NucMath/src/datatable.h \
        external/NucMath/src/minimizer.h \
        external/NucMath/src/minimizerdownhillsimplex.h \
        external/NucMath/src/hist.h \
        external/NucMath/src/regression.h \
        external/NucMath/src/functions.h \
        external/NucMath/src/integration.h \
        external/NucMath/src/hist2.h \
        external/MbsClient/mbsclient.h \
        external/MbsClient/readoutmarkers.h \
        external/MbsClient/GSI_MBS_API/MbsAPI/f_evt.h \
        external/MbsClient/GSI_MBS_API/MbsAPI/fLmd.h \
        external/VmeModules/elb_vfb6_unpacker.h \

FORMS    += mainwindow.ui

INCLUDEPATH += "external/qcustomplotzoom/" \
                "external/MbsClient/GSI_MBS_API/MbsAPIbase/" \
                "external/MbsClient/GSI_MBS_API/MbsAPI/" \
                "external/MbsClient/"

win32-g++{
    message("load win64 g++ configuration")
    QMAKE_CXXFLAGS += -std=c++17
    DEFINES += GSI__WINNT=1, WIN64=1
    DEFINES -= WIN32
    LIBS += -lws2_32 -lstdc++fs
}
win32-msvc*{
    message("load win64 msvc configuration")
    QMAKE_CXXFLAGS += /std:c++17
    DEFINES += GSI__WINNT=1, WIN64=1
    LIBS += -lws2_32 -pthread
}
linux{
    message("load linux configuration")
    QMAKE_CXXFLAGS += -std=c++17 -O2 -march=native
    DEFINES += Linux=1
    LIBS += -lstdc++fs
}

CONFIG += force_debug_info
