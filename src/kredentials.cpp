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
#include <qevent.h>

#include <kapplication.h>
#include <kmainwindow.h>
#include <klocale.h>
#include <kdebug.h>
#include <kiconloader.h>
#include <kmessagebox.h>
#include <kpassivepopup.h>

#include <krb5.h>

#include <time.h>
#include <string.h>

#ifdef DEBUG
#define DEFAULT_RENEWAL_INTERVAL 20
#define LOG kdDebug()
#else
#define DEFAULT_RENEWAL_INTERVAL 3600
#define LOG kndDebug()
#endif /*DEBUG*/

kredentials::kredentials( int notify, int aklog )
    : KSystemTray()
{
    // set the shell's ui resource file
    //setXMLFile("kredentialsui.rc");

	LOG << "kredentials constructor called" << endl;

	doNotify = notify;
	doAklog  = aklog;
	secondsToNextRenewal = DEFAULT_RENEWAL_INTERVAL;
	this->setPixmap(this->loadIcon("kredentials"));
	menu = new QPopupMenu();
	//menu->insertItem("Renew Tickets", this, SLOT(renewTickets()), CTRL+Key_R);
	//menu->insertItem("Exit", i18n("Quit"), KApplication::kApplication(), SLOT(quit()));
	renewAct = new KAction(i18n("&Renew credentials"), "1rightarrow", 0,
                               this, SLOT(tryRenewTickets()), actionCollection(), "renew");
	
	renewAct->plug(menu);
	statusAct = new KAction(i18n("&Credential Status"), "", 0, this, SLOT(showTicketCache()), actionCollection(), "status");
	statusAct->plug(menu);
	menu->insertItem(SmallIcon("exit"), i18n("Quit"), kapp, SLOT(quit()));
		
	initKerberos();
	hasCurrentTickets();
	
	killTimers();
	startTimer(1000);

	LOG << "Using Kerberos KRB5CCNAME of " << krb5_cc_get_name(ctx, cc) << endl;
	LOG << "kredentials constructor returning" << endl;

}

kredentials::~kredentials()
{
}

void kredentials::initKerberos()
{
	kerror = 0;
	kerror = krb5_init_context(&ctx);
	if(kerror)
		kdDebug() << "Kerberos returned " << kerror << endl;
	kerror = krb5_cc_default(ctx, &cc);
	if(kerror)
		kdDebug() << "Kerberos returned " << kerror << endl;
	kdDebug() << "Set cc to " << krb5_cc_get_name(ctx, cc) << endl;
	kerror = krb5_cc_get_principal(ctx, cc, &me);
	if(kerror)
	{
		kdDebug() << "Kerberos returned " << kerror << endl;
	}
	
	return;
}

void kredentials::mousePressEvent(QMouseEvent *e)
{
	if(e->button() == RightButton)
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

	// Can't renew tickets if we don't have any to renew
	if(!authenticated)
		return -1;
		
	krb5_get_init_creds_opt_init(&options);
	memset(&my_creds, 0, sizeof(my_creds));
		

	LOG << "renewing tickets for " << me->data->data << "@" << me->realm.data << endl;


	kerror = krb5_get_renewed_creds(ctx, &my_creds, me, cc,
							NULL);

	if(kerror)
	{
		kdDebug() << "Kerberos returned " << kerror << 
			" while renewing creds" << endl;
		return kerror;
	}
	kerror = krb5_cc_initialize(ctx, cc, me);
	if(kerror)
	{
		kdDebug() << "Kerberos returned " << kerror << 
			" while initializing cred cache" << endl;
		return code;
	}
	kerror = krb5_cc_store_cred(ctx, cc, &my_creds);
	if(kerror)
	{
		kdDebug() << "Kerberos returned " << kerror << 
			" while storing credentials" << endl;
		return kerror;
	}

	LOG << "Successfully renewed tickets" << endl;

	if(doNotify)
	{
		KPassivePopup::message("Kerberos tickets have been renewed", 0);
	}
	return 0;

}

void kredentials::tryRenewTickets()
{
	time_t now = time(0);
	killTimers();
	
	if(tktRenewableExpirationTime == 0)
	{
		KMessageBox::information(0, "You do not have renewable tickets.", 0, 0);
	}
	else if(tktRenewableExpirationTime < now)
	{
		KMessageBox::information(0, "Your tickets have outlived their renewable lifetime and can't be renewed.", 0, 0);
	}
	else if(renewTickets() != 0)
	{

		LOG << "renewTickets did not get new tickets" << endl;

		hasCurrentTickets();
		if(authenticated == 0)
		{
			KMessageBox::information(0, "Your tickets have expired. Please run 'renew' in a shell.", "Kerberos", 0, 0);
		}
	}
	else
	{
		// tickets were successfully renewed
		
	}
	// restart the timer here, regardless of whether we currently have tickets now or not.
	// The user may get tickets before the next timeout, and we need to be able to renew them
	secondsToNextRenewal = DEFAULT_RENEWAL_INTERVAL;
	startTimer(1000);
	if(authenticated > 0)
	{
		if(doAklog)
		{
			LOG << "Calling aklog" << endl;
			int ret = system("aklog");
			if( ret == -1 )
			{
				KMessageBox::sorry(0, "Unable to run aklog", 0, 0);
			}
			else if( ret )
			{
				KMessageBox::sorry(0, "aklog failed to obtain new AFS tokens.", 0, 0);
			}
		}
		
		if((tktRenewableExpirationTime - now) < 3600 &&
		((now % 900) == 0))
		{
			// tickets expire in less than 1 hour
			KPassivePopup::message("Kerberos tickets expire in less than one hour.  You may wish to renew soon.", 0);
		}
	}
	return;
}


void kredentials::hasCurrentTickets()
{
	int noTix = 1;
	krb5_cc_cursor cur;
	krb5_creds creds;
	krb5_principal princ;
	krb5_int32 now;
	
		
	LOG << "Called hasCurrentTickets()" << endl;

	/* if kerberos is not currently happy, try reinitializing.  The user may 
	   have obtained new tickets since we last initialized.
	*/
	if(kerror)
	{

		LOG << "hasCurrentTickets(): kerror = " << kerror << endl;
		LOG << "Trying to reinitialize kerberos..." << endl;

		initKerberos();
		if(kerror)
		{
			/* If kerberos is still unhappy, we are not authenticated and can
			   return now. */
			authenticated = 0;
			return;
		}
	}
	
	memset(&cur, 0, sizeof(cur));
	memset(&princ, 0, sizeof(princ));

	now = time(0);
	
	if(kerror = krb5_cc_get_principal(ctx, cc, &princ))
	{
		kdDebug() << "While retrieving principal name, Kerberos returned " << kerror << endl;
		authenticated = 0;
		return;
	}
	kerror = krb5_cc_start_seq_get(ctx, cc, &cur);
	if(kerror)
	{
		kdDebug() << "While beginning CC iterations, kerberos returned " << kerror << endl;
		authenticated = 0;
		return;
	}
	
	while (!(kerror = krb5_cc_next_cred(ctx, cc, &cur, &creds)))
	{
		if (noTix && creds.server->length == 2 &&
			strcmp(creds.server->realm.data, princ->realm.data) == 0 &&
			strcmp((char *)creds.server->data[0].data, "krbtgt") == 0 &&
			strcmp((char *)creds.server->data[1].data,
			princ->realm.data) == 0 &&
			creds.times.endtime > now)
		{
			noTix = 0;
			tktExpirationTime = creds.times.endtime;
			tktRenewableExpirationTime = creds.times.renew_till;
		}
	}
	noTix == 0 ? authenticated = 1 : authenticated = 0;


	LOG << "hasCurrentTickets set authenticated=" << authenticated << endl;


	return;
}

void kredentials::timerEvent(QTimerEvent *e)
{

	LOG << "timerEvent triggered, secondsToNextRenewal == " << secondsToNextRenewal << endl;

	secondsToNextRenewal--;
	if(secondsToNextRenewal < 0)
	{
		tryRenewTickets();
	}
	return;
}

void kredentials::showTicketCache()
{
	hasCurrentTickets();
	QString msgString;
	
	if(!authenticated)
	{
		KMessageBox::information(0, "You do not have any valid tickets.", "Kerberos", 0, 0);
	}
	else
	{
		msgString = QString("Your tickets are valid until ") + QString(ctime(&tktExpirationTime));
		if(tktRenewableExpirationTime > time(0))
		{
			msgString += QString("\nRenewable until ") + QString(ctime(&tktRenewableExpirationTime));
		}
		else
		{
			msgString += QString("\nTickets are not renewable");
		}
		KMessageBox::information(0, msgString, "Kerberos", 0, 0);
	}
	return;
}

void kredentials::setDoNotify(int state)
{
	doNotify = state;
}

void kredentials::setDoAklog(int state)
{
	doAklog = state;
}

#include "kredentials.moc"
