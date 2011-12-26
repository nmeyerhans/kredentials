/*
 * Copyright 2006 by Martin Ginkel, MPI-Magdeburg <mginkel@mpi-magdeburg.mpg.de>
 * and Copyright 2004 by the Massachusetts Institute of Technology.
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

#ifndef KRB5_WRAP_H
#define KRB5_WRAP_H
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <krb5.h>
#include <string>
//#undef DEBUG
/**
   Namespace for kerberos wrapper objects
*/
namespace krb5{
    /**
       Baseclass for error handling
     */
    class base{
    protected: 
	/**
	   errorcode of last krb5_xxx operation
	 */
	krb5_error_code kerror;
    public:
	/**
	   initialize error to 0
	 */
	base():kerror(0){};
	/**
	   get the error of last operation
	 */
	krb5_error_code error() const;
    };

    /**
       wrapper for krb5_context
     */
    class context: public virtual base{
    private:
	krb5_context _ctx;

    public:
	context();
	virtual ~context();
	krb5_context operator() () const;
	void reinit();
    };

    class ccache;
    class principal;
    /**
       wrapper for krb5_creds
     */
    class creds: public virtual base{
    private:
	context& ctx;
	krb5_creds * _creds;
	principal * server;
    public:
	creds(context& _ctx);
	// Take given creds and own them!
	creds(context& _ctx,krb5_creds* o);
	// Dont use, bug? in krb5_copy_creds
	creds(const creds& o);

	virtual ~creds();

	const krb5_creds * operator() () const;
	context& getCtx() const;
	const principal* getServer() const;
	long getStartTime() const;
	long getEndTime() const;
	long getRenewTill() const;
	
	void clear();
	krb5_creds * operator() () ;
	// Dont use, bug? in krb5_copy_creds
	creds& operator= (const creds& o);
	void calcServer();
    };

    class principal: public virtual base{
    private:
	bool free;
	context& ctx;
	krb5_principal  _principal;

    public:
	principal(context& _ctx, ccache& _cc);
	principal(principal& o);
	principal(context& _ctx,krb5_principal p, bool copy=false);
	virtual ~principal();
	krb5_principal operator() ();

	context& getCtx() const;
	const char* getRealm() const;
	int getDataLength() const ;
	const char* getData(const int i) const; 
	const std::string getName() const;
    };

    class ccIter: public virtual base{
    private:
	ccache& cc;
	krb5_cc_cursor cur;
	creds * pcreds;
	void next();
    public:
	ccIter(ccache& _cc);
	virtual ~ccIter();
	const creds& operator*();
	ccIter& operator++(int dummy);

    };

    class ccache: public virtual base{
    private:
	krb5_ccache   _cc;
	context&      ctx;
	std::string _name;
	principal*  _principal;
	creds*      _creds;
    public:
	ccache(context& _ctx);
	virtual ~ccache();
	krb5_ccache operator() () const;
	context& getCtx() const;
	ccIter iterator();
	const std::string& name();
	void store(creds& creds);
	void setPrincipal(principal& me);
	principal* getPrincipal();
	creds& renew_creds();
	ccache& operator=(const ccache& o);
	void destroy();
    };

    class tixmgr:public virtual base{
	
    protected:

	bool doAklog;
	int kerror;
	int authenticated;
	time_t tktExpirationTime;
	time_t tktRenewableExpirationTime;
	context ctx;
	ccache cc;


    public:
	tixmgr(bool _doAklog=TRUE);
	virtual ~tixmgr();
	std::string readPass(int length=1024);
	/**
	   return a fresh principal from the user-info of the OS
	 */
	virtual principal* osPrincipal();
 	virtual bool renewTickets();
	virtual bool hasCurrentTickets();
	virtual bool initKerberos();
	virtual bool passGetCreds(const std::string& pass);
	virtual bool runAklog();
	virtual bool runUnlog();
	virtual bool destroyTickets();
    };



}

#endif
