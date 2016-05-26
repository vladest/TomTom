TARGET = qtgeoservices_tomtom
QT += location-private positioning-private network

PLUGIN_TYPE = geoservices
PLUGIN_CLASS_NAME = QGeoServiceProviderFactoryTomtom
load(qt_plugin)

HEADERS += \
    qgeoserviceproviderplugintomtom.h \
    qgeocodingmanagerenginetomtom.h \
    qgeocodereplytomtom.h \
    qgeoroutingmanagerenginetomtom.h \
    qgeoroutereplytomtom.h \
    qplacemanagerenginetomtom.h \
    qplacesearchreplytomtom.h \
    qplacecategoriesreplytomtom.h \
    qgeomapreplytomtom.h \
    qgeotiledmaptomtom.h \
    qgeotiledmappingmanagerenginetomtom.h \
    qgeotilefetchertomtom.h

SOURCES += \
    qgeoserviceproviderplugintomtom.cpp \
    qgeocodingmanagerenginetomtom.cpp \
    qgeocodereplytomtom.cpp \
    qgeoroutingmanagerenginetomtom.cpp \
    qgeoroutereplytomtom.cpp \
    qplacemanagerenginetomtom.cpp \
    qplacesearchreplytomtom.cpp \
    qplacecategoriesreplytomtom.cpp \
    qgeomapreplytomtom.cpp \
    qgeotiledmaptomtom.cpp \
    qgeotiledmappingmanagerenginetomtom.cpp \
    qgeotilefetchertomtom.cpp


OTHER_FILES += \
    tomtom_plugin.json

