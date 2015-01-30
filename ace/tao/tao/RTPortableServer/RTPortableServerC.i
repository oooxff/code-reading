// -*- C++ -*-
//
// RTPortableServerC.i,v 1.1 2001/06/12 21:19:19 fhunleth Exp

// ****  Code generated by the The ACE ORB (TAO) IDL Compiler ****
// TAO and the TAO IDL Compiler have been developed by:
//       Center for Distributed Object Computing
//       Washington University
//       St. Louis, MO
//       USA
//       http://www.cs.wustl.edu/~schmidt/doc-center.html
// and
//       Distributed Object Computing Laboratory
//       University of California at Irvine
//       Irvine, CA
//       USA
//       http://doc.ece.uci.edu/
//
// Information about TAO is available at:
//     http://www.cs.wustl.edu/~schmidt/TAO.html


#if !defined (_RTPORTABLESERVER_POA___CI_)
#define _RTPORTABLESERVER_POA___CI_

ACE_INLINE RTPortableServer::POA_ptr
tao_RTPortableServer_POA_duplicate (
    RTPortableServer::POA_ptr p
  )
{
  return RTPortableServer::POA::_duplicate (p);
}

ACE_INLINE void
tao_RTPortableServer_POA_release (
    RTPortableServer::POA_ptr p
  )
{
  CORBA::release (p);
}

ACE_INLINE RTPortableServer::POA_ptr
tao_RTPortableServer_POA_nil (
    void
  )
{
  return RTPortableServer::POA::_nil ();
}

ACE_INLINE RTPortableServer::POA_ptr
tao_RTPortableServer_POA_narrow (
    CORBA::Object *p,
    CORBA::Environment &ACE_TRY_ENV
  )
{
  return RTPortableServer::POA::_narrow (p, ACE_TRY_ENV);
}

ACE_INLINE CORBA::Object *
tao_RTPortableServer_POA_upcast (
    void *src
  )
{
  RTPortableServer::POA **tmp =
    ACE_static_cast (RTPortableServer::POA **, src);
  return *tmp;
}


#endif /* end #if !defined */

