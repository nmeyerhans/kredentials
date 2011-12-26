
include_directories(${KDE4_INCLUDES} ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} )


########### next target ###############

set(kredentials_SRCS main.cpp kredentials.cpp krb5_wrap.cpp)

kde4_add_executable(kredentials ${kredentials_SRCS})

target_link_libraries(kredentials ${QT_QTGUI_LIBRARY} QtGui)
target_link_libraries(kredentials ${KDE4_KDECORE_LIBS} kdeui)
target_link_libraries(kredentials ${KRB5_LIBRARIES} krb5)
target_link_libraries(kredentials ${QT_QT3SUPPORT_LIBRARY} Qt3Support)

install(TARGETS kredentials ${INSTALL_TARGETS_DEFAULT_ARGS})

########### install files ###############

install(FILES  kredentials.desktop DESTINATION share/applnk/Utilities)

kde4_install_icons(${ICON_INSTALL_DIR})



#original Makefile.am contents follow:

## set the include path for X, qt and KDE
#INCLUDES = $(all_includes)
#
## these are the headers for your project
#noinst_HEADERS = kredentials.h krb5_wrap.h
#
## let automoc handle all of the meta source files (moc)
#METASOURCES = AUTO
#
#messages: rc.cpp
#	$(XGETTEXT) *.cpp -o $(podir)/kredentials.pot
#
#KDE_ICON = kredentials
#
##########################################################################
## APPLICATION SECTION
##########################################################################
## this is the program that gets installed.  it's name is used for all
## of the other Makefile.am variables
## the second program is for testing only
#bin_PROGRAMS = kredentials 
#check_PROGRAMS = krbcheck
#TESTS = krbcheck
#
## the application source, library search path, and link libraries
#kredentials_SOURCES = main.cpp kredentials.cpp krb5_wrap.cpp
#kredentials_LDFLAGS = $(KDE_RPATH) $(all_libraries)
#kredentials_LDADD = $(LIB_KDEUI)
#
#
#krbcheck_SOURCES = krbcheck.cpp krb5_wrap.cpp
#krbcheck_LDFLAGS = $(all_libraries)
#krbcheck_LDADD = $(LIB_KDEUI)
#
## this is where the desktop file will go 
#shelldesktopdir   = $(kde_appsdir)/Utilities
#shelldesktop_DATA = kredentials.desktop
#
## this is where the shell's XML-GUI resource file goes
#shellrcdir   = $(kde_datadir)/kredentials
#shellrc_DATA = kredentialsui.rc
#
