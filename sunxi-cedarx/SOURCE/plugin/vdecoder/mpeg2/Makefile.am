
include $(top_srcdir)/Makefile.am.inc

pkglib_LTLIBRARIES = libcedar_plugin_vd_mpeg2.la

libcedar_plugin_vd_mpeg2_la_SOURCES = \
							mpeg2Dec.c \
						   	mpeg2Hal.c \
						   	mpeg2.c 

LOCAL_INCLUDE = -I. \
			 -I$(top_srcdir)/base/include \
			 -I$(top_srcdir)/common/include \
			 -I$(top_srcdir)/vdecoder/include

libcedar_plugin_vd_mpeg2_la_CFLAGS = $(CEDAR_CFLAGS)
libcedar_plugin_vd_mpeg2_la_CFLAGS += $(LOCAL_INCLUDE)

libcedar_plugin_vd_mpeg2_la_LIBADD = \
			$(top_srcdir)/base/libcedar_base.la \
		    $(top_srcdir)/common/libcedar_common.la \
		    $(top_srcdir)/vdecoder/libcedar_vdecoder.la

libcedar_plugin_vd_mpeg2_la_LDFLAGS = $(CEDAR_LDFLAGS) -module

