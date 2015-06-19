
include $(top_srcdir)/Makefile.am.inc

pkglib_LTLIBRARIES = libcedar_common.la
  
libcedar_common_la_SOURCES = memoryAdapter.c \
						   ve.c \
						   ve_alloc.c   
 
libcedar_common_la_CFLAGS = $(CEDAR_CFLAGS)
LOCAL_INCLUDE = -I./include \
                -I$(top_srcdir)/base/include
libcedar_common_la_CFLAGS += $(LOCAL_INCLUDE)

libcedar_common_la_LIBADD = $(top_srcdir)/base/libcedar_base.la 

libcedar_common_la_LDFLAGS = $(CEDAR_LDFLAGS)


