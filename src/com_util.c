/*******************************************************************************
 *  RProxy: Connector implementation between application and R language
 *  Copyright (C) 1999--2009 Thomas Baier
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; version 2 of the License.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 *  basic COM utility functions
 *
 ******************************************************************************/

#include "SC_system.h"
#if defined(__WINDOWS__)
#include "rproxy.h"
#include "com_util.h"

static void _com_object_finalizer(SEXP sexp)
{
  RCOM_OBJHANDLE handle = com_getHandle(sexp);
  if(handle == RCOM_NULLHANDLE) {
    RPROXY_ERR(printf("_com_object_finalizer() called for non-pointer SEXP or non-object pointer\n"));
  } else {
    RPROXY_TRACE(printf("COM object at %p gets garbage collected\n",
			R_ExternalPtrAddr(sexp)));
#ifdef __cplusplus
    com_getObject(handle)->Release();
#else
    com_getObject(handle)->lpVtbl->Release(com_getObject(handle));
#endif
  }
}


RCOM_OBJHANDLE com_getHandle(SEXP handle)
{
  SEXP cls;
  if (TYPEOF (handle) != EXTPTRSXP) {
    RPROXY_TRACE(printf("com_getHandle(): not an external pointer\n"));
    return NULL;
  }
  cls = getAttrib (handle,R_ClassSymbol);
  if (TYPEOF (cls) != STRSXP) {
    RPROXY_TRACE(printf("com_getHandle(): no class symbol (type %d)\n",
			 TYPEOF (cls)));
    return NULL;
  }
  if (strcmp (CHAR(STRING_ELT(cls,0)),RCOM_CLSNAME) != 0) {
    RPROXY_TRACE(printf("com_getHandle(): wrong class symbol \"%s\"\n",
			 CHAR (STRING_ELT (cls,0))));
    return NULL;
  }
  return R_ExternalPtrAddr (handle);
}

/** get the COM object by handle */
LPDISPATCH com_getObject(RCOM_OBJHANDLE handle)
{
  return (LPDISPATCH) handle;
}

/** add the COM object and return the new handle */
RCOM_OBJHANDLE com_addObject(LPDISPATCH object)
{
  return object;
}

SEXP com_createSEXP(RCOM_OBJHANDLE handle)
{
  SEXP sexp = R_NilValue;
  SEXP strsexp;

  if (handle == RCOM_NULLHANDLE) {
    RPROXY_ERR(printf ("com_createSEXP: error, invalid object handle\n"));
    return R_NilValue;
  }
  sexp = R_MakeExternalPtr(handle,R_NilValue,R_NilValue);
  R_RegisterCFinalizerEx(sexp,_com_object_finalizer,(Rboolean) TRUE);
  RPROXY_TRACE(printf("COM object watcher: finalizer for object at %p registered\n",
		    handle));
  PROTECT(strsexp = mkString(RCOM_CLSNAME));
  setAttrib (sexp, R_ClassSymbol, strsexp);
  UNPROTECT(1);
  return sexp;
}
#else
static int dummy = 0;
#endif
