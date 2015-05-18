
include $(top_srcdir)/Makefile.am.inc

pkglib_LTLIBRARIES = libcedar_base.la

libcedar_base_la_SOURCES = AwPool.c CdxQueue.c CdxList.c CdxUtils.c 

libcedar_base_la_CFLAGS = $(CEDAR_CFLAGS)
LOCAL_INCLUDE = -I./include
libcedar_base_la_CFLAGS += $(LOCAL_INCLUDE)

libcedar_base_la_LDFLAGS = $(CEDAR_LDFLAGS)
LOCAL_LIB = -lpthread
libcedar_base_la_LDFLAGS += $(LOCAL_LIB)

