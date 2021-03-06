// small_union.h,v 1.1 2000/05/03 23:25:34 coryan Exp

// ============================================================================
//
// = LIBRARY
//    TAO/tests/Param_Test
//
// = FILENAME
//    small_union.h
//
// = DESCRIPTION
//     Tests Anys
//
// = AUTHORS
//      Aniruddha Gokhale
//
// ============================================================================

#ifndef PARAM_TEST_SMALL_UNION_H
#define PARAM_TEST_SMALL_UNION_H

#include "param_testCli.h"

// =*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
//                        test Anys
// =*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
class Test_Small_Union
{
public:
  Test_Small_Union (void);
  // ctor

  ~Test_Small_Union (void);
  // dtor

  int run_sii_test (Param_Test_ptr objref,
                    CORBA::Environment &env);
  // run the SII test

  const char *opname (void) const;
  // return operation name

  int init_parameters (Param_Test_ptr objref,
                       CORBA::Environment &env);
  // set values for parameters

  int reset_parameters (void);
  // reset values for CORBA

  CORBA::Boolean check_validity (void);
  // check if results are valid

  CORBA::Boolean check_validity (CORBA::Request_ptr req);
  // check if results are valid. This is used for DII results

  void print_values (void);
  // print all the values

  void dii_req_invoke (CORBA::Request *, CORBA::Environment &);
  // invoke DII request with appropriate exception handling.

private:
  char *opname_;
  // operation name

  Coffee_var cobj_;
  // Coffee object used to test Small_Union with object.

  static size_t counter;
  // test counter.

  Param_Test::Small_Union in_;
  // in parameter

  Param_Test::Small_Union inout_;
  // inout parameter

  Param_Test::Small_Union_var out_;
  // out parameter

  Param_Test::Small_Union_var ret_;
  // return value
};

#endif /* PARAM_TEST_SMALL_UNION_H */
