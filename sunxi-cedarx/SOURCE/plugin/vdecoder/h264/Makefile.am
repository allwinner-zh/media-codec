
include $(top_srcdir)/Makefile.am.inc

pkglib_LTLIBRARIES = libcedar_plugin_vd_h264.la

libcedar_plugin_vd_h264_la_SOURCES = h264.c \
									 h264_dec.c \
									 h264_nalu.c \
									 h264_hal.c  \
									 h264_mmco.c 

LOCAL_INCLUDE = -I. \
			 -I$(top_srcdir)/base/include \
			 -I$(top_srcdir)/common/include \
			 -I$(top_srcdir)/vdecoder/include

libcedar_plugin_vd_h264_la_CFLAGS = $(CEDAR_CFLAGS)
libcedar_plugin_vd_h264_la_CFLAGS += $(LOCAL_INCLUDE)

libcedar_plugin_vd_h264_la_LIBADD = \
			$(top_srcdir)/base/libcedar_base.la \
		    $(top_srcdir)/common/libcedar_common.la \
		    $(top_srcdir)/vdecoder/libcedar_vdecoder.la

libcedar_plugin_vd_h264_la_LDFLAGS = $(CEDAR_LDFLAGS) -module

