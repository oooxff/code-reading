/* -*- C++ -*- */
// WstringDef_i.h,v 1.4 2001/03/30 16:43:25 parsons Exp

// ============================================================================
//
// = LIBRARY
//    TAO/orbsvcs/IFR_Service
//
// = FILENAME
//    WstringDef_i.h
//
// = DESCRIPTION
//    WstringDef servant class.
//
// = AUTHOR
//    Jeff Parsons <parsons@cs.wustl.edu>
//
// ============================================================================

#ifndef TAO_WSTRINGDEF_I_H
#define TAO_WSTRINGDEF_I_H

#include "IDLType_i.h"

#if !defined (ACE_LACKS_PRAGMA_ONCE)
# pragma once
#endif /* ACE_LACKS_PRAGMA_ONCE */

#if defined(_MSC_VER)
#if (_MSC_VER >= 1200)
#pragma warning(push)
#endif /* _MSC_VER >= 1200 */
#pragma warning(disable:4250)
#endif /* _MSC_VER */

class TAO_WstringDef_i : public virtual TAO_IDLType_i
{
  // = TITLE
  //    TAO_WstringDef_i
  //
  // = DESCRIPTION
  //    Represents a bounded wstring (unbounded wstrings
  //    are included in PrimitiveDef).
  //
public:
  TAO_WstringDef_i (TAO_Repository_i *repo,
                    ACE_Configuration_Section_Key section_key);
  // Constructor.

  virtual ~TAO_WstringDef_i (void);
  // Destructor.

  virtual CORBA::DefinitionKind def_kind (
      CORBA::Environment &ACE_TRY_ENV =
        TAO_default_environment ()
    )
    ACE_THROW_SPEC ((CORBA::SystemException));
  // Return our definition kind.

  virtual void destroy (
      CORBA::Environment &ACE_TRY_ENV =
        TAO_default_environment ()
    )
    ACE_THROW_SPEC ((CORBA::SystemException));
  // Remove the repository entry.

  virtual void destroy_i (
      CORBA::Environment &ACE_TRY_ENV =
        TAO_default_environment ()
    )
    ACE_THROW_SPEC ((CORBA::SystemException));

  virtual CORBA::TypeCode_ptr type (
      CORBA::Environment &ACE_TRY_ENV =
        TAO_default_environment ()
    )
    ACE_THROW_SPEC ((CORBA::SystemException));
  // From IDLType_i's virtual function.

  virtual CORBA::TypeCode_ptr type_i (
      CORBA::Environment &ACE_TRY_ENV =
        TAO_default_environment ()
    )
    ACE_THROW_SPEC ((CORBA::SystemException));
  // From IDLType_i's virtual function.

  virtual CORBA::ULong bound (
      CORBA::Environment &ACE_TRY_ENV = 
        TAO_default_environment ()
    )
    ACE_THROW_SPEC ((CORBA::SystemException));

  CORBA::ULong bound_i (
      CORBA::Environment &ACE_TRY_ENV = 
        TAO_default_environment ()
    )
    ACE_THROW_SPEC ((CORBA::SystemException));

  virtual void bound (
      CORBA::ULong bound,
      CORBA::Environment &ACE_TRY_ENV = 
        TAO_default_environment ()
    )
    ACE_THROW_SPEC ((CORBA::SystemException));

  void bound_i (
      CORBA::ULong bound,
      CORBA::Environment &ACE_TRY_ENV = 
        TAO_default_environment ()
    )
    ACE_THROW_SPEC ((CORBA::SystemException));
};

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma warning(pop)
#endif /* _MSC_VER */

#endif /* TAO_WSTRINGDEF_I_H */

