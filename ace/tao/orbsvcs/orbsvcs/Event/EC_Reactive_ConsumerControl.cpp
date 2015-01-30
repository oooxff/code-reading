// EC_Reactive_ConsumerControl.cpp,v 1.10 2001/03/29 00:46:26 coryan Exp

#include "EC_Reactive_ConsumerControl.h"
#include "EC_Event_Channel.h"
#include "EC_ConsumerAdmin.h"
#include "EC_ProxySupplier.h"
#include "EC_ProxyConsumer.h" // @@ MSVC 6 bug

#include "tao/Messaging.h"
#include "tao/ORB_Core.h"

#include "ace/Reactor.h"

#if ! defined (__ACE_INLINE__)
#include "EC_Reactive_ConsumerControl.i"
#endif /* __ACE_INLINE__ */

ACE_RCSID(Event, EC_Reactive_ConsumerControl, "EC_Reactive_ConsumerControl.cpp,v 1.10 2001/03/29 00:46:26 coryan Exp")

TAO_EC_Reactive_ConsumerControl::
     TAO_EC_Reactive_ConsumerControl (const ACE_Time_Value &rate,
                                      TAO_EC_Event_Channel *ec,
                                      CORBA::ORB_ptr orb)
  : rate_ (rate),
    adapter_ (this),
    event_channel_ (ec),
    orb_ (CORBA::ORB::_duplicate (orb))
{
  this->reactor_ =
    this->orb_->orb_core ()->reactor ();
}

TAO_EC_Reactive_ConsumerControl::~TAO_EC_Reactive_ConsumerControl (void)
{
}

void
TAO_EC_Reactive_ConsumerControl::query_consumers (
      CORBA::Environment &ACE_TRY_ENV)
{
  TAO_EC_Ping_Consumer worker (this);
  this->event_channel_->consumer_admin ()->for_each (&worker,
                                                     ACE_TRY_ENV);
  ACE_CHECK;
}

void
TAO_EC_Reactive_ConsumerControl::handle_timeout (
      const ACE_Time_Value &,
      const void *)
{
  ACE_TRY_NEW_ENV
    {
      // Query the state of the Current object *before* we initiate
      // the iteration...
      CORBA::PolicyTypeSeq types;
      CORBA::PolicyList_var policies =
        this->policy_current_->get_policy_overrides (types,
                                                     ACE_TRY_ENV);
      ACE_TRY_CHECK;

      // Change the timeout
      this->policy_current_->set_policy_overrides (this->policy_list_,
                                                   CORBA::ADD_OVERRIDE,
                                                   ACE_TRY_ENV);
      ACE_TRY_CHECK;

      // Query the state of the consumers...
      this->query_consumers (ACE_TRY_ENV);
      ACE_TRY_CHECK;

      this->policy_current_->set_policy_overrides (policies.in (),
                                                   CORBA::SET_OVERRIDE,
                                                   ACE_TRY_ENV);
      ACE_TRY_CHECK;
      for (CORBA::ULong i = 0; i != policies->length (); ++i)
        {
          policies[i]->destroy (ACE_TRY_ENV);
          ACE_TRY_CHECK;
        }
    }
  ACE_CATCHANY
    {
      // Ignore all exceptions
    }
  ACE_ENDTRY;
}

int
TAO_EC_Reactive_ConsumerControl::activate (void)
{
#if defined (TAO_HAS_CORBA_MESSAGING) && TAO_HAS_CORBA_MESSAGING != 0
  long id = this->reactor_->schedule_timer (&this->adapter_,
                                            0,
                                            this->rate_,
                                            this->rate_);
  if (id == -1)
    return -1;

  ACE_TRY_NEW_ENV
    {
      // Get the PolicyCurrent object
      CORBA::Object_var tmp =
        this->orb_->resolve_initial_references ("PolicyCurrent",
                                                ACE_TRY_ENV);
      ACE_TRY_CHECK;

      this->policy_current_ =
        CORBA::PolicyCurrent::_narrow (tmp.in (),
                                       ACE_TRY_ENV);
      ACE_TRY_CHECK;

      // Pre-compute the policy list to the set the right timeout
      // value...
      // @@ TODO It is currently hard-coded to 10 milliseconds
      TimeBase::TimeT timeout = 10 * 10000;
      CORBA::Any any;
      any <<= timeout;

      this->policy_list_.length (1);
      this->policy_list_[0] =
        this->orb_->create_policy (
               Messaging::RELATIVE_RT_TIMEOUT_POLICY_TYPE,
               any,
               ACE_TRY_ENV);
      ACE_TRY_CHECK;
    }
  ACE_CATCHANY
    {
      return -1;
    }
  ACE_ENDTRY;
#endif /* TAO_HAS_CORBA_MESSAGING */

  return 0;
}

int
TAO_EC_Reactive_ConsumerControl::shutdown (void)
{
  return this->reactor_->remove_handler (&this->adapter_,
                                         ACE_Event_Handler::DONT_CALL);
}

void
TAO_EC_Reactive_ConsumerControl::consumer_not_exist (
      TAO_EC_ProxyPushSupplier *proxy,
      CORBA::Environment &ACE_TRY_ENV)
{
  ACE_TRY
    {
      ACE_DEBUG ((LM_DEBUG,
                  "Consumer %x does not exists\n", long(proxy)));
      proxy->disconnect_push_supplier (ACE_TRY_ENV);
      ACE_TRY_CHECK;
    }
  ACE_CATCHANY
    {
      ACE_PRINT_EXCEPTION (ACE_ANY_EXCEPTION,
                           "Reactive_ConsumerControl::consumer_not_exist");
      // Ignore all exceptions..
    }
  ACE_ENDTRY;
}

void
TAO_EC_Reactive_ConsumerControl::system_exception (
      TAO_EC_ProxyPushSupplier *proxy,
      CORBA::SystemException & /* exception */,
      CORBA::Environment &ACE_TRY_ENV)
{
  ACE_TRY
    {
      // The current implementation is very strict, and kicks out a
      // client on the first system exception. We may
      // want to be more lenient in the future, for example,
      // this is TAO's minor code for a failed connection.
      //
      // if (CORBA::TRANSIENT::_narrow (&exception) != 0
      //     && exception->minor () == 0x54410085)
      //   return;

      // Anything else is serious, including timeouts...
      proxy->disconnect_push_supplier (ACE_TRY_ENV);
      ACE_TRY_CHECK;
    }
  ACE_CATCHANY
    {
      // Ignore all exceptions..
    }
  ACE_ENDTRY;
}

// ****************************************************************

TAO_EC_ConsumerControl_Adapter::TAO_EC_ConsumerControl_Adapter (
      TAO_EC_Reactive_ConsumerControl *adaptee)
  :  adaptee_ (adaptee)
{
}

int
TAO_EC_ConsumerControl_Adapter::handle_timeout (
      const ACE_Time_Value &tv,
      const void *arg)
{
  this->adaptee_->handle_timeout (tv, arg);
  return 0;
}

// ****************************************************************

void
TAO_EC_Ping_Consumer::work (TAO_EC_ProxyPushSupplier *supplier,
                            CORBA::Environment &ACE_TRY_ENV)
{
  ACE_TRY
    {
      CORBA::Boolean disconnected;
      CORBA::Boolean non_existent =
        supplier->consumer_non_existent (disconnected,
                                         ACE_TRY_ENV);
      ACE_TRY_CHECK;
      if (non_existent && !disconnected)
        {
          this->control_->consumer_not_exist (supplier, ACE_TRY_ENV);
          ACE_TRY_CHECK;
        }
    }
  ACE_CATCH (CORBA::OBJECT_NOT_EXIST, ex)
    {
      this->control_->consumer_not_exist (supplier, ACE_TRY_ENV);
      ACE_TRY_CHECK;
    }
  ACE_CATCH (CORBA::TRANSIENT, transient)
    {
      // This is TAO's minor code for a failed connection, we may
      // want to be more lenient in the future..
      // if (transient.minor () == 0x54410085)
      this->control_->consumer_not_exist (supplier, ACE_TRY_ENV);
      ACE_TRY_CHECK;
    }
  ACE_CATCHANY
    {
      // Ignore all exceptions
    }
  ACE_ENDTRY;
}

#if defined (ACE_HAS_EXPLICIT_TEMPLATE_INSTANTIATION)

#elif defined(ACE_HAS_TEMPLATE_INSTANTIATION_PRAGMA)

#endif /* ACE_HAS_EXPLICIT_TEMPLATE_INSTANTIATION */
