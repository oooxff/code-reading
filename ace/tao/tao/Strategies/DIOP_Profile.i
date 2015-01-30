// -*- C++ -*-
// DIOP_Profile.i,v 1.1 2001/07/03 23:53:23 fhunleth Exp

ACE_INLINE const TAO_ObjectKey &
TAO_DIOP_Profile::object_key (void) const
{
  return this->object_key_;
}

ACE_INLINE TAO_ObjectKey *
TAO_DIOP_Profile::_key (void) const
{
  TAO_ObjectKey *key = 0;

  ACE_NEW_RETURN (key,
                  TAO_ObjectKey (this->object_key_),
                  0);

  return key;
}




