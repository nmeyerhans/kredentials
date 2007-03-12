#include <krb5.h>

namespace krb5{
    class creds{
    private:
	krb5_creds * creds;
	const krb5_context * ctx;
    public:
	creds(const krb5_context * c):ctx(c){
	    creds=new krb5_creds();
	}
	creds~(){
	    krb5_free_creds(ctx,creds);
	}
	krb5_creds * this(){
	    return creds;
	}
    };

    class principal{
    private:
	krb5_principal * principal;
	krb5_context * ctx;
    public:
	principal(const krb5_context * c):ctx(c){
	    creds=new krb5_principal();
	}
	principal~(){
	    krb5_free_principal(ctx,principal);
	}
	krb5_principal * this(){
	    return principal;
	}
	
    };

    

}
