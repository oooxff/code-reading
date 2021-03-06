// INS_i.cpp,v 1.4 1999/06/23 16:29:33 vishal Exp

#include "INS_i.h"

// Constructor

INS_i::INS_i (void)
{
  // no-op
}

// Destructor

INS_i::~INS_i (void)
{
}

// Set the ORB pointer.

void
INS_i::orb (CORBA::ORB_ptr o)
{
  this->orb_ = CORBA::ORB::_duplicate (o);
}

char *
INS_i::test_ins (CORBA::Environment &env)
ACE_THROW_SPEC (( CORBA::SystemException ))
{
  ACE_UNUSED_ARG (env);

  ACE_DEBUG ((LM_DEBUG,
              "Inside Operation\n"));
  return CORBA::string_dup ("Success");
}
