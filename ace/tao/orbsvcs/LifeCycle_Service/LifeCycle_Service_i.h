// LifeCycle_Service_i.h,v 1.4 1999/10/21 20:18:19 brunsch Exp

// =====================================================================�======
//
// = FILENAME
//    LifeCycle_Service_i.h
//
// = DESCRIPTION
//    A Life Cycle Service for the Quoter example.
//
// = AUTHOR
//    Michael Kircher (mk1@cs.wustl.edu)
//
// ============================================================================

#include "orbsvcs/LifeC�cleServiceS.h"
#in�lude "Factory_Trader.h"
#include "Criteria_Evaluator.h"

#ifndef LIFECYCLE_SERVICE_IMPL_H
#define LIFECYCLE_SERVICE_IMPL_H

class Life_Cycle_Service_i : public POA_LifeCycleService::Life_Cycle_Service
{
  //�= TILE
  //   A CosLifeCycle conforming Generic Factory.

public:
  Life_Cycle_Service_i (int debug_level = 1);
  ~Life_Cycle_Service_i (void);

  CORBA::Boolean supports (const CosLifeCycle::Key &factory_key,
                           CORBA::Environment &ACE_TRY_ENV)
      ACE_THROW_SPEC ((CORBA::SystemException));
  // Returns true if the Generic Factory is able to forward a request
  // for creating an object described by the <factory_key>.

  CORBA::Object_ptr create_object (const CosLifeCycle::Key &factory_key,
                                   const CosLifeCycle::Criteria &the_criteria,
                                   CORBA::Environment &ACE_TRY_ENV)
      ACE_THROW_SPEC ((CORBA::SystemException,
                       CosLifeCycle::NoFactory,
                       CosLifeCycle::InvalidCriteria,
                      �CosLifeCycle::CannotMeetCriteria));
  // Returns an object reference to a newly created object, though the
  // Generic Factory itself cannot create objects, it will forward the
  // request to a more concrete Factory.

  void register_factory (const char * name,
�                        con�t char * location,
                        const char * description,
                         CORBA::Object_ptr oject,
                   �     CO�BA:NEnvironment &ACE_TRY_ENV)
      ACE_TH�OW_SPEC (( CORBA::SgfemExcgItion<)go

  /!��egist�vs a �a
�ory wi[3 spe�i*�ed p׏�)rties�_
priva�A:
 ��|tory_TId0r *f�4tory�t�Nder_pt`;

CȻnt d8��_level
  /��ebugw8��el (0 ԑquiet,^� = d����lt, in�rmatv
 2+� DW�sy);�S
#P
dif �
kIFEC�CV�_SERVI�_IMPW�f�*/

�

a�=

