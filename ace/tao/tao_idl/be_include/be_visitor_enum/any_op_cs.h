/* -*- C++ -*- */
//
// any_op_cs.h,v 1.2 1998/10/20 02:32:23 levine Exp
//

// ============================================================================
//
// = LIBRARY
//    TAO IDL
//
// = FILENAME
//    any_op_cs.h
//
// = DESCRIPTION
//    Concrete visitor for Enums generating code for the Any operators
//
// = AUTHOR
//    Aniruddha Gokhale
//
// ============================================================================

#ifndef _BE_VISITOR_ENUM_ANY_OP_CS_H_
#define _BE_VISITOR_ENUM_ANY_OP_CS_H_

class be_visitor_enum_any_op_cs : public be_visitor_scope
{
  //
  // = TITLE
  //   be_visitor_enum_any_op_cs
  //
  // = DESCRIPTION
  //   This is a concrete visitor for enum that generates the Any operator
  //   implementations
  //

public:
  be_visitor_enum_any_op_cs (be_visitor_context *ctx);
  // constructor

  ~be_visitor_enum_any_op_cs (void);
  // destructor

  virtual int visit_enum (be_enum *node);
  // visit enum
};

#endif /* _BE_VISITOR_ENUM_ANY_OP_CS_H_ */
