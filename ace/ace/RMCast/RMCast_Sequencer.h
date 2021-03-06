/* -*- C++ -*- */
// RMCast_Sequencer.h,v 1.1 2000/10/11 00:57:08 coryan Exp

// ============================================================================
//
// = LIBRARY
//    ace/RMCast
//
// = AUTHOR
//    Carlos O'Ryan <coryan@uci.edu>
//
// ============================================================================

#ifndef ACE_RMCAST_SEQUENCER_H
#define ACE_RMCAST_SEQUENCER_H
#include "ace/pre.h"

#include "RMCast_Module.h"
#include "ace/Synch.h"

#if !defined (ACE_LACKS_PRAGMA_ONCE)
# pragma once
#endif /* ACE_LACKS_PRAGMA_ONCE */

/// Assign sequence numbers to outgoing messages
/**
 * On the sender side we must assign sequence numbers to the messages
 * <B>before</B> they are put in the retransmission queue.
 */
class ACE_RMCast_Export ACE_RMCast_Sequencer : public ACE_RMCast_Module
{
public:
  /// Constructor
  ACE_RMCast_Sequencer (void);

  /// Destructor
  virtual ~ACE_RMCast_Sequencer (void);

  virtual int data (ACE_RMCast::Data &);

protected:
  /// Create the sequence numbers
  ACE_UINT32 sequence_number_generator_;

  /// Synchronization
  ACE_SYNCH_MUTEX mutex_;
};

#if defined (__ACE_INLINE__)
#include "RMCast_Sequencer.i"
#endif /* __ACE_INLINE__ */

#include "ace/post.h"
#endif /* ACE_RMCAST_SEQUENCER_H */
