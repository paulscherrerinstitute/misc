include /ioc/tools/driver.makefile

HEADERS += require.h
BUILDCLASSES += Linux

SOURCES += require.c
SOURCES += listRecords.c
SOURCES += updateMenuConvert.c
SOURCES += addScan.c
SOURCES += disctools.c
SOURCES += exec.c
SOURCES += mlock.c

SOURCES_vxWorks += bootNotify.c
