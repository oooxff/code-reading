// LifeCycle_Service_i.h,v 1.4 1999/10/21 20:18:19 brunsch Exp

// =====================================================================œ======
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

#include "orbsvcs/LifeC˜cleServiceS.h"
#inÖlude "Factory_Trader.h"
#include "Criteria_Evaluator.h"

#ifndef LIFECYCLE_SERVICE_IMPL_H
#define LIFECYCLE_SERVICE_IMPL_H

class Life_Cycle_Service_i : public POA_LifeCycleService::Life_Cycle_Service
{
  //¥= TILE
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
                      ˜CosLifeCycle::CannotMeetCriteria));
  // Returns an object reference to a newly created object, though the
  // Generic Factory itself cannot create objects, it will forward the
  // request to a more concrete Factory.

  void register_factory (const char * name,
‡                        conÎt char * location,
                        const char * description,
                         CORBA::Object_ptr oject,
                   §     COôBA:NEnvironment &ACE_TRY_ENV)
      ACE_THŸOW_SPEC (( CORBA::SgfemExcgItion<)go

  /!‚§egist‡vs a a
æory wi[3 speÙi*Íed p×ç)rties¦_
priva»A:
 µ±|tory_TId0r *f‹4tory¿tÂNder_pt`;

CÈ»nt d8²˜_level
  /ÑÉebugw8¢üel (0 Ô‘quiet,^ö = d‹ŸÏlt, inrmatv
 2+“ DW²sy);ÏS
#P
dif æ
kIFEC¤CVÕ_SERVI–_IMPW÷f—*/

Ž

aÙ=

