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

#include <kmainwindow.h>
#include <klocale.h>
#include <kdebug.h>

#include "krb_defs.h"
#include <krb5.h>

kredentials::kredentials()
    : KSystemTray()
{
    // set the shell's ui resource file
    //setXMLFile("kredentialsui.rc");

	menu = new QPopupMenu();
	renewAct = new KAction(i18n("&Renew credentials"), "1rightarrow", 0,
                               this, SLOT(renewTickets()), actionCollection(), "renew")

	kerror = 0;
	k5_opts = (k_opts*)malloc(sizeof(struct k_opts));
		k5_opts->action = INIT_PW;

	k5 = (k5_data*)malloc(sizeof(struct k5_data));
	memset(k5, 0, sizeof(*k5));
	err_code = krb5_init_context(&k5->ctx);
	if(err_code)
		kdDebug() << "Kerberos returned " << err_code << endl;
	err_code = krb5_cc_default(k5->ctx, &k5->cc);
	if(err_code)
		kdDebug() << "Kerberos returned " << err_code << endl;
	kdDebug() << "Set k5->cc to " << k5->cc << endl;

}

kredentials::~kredentials()
{
}

void kredentials::mousePressEvent(QMouseEvent *e)
{
	if(event->button() == RightButton)
	{
		menu->popup(QCursor::pos());
	}
}

int kredentials::renewTickets()
{
		int notix = 1;
		krb5_keytab keytab = 0;
		krb5_creds my_creds;
		krb5_error_code code = 0;
		krb5_get_init_creds_opt options;
		//const char *err_mesg;

		krb5_get_init_creds_opt_init(&options);
		memset(&my_creds, 0, sizeof(my_creds));

		err_code = krb5_cc_get_principal(k5->ctx, k5->cc,
				&k5->me);
		if(err_code)
		{
				kdDebug() << "Kerberos returned " << err_code << endl;
				return err_code;
		}
		err_code = krb5_get_renewed_creds(k5->ctx, &my_creds, k5->me, k5->cc,
								k5_opts->service_name);

		if(code)
		{
				//err_mesg = error_message(code);
				//kdDebug() << "Kerberos returned " << code << ": " << err_mesg << endl;
				kdDebug() << "Kerberos returned " << code << endl;
				return code;
		}
	err_code = krb5_cc_initialize(k5->ctx, k5->cc, k5->me);
		if(err_code)
		{
				kdDebug() << "Kerberos returned " << err_code << endl;
				return code;
		}
		err_code = krb5_cc_store_cred(k5->ctx, k5->cc, &my_creds);
		if(err_code)
		{
				kdDebug() << "Kerberos returned " << err_code << endl;
				return code;
		}
		return 0;

}
#include "kredentials.moc"
