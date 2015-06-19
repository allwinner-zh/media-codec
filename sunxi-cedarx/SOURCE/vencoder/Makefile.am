
include $(top_srcdir)/Makefile.am.inc

pkglib_LTLIBRARIES = libcedar_vencoder.la
 
libcedar_vencoder_la_SOURCES =  BitstreamManager.c \
							   	EncAdapter.c \
							   	FrameBufferManager.c  \
								include  \
								venc_device.c \
							   	vencoder.c
 
LOCAL_INCLUDE = -I./include \
             -I$(top_srcdir)/base/include \
             -I$(top_srcdir)/common/include 

libcedar_vencoder_la_CFLAGS = $(CEDAR_CFLAGS)
libcedar_vencoder_la_CFLAGS += $(LOCAL_INCLUDE) 

libcedar_vencoder_la_LIBADD = $(top_srcdir)/base/libcedar_base.la \
							  $(top_srcdir)/common/libcedar_common.la

libcedar_vencoder_la_LDFLAGS = $(CEDAR_LDFLAGS) -ldl


