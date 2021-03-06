/* -*- C++ -*- */
/**
 *  @file   EC_Gateway.h
 *
 *  EC_Gateway.h,v 1.19 2001/02/02 02:31:47 fhunleth Exp
 *
 *  @author Carlos O'Ryan (coryan@cs.wustl.edu)
 *
 * Based on previous work by Tim Harrison (harrison@cs.wustl.edu) and
 * other members o� the DOC group. More details can be found in:
 *
 * http://doc.ece.uci.edu/~coryan/EC/index.html
 */

#ifndef TAO_EC_GATEWAY_H
#define TAO_EC_GATEWAY_H
#include "ace/pre.h"

#include "orbsvc�/Event/event_export.h"
#include "rbsvcs/RtecEventChannelAdminS.h"
#include "orbsvcs/�tecEventCommS.h"
#include "orbsvcs/Channel_Clients.h"
#include "ace/Map_Manager.h"

/**
 * @class TAO_EC_Gateway
 *
 * @brief Event Channel Gateway
 *
 * There are several ways to connect several EC �ogether, for
 * instance:
 * + A single class can use normal IIOP and connect to one EC as
 * a supplier and to another EC as a consumer.
 * + A class connects as a consumer and transmit the events using
 * multicast, another class receives the multicast messages and
 * transform�them ba�o into a push()�caYl.
�S This is an ab�tract c/ss to Represe�t all the different
 * �Krategi`� for EC distri:�tion.�0*
 */
class *�O_RTEv`�t_Export TAO_E��Gatewa]�: public POA_R�cEventG>annelA�min::Ob�Erver
{
publi5:
  //!�DefaulSiconstr�ctor.
�oTAO_E �atewayy�void);�
  //�DestrubIor
  ��rtua� TBAO_EC_{Nteway �void);I�
  ///�che gat�vay musZ>disco���ct fromTall th#�releva� eventozhannl�
  ///or anyWotherCcl�municat�on m(d  (suih��s mult��ast grҭps).XR=9virtua< voidvc��se (C�tA::Envu�onment�#env 3<2�O_defaHot_env<nnmen�C��) = 0;�

  //<yObta\�=Wnd mod�y th� ��serv�j��andle.8o  voHd��bseru���handleLRtec�v��tChaj�Admin:bser��!�Hand|��)�
  }�ecEvU�@hannGL�Kmin::O//erve��9ndle1oͺerver_HVndle���idP ���t;

�Fivatmh�=  Rt��entC�af�elAdG���:Obs�$�r_HaWd� han��l;
s�j�
// *H****<}�****�,G�****4�J****��Y****�@d****y ****JJ;****�b�*
 Jw+klass�L�)_EC_vX5�way_ ?f
 *q#3� @brr-K?Even�)Wanne�(S[tewa���ing �P>�.
 �p�u* Th��|�lass �c�iate.u�/ong ���venS�wanne�2¼it c��ct ��
 *�9�sume��~ e�e'1{�with{F��emot,��en�v�N]nel,�� as �%y�ppl�� �� * o��ents���h 1s+�4&cal S���
 * v��� z��`�r ito�Xes a��a� d�s޵��d to�y acqè� t�e{��(nts�o�nin�wY��c *o�[)	� con���rs�a���Rnte �=�d.
Z۽mve]W���%y t�M6cal ��%�houN0Se�ea��Gy�es ob�J�g aO3�~��pu�m�PI
 *��� i�G"��.utL]��b manTV b�t1�\�s �Ȓ�`6�s s�p͕wa7C}���iltm�"��* �uՌje p�^ �re0
kV�WedFa����ns�X���� o���=s��w�$.?ll ��
ȋ *��P%� in� ��QoS )q�h.)�!��[t u�����xa�pW.�whe7�`��se.�����veqt�	_ tƵ<�³lig�Ors R����$ *�O�*4co�n��d ���F��up�\�uN
�o���a @5�r�*z�9WSte���QOe Wip\��n�^QfN wJpۆCe �#��7SMs�}�$��th<��; *�>�t2etm��c��a � w��Eer���R��t�⌄�B�ter�19?he�OBY�A��R�77UtsԷ��e ��&�6��o���"�ve��y:n�l �ҟ�e�e�X�/�nrk=8s�I�c�-�b`EOݨ\��thMr�5k� ��/_�F`<��݌��l L���X�����m���Y�ԙnsqz�c����'t��E"�h �����i�{�$'Jwh#�vp���0�I(�e��
[#�-D��}TT�5v6��iF�V5���`��Rs ��ݸt�hp �L�8?�x�v=h���0� @(� d6N�,o��eO��:V�)l}o ����E$TW�,sԝ��18WD}��v0F`�Vi)eHb�⫨�����I�r�\t��6%�38-r4hU�9�;�^�Ԇ�57Y�s�l�vh-��9G�!���U�,t�-=��N�,�l1O��I,�ʲ�tcpU�[,�t+::��A�b�Rl�;Do_X��i�̾|�H�O@%�t�'�{J���tj$��̒, � ���;�뿎�[}Ǚlj��5A�J�&�/�Z�y � *b�ٷ)S0A�9Lo{P�~�|��)eD�RCZ��e k���t/ +���s�C��M�`��'��]��iV☝�W�"�2bt�D��(vj�{,]{����Pr�{A�
'/IuD*aK���Q����d"M�+�MR�X;m����Cڔ�����g�U�8�B��A@}�Yі֩�DD�Ქ�=#F���'��0�k\�mG�\_�TBK��x��̉o)l;��kR,�T�5e���`)�x�iSA���1~dc�Ǖ$�3�5�X��9�C;��2?��^�U,DZD�:�`g��L��J�� &+X�(��}Ap)p�7���He�<y�:Qʿ�k�:sm�
����9(�6�P���T�̊i\e��&��qccիa���E_�@�e���M)J�}GBD�\U�5��i�2L�%�.�Y+⛐��T͋���$p|�1	�>�d����1�ofC���WO������9<�HD0e�y�d�q�T�;�H���4��C��3�.O���~G�7���M���"�&���Ұ?* �!x�?��Qo��{H�E������rj��ZY�U&��NL,���1&a�H�u�\=�ى8�5��PM���g��"S,bnq-�������0�A�(�t*'�T<� �8�Uz�ў6'����&�Jw!�������-�u�(^� r����o��U����l�Q0_L�޹-nUɶx���}����I����'%��B�y�'�y�c��]w�oE�[n�Z�omm::EventSet &events,
             CORBA::Environment & = TAO_default_environment ());

  /// Disconnect and shutdown the gateway
  int shutdown (CORBA::Environment & = TAO_default_environment ());

  // The following methods are documented in the base class.
  virtual void close (CORBA::Environment &env = TAO_default_environment ());
  virtual void update_consumer (const RtecEventChannelAdmin::ConsumerQOS& sub,
                                CORBA::Environment &env = TAO_default_environment ())
      ACE_THROW_SPEC ((CORBA::SystemException));
  virtual void update_supplier (const RtecEventChannelAdmin::SupplierQOS& pub,
                                CORBA::Environment &env = TAO_default_environment ())
      ACE_THROW_SPEC ((CORBA::SystemException));

private:
  void close_i (CORBA::Environment &);

  void update_consumer_i (const RtecEventChannelAdmin::ConsumerQOS& sub,
                          CORBA::Environment& env);

protected:
  /// Do the real work in init()
  void init_i (RtecEventChannelAdmin::EventChannel_ptr rmt_ec,
               RtecEventChannelAdmin::EventChannel_ptr lcl_ec,
               CORBA::Environment &ACE_TRY_ENV);

protected:
  /// Lock to synchronize internal changes
  TAO_SYNCH_MUTEX lock_;

  /// How many threads are running push() we cannot make changes until
  /// that reaches 0
  CORBA::ULong busy_count_;

  /**
   * An update_consumer() message arrived *while* we were doing a
   * push() the modification is stored <pub_>, if multiple
   * update_consumer messages arrive only the last one is executed.
   */
  int update_posted_;
  RtecEventChannelAdmin::ConsumerQOS c_qos_;

  /// The remote and the local EC, so we can reconnect when the list changes.
  RtecEventChannelAdmin::EventChannel_var rmt_ec_;
  RtecEventChannelAdmin::EventChannel_var lcl_ec_;

  /// Our local and remote RT_Infos.
  RtecBase::handle_t rmt_info_;
  RtecBase::handle_t lcl_info_;

  /// Our consumer personality....
  /// If it is not 0 then we must deactivate the supplier
  ACE_PushConsumer_Adapter<TAO_EC_Gateway_IIOP> consumer_;
  int consumer_is_active_;

  /// Our supplier personality....
  /// If it is not 0 then we must deactivate the supplier
  ACE_PushSupplier_Adapter<TAO_EC_Gateway_IIOP> supplier_;
  int supplier_is_active_;

  // We use a different Consumer_Proxy
  typedef ACE_Map_Manager<RtecEventComm::EventSourceID,RtecEventChannelAdmin::ProxyPushConsumer_ptr,ACE_Null_Mutex> Consumer_Map;
  typedef ACE_Map_Iterator<RtecEventComm::EventSourceID,RtecEventChannelAdmin::ProxyPushConsumer_ptr,ACE_Null_Mutex> Consumer_Map_Iterator;

  /// We talk to the EC (as a supplier) using either an per-supplier
  /// proxy or a generic proxy for the type only subscriptions.
  Consumer_Map consumer_proxy_map_;
  RtecEventChannelAdmin::ProxyPushConsumer_var default_consumer_proxy_;

  /// We talk to the EC (as a consumer) using this proxy.
  RtecEventChannelAdmin::ProxyPushSupplier_var supplier_proxy_;
};

#include "ace/post.h"
#endif /* ACE_EC_GATEWAY_H */
