//server.cpp,v 1.13 2001/07/23 02:59:06 irfan Exp

////////////////////////////////////////////////////////////////////////
// This files tests the Client exposed policies:
//     - PriorityModelPolicy
//     - PriorityBandedConnectionPolicy
//     - ClientProtocolPolicy
//
// This policies are embedded in the object reference, by writem
// them into the IOR.
//
//
// -- App. Specific Include --
#include "Policy_Tester.h"

#include "tao/Strategies/advanced_resource.h"

ACE_RCSID(tao, server, "server.cpp,v 1.13 2001/07/23 02:59:06 irfan Exp")


int
main (int argc, char *argv[])
{
  ACE_DECLARE_NEW_CORBA_ENV;

  ACE_TRY
    {
      Policy_Tester policy_tester;

      int result = policy_tester.init (argc, argv, ACE_TRY_ENV);
      ACE_TRY_CHECK;

      if (result != 0)
        return result;

      policy_tester.run (ACE_TRY_ENV);
      ACE_TRY_CHECK;
    }

  ACE_CATCHANY
    {
      ACE_PRINT_EXCEPTION (ACE_ANY_EXCEPTION,
                           ACE_TEXT ("CORBA Exception Raised."));
      return 1;
    }

  ACE_ENDTRY;

  return 0;
}
