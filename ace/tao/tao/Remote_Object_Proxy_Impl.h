
//=============================================================================
/**
 *  @file    Remote_Object_Proxy_Impl.h
 *
 *  Remote_Object_Proxy_Impl.h,v 1.5 2001/03/30 16:56:32 parsons Exp
 *
 *  This files contains the definition of the remote proxy
 *  for the CORBA::Object class.
 *
 *
 *  @author  Angelo Corsaro <corsaro@cs.wustl.edu>
 */
//=============================================================================


#ifndef TAO_REMOTE_OBJECT_PROXY_IMPL_H_
#define TAO_REMOTE_OBJECT_PROXY_IMPL_H_

#include "ace/pre.h"
#include "tao/corbafwd.h"

#include "tao/Object_Proxy_Impl.h"

/**
 * @class TAO_Remote_Object_Proxy_Impl
 *
 * @brief TAO_Remote_Object_Proxy_Impl
 *
 * This class implements the remote proxy for the CORBA::Object class.
 */
class TAO_Export TAO_Remote_Object_Proxy_Impl : public virtual TAO_Object_Proxy_Impl
{
public:

  TAO_Remote_Object_Proxy_Impl (void);

  virtual ~TAO_Remote_Object_Proxy_Impl (void);

  virtual CORBA::Boolean _is_a (const CORBA::Object_ptr target,
                                const CORBA::Char *logical_type_id,
                                CORBA_Environment &ACE_TRY_ENV);

#if (TAO_HAS_MINIMUM_CORBA == 0)

  virtual CORBA::Boolean _non_existent (const CORBA::Object_ptr target,
                                        CORBA_Environment &ACE_TRY_ENV);

  virtual CORBA_InterfaceDef_ptr _get_interface (
      const CORBA::Object_ptr target,
      CORBA_Environment &ACE_TRY_ENV
    );

#endif /* TAO_HAS_MINIMUM_CORBA == 0 */

};

#include "ace/post.h"

#endif /* TAO_REMOTE_OBJECT_PROXY_IMPL */
