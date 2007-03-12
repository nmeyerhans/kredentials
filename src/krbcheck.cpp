/*
 * $Id: main.cpp 79 2006-01-19 18:19:07Z noahm $
 */

#include "kredentials.h"
#include "krb5_wrap.h"
#include <iostream>



/**
 Debuggable main without detaching for testing the krb5_functions.
*/
int main(int , char)
{
    krb5::tixmgr tix(TRUE);
    for(int i=0;i<3;i++){
	tix.initKerberos();
	tix.hasCurrentTickets();
	tix.renewTickets();
	tix.runAklog();
    }
    tix.destroyTickets();
    tix.runAklog();
    tix.hasCurrentTickets();
    tix.renewTickets();
    tix.initKerberos();
    std::cout<<"Supply password: ";
    tix.passGetCreds(tix.readPass());
    for(int i=0;i<3;i++){
	tix.initKerberos();
	tix.hasCurrentTickets();
	tix.renewTickets();
	tix.runAklog();
    }
    return 0;
}

