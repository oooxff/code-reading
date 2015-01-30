/* -*- C++ -*- ssl_endpoints.h,v 1.2 2000/09/26 03:53:02 marina Exp */

// ******  Code generated by the The ACE ORB (TAO) IDL Compiler *******
// TAO and the TAO IDL Compiler have been developed by the Center for
// Distributed Object Computing at Washington University, St. Louis.
//
// Information about TAO is available at:
//                 http://www.cs.wustl.edu/~schmidt/TAO.html

#ifndef _TAO_IDL_SSL_ENDPOINTSC_H_
#define _TAO_IDL_SSL_ENDPOINTSC_H_

#include "ace/pre.h"
#include "tao/corbafwd.h"

#if !defined (ACE_LACKS_PRAGMA_ONCE)
# pragma once
#endif /* ACE_LACKS_PRAGMA_ONCE */

#include "orbsvcs/SSLIOPC.h"

#if defined (TAO_EXPORT_MACRO)
#undef TAO_EXPORT_MACRO
#endif
#define TAO_EXPORT_MACRO TAO_SSLIOP_Export

#if defined (TAO_EXPORT_NESTED_CLASSES)
#  if defined (TAO_EXPORT_NESTED_MACRO)
#    undef TAO_EXPORT_NESTED_MACRO
#  endif /* defined (TAO_EXPORT_NESTED_MACRO) */
#  define TAO_EXPORT_NESTED_MACRO TAO_SSLIOP_Export
#endif /* TAO_EXPORT_NESTED_CLASSES */

#if defined(_MSC_VER)
#if (_MSC_VER >= 1200)
#pragma warning(push)
#endif /* _MSC_VER >= 1200 */
#pragma warning(disable:4250)
#endif /* _MSC_VER */


#if !defined (TAO_USE_SEQUENCE_TEMPLATES)

#if !defined (__TAO_UNBOUNDED_SEQUENCE_TAO_SSLENDPOINTSEQUENCE_CH_)
#define __TAO_UNBOUNDED_SEQUENCE_TAO_SSLENDPOINTSEQUENCE_CH_

  class TAO_EXPORT_NESTED_MACRO _TAO_Unbounded_Sequence_TAO_SSLEndpointSequence : public TAO_Unbounded_Base_Sequence
  {
  public:
    // = Initialization and termination methods.

    _TAO_Unbounded_Sequence_TAO_SSLEndpointSequence (void); // Default constructor.
    _TAO_Unbounded_Sequence_TAO_SSLEndpointSequence (CORBA::ULong maximum);
    _TAO_Unbounded_Sequence_TAO_SSLEndpointSequence (CORBA::ULong maximum,
      CORBA::ULong length,
      SSLIOP::SSL *data,
      CORBA::Boolean release = 0);
    _TAO_Unbounded_Sequence_TAO_SSLEndpointSequence (const _TAO_Unbounded_Sequence_TAO_SSLEndpointSequence &rhs);
    _TAO_Unbounded_Sequence_TAO_SSLEndpointSequence &operator= (const _TAO_Unbounded_Sequence_TAO_SSLEndpointSequence &rhs);
    virtual ~_TAO_Unbounded_Sequence_TAO_SSLEndpointSequence (void); // Dtor.
    // = Accessors.
    SSLIOP::SSL &operator[] (CORBA::ULong i);
    const SSLIOP::SSL &operator[] (CORBA::ULong i) const;
    // = Static operations.
    static SSLIOP::SSL *allocbuf (CORBA::ULong size);
    static void freebuf (SSLIOP::SSL *buffer);
    virtual void _allocate_buffer (CORBA::ULong length);
    virtual void _deallocate_buffer (void);
    // Implement the TAO_Base_Sequence methods (see Sequence.h)

    SSLIOP::SSL *get_buffer (CORBA::Boolean orphan = 0);
    const SSLIOP::SSL *get_buffer (void) const;
    void replace (CORBA::ULong max,
      CORBA::ULong length,
      SSLIOP::SSL *data,
      CORBA::Boolean release);
  };

#endif /* end #if !defined */


#endif /* !TAO_USE_SEQUENCE_TEMPLATES */

#if !defined (_TAO_SSLENDPOINTSEQUENCE_CH_)
#define _TAO_SSLENDPOINTSEQUENCE_CH_

class TAO_SSLEndpointSequence;
class TAO_SSLEndpointSequence_var;

// *************************************************************
// TAO_SSLEndpointSequence
// *************************************************************

class TAO_SSLIOP_Export TAO_SSLEndpointSequence : public
#if !defined (TAO_USE_SEQUENCE_TEMPLATES)
  _TAO_Unbounded_Sequence_TAO_SSLEndpointSequence
#else /* TAO_USE_SEQUENCE_TEMPLATES */
  TAO_Unbounded_Sequence<SSLIOP::SSL>
#endif /* !TAO_USE_SEQUENCE_TEMPLATES */
{
public:
  TAO_SSLEndpointSequence (void); // default ctor
  TAO_SSLEndpointSequence (CORBA::ULong max); // uses max size
  TAO_SSLEndpointSequence (
    CORBA::ULong max,
    CORBA::ULong length,
    SSLIOP::SSL *buffer,
    CORBA::Boolean release=0
  );
  TAO_SSLEndpointSequence (const TAO_SSLEndpointSequence &); // copy ctor
  ~TAO_SSLEndpointSequence (void);
  static void _tao_any_destructor (void*);

#if !defined(__GNUC__) || !defined (ACE_HAS_GNUG_PRE_2_8)
  typedef TAO_SSLEndpointSequence_var _var_type;
#endif /* ! __GNUC__ || g++ >= 2.8 */

};

#endif /* end #if !defined */


#if !defined (_TAO_SSLENDPOINTSEQUENCE___VAR_CH_)
#define _TAO_SSLENDPOINTSEQUENCE___VAR_CH_

// *************************************************************
// class TAO_SSLEndpointSequence_var
// *************************************************************

class TAO_SSLIOP_Export TAO_SSLEndpointSequence_var
{
public:
  TAO_SSLEndpointSequence_var (void); // default constructor
  TAO_SSLEndpointSequence_var (TAO_SSLEndpointSequence *);
  TAO_SSLEndpointSequence_var (const TAO_SSLEndpointSequence_var &); // copy constructor
  TAO_SSLEndpointSequence_var (const TAO_SSLEndpointSequence &); // fixed-size base types only
  ~TAO_SSLEndpointSequence_var (void); // destructor

  TAO_SSLEndpointSequence_var &operator= (TAO_SSLEndpointSequence *);
  TAO_SSLEndpointSequence_var &operator= (const TAO_SSLEndpointSequence_var &);
  TAO_SSLEndpointSequence_var &operator= (const TAO_SSLEndpointSequence &); // fixed-size base types only
  TAO_SSLEndpointSequence *operator-> (void);
  const TAO_SSLEndpointSequence *operator-> (void) const;

  operator const TAO_SSLEndpointSequence &() const;
  operator TAO_SSLEndpointSequence &();
  operator TAO_SSLEndpointSequence &() const;

  SSLIOP::SSL &operator[] (CORBA::ULong index);
  // in, inout, out, _retn
  const TAO_SSLEndpointSequence &in (void) const;
  TAO_SSLEndpointSequence &inout (void);
  TAO_SSLEndpointSequence *&out (void);
  TAO_SSLEndpointSequence *_retn (void);
  TAO_SSLEndpointSequence *ptr (void) const;

private:
  TAO_SSLEndpointSequence *ptr_;
};


#endif /* end #if !defined */

extern TAO_SSLIOP_Export CORBA::TypeCode_ptr  _tc_TAO_SSLEndpointSequence;

#ifndef __ACE_INLINE__


#if !defined _TAO_CDR_OP_TAO_SSLEndpointSequence_H_
#define _TAO_CDR_OP_TAO_SSLEndpointSequence_H_

TAO_SSLIOP_Export CORBA::Boolean operator<< (
    TAO_OutputCDR &,
    const TAO_SSLEndpointSequence &
  );
TAO_SSLIOP_Export CORBA::Boolean operator>> (
    TAO_InputCDR &,
    TAO_SSLEndpointSequence &
  );

#endif /* _TAO_CDR_OP_TAO_SSLEndpointSequence_H_ */


#endif /* __ACE_INLINE__ */


#if defined (__ACE_INLINE__)
#include "ssl_endpoints.i"
#endif /* defined INLINE */

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma warning(pop)
#endif /* _MSC_VER */

#include "ace/post.h"
#endif /* ifndef */
