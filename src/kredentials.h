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

#ifndef _KREDENTIALS_H_
#define _KREDENTIALS_H_

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <ksystemtrayicon.h>
#include <q3popupmenu.h>
#include <qtimer.h>
#include <qevent.h>
#include <qpixmap.h>
//Added by qt3to4:
#include <QMouseEvent>
#include <QTimerEvent>
#include <kuser.h>
#include <kaction.h>

#include <time.h>
#include "krb5_wrap.h"
/**
 * @short Application Main Window
 * @author Noah Meyerhans <noahm@csail.mit.edu>
 * @version 0.9.2
 */
class kredentials : public KSystemTrayIcon,public krb5::tixmgr
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
    //int renewTickets();
	void tryRenewTickets();
    // void hasCurrentTickets();
	void showTicketCache();
	void tryPassGetTickets();
	void tryPassGetTicketsScreenSaverSafe();
	bool destroyTickets();
	
protected:
	//void paintEvent(QPaintEvent *);
	void mousePressEvent(QMouseEvent *);
	void renewOrGetNewTicekts();
	void timerEvent(QTimerEvent *);

private:
	int secondsToNextRenewal;
        int renewWarningFlag;
        int renewWarningTime;

	Q3PopupMenu *menu;
	KUser *kerberosUser;
	KAction *renewAct, *reInitAct, *statusAct, *destroyAct, *freshTixAct;
	QPixmap panelIcon;
	
	int doNotify;

};

#endif // _KREDENTIALS_H_
