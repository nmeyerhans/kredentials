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

#ifndef _KREDENTIALS_H_
#define _KREDENTIALS_H_

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <ksystemtray.h>
#include <qpopupmenu.h>
#include <qtimer.h>
#include <qevent.h>
#include <qpixmap.h>
#include <kuser.h>
#include <kaction.h>
#include <kdialogbase.h>

#include <time.h>
#include <krb5.h>

/**
 * @short Application Main Window
 * @author Noah Meyerhans <noahm@csail.mit.edu>
 * @version 0.1
 */
class kredentials : public KSystemTray
{
    Q_OBJECT
public:
    /**
     * Default Constructor
     */
    //kredentials();
	kredentials( int doNotify=0, int doAklog=1 );
	
    /**
     * Default Destructor
     */
    virtual ~kredentials();
	void setDoNotify(int);
	void setDoAklog(int);
	
protected slots:
	int renewTickets();
	void tryRenewTickets();
	void hasCurrentTickets();
	void showTicketCache();
	
protected:
	//void paintEvent(QPaintEvent *);
	void mousePressEvent(QMouseEvent *);
	void renewOrGetNewTicekts();
	void timerEvent(QTimerEvent *);

	KDialogBase *noAuthDlg;
	
private:
	int kerror;
	int authenticated;
	time_t tktExpirationTime;
	time_t tktRenewableExpirationTime;
	int secondsToNextRenewal;
	QPopupMenu *menu;
	KUser *kerberosUser;
	KAction *renewAct, *reInitAct, *statusAct;
	QPixmap panelIcon;
	
	krb5_context ctx;
	krb5_ccache cc;
	krb5_principal me;
	char* name;
	void initKerberos();
	
	int doNotify;
	int doAklog;

};

#endif // _KREDENTIALS_H_
