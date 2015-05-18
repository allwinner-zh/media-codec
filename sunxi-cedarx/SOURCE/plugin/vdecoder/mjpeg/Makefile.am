
include $(top_srcdir)/Makefile.am.inc

pkglib_LTLIBRARIES = libcedar_plugin_vd_mjpeg.la

libcedar_plugin_vd_mjpeg_la_SOURCES = \
							mjpeg_hal.c \
							mjpeg.c \
					   		mjpeg_dec_lib.c

LOCAL_INCLUDE = -I. \
			 -I$(top_srcdir)/base/include \
			 -I$(top_srcdir)/common/include \
			 -I$(top_srcdir)/vdecoder/include

libcedar_plugin_vd_mjpeg_la_CFLAGS = $(CEDAR_CFLAGS)
libcedar_plugin_vd_mjpeg_la_CFLAGS += $(LOCAL_INCLUDE)

libcedar_plugin_vd_mjpeg_la_LIBADD = \
			$(top_srcdir)/base/libcedar_base.la \
		    $(top_srcdir)/common/libcedar_common.la \
		    $(top_srcdir)/vdecoder/libcedar_vdecoder.la

libcedar_plugin_vd_mjpeg_la_LDFLAGS = $(CEDAR_LDFLAGS) -module

