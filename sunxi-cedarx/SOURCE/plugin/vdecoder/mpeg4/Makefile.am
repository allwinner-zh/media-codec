
include $(top_srcdir)/Makefile.am.inc

pkglib_LTLIBRARIES = libcedar_plugin_vd_mpeg4.la

libcedar_plugin_vd_mpeg4_la_SOURCES = \
							mp4_dec_h263.c \
						  	mp4_hal.c  \
						  	mp4_vld.c  \
						    mp4_deccommon.c  \
							mp4_dec_xvid.c  \
						   	mp4_header.c   \
						  	mp4_table.c  \
							mpeg4.c \
						   	mp4_get_bits.c \
						  	mp4_register.c

LOCAL_INCLUDE = -I. \
			 -I$(top_srcdir)/base/include \
			 -I$(top_srcdir)/common/include \
			 -I$(top_srcdir)/vdecoder/include

libcedar_plugin_vd_mpeg4_la_CFLAGS = $(CEDAR_CFLAGS) 
libcedar_plugin_vd_mpeg4_la_CFLAGS += -Wno-strict-aliasing
libcedar_plugin_vd_mpeg4_la_CFLAGS += $(LOCAL_INCLUDE)

libcedar_plugin_vd_mpeg4_la_LIBADD = \
			$(top_srcdir)/base/libcedar_base.la \
		    $(top_srcdir)/common/libcedar_common.la \
		    $(top_srcdir)/vdecoder/libcedar_vdecoder.la

libcedar_plugin_vd_mpeg4_la_LDFLAGS = $(CEDAR_LDFLAGS) -module

