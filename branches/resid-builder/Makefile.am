
ACLOCAL_AMFLAGS = -I m4

AUTOMAKE_OPTIONS = foreign subdir-objects

if HAVE_VISIBILITY
  AM_CXXFLAGS = -fvisibility=hidden -fvisibility-inlines-hidden
endif

AM_CPPFLAGS = -I $(top_builddir)/builders/residfp-builder \
-I $(top_builddir)/builders/residfp-builder/residfp

#=========================================================
EXTRA_DIST = \
sidplay/psiddrv.bin \
sidplay/kernal.bin \
sidplay/poweron.bin \
sidplay/basic.bin \
sidplay/char.bin \
sidplay/c64/c64cia.h \
sidplay/c64/c64vic.h \
sidplay/c64/c64xsid.h \
sidplay/mos6510/cycle_based \
sidplay/AUTHORS \
sidplay/ChangeLog \
sidplay/COPYING \
sidplay/README \
sidplay/TODO \
utils/ini/headings.i \
utils/ini/keys.i \
utils/ini/list.i \
utils/ini/types.i \
utils/AUTHORS \
utils/ChangeLog \
utils/COPYING \
utils/README \
utils/TODO \
builders/residfp-builder/residfp/AUTHORS \
builders/residfp-builder/residfp/ChangeLog \
builders/residfp-builder/residfp/COPYING \
builders/residfp-builder/residfp/INSTALL \
builders/residfp-builder/residfp/README \
builders/residfp-builder/residfp/README.VICE \
builders/residfp-builder/residfp/NEWS

#=========================================================
# Libraries
lib_LTLIBRARIES = \
sidplay/libsidplay2.la \
utils/libsidutils.la \
builders/residfp-builder/libresidfp-builder.la

if HARDSID
  lib_LTLIBRARIES += builders/hardsid-builder/libhardsid-builder.la
endif

noinst_LIBRARIES = \
builders/residfp-builder/residfp/libresidfp.a

#=========================================================
# libsidplay
sidplay_libsidplay2_la_SOURCES = \
sidplay/config.cpp \
sidplay/event.cpp \
sidplay/player.cpp \
sidplay/player.h \
sidplay/psiddrv.cpp \
sidplay/mixer.cpp \
sidplay/reloc65.c \
sidplay/sidplay2.cpp \
sidplay/nullsid.h \
sidplay/psiddrv.a65 \
sidplay/sidenv.h \
sidplay/mos656x/mos656x.cpp \
sidplay/mos656x/mos656x.h \
sidplay/mos6510/mos6510.cpp \
sidplay/mos6510/mos6510.h \
sidplay/mos6510/conf6510.h \
sidplay/mos6510/opcodes.h \
sidplay/mos6526/mos6526.cpp \
sidplay/mos6526/mos6526.h \
sidplay/sid6526/sid6526.cpp \
sidplay/sid6526/sid6526.h \
sidplay/sidtune/IconInfo.cpp \
sidplay/sidtune/InfoFile.cpp \
sidplay/sidtune/MUS.cpp \
sidplay/sidtune/p00.cpp \
sidplay/sidtune/PP20.cpp \
sidplay/sidtune/PP20.h \
sidplay/sidtune/PP20_Defs.h \
sidplay/sidtune/prg.cpp \
sidplay/sidtune/PSID.cpp \
sidplay/sidtune/SidTune.cpp \
sidplay/sidtune/SidTuneCfg.h \
sidplay/sidtune/SidTuneTools.cpp \
sidplay/sidtune/SidTuneTools.h \
sidplay/xsid/xsid.cpp \
sidplay/xsid/xsid.h

sidplay_libsidplay2_la_LDFLAGS = -version-info $(LIBSIDPLAYVERSION)

sidplay_libsidplay2_ladir = $(includedir)/sidplay

sidplay_libsidplay2_la_HEADERS = \
sidplay/sidconfig.h \
sidplay/Buffer.h \
sidplay/c64env.h \
sidplay/component.h \
sidplay/event.h \
sidplay/sid2types.h \
sidplay/sidbuilder.h \
sidplay/sidendian.h \
sidplay/sidplay2.h \
sidplay/SidTune.h \
sidplay/sidtypes.h \
sidplay/sidusage.h \
sidplay/SmartPtr.h

#=========================================================
# libsidutils
utils_libsidutils_ladir = $(includedir)/sidplay/utils

utils_libsidutils_la_HEADERS = \
utils/libini.h \
utils/SidDatabase.h \
utils/SidFilter.h \
utils/SidTuneMod.h \
utils/SidUsage.h

utils_libsidutils_la_SOURCES = \
utils/SidDatabase.cpp \
utils/SidFilter.cpp \
utils/SidTuneMod.cpp \
utils/SidUsage.cpp \
utils/smm.cpp \
utils/smm0.h \
utils/ini/headings.h \
utils/ini/ini.cpp \
utils/ini/ini.h \
utils/ini/keys.h \
utils/ini/list.h \
utils/MD5/MD5.cpp \
utils/MD5/MD5.h \
utils/MD5/MD5_Defs.h

utils_libsidutils_la_LIBADD = sidplay/libsidplay2.la

utils_libsidutils_la_LDFLAGS = -version-info $(LIBSIDUTILSVERSION)

#=========================================================
# resid

if USE_SSE
  convolve_sse_src = builders/residfp-builder/residfp/convolve-sse.cc
else
  convolve_sse_src = 
endif

builders_residfp_builder_residfp_libresidfp_a_SOURCES = \
builders/residfp-builder/residfp/sid.h \
builders/residfp-builder/residfp/voice.h \
builders/residfp-builder/residfp/wave.h \
builders/residfp-builder/residfp/envelope.h \
builders/residfp-builder/residfp/filter.h \
builders/residfp-builder/residfp/extfilt.h \
builders/residfp-builder/residfp/pot.h \
builders/residfp-builder/residfp/sid.cc \
builders/residfp-builder/residfp/voice.cc \
builders/residfp-builder/residfp/wave.cc \
builders/residfp-builder/residfp/envelope.cc \
builders/residfp-builder/residfp/filter.cc \
builders/residfp-builder/residfp/extfilt.cc \
builders/residfp-builder/residfp/pot.cc \
builders/residfp-builder/residfp/version.cc \
builders/residfp-builder/residfp/convolve.cc \
$(convolve_sse_src)

builders_residfp_builder_residfp_libresidfp_a_CXXFLAGS = @RESID_FLAGS@

#=========================================================
# builders
builders_residfp_builder_libresidfp_builder_ladir = $(includedir)/sidplay/builders
builders_residfp_builder_libresidfp_builder_la_HEADERS = \
builders/residfp-builder/residfp.h

builders_residfp_builder_libresidfp_builder_la_SOURCES = \
builders/residfp-builder/residfp.cpp \
builders/residfp-builder/residfp-builder.cpp \
builders/residfp-builder/residfp-emu.h

builders_residfp_builder_libresidfp_builder_la_LIBADD = \
builders/residfp-builder/residfp/libresidfp.a

builders_residfp_builder_libresidfp_builder_la_LDFLAGS = -version-info $(LIBRESIDVERSION)


builders_hardsid_builder_libhardsid_builder_ladir = $(includedir)/sidplay/builders
builders_hardsid_builder_libhardsid_builder_la_HEADERS = \
builders/hardsid-builder/hardsid.h

if MINGW32
  hardsid_src = builders/hardsid-builder/hardsid-win.cpp
else
  hardsid_src = builders/hardsid-builder/hardsid-unix.cpp
endif

builders_hardsid_builder_libhardsid_builder_la_SOURCES = \
$(hardsid_src) \
builders/hardsid-builder/hardsid-builder.cpp  \
builders/hardsid-builder/hardsid-emu.h

builders_hardsid_builder_libhardsid_builder_la_LDFLAGS = -version-info $(LIBHARDSIDVERSION)

#=========================================================
pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libsidplay2.pc libsidutils.pc

#=========================================================
# Remove SVN directories from any subdirs that get
# included completely (e.g. win, directx,...).

dist-hook:
	rm -rf `find $(distdir) -path \*/.svn`
