#include "StdAfx.h"
#include "FrameWork.Communication3.h"

using namespace FC3::video;

const message* slot::p_get( const wchar_t* p_name, const int slot_no )
{	// Get the slot
	FC3i::message_slot *p_slot = FC3i::slot_cache::get_cache().get_slot( p_name );
	if ( !p_slot ) return NULL;

	// Get the message
	message* p_msg = p_slot->get< message >( slot_no );
	if ( !p_msg ) return NULL;

	// Release it if it was not a correct message
	if ( p_msg->error() )
	{	p_msg->release();
		return NULL;
	}

	// Return the results
	return p_msg;
}

const message* slot::p_replace( const wchar_t* p_name, const int slot_no, message* p_msg )
{	// Get the slot
	FC3i::message_slot *p_slot = FC3i::slot_cache::get_cache().get_slot( p_name );
	if ( !p_slot ) return NULL;

	// Get the message
	message* p_msg_ret = p_slot->replace< message >( slot_no, p_msg );

	// Release it if it was not a correct message
	if ( p_msg_ret->error() )
	{	p_msg_ret->release();
		return NULL;
	}

	// Return the results
	return p_msg_ret;
}

const bool slot::put( const wchar_t* p_name, const int slot_no, message* p_msg )
{	// Get the slot
	FC3i::message_slot *p_slot = FC3i::slot_cache::get_cache().get_slot( p_name );
	if ( !p_slot ) return false;

	// Replace this item on the slot
	return p_slot->put< message >( slot_no, p_msg );
}