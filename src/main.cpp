/*
 * Copyright 2004 by the Massachusetts Institute of Technology.
 * All Rights Reserved.
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
 */

#include "kredentials.h"
#include <kapplication.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <klocale.h>

static const char description[] =
    I18N_NOOP("A KDE KPart Application");

static const char version[] = "0.1";

static KCmdLineOptions options[] =
{
//    { "+[URL]", I18N_NOOP( "Document to open." ), 0 },
	{ "i", 0, 0},
	{ "inform", "Inform the user when credentials are renewed", 0},
	KCmdLineLastOption
};

int main(int argc, char **argv)
{
	KAboutData about("kredentials", I18N_NOOP("kredentials"), version, description,
					KAboutData::License_Custom, "(C) 2004 Noah Meyerhans", 0, 0, "noahm@csail.mit.edu");
	about.addAuthor( "Noah Meyerhans", 0, "noahm@csail.mit.edu" );
	KCmdLineArgs::init(argc, argv, &about);
	KCmdLineArgs::addCmdLineOptions( options );
	KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
	KApplication app;
	kredentials *mainWin = 0;

	mainWin = new kredentials();
	if(args->isSet("inform"))
	{
		mainWin->setNotify(true);
	}
	app.setMainWidget( mainWin );
	mainWin->show();

	//args->clear();

    // mainWin has WDestructiveClose flag by default, so it will delete itself.
    return app.exec();
}

