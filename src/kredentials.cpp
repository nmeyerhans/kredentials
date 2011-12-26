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
 *
 * $Id$
 */

#include "kredentials.h"

#include <qlabel.h>
#include <qcursor.h>
#include <qevent.h>
#include <QMouseEvent>
#include <QTimerEvent>
#include <kmenu.h>

#include <kapplication.h>
#include <kmainwindow.h>
#include <klocale.h>
#include <kdebug.h>
#include <kiconloader.h>
#include <kmessagebox.h>
#include <kpassivepopup.h>
#include <kpassworddialog.h>

// XXX TEMPORARILY disable dcop stuff while porting to KDE4
#if 0
// for DCOP access to screen saver
#include <dcopref.h>
#endif

//#include <krb5.h>

#include <time.h>
#include <string.h>
#include <iostream>
#include <memory>

#ifndef NDEBUG
#define DEFAULT_RENEWAL_INTERVAL 20
#define DEFAULT_WARNING_INTERVAL 3600
#define LOG kDebug()
#else
/* These intervals really need to be configurable.  In some locations
 * new tickets need to be obtained every day, while others grant
 * tickets with a long renew lifetime and only expect to get new ones
 * once every week or so.  Different intervals are appropriate at
 * different sites...
 */
#define DEFAULT_RENEWAL_INTERVAL 3600
#define DEFAULT_WARNING_INTERVAL 86400
#define LOG kDebugDevNull()
#endif /*DEBUG*/

kredentials::kredentials(int notify, int aklog)
    : KSystemTrayIcon(),tixmgr(aklog){
    // set the shell's ui resource file
    //setXMLFile("kredentialsui.rc");

    LOG << "kredentials constructor called " <<kerror;

    doNotify = notify;
    doAklog  = aklog;
    renewWarningTime = DEFAULT_WARNING_INTERVAL;
    secondsToNextRenewal = DEFAULT_RENEWAL_INTERVAL;
    renewWarningFlag = 0;
    this->setIcon(this->loadIcon("kredentials"));
    menu = new KMenu();

    renewAct = new KAction(KIcon("1rightarrow"), i18n("Renew credentials"), this);
    connect(renewAct, SIGNAL(triggered()), this, SLOT(tryRenewTickets()));
    menu->addAction(renewAct);

    freshTixAct = new KAction(KIcon(""), i18n("&Get new credentials"), this);
    connect(freshTixAct, SIGNAL(triggered()), this, SLOT(tryPassGetTickets()));
    menu->addAction(freshTixAct);
	
    statusAct = new KAction(KIcon(""), i18n("&Credential Status"), this);
    connect(statusAct, SIGNAL(triggered()), this, SLOT(showTicketCache()));
    menu->addAction(statusAct);

    destroyAct = new KAction(KIcon(""), i18n("&Destroy credentials"), this);
    menu->addAction(destroyAct);

    quitAct = new KAction(KIcon(""), i18n("Quit"), this);
    connect(quitAct, SIGNAL(triggered()), kapp, SLOT(quit()));
    menu->addAction(quitAct);
		
    setContextMenu(menu);

    //initKerberos();
    hasCurrentTickets();
	
    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(ticketTimerEvent()));
    timer->start(1000);

    LOG << "Using Kerberos KRB5CCNAME of " << cc.name().c_str();
    LOG << "kredentials constructor returning";

}

kredentials::~kredentials()
{
}



void kredentials::mousePressEvent(QMouseEvent *e)
{
    if(e->button() == Qt::RightButton)
    {
	menu->popup(QCursor::pos());
    }
}

bool kredentials::destroyTickets(){
    bool res=FALSE;
    if(!(res=tixmgr::destroyTickets())){
	KMessageBox::sorry(0, i18n("Unable to destroy your tickets."), 0, 0);
    }else{
	KMessageBox::information(0,i18n("Your tickets have been destroyed."), 
				 0, 0);
    }
    return res;
}

void kredentials::tryPassGetTickets(){
    QString password;
    QString prompt = QString("Please give the password for ");
    std::auto_ptr<krb5::principal> osMe(NULL);
    krb5::principal* pme=cc.getPrincipal();
    if(!pme){
	osMe.reset(osPrincipal());
	pme=osMe.get();
    }
    if(pme){
	prompt.append(QString(pme->getName().c_str()));
    }else{
	prompt.append(QString("unknown user"));
    }
    timer->stop();
    LOG<<"Getting Pass";

    KPasswordDialog *dlg = new KPasswordDialog(NULL, 0);
    dlg->setPrompt(prompt);
    if(!dlg->exec()) {
	KMessageBox::sorry(0, i18n("Never mind..."), 0, 0);
	return;
    }

    while (1) // infinite loop until receives a valid password
    {
	std::string pass(dlg->password().toAscii());
	LOG<<"Getting Creds";
	bool res=passGetCreds(pass);
	LOG<<"Finished Creds";
	if(!res){
	    KMessageBox::sorry(0, i18n("Your password was probably wrong"), 0, 0);

	    break; // I'll try again
	}else{
	    hasCurrentTickets();
	    if( !runAklog() ){
		KMessageBox::sorry(0, i18n("Unable to run aklog"), 0, 0);
	    }
	    timer->start(1000);
	    //that's it
	    break;
	}
    }
    delete dlg;
    return;
}

void kredentials::tryPassGetTicketsScreenSaverSafe(){
	// check if screen saver is active (blanked)
	// if so, don't prompt the password dialog this time
	// and don't try to get new tickets, either the screen
	// saver unlock will do it, either it will pop-up once
	// screen saver will be deactivated

// XXX TEMPORARILY disable dcop stuff while porting to KDE4
#if 0
	DCOPRef screensaver("kdesktop", "KScreensaverIface");
	DCOPReply reply = screensaver.call("isBlanked");

	if (!reply.isValid())
	{
	    LOG<<"There is some error using DCOP to access screensaver status";
	} else if (reply) {
		// screen saver is running
		return;
	}
#endif

	tryPassGetTickets();
}



void kredentials::tryRenewTickets()
{
    time_t now = time(0);
    timer->stop();

    if(!hasCurrentTickets()){
	tryPassGetTicketsScreenSaverSafe();
    }else if(tktRenewableExpirationTime == 0){
	// can not be renewed
	// check if it will expire soon 
	if ( tktExpirationTime - now < renewWarningTime)
		tryPassGetTicketsScreenSaverSafe();
    }
    else if(tktRenewableExpirationTime < now)
    {
	KMessageBox::information(0, "Your tickets have outlived their renewable lifetime and can't be renewed.", 0, 0);
	LOG << "tktRenewableExpirationTime has passed: "
	    << "tktRenewableExpirationTime = " << 
	    tktRenewableExpirationTime << ", now = " << now;
	tryPassGetTicketsScreenSaverSafe();
    }
    else if(!renewTickets())
    {
	LOG << "renewTickets did not get new tickets";


	if(!hasCurrentTickets()){
	    tryPassGetTicketsScreenSaverSafe();
	}
    }
    else
    {
	if(doNotify){
	    KPassivePopup::message(i18n("Kerberos tickets have been renewed"), this);
	}
    }
    // restart the timer here, regardless of whether we currently
    // have tickets now or not.  The user may get tickets before
    // the next timeout, and we need to be able to renew them
    secondsToNextRenewal = DEFAULT_RENEWAL_INTERVAL;
    timer->start(1000);
    if(authenticated > 0){
	if( !runAklog() ){
	    KMessageBox::sorry(0, "Unable to run aklog", 0, 0);
	}
		
	LOG << "WarnTime: " << renewWarningTime << " " << doNotify;
	if(doNotify && 
	   tktRenewableExpirationTime - now < renewWarningTime &&
	   tktRenewableExpirationTime!=0)
	{
	    LOG << "Renew=" << renewWarningFlag;
	    if(renewWarningFlag == 0) {
		renewWarningFlag = 1;
		LOG << "RESET: Renew=" << renewWarningFlag;

		QString msgString = 
		    QString("Kerberos tickets will permanently expire on ")
		    +  QString(ctime(&tktRenewableExpirationTime)) +
		    QString(" You may want to renew them now.");
		KMessageBox::information(0, msgString, 0, 0);
	    }

	}
	else 
	{
	    renewWarningFlag = 0;
	    LOG << "RESET: Renew=" << renewWarningFlag;
	}
    }
    return;
}



void kredentials::ticketTimerEvent()
{

    LOG << "ticketTimerEvent triggered, secondsToNextRenewal == " 
	<< secondsToNextRenewal;

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
	KMessageBox::information(0, 
				 "You do not have any valid tickets.", 
				 "Kerberos", 0, 0);
    }
    else
    {
	const krb5::principal* pme=cc.getPrincipal();
	if(pme){
	    const krb5::principal& me=*pme;
	    if(me.getDataLength()){
		msgString = QString("Your tickets as ")
		    +QString(me.getName().c_str())+QString(" ");
	    }else{
		msgString = QString("Your tickets ");
	    }
	}
	msgString+=QString("are\n Valid until ") + 
	    QString(ctime(&tktExpirationTime));
	if(tktRenewableExpirationTime > time(0))
	{
	    msgString += QString("\nRenewable until ") + 
		QString(ctime(&tktRenewableExpirationTime));
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
