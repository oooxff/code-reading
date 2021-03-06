/**
*Copyright (c) 2000-2001, Jim Crafton
*All rights reserved.
*Redistribution and use in source and binary forms, with or without
*modification, are permitted provided that the following conditions
*are met:
*	Redistributions of source code must retain the above copyright
*	notice, this list of conditions and the following disclaimer.
*
*	Redistributions in binary form must reproduce the above copyright
*	notice, this list of conditions and the following disclaimer in 
*	the documentation and/or other materials provided with the distribution.
*
*THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
*AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
*LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
*A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS
*OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
*EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
*PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
*PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
*LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
*NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS 
*SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*
*NB: This software will not save the world.
*/

/* Generated by Together */

#ifndef LISTENER_H
#define LISTENER_H



/*
*
* Below are 5 macros to simplify the job of adding listener
* support to a class. The 5 macros are:
* 
*    LISTENER_LIST     - declares a listener list (a vector)
*    ADD_LISTENER	   - generates a addListener method
*    REMOVE_LISTENER   - generates a removeListener method
*    HAS_LISTENER      - generates a hasListener method
*    NOTIFIER          - generates a notify method
*
*
* Here's an example class that uses the macros:
*
*
*	class Button {
*	private:
*		LISTENER_LIST(ButtonListener);
*
*	public:
*		ADD_LISTENER(ButtonListener);
*		REMOVE_LISTENER(ButtonListener);
*   
*		NOTIFIER(ButtonListener, handlePress, ButtonEvent);
*		NOTIFIER(ButtonListener, handleClick, ButtonEvent);
*	};
*
* The class that is generated looks as follows:
*
*
*	class Adapter {
*	private:
*		std::vector<ButtonListener*> m_ButtonListenerList;
*
*	public:
*		void addListener(ButtonListener *l);
*		void removeListener(ButtonListener *l);
*
*		void notify_handlePress(ButtonEvent *e);
*		void notify_handleClick(ButtonEvent *e);
*	};
*
*
* Notes About Listener Notification
* ---------------------------------
* During listener notication (i.e. within a listener's callback method):
*
* 1. It is legal for the listener to use addListener/removeListener to change the 
* listener list. The notify_* routine handles this properly. 
* 
* 
* 2. It is legal for the listener to *delete* the object that owns the listener
* list. However, in this case, the listener must call the Event.consume()
* method to stop other listeners from being invoked on a dead object.
* 
*/

#include "Event.h"
#include <map>

namespace VCF {

/**
*
*/
class FRAMEWORK_API Listener : public Interface {
public:
	virtual ~Listener(){};
};


/*
#define LINK_LISTENER( method, handler, obj, member, eventMethod, listener, name )\
	VCF::##handler* obj##handler = new VCF::##handler(obj); \
	obj##handler->member = (eventMethod)method; \
	obj->add##listener( obj##handler ); \
	obj->addEventHandler( name, obj##handler ); \
*/

/**
*Generic template class for handling events. Used for deriving specific handler classes
*from a given Listener interface. The template type LISTENER_TYPE specifies the source
*class type is listening for events. The template type EVENT_TYPE represents the event
*class the Listener methods receive as a result of being called. The Handler keeps 
*a array of method pointers that are invoke when a particular Listener method is fired off.
*/
template <class LISTENER_TYPE, class EVENT_TYPE> class FRAMEWORK_API EventHandler : public Object {
public:
	/**
	*This typedef represents a specific member function pointer of the 
	*LISTENER_TYPE class
	*/
	typedef void (LISTENER_TYPE::*OnEventMethodPtr)( EVENT_TYPE* event );
	
	/**
	*The constructor for a Handler takes a source object, of type LISTENER_TYPE
	*and a count of the number of methods that are implemented in the derived 
	*Handler class as a result of implementing a specific Listener interface.
	*@param LISTENER_TYPE source - the source object for the handler. The source 
	*object's method pointers (of type OnEventMethodPtr) will be called.
	*@param long listenerInterfaceMethodCount - the numbner of methods that the 
	*Listener interface contains
	*/
	EventHandler( LISTENER_TYPE* source, const long& listenerInterfaceMethodCount ) {
		m_source = source;
		m_listenerInterfaceMethodCount = listenerInterfaceMethodCount;		
		m_listenerMethods.resize( m_listenerInterfaceMethodCount, NULL );
	}
	
	virtual ~EventHandler(){
		m_listenerMethods.clear();
	};

	/**
	*Generic event handler. To use you must pass in the index of the 
	*method you want to invoke, and the event object for that method. 
	*The index is 0 based and one less than the listenerInterfaceMethodCount
	*parameter passed into the constructor of the Handler. For a Listener
	*Interface with 3 methods, to invoke the first method, you would pass in
	*0 for the methodID.
	*@param long methodID - the index of the method to invoke in the m_listenerMethods
	*array.
	*@param EVENT_TYPE* event the event passed into the Listener method from the 
	*event source, wherever that may be.
	*/
	void handleEvent( const long& methodID, EVENT_TYPE* event ) {
		OnEventMethodPtr method = m_listenerMethods[methodID];
		if ( (NULL != m_source) && (NULL != method) ) {
			(m_source->*method) ( event );
		}
	}

	/**
	*convenience method for assigning method pointers to the Handler
	*/
	OnEventMethodPtr operator [] ( const long index ) {
		return m_listenerMethods[index];
	}	

	void setMethodPtr( OnEventMethodPtr methodPtr, const long& index ) {
		m_listenerMethods[index] = methodPtr;
	}

	long getListenerInterfaceMethodCount() {
		return m_listenerInterfaceMethodCount;
	}
protected:
	/**
	*an array of method pointers whose size is determined by the listenerInterfaceMethodCount
	*passed into the constructor;
	*/
	std::vector<OnEventMethodPtr> m_listenerMethods;

	LISTENER_TYPE* m_source;
	long m_listenerInterfaceMethodCount;
	
};


/**
*Macros for adding event firing and listener list implmentation to a class
*/

#define LISTENER_LIST(ListenerType) \
    std::vector<ListenerType*> m_##ListenerType##List; \
    bool m_##ListenerType##Changed;


#define ADD_LISTENER(ListenerType) \
    void add##ListenerType(ListenerType *__l) { \
		std::vector<ListenerType*>::iterator __p; \
		__p = m_##ListenerType##List.begin(); \
		while (__p != m_##ListenerType##List.end()) { \
            if ( *__p++ == __l ) { \
                return; \
			} \
		} \
        m_##ListenerType##List.push_back(__l); \
		\
	} \

#define REMOVE_LISTENER(ListenerType) \
    void remove##ListenerType(ListenerType *__l) { \
		std::vector<ListenerType*>::iterator __p; \
		__p = m_##ListenerType##List.begin(); \
		for ( ; __p != m_##ListenerType##List.end(); __p++) { \
            if (*__p == __l) { \
				m_##ListenerType##List.erase(__p); \
				m_##ListenerType##Changed = true; \
				break; \
			} \
		} \
	} \

#define HAS_LISTENER(ListenerType) \
    bool has##ListenerType(ListenerType *__l) { \
		std::vector<ListenerType*>::iterator __p; \
		__p = m_##ListenerType##List.begin(); \
		while (__p != m_##ListenerType##List.end()) { \
            if (*__p++ == __l) { \
				return true; /* already there? */ \
			} \
		} \
        return false; \
	} \

#define NOTIFIER(ListenerType, Msg, EventType) \
    void notify_##Msg( EventType *__e) { \
		/* make copy of listener list to be safe */ \
		std::vector<ListenerType*> __listeners = m_##ListenerType##List; \
		std::vector<ListenerType*>::iterator __p; \
		__p = __listeners.begin(); \
		__e->m_consumed = false; \
		bool __changed = m_##ListenerType##Changed; \
		m_##ListenerType##Changed = false; \
		while (__p != __listeners.end()) { \
			if (__e->m_consumed) \
				break; \
			if (m_##ListenerType##Changed) { \
				/* a listener has been removed from list during notification. */\
				/* must re-examine list to see if this listener is still there */\
				std::vector<ListenerType*>::iterator __q; \
				__q = m_##ListenerType##List.begin(); \
				bool __found = false; \
				while (__q != m_##ListenerType##List.end()) { \
					if (*__q++ == *__p) { \
						__found = true; \
						break; \
					} \
				} \
				if (!__found) \
					continue; \
			} \
			(*__p)->Msg(__e); __p++;\
		} \
		m_##ListenerType##Changed = __changed; \
    } \


}; //end of namespace VCF


#endif //LISTENER_H

