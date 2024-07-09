TEMPLATE = app
TARGET = KRSW
QT += core gui charts

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets 

#QMAKE_CXXFLAGS += -g  #for debugging

QT_QPA_PLATFORM=wayland

SOURCES += \
  src/main.cpp \
  src/windows/main_window.cpp \
  src/windows/add_dialog.cpp \
  src/windows/edit_dialog.cpp \
  src/windows/info_dialog.cpp \
  src/windows/widgets/toolbar.cpp \
  src/windows/widgets/searchbar.cpp \
  src/windows/widgets/chartdock.cpp \
  src/view/chart.cpp \
  src/view/area_chart.cpp \
  src/view/pie_chart.cpp \
  src/view/stackedbars_chart.cpp \
  src/view/line_chart.cpp \
  src/model/sensor.cpp \
  src/model/cpu_sensor.cpp \
  src/model/memory_sensor.cpp \
  src/model/network_sensor.cpp \
  src/model/disk_sensor.cpp \
  src/utils/chart_factory.cpp \
  src/utils/sensor_factory.cpp 


INCLUDEPATH += \
  src/ \
  src/controller/ \
  src/model/ \
  src/utils/ \
  src/view/ 

HEADERS += \
  src/windows/main_window.h \  
  src/windows/add_dialog.h \
  src/windows/edit_dialog.h \
  src/windows/info_dialog.h \
  src/windows/widgets/toolbar.h \
  src/windows/widgets/searchbar.h \
  src/windows/widgets/chartdock.h \
  src/view/chart.h \
  src/view/area_chart.h \
  src/view/pie_chart.h \
  src/view/stackedbars_chart.h \
  src/view/line_chart.h \
  src/model/sensor.h \
  src/model/cpu_sensor.h \
  src/model/memory_sensor.h \
  src/model/network_sensor.h \
  src/model/disk_sensor.h \
  src/utils/chart_factory.h \
  src/utils/sensor_factory.h
  src/utils/enums.h

RESOURCES += \
  assets/resources.qrc

OBJECTS_DIR = tmp/release/.obj
MOC_DIR = tmp/release/.moc
RCC_DIR = tmp/release/.qrc

#QMAKE_POST_LINK += echo "Post link phase started" && \
#                   ./scripts/desktopfile.sh && \
#                   echo "Post link phase ended"
                   
desktop.path = $$(HOME)/.local/share/applications
icon.path = $$(HOME)/.local/share/icons

desktop.files = ./tmp/krsw.desktop
icon.files = assets/KRSW.png

# Add the .desktop file to the INSTALLS variable so it gets installed
INSTALLS += desktop icon


# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = $$(HOME)/.local/bin
!isEmpty(target.path): INSTALLS += target