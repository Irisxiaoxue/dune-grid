# $Id: Makefile.am 2549 2006-04-28 13:09:28Z christi $

alugriddir = $(includedir)/dune/grid/alugrid/

SUBDIRS = common 2d 3d

noinst_LTLIBRARIES = libalu.la

ALULIB = 2d/libalu2d.la 3d/libalu3d.la

# construct only from sub-libraries
libalu_la_SOURCES =
# This forces automake to use the C++ linker
# (see the automake manual, section "Libtool Convenience Libraries")
nodist_EXTRA_libalu_la_SOURCES = dummy.cc
sourcescheck_DUMMY = dummy.cc
libalu_la_LIBADD = $(ALULIB) $(DUNE_LIBS)

include $(top_srcdir)/am/global-rules

EXTRA_DIST = CMakeLists.txt
