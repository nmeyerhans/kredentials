/*
 * Copyright 2004,2006,2011 by the Massachusetts Institute of
 * Technology.  All Rights Reserved.
 * Copyright 2007, Martin Ginkel <mginkel@mpi-magdeburg.mpg.de>
 *
 * Permission to use, copy, modify, and distribute this software and
 * its documentation for any purpose and without fee is hereby
 * granted, provided that the above copyright notice appear in all
 * copies and that both that copyright notice and this permission
 * notice appear in supporting documentation, and that the name of
 * M.I.T. not be used in advertising or publicity pertaining to
 * distribution of the software without specific, written prior
 * permission.  Furthermore if you modify this software you must label
 * your software as modified software and not distribute it in such a
 * fashion that it might be confused with the original
 * M.I.T. software.  M.I.T. makes no representations about the
 * suitability of this software for any purpose.  It is provided "as
 * is" without express or implied warranty.
 *
 * $Id$
 */

#include "kredentials.h"
#include <kuniqueapplication.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <klocale.h>
#include <kdebug.h>
#include <kmainwindow.h>

static const char version[] = "2.0-alpha";

int main(int argc, char **argv)
{
    KAboutData about("kredentials",
		     "kredentials",
		     ki18n("kredentials"),
		     version,
		     ki18n("Monitor and update authentication tokens"),
		     KAboutData::License_Custom, 
		     ki18n("(C) 2004 Noah Meyerhans"));
    about.addAuthor( ki18n("Noah Meyerhans"), ki18n("developer"), 
		     "noahm@csail.mit.edu", 0 );
    KCmdLineArgs::init(argc, argv, &about);
    KCmdLineOptions options;
    options.add("i").add("inform", 
			 ki18n("Inform the user when credentials are renewed"));
    options.add("d").add("disable-aklog", 
			 ki18n("Don't run aklog after renewing Kerberos tickets"));
    KCmdLineArgs::addCmdLineOptions( options );
    KUniqueApplication::addCmdLineOptions();

	if (!KUniqueApplication::start()) {
	    kError() << "Kredentials is already running!";
	    exit(0);
	}

	KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
	KUniqueApplication app;
	app.disableSessionManagement();
	kredentials *k_obj = 0;
	KMainWindow *kmw = new KMainWindow();
	kmw->setObjectName("kredentials");

	k_obj = new kredentials();
	if(args->isSet("inform"))
	{
	    k_obj->setDoNotify(true);
	}
	if(args->isSet("disable-aklog"))
	{
		k_obj->setDoAklog(false);
	}

	k_obj->show();

    return app.exec();
}

