// -*- C++ -*-

#include "QOPPolicy.h"

ACE_RCSID (Security,
           QOPPolicy,
           "QOPPolicy.cpp,v 1.2 2001/07/11 05:59:56 othman Exp")

TAO_QOPPolicy::TAO_QOPPolicy (Security::QOP qop)
  : qop_ (qop)
{
}

TAO_QOPPolicy::~TAO_QOPPolicy (void)
{
}

CORBA::PolicyType
TAO_QOPPolicy::policy_type (CORBA::Environment & /* ACE_TRY_ENV */)
  ACE_THROW_SPEC ((CORBA::SystemException))
{
  return Security::SecQOPPolicy;
}

CORBA::Policy_ptr
TAO_QOPPolicy::copy (CORBA::Environment &ACE_TRY_ENV)
  ACE_THROW_SPEC ((CORBA::SystemException))
{
  TAO_QOPPolicy *policy = 0;
  ACE_NEW_THROW_EX (policy,
                    TAO_QOPPolicy (this->qop_),
                    CORBA::NO_MEMORY (
                      CORBA::SystemException::_tao_minor_code (
                        TAO_DEFAULT_MINOR_CODE,
                        ENOMEM),
                      CORBA::COMPLETED_NO));
  ACE_CHECK_RETURN (CORBA::Policy::_nil ());

  return policy;
}

void
TAO_QOPPolicy::destroy (CORBA::Environment & /* ACE_TRY_ENV */)
    ACE_THROW_SPEC ((CORBA::SystemException))
{
}

Security::QOP
TAO_QOPPolicy::qop (CORBA::Environment & /* ACE_TRY_ENV */)
  ACE_THROW_SPEC ((CORBA::SystemException))
{
  return this->qop_;
}
