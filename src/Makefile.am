# set the include path for X, qt and KDE
INCLUDES = $(all_includes)

# these are the headers for your project
noinst_HEADERS = kredentials.h krb_defs.h

# let automoc handle all of the meta source files (moc)
METASOURCES = AUTO

messages: rc.cpp
	$(XGETTEXT) *.cpp -o $(podir)/kredentials.pot

KDE_ICON = kredentials

#########################################################################
# APPLICATION SECTION
#########################################################################
# this is the program that gets installed.  it's name is used for all
# of the other Makefile.am variables
bin_PROGRAMS = kredentials

# the application source, library search path, and link libraries
kredentials_SOURCES = main.cpp kredentials.cpp
kredentials_LDFLAGS = $(KDE_RPATH) $(all_libraries)
kredentials_LDADD = $(LIB_KDEUI)

# this is where the desktop file will go 
shelldesktopdir   = $(kde_appsdir)/Utilities
shelldesktop_DATA = kredentials.desktop

# this is where the shell's XML-GUI resource file goes
shellrcdir   = $(kde_datadir)/kredentials
shellrc_DATA = kredentialsui.rc

