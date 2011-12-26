set(KREDENTIALS_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(CMAKE_MODULE_PATH ${KREDENTIALS_SOURCE_DIR}/cmake/modules ${CMAKE_MODULE_PATH})

include_directories(${KRB5_INCLUDE_DIRS})
set(LIBS ${LIBS} ${KRB5_LIBRARIES})

find_package(KDE4 REQUIRED)
find_package(Kerberos5 REQUIRED)

add_definitions(${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
add_definitions (-DQT3_SUPPORT -DQT3_SUPPORT_WARNINGS)
add_definitions(-DKDE_DEFAULT_DEBUG_AREA=9876)

include(KDE4Defaults)

include(MacroLibrary)

# include(ConvenienceLibs.cmake)

# include(ManualStuff.cmake)

# include(ConfigureChecks.cmake)
add_subdirectory(src)
add_subdirectory(cmake)
include_directories(${KDE4_INCLUDES} ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} )

### UNCOMMENT the following to enable debug builds
# remove_definitions(-DNDEBUG -DQT_NO_DEBUG)
# add_definitions(-DDEBUG)

message(STATUS "${CMAKE_CURRENT_SOURCE_DIR}: skipped subdir $(TOPSUBDIRS)")

########### install files ###############




#original Makefile.am contents follow:

#SUBDIRS = $(TOPSUBDIRS)
#
#configure.in: configure.in.in $(top_srcdir)/subdirs
#	$(MAKE) -f admin/Makefile.common configure.in ;
#
#$(top_srcdir)/subdirs:
#	cd $(top_srcdir) && $(MAKE) -f admin/Makefile.common subdirs
#
#$(top_srcdir)/acinclude.m4: $(top_srcdir)/admin/acinclude.m4.in $(top_srcdir)/admin/libtool.m4.in
#	@cd $(top_srcdir) && cat admin/acinclude.m4.in admin/libtool.m4.in > acinclude.m4
#
#MAINTAINERCLEANFILES = subdirs configure.in acinclude.m4 configure.files 
#
#package-messages:
#	cd $(top_srcdir) && $(MAKE) -f admin/Makefile.common package-messages
#	$(MAKE) -C po merge
#
#EXTRA_DIST = admin  configure.in.in
#
#dist-hook:
#	cd $(top_distdir) && perl admin/am_edit -padmin
#	cd $(top_distdir) && $(MAKE) -f admin/Makefile.common subdirs
