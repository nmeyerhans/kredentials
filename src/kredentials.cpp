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

#include <qlabel.h>
#include <qcursor.h>

#include <kapplication.h>
#include <kmainwindow.h>
#include <klocale.h>
#include <kdebug.h>
#include <kiconloader.h>

#include <krb5.h>

#include <time.h>
#include <string.h>

kredentials::kredentials()
    : KSystemTray()
{
    // set the shell's ui resource file
    //setXMLFile("kredentialsui.rc");

#ifdef DEBUG
	kdDebug() << "kredentials constructor called" << endl;
#endif /* DEBUG */
	menu = new QPopupMenu();
	//menu->insertItem("Renew Tickets", this, SLOT(renewTickets()), CTRL+Key_R);
	//menu->insertItem("Exit", i18n("Quit"), KApplication::kApplication(), SLOT(quit()));
	renewAct = new KAction(i18n("&Renew credentials"), "1rightarrow", 0,
                               this, SLOT(renewTickets()), actionCollection(), "renew");
	
	renewAct->plug(menu);
	statusAct = new KAction(i18n("&Credential Status"), "", 0, this, SLOT(hasCurrentTickets()), actionCollection(), "status");
	statusAct->plug(menu);
	menu->insertItem(SmallIcon("exit"), i18n("Quit"), kapp, SLOT(quit()));
		
	kerror = 0;

	kerror = krb5_init_context(&ctx);
	if(kerror)
		kdDebug() << "Kerberos returned " << kerror << endl;
	kerror = krb5_cc_default(ctx, &cc);
	if(kerror)
		kdDebug() << "Kerberos returned " << kerror << endl;
	kdDebug() << "Set cc to " << cc << endl;
	kerror = krb5_cc_get_principal(ctx, cc, &me);
	if(kerror)
	{
		kdDebug() << "Kerberos returned " << kerror << endl;
	}
#ifdef DEBUG
	kdDebug() << "Using Kerberos KRB5CCNAME of " << cc << endl;
	kdDebug() << "kredentials constructor returning" << endl;
#endif /* DEBUG */
}

kredentials::~kredentials()
{
}

void kredentials::mousePressEvent(QMouseEvent *e)
{
	if(e->button() == LeftButton)
	{
		menu->popup(QCursor::pos());
	}
}

int kredentials::renewTickets()
{
		krb5_creds my_creds;
		krb5_error_code code = 0;
		krb5_get_init_creds_opt options;
		//const char *err_mesg;

		krb5_get_init_creds_opt_init(&options);
		memset(&my_creds, 0, sizeof(my_creds));
		
#ifdef DEBUG
		kdDebug() << "renewing tickets for " << me->data->data << "@" << me->realm.data << endl;
#endif /* DEBUG */

		kerror = krb5_get_renewed_creds(ctx, &my_creds, me, cc,
								NULL);

		if(kerror)
		{
				kdDebug() << "Kerberos returned " << kerror << endl;
				return kerror;
		}
		kerror = krb5_cc_initialize(ctx, cc, me);
		if(kerror)
		{
				kdDebug() << "Kerberos returned " << kerror << endl;
				return code;
		}
		kerror = krb5_cc_store_cred(ctx, cc, &my_creds);
		if(kerror)
		{
				kdDebug() << "Kerberos returned " << kerror << endl;
				return kerror;
		}
#ifdef DEBUG
		kdDebug() << "Successfully renewed tickets" << endl;
#endif
		return 0;

}

int kredentials::hasCurrentTickets()
{
	int noTix = 1;
	krb5_cc_cursor cur;
	krb5_creds creds;
	krb5_principal princ;
	krb5_flags flags;
	krb5_int32 now;
	
	memset(&cur, 0, sizeof(cur));
	memset(&princ, 0, sizeof(princ));
	
#ifdef DEBUG
	kdDebug() << "Called hasCurrentCredentials()" << endl;
#endif /* DEBUG */

	now = time(0);
	
	flags = 0;
	kerror = krb5_cc_set_flags(ctx, cc, flags);
	if(kerror == KRB5_FCC_NOFILE)
	{
		kdDebug() << "error: no credentials cache exists" << endl;
		authenticated = 0;
	}
	
	if(kerror = krb5_cc_get_principal(ctx, cc, &princ))
	{
		kdDebug() << "While retrieving principal name, Kerberos returned " << kerror << endl;
	}
	kerror = krb5_cc_start_seq_get(ctx, cc, &cur);
	if(kerror)
	{
		kdDebug() << "While beginning CC iterations, kerberos returned " << kerror << endl;
	}
	
	while (!(kerror = krb5_cc_next_cred(ctx, cc, &cur, &creds)))
	{
		if (noTix && creds.server->length == 2 &&
			strcmp(creds.server->realm.data, princ->realm.data) == 0 &&
			strcmp((char *)creds.server->data[0].data, "krbtgt") == 0 &&
			strcmp((char *)creds.server->data[1].data,
			princ->realm.data) == 0 &&
			creds.times.endtime > now)
				noTix = 0;
	}
	noTix == 0 ? authenticated = 1 : authenticated = 0;

#ifdef DEBUG
	kdDebug() << "hasCurrentCredentials set authenticated=" << authenticated << endl;
#endif /* DEBUG */

	return noTix;
}
#include "kredentials.moc"
