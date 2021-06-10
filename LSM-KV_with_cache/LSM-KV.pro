QT -= gui

CONFIG += c++14 console
CONFIG -= app_bundle

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
        BloomFilter.cpp \
        SSTable.cpp \
        SkipList.cpp \
        kvstore.cc \
        correctness.cc \
#        persistence.cc \
#        performance.cpp



# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

HEADERS += \
    BloomFilter.h \
    DataStructs.h \
    MurmurHash3.h \
    SSTable.h \
    SkipList.h \
    kvstore.h \
    kvstore_api.h \
    test.h \
    utils.h
