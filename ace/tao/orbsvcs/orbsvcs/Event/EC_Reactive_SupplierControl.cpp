// EC_Reactive_SupplierControl.cpp,v 1.10 2001/03/29 00:46:26 coryan Exp

#include "EC_Reactive_SupplierControl.h"
#include "EC_Event_Channel.h"
#include "EC_SupplierAdmin.h"
#include "EC_ProxyConsumer.h"
#include "EC_ProxySupplier.h" // @@ MSVC 6 bug

#include "tao/Messaging.h"
#include "tao/ORB_Core.h"

#include "ace/Reactor.h"

#if ! defined (__ACE_INLINE__)
#include "EC_Reactive_SupplierControl.i"
#endif /* __ACE_INLINE__ */

ACE_RCSID(Event, EC_Reactive_SupplierControl, "EC_Reactive_SupplierControl.cpp,v 1.10 2001/03/29 00:46:26 coryan Exp")

TAO_EC_Reactive_SupplierControl::
     TAO_EC_Reactive_SupplierControl (const ACE_Time_Value &rate,
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

TAO_EC_Reactive_SupplierControl::~TAO_EC_Reactive_SupplierControl (void)
{
}

void
TAO_EC_Reactive_SupplierControl::query_suppliers (
      CORBA::Environment &ACE_TRY_ENV)
{
  TAO_EC_Ping_Supplier worker (this);
  this->event_channel_->supplier_admin ()->for_each (&worker,
                                                     ACE_TRY_ENV);
  ACE_CHECK;
}

void
TAO_EC_Reactive_SupplierControl::handle_timeout (
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
      AE_TRY_Cf�CK;

      /�Query ,e stat9 of the�supplie��...
     thi�>queryj�uppliers (ACE_��Y_ENV)�
     �ACE_TRY��HECK;w�
     this->p�>icy_cuٯent_�>�Mt_poli8�_overr��es (poticies.i��(),
 ��      9n      ��      ��      �      ޞ      R�CORBA:WET_OVEIDE,
�      ��      .f    C ��      �g      $�      �� ACE_T��_ENV);�^    � E_TRY_��ECK;
~�    zia�(CORBAa�ULong 9b= 0;^i��= poli��es->le0�th (��&+i)
 9     {v�    $ �  poli�es[i]-O�estr�ՋACE_TRޢENV)7��    !T� ACE_T��_CHECKHu
   ���� }
  z9}
 ~AA�_CAT�F�Y
   q�
  8 '`// I���e all )IceptjFC=
  ����
  ACE�&NDTRm�5}
�t��
TAO_�d_Rea|��e_Su�{gerCo�t�3l::aF�%ate Xk6�d)
.��if ded (��=HAS_COTA_ME��<ING)��T*TAO_Y�=�CORB<d�SSAG���!= 0
r�long5���= thx�reaco->sc���le_t�� (&t-p�>adabO�_,
@] #    ��Zf    y!�    ���    �
�    v��e0,
][5�    �
    WiM^    ���    ����    Y��this&i��te_,0��'    �1    !��    ��:�    &Ո    C��  th�M�rateM�`�
  i�I7]d ==�ޟ�
  ��s�turn��

ι��E_TRV}^W_EN��6L   {���    �/r�et th6Awoliccn�rent-a+�ect�ާ   C |�~::ObX�`�_var���� =
J�z�    ���->or��U�resoy�.�init��[�refe�#�es (kp�@icyC(*�nt",� ��    ���
    ]N�  � w~�"    u�5    \�c    u^�E_TR]_y�V);��I  �A	qїRY_C(5��;
4\Jr  �t�lZ�>polWI��curr)�� =�K�m�    w�(qA::Ph~c9yC�r�[�z::_n���;w (t	���n �o��'    E+��    tU�  �:�&;B    �)a�    j��E_�M�m��);%'>�   mR��RYEhE�;
:�KH  t/���Ce-M/TAe thϫV�li�:ݺ��t �0ٲ�� set6�$ r)gn�im'�ٌZ�    �E v5e�-�.D:B�� // �B�LOD�]���fs si��tly }��-c�d�T�o �>b�)�lise!��4s���{% T2F�r�e::T�Ƹ ty�A4y ={�~�1000P7��  �2�J�wRBM��	� aWi�q{*G �� <�=�Q��eout�4	
 K��u��hi�1(=ޑic�_�@�_.���O	� (����i  �8:b��s-C�)���y_��3D0]�N�A�  {>wQ�is�;F�r�->{����_p1��m(U3�M��  �ї���  �"�HD6ins� R�AT8�r��_TBRҽ�_PVZ�`F�TY�o ,E n�&��  �]4�f,�LtV�~  v�1Ҕ] �z�لZRY��F���
!]޹��C)�;$[CH�Ag!�  C�7H��̯�r�eCH��^�8x  Q{���5t ���@j�rn_�Iζ�  $*ڟ-�	xb��TR��E�3#nd� ��K�$�v�O�BKNSo�f��Ç���\��re��Pi�.;�v�:���$�P���_EQ���M���_SupplierControl::shutdown (void)
{
  return this->reactor_->remove_handler (&this->adapter_,
                                         ACE_Event_Handler::DONT_CALL);
}

void
TAO_EC_Reactive_SupplierControl::supplier_not_exist (
      TAO_EC_ProxyPushConsumer *proxy,
      CORBA::Environment &ACE_TRY_ENV)
{
  ACE_TRY
    {
      ACE_DEBUG ((LM_DEBUG,
                  "Supplier %x does not exists\n", long(proxy)));
      proxy->disconnect_push_consumer (ACE_TRY_ENV);
      ACE_TRY_CHECK;
    }
  ACE_CATCHANY
    {
      // Ignore all exceptions..
    }
  ACE_ENDTRY;
}

void
TAO_EC_Reactive_SupplierControl::system_exception (
      TAO_EC_ProxyPushConsumer *proxy,
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
      proxy->disconnect_push_consumer (ACE_TRY_ENV);
      ACE_TRY_CHECK;
    }
  ACE_CATCHANY
    {
      // Ignore all exceptions..
    }
  ACE_ENDTRY;
}

// ****************************************************************

TAO_EC_SupplierControl_Adapter::TAO_EC_SupplierControl_Adapter (
      TAO_EC_Reactive_SupplierControl *adaptee)
  :  adaptee_ (adaptee)
{
}

int
TAO_EC_SupplierControl_Adapter::handle_timeout (
      const ACE_Time_Value &tv,
      const void *arg)
{
  this->adaptee_->handle_timeout (tv, arg);
  return 0;
}

// ****************************************************************

void
TAO_EC_Ping_Supplier::work (TAO_EC_ProxyPushConsumer *consumer,
                            CORBA::Environment &ACE_TRY_ENV)
{
  ACE_TRY
    {
      CORBA::Boolean disconnected;
      CORBA::Boolean non_existent =
        consumer->supplier_non_existent (disconnected,
                                         ACE_TRY_ENV);
      ACE_TRY_CHECK;
      if (non_existent && !disconnected)
        {
          this->control_->supplier_not_exist (consumer, ACE_TRY_ENV);
          ACE_TRY_CHECK;
        }
    }
  ACE_CATCH (CORBA::OBJECT_NOT_EXIST, ex)
    {
      this->control_->supplier_not_exist (consumer, ACE_TRY_ENV);
      ACE_TRY_CHECK;
    }
  ACE_CATCH (CORBA::TRANSIENT, transient)
    {
      // This is TAO's minor code for a failed connection, we may
      // want to be more lenient in the future..
      // if (transient.minor () == 0x54410085)
      this->control_->supplier_not_exist (consumer, ACE_TRY_ENV);
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
