
include $(top_srcdir)/Makefile.am.inc

pkglib_LTLIBRARIES = libcedar_omxadapter.la
  
libcedar_omxadapter_la_SOURCES = aw_omx_core.c \
								 transform_color_format.c \
								 omx_vdec.cpp  \
								 aw_registry_table.c  \
								 omx_core_cmp.cpp 
 
LOCAL_INCLUDE = -I./include \
             -I$(top_srcdir)/base/include \
             -I$(top_srcdir)/common/include \
             -I$(top_srcdir)/vdecoder/include 

libcedar_omxadapter_la_CFLAGS = $(CEDAR_CFLAGS)
libcedar_omxadapter_la_CFLAGS += $(LOCAL_INCLUDE)

libcedar_omxadapter_la_CPPFLAGS = $(CEDAR_CFLAGS)
libcedar_omxadapter_la_CPPFLAGS += $(LOCAL_INCLUDE)

libcedar_omxadapter_la_LIBADD = $(top_srcdir)/base/libcedar_base.la \
							    $(top_srcdir)/common/libcedar_common.la \
							    $(top_srcdir)/vdecoder/libcedar_vdecoder.la

libcedar_omxadapter_la_LDFLAGS = $(CEDAR_LDFLAGS) -ldl -lpthread -module

