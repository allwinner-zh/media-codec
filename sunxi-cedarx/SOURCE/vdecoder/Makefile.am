
include $(top_srcdir)/Makefile.am.inc

pkglib_LTLIBRARIES = libcedar_vdecoder.la
 
libcedar_vdecoder_la_SOURCES = adapter.c \
							   pixelFormat.c \
							   fbm.c \
							   sbm.c \
							   vdecoder.c \
							   videoengine.c
 
LOCAL_INCLUDE = -I./include \
             -I$(top_srcdir)/base/include \
             -I$(top_srcdir)/common/include 

libcedar_vdecoder_la_CFLAGS = $(CEDAR_CFLAGS)
libcedar_vdecoder_la_CFLAGS += $(LOCAL_INCLUDE) 

libcedar_vdecoder_la_LIBADD = $(top_srcdir)/base/libcedar_base.la \
							  $(top_srcdir)/common/libcedar_common.la

libcedar_vdecoder_la_LDFLAGS = $(CEDAR_LDFLAGS) -ldl


