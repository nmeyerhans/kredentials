# - Try to find kerberos
# Once done this will define
#
#  KRB5_FOUND - system has Kerberos
#  KRB5_INCLUDE_DIR - the Kerberos include directory
#  KRB5_LIBRARIES - Link these to use Kerberos

FIND_PATH(KRB5_INCLUDE_DIR krb5.h)

FIND_LIBRARY(KRB5_LIBRARY krb5)

set(KRB5_LIBRARIES ${KRB5_LIBRARY} )
set(KRB5_INCLUDE_DIRS ${KRB5_INCLUDE_DIR} )

# handle the QUIETLY and REQUIRED arguments and set KRB5_FOUND to TRUE if 
# all listed variables are TRUE
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(Kerberos5 DEFAULT_MSG KRB5_LIBRARIES KRB5_INCLUDE_DIRS)

MARK_AS_ADVANCED(KRB5_INCLUDE_DIR KRB5_LIBRARIES)

