// URL_Locator.cpp,v 1.3 1998/07/31 23:36:45 gonzo Exp

#if !defined (ACE_URL_LOCATOR_C)
#define ACE_URL_LOCATOR_C

#include "URL_Locator.h"

#if !defined (__ACE_INLINE__)
#include "URL_Locator.i"
#endif

ACE_RCSID(Caching, URL_Locator, "URL_Locator.cpp,v 1.3 1998/07/31 23:36:45 gonzo Exp")

const char * const
ACE_URL_Locator::opname[] =
// Human readable operation name
{
  "Query",
  "Export",
  "Withdraw",
  "Describe",
  "Modify",
  "Invalid Operation"
};

const char * const
ACE_URL_Locator::selection_name[] =
{
  "None",
  "Some",
  "All",
  "Invalid Selection"
};

const char * const
ACE_URL_Locator::err_name[] =
{
  "No error",
  "Offer already exist",
  "no such offer",
  "invalid argument",
  "function not implemented",
  "unknown error"
};

ACE_URL_Locator::~ACE_URL_Locator (void)
{
}

const char *
ACE_URL_Locator::error_status (void)
{
  return "Not implemented yet.";
}

#endif /* ACE_URL_LOCATOR_C */
