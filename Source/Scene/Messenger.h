/*******************************************
	Messenger.h

	Entity messenger class definitions
********************************************/

#pragma once

#include <map>
using namespace std;

#include "Defines.h"
#include "Entity.h"
#include "Attacks.h"

namespace gen
{

/////////////////////////////////////
//	Public types

// Some basic message types for demonstration purposes
enum EMessageType
{
	Msg_Go,
	Msg_Stop,
	Msg_Act,
	Msg_Attacked,
	Msg_HealthRestored,
	Msg_MagicRestored,
	Msg_Poison,
	Msg_Revive
};

// A message contains a type and the UID that sent it. The message types for this exercise don't
// really require extra data, but it is possible add additional data for new message types.
// E.g.in the entity intro lab from last year, this structure also contained an int to pass damage
// from a monster to the player
struct SMessage
{
	EMessageType type;
	TEntityUID   from;
	SAttack      attack;
	int          itemEffect;
	int order;  // The position in the attack order the current actor is
};


// Messenger class allows the sending and receipt of messages between entities - addressed
// by UID
class CMessenger
{
/////////////////////////////////////
//	Constructors/Destructors
public:
	// Default constructor
	CMessenger() {}

	// No destructor needed

private:
	// Disallow use of copy constructor and assignment operator (private and not defined)
	CMessenger( const CMessenger& );
	CMessenger& operator=( const CMessenger& );


/////////////////////////////////////
//	Public interface
public:

	/////////////////////////////////////
	// Message sending/receiving

	// Send the given message to a particular UID, does not check if the UID exists
	void SendMessage( TEntityUID to, const SMessage& msg );

	// Fetch the next available message for the given UID, returns the message through the given 
	// pointer. Returns false if there are no messages for this UID
	bool FetchMessage( TEntityUID to, SMessage* msg );


/////////////////////////////////////
//	Private interface
private:

	// A multimap has properties similar to a hash map - mapping a key to a value. Here we
	// have the key as an entity UID and the value as a message for that UID. The stored
	// key/value pairs in a multimap are sorted by key, which means all the messages for a
	// particular UID are together. Key look-up is somewhat slower than for a hash map though
	// Define some types to make usage easier
	typedef multimap<TEntityUID, SMessage> TMessages;
	typedef TMessages::iterator TMessageIter;
    typedef pair<TEntityUID, SMessage> UIDMsgPair; // The type stored by the multimap

	TMessages m_Messages;
};


} // namespace gen
