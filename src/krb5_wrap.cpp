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

#include <KDebug>
#include <krb5_wrap.h>
#include <cstdio>
#include <iostream>
#include <memory>
#include <termios.h>
#include <pwd.h>

#ifndef NDEBUG
  #define LOG kDebug()
#else
//#define LOG kDebugDevNull()
#define LOG kDebug()
#endif

namespace krb5{
    using namespace std;
    


    int base::error()const {return kerror;};



    context::context():base(){
	kerror=krb5_init_context(&_ctx);
	if(kerror){
	    LOG<<"context() Kerberos returned "<<kerror<<endl;
	    return;
	}
		/*
		  throw(std::string("Kerberos returned ")+kerror);
		*/
    }

    context::~context(){
	krb5_free_context(_ctx);
    }
	    
    krb5_context context::operator() () const{
	return _ctx;
    }

    void context::reinit(){
	krb5_free_context(_ctx);
	kerror=krb5_init_context(&_ctx);
	if(kerror){
	    LOG<<"context::reinit() Kerberos returned "<<kerror<<endl;
	    return;
	}
    }


    ccache::ccache(context& _ctx):base(),ctx(_ctx),
				  _principal(NULL),
				  _creds(NULL){
	kerror=krb5_cc_default(ctx(),&_cc);
	if(kerror){
	    LOG<<"ccache() Kerberos returned "<<kerror<<endl;
	    return;
	}
	    /*
	      throw(std::string("Kerberos returned ")+kerror);

	    */
    }

    ccache::~ccache(){
	if(_principal)
	    delete _principal;
	if(_creds)
	    delete _creds;
    }
	    
    krb5_ccache ccache::operator() () const{
	return _cc;
    }

    context& ccache::getCtx() const{
	return ctx;
    }

    ccIter ccache::iterator(){
	return ccIter(*this);
    }

    const std::string& ccache::name(){
	_name=krb5_cc_get_name(ctx(),_cc);
	return _name;
    }

    principal* ccache::getPrincipal(){
	if(_principal==NULL){
	    _principal=new principal(ctx,*this);
	    kerror=_principal->error();
	    if(kerror){
		LOG<<"ccache() Kerberos returned "<<kerror<<endl;
		delete _principal;
		_principal=NULL;
	    }
	}
	return _principal;
    }

    void ccache::setPrincipal(principal& me){
	principal* pme=getPrincipal();
	if(pme){
	    if(krb5_principal_compare(ctx(),(*pme)(),me()))
		return;
	}
	kerror = krb5_cc_initialize(ctx(), _cc, me());
    }

    creds& ccache::renew_creds(){
	if(_creds==NULL)
	    _creds=new creds(ctx);
	_creds->clear();
	principal* pme=getPrincipal();
	if(pme){
	    kerror = krb5_get_renewed_creds(ctx(), (*_creds)(), 
					    (*pme)(), _cc, NULL);
	}
	return *_creds;
    }

    void ccache::store(creds& creds){
	principal* pme=getPrincipal();
	if(pme){
	    kerror = krb5_cc_initialize(ctx(), _cc, (*pme)());
	    if(kerror) return;
	    kerror = krb5_cc_store_cred(ctx(), _cc, creds());
	}
    }
    
    ccache& ccache::operator=(const ccache& o){
	if(_principal){ delete _principal; _principal=NULL ;};
	if(_creds){ delete _creds; _creds=NULL; };
	ctx=o.getCtx();
	kerror=krb5_cc_default(ctx(),&_cc);	
	return *this;
    }

    void ccache::destroy(){
	kerror=krb5_cc_destroy(ctx(),_cc);
	if(kerror) return;
	kerror=krb5_cc_default(ctx(),&_cc);	
    }

    ccIter::ccIter(ccache& _cc):base(),cc(_cc),pcreds(NULL){
	kerror=krb5_cc_start_seq_get(cc.getCtx()(), cc(), &cur);
	if(kerror){
	    LOG<<"ccIter() Kerberos returned "<<kerror<<endl;
	    return;
	}

    }
    
    ccIter::~ccIter(){
	if(pcreds!=NULL){
	    delete pcreds;
	}
    }
    
    void ccIter::next(){
	if(pcreds!=NULL){
	    delete pcreds;
	    pcreds=NULL;
	}
	krb5_creds* tmpCreds=new krb5_creds();
	kerror=krb5_cc_next_cred(cc.getCtx()(),cc(),&cur,tmpCreds);
	if(kerror){
	    LOG<<"ccIter::next() Kerberos returned "<<kerror<<" : "<<this->error()<<endl;
	    delete tmpCreds;
	    return;
	}
	pcreds=new creds(cc.getCtx(),tmpCreds);	
    }
    
    const creds& ccIter::operator*(){
	if(pcreds==NULL)
	    next();
	return *pcreds;
    }
    
    ccIter& ccIter::operator++(int ){
	if(!kerror){
	    next();
	}
	return *this;
    }



    principal::principal(context& _ctx, ccache& _cc):base(),free(TRUE),ctx(_ctx),_principal(NULL){
	kerror=krb5_cc_get_principal(ctx(), _cc(), &_principal);
	if(kerror)
	    return;
	/*
	  throw(std::string("Kerberos returned ")+kerror);
	*/
    };

    principal::principal(context& _ctx, krb5_principal p, bool copy):base(),free(copy),ctx(_ctx),_principal(NULL){
	if(copy)
	    kerror=krb5_copy_principal(ctx(),p,&_principal);
	else
	    _principal=p;
	if(kerror)
	    return;
	/*
	  throw(std::string("Kerberos returned ")+kerror);
	*/
    };

    principal::principal(principal& o):base(),free(TRUE),ctx(o.getCtx()),_principal(NULL){
	if(o()!=NULL)
	    krb5_copy_principal(ctx(),o(),&_principal);
    };

    principal::~principal(){
	if((_principal!=NULL)&&free)
	    krb5_free_principal(ctx(),_principal);
    };

    krb5_principal principal::operator() (){
	return _principal;
    };

    context& principal::getCtx() const{
	return ctx;
    }

    const char* principal::getRealm() const{
#ifdef HEIMDAL
		return krb5_realm_data(_principal)->realm; 
#else
		return _principal->realm.data;
#endif
    }
    const int principal::getDataLength() const{
	if(_principal)
#ifdef HEIMDAL
		return _principal->name.name_string.len;
#else
		return _principal->length;
#endif
	else
	    return 0;
    }
    const char* principal::getData(const int i) const{
	if((i>=0)&&(i < getDataLength() ))
#ifdef HEIMDAL
		return _principal->name.name_string.val[i];
#else
		return (char *)(_principal->data[i].data);
#endif
	else
	    return NULL;
    }
    
    const string principal::getName() const{
	string res;
	if(_principal){
	    int len=getDataLength();
	    for(int i=0;i<len;i++){
		res=res.append(getData(i));
		if(i<(len-1))
		    res.append("/");
	    }
	    res.append("@");
	    res.append(getRealm());
	}
	return string(res);
    }


    creds::creds(context& _ctx):base(),ctx(_ctx),_creds(NULL),server(NULL){
	_creds=new krb5_creds();
	memset(_creds, 0, sizeof(*_creds));
    };

    creds::creds(context& _ctx,krb5_creds* c):base(),ctx(_ctx),_creds(c),server(NULL){
	calcServer();
    };


    creds::creds(const creds& o):base(),ctx(o.getCtx()),_creds(NULL),server(NULL){
	//_creds=new krb5_creds();
	*this=o;
    }

    creds::~creds(){
	if(server!=NULL)
	    delete server;
	if(_creds!=NULL){
	    krb5_free_creds(ctx(),_creds);
	};
    };

    void creds::clear(){
	if(server!=NULL){
	    delete server;
	    server=NULL;
	}
	if(_creds!=NULL){
	    krb5_free_creds(ctx(),_creds);
	    _creds=NULL;
	}
	_creds=new krb5_creds();
	memset(_creds, 0, sizeof(*_creds));

    }

    krb5_creds * creds::operator() (){
	return _creds;
    };

    const krb5_creds * creds::operator() () const{
	return _creds;
    };

    const principal* creds::getServer() const{
	return server;
    }

    void creds::calcServer(){
	if(server!=NULL){
	    delete server;
	    server=NULL;
	}
	if((_creds!=NULL) && (_creds->server!=NULL))
	    server=new principal(ctx,_creds->server,false);
    };

    long creds::getStartTime() const{
	return _creds->times.starttime;
    }
    long creds::getEndTime() const{
	return _creds->times.endtime;
    }
    long creds::getRenewTill() const{
	return _creds->times.renew_till;
    }

    creds& creds::operator= (const creds& o){
	clear();
	if(o()->client!=NULL){
	    if(_creds!=NULL){
		krb5_free_creds(ctx(),_creds);
		_creds=NULL;
	    }
	    kerror=krb5_copy_creds(ctx(),o(),&_creds);
	}
	calcServer();
	return *this;
    };

    context& creds::getCtx() const{
	return ctx;
    }

    tixmgr::tixmgr(bool _doAklog):base(),doAklog(_doAklog),cc(ctx){
	kerror=ctx.error();
	if(!kerror)
	    kerror=cc.error();
    }

    tixmgr::~tixmgr(){
    }

    string tixmgr::readPass(int length){
	char buf[length+1];
	int fd=dup(0);
	FILE* fp=fdopen(fd,"r");
	setvbuf(fp, NULL, _IONBF, 0);
	fd=fileno(fp);
	struct termios tparm;
	struct termios saveparm;
	tcgetattr(fd, &tparm);
	saveparm=tparm;
	tparm.c_lflag &= ~(ECHO|ECHONL);
	tcsetattr(fd, TCSANOW, &tparm);
	char* ret=fgets(&buf[0], length,fp);
	tcsetattr(0, TCSANOW, &saveparm);	
	if(ret==&buf[0]){
	    // remove newline
	    int len=strlen(&buf[0]);
	    if( (len>0) && (buf[len-1]=='\n') )
		buf[len-1]=0;
	    return string(buf);
	}else
	    return string("");
    }

    bool tixmgr::initKerberos(){
	kerror = 0;
	ctx.reinit();
	kDebug();
	if((kerror=ctx.error())){
	    LOG << "Kerberos returned on context reinit" << kerror << endl;
	    return FALSE;
	}
	cc=krb5::ccache(ctx);
	if((kerror=cc.error())){
	    LOG << "Kerberos returned on ccache init" << kerror << endl;
	    return FALSE;
	}
	LOG<< "Set cc to " << cc.name().c_str();
	const krb5::principal* pme=cc.getPrincipal();
	if(pme){
	    const krb5::principal& me=*pme; 
	    if((kerror=cc.error()))
	    {
		LOG << "Kerberos returned on principal retrieval" << kerror << endl;
		return FALSE;
	    }
	    LOG<< "Principal is " << me.getName().c_str() << endl;
	}
	return TRUE;
    }

    bool tixmgr::renewTickets(){
	// Can't renew tickets if we don't have any to renew
	if(!authenticated)
	    return FALSE;

	const krb5::principal* pme=cc.getPrincipal();
	
	if(!pme){
	    LOG << "no principal found"<< endl;
	    kerror=cc.error();
	    return FALSE;
	}
	{const krb5::principal& me=*pme;
	    LOG << "renewing tickets for " << me.getData(0) << "@" 
		<< me.getRealm() << endl;
	    //LOG << "renewing tickets for " << me.getName() << endl;

	krb5::creds my_creds(cc.renew_creds());
	if((kerror=cc.error()))
	{
		LOG << "Kerberos returned " << kerror << 
			" while renewing creds" << endl;
		return FALSE;
	}

	cc.store(my_creds);
	if((kerror=cc.error()))
	{
		LOG << "Kerberos returned " << kerror << 
			" while storing credentials" << endl;
		return FALSE;
	}
	LOG << "Successfully renewed tickets" << endl;

	}
	return TRUE;

    }

    bool tixmgr::hasCurrentTickets(){
	int noTix = 1;

	long now;

	LOG << "Called hasCurrentTickets()" << endl;

	/* if kerberos is not currently happy, try reinitializing.  The user may 
	   have obtained new tickets since we last initialized.
	*/
	if(kerror)
	{
		LOG << "hasCurrentTickets(): kerror = " << kerror << endl;
		LOG << "Trying to reinitialize kerberos..." << endl;
		initKerberos();
		authenticated = 0;
	}

	now = time(0);
	krb5::principal* pme=cc.getPrincipal();
	if((kerror=cc.error())||(!pme))
	{
		LOG << "While retrieving principal name, Kerberos returned " << kerror << endl;
		authenticated = 0;
		return FALSE;
	}
	krb5::principal& me=*pme;
	LOG << "Princ is " << me.getName().c_str() << endl;
	krb5::ccIter iter=cc.iterator();
	if((kerror=iter.error()))
	{
		LOG << "While beginning CC iterations, kerberos returned " << kerror << endl;
		authenticated = 0;
		return FALSE;
	}
	LOG<<"Start iter";
	while (!(iter.error())){
	    const krb5::creds& creds=*iter;
	    const principal* pserver=NULL;
	    if(!iter.error())
		pserver=creds.getServer();
	    else{
		LOG<< "Error: Iterator returned no creds";
		break;
	    }
	    if(pserver){
		const principal& server=*pserver;
		if (noTix && server.getDataLength() == 2 &&
		    string(server.getRealm())==me.getRealm() &&
		    string(server.getData(0))=="krbtgt" &&
		    string(server.getData(1))==me.getRealm() &&
		    creds.getEndTime() > now){
		    noTix = 0;
		    tktExpirationTime = creds.getEndTime();
		    tktRenewableExpirationTime = creds.getRenewTill();
		    break;;
		}}
	    iter++;
	}
	noTix == 0 ? authenticated = 1 : authenticated = 0;
	LOG << "hasCurrentTickets set authenticated=" << authenticated;
	return authenticated;
    };


    bool tixmgr::passGetCreds(const string& pass){
	creds my_creds(ctx);
	auto_ptr<principal> osMe(NULL);
	krb5::principal* pme=cc.getPrincipal();
	if(!pme || cc.error()){
	    initKerberos();
	    osMe.reset(osPrincipal());
	    if(!(pme=osMe.get())){
		LOG<<"Could not get Principal from os";
		kerror=-1;
		return FALSE;
	    };
	}
	krb5::principal& me=*pme;
	char* ppass=(char*)pass.c_str();
	if(pass=="") return false;
	kerror=
	    krb5_get_init_creds_password(ctx(), my_creds(), me(),
					     ppass, 0, 0,
					     0, 
					     0,
					 NULL);
	if(kerror){
	    LOG<<"Error on passGetCreds "<<kerror;
	    return FALSE;
	}
	cc.store(my_creds);
	if((kerror=cc.error())){
	    LOG<<"Error on storing creds in passGetCreds "<<kerror;
	    return FALSE;
	}
	return TRUE;
    };

    bool tixmgr::runAklog(){
	if(!doAklog) return TRUE;
	if(authenticated){
	    LOG << "Calling aklog";
		int ret = system("aklog");
		if(ret==-1)
		    LOG<<"Unable to run aklog";
		if(ret>0)
		    LOG<<"aklog failed with"<<ret;
		if( ret != 0 )
		    return FALSE;
		else
		    return TRUE;
	}else 
	    return FALSE;

    };

    bool tixmgr::runUnlog(){
	if(!doAklog) return TRUE;
	if(authenticated){
	    LOG << "Calling unlog";
		int ret = system("unlog");
		if(ret==-1)
		    LOG<<"Unable to run unlog";
		if(ret>0)
		    LOG<<"unlog failed with"<<ret;
		if( ret != 0 )
		    return FALSE;
		else
		    return TRUE;
	}else 
	    return FALSE;

    };

    principal* tixmgr::osPrincipal(){
	principal* ret=NULL;
	struct passwd *pw;
	if ((pw = getpwuid((int) getuid()))){
	    krb5_principal pOsMe;
	    kerror=krb5_parse_name(ctx(), pw->pw_name, &pOsMe);
	    ret=new principal(ctx,pOsMe,TRUE);
	    krb5_free_principal(ctx(),pOsMe);
	    if(kerror){
		LOG<<"Parsing user name failed";
		delete ret;
		return NULL;
	    }
	    cc.setPrincipal(*ret);
	    return ret;
	}else{
	    LOG<< "Could not get user name";
	    return NULL;
	}
    }

    bool tixmgr::destroyTickets(){
	cc.destroy();
	if(cc.error())
	    return FALSE;
	else
	    return runUnlog();
    }

}
