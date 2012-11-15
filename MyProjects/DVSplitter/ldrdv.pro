# $Id: ldrdv.pro,v 1.1 2000/06/20 09:17:38 mikes Exp $

TEMPLATE                        =  lib
CONFIG                          =  warn_on dll
TARGET                          =  ldrdv
unix:DESTDIR                    =  ../../../build/loader
win32:DLLDESTDIR                =  ../../../build/loader

SOURCES                         =  ldrdv.cpp ../../filter_functions.c
#    jcomapi.c   jdcolor.c   jdmaster.c  jerror.c    jidctint.c  jutils.c \
#    jdapimin.c  jddctmgr.c  jdmerge.c   jfdctflt.c  jidctred.c \
#    jdapistd.c  jdhuff.c    jdphuff.c   jfdctfst.c  jmemansi.c \
#    jdatadst.c  jdinput.c   jdpostct.c  jfdctint.c  jmemmgr.c \
#    jdatasrc.c  jdmainct.c  jdsample.c  jidctflt.c  jquant1.c \
#    jdcoefct.c  jdmarker.c  jdtrans.c   jidctfst.c  jquant2.c

HEADERS                         =  ldrdv.h
INCLUDEPATH                     =  ../../../H .
#DEF_FILE                       =  ldrdv.def

unix:DEFINES                    =  _REENTRANT

win32:DEFINES                   =  WIN32
win32:TMAKE_CFLAGS              += -MD
win32:TMAKE_CXXFLAGS            += -MD

unix:LIBS                       =  -L../../mcdv32/ -lmadventry

win32:LIBS                      =  ../../lib/dv.w32/mcdvd_32.lib

debug:DEFINES                   += LDSV_DEBUG=2
