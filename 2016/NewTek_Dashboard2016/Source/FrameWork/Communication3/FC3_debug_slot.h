#pragma once

struct FRAMEWORKCOMMUNICATION3_API slot
{	// This will get a message from an existing slot. NULL is returned if the slot
	// does not exist, the slot is empty, or if the slot has a message of a different type.
	static const message* p_get( const wchar_t* p_name, const int slot_no );
	static const message* p_get( const wchar_t* p_name ) { return p_get( p_name, 0 ); }

	// This will put a message on the queue and replace the one that is there, while
	// also return the one that was there. You MUST release the returned message of
	// course.
	static const message* p_replace( const wchar_t* p_name, const int slot_no, message* p_msg );
	static const message* p_replace( const wchar_t* p_name, message* p_msg ) { return p_replace( p_name, 0, p_msg ); }

	// Put a message into a given slot
	static const bool put( const wchar_t* p_name, const int slot_no, message* p_msg );
	static const bool put( const wchar_t* p_name, message* p_msg ) { return put( p_name, 0, p_msg ); }
};