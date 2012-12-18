#include "StdAfx.h"
#include "FrameWork.Communication3.h"

using namespace FC3;
using namespace FC3::implementation;

// This is a utility that allows you to flush the queue on a remote source. This is obviously slightly dangerous
void FC3::utilities::flush_queue( const wchar_t server_name[] )
{	// Get the server pointer
	implementation::server *p_server = implementation::server_cache::get_cache().get_server( server_name );
	if ( !p_server ) return;

	// We first need to lock the queue
	const DWORD write_pos = p_server->lock_write();

	// While the queue is not empty
	DWORD	block_id, addr;
	while( p_server->get_message( 0, block_id, addr ) )
		struct null_message : public message { null_message( const DWORD block_id, const DWORD addr ) : message( block_id, addr ) {} }
			// We just instantiate a message on the stack so that it is freed
			a_message( block_id, addr );

	// Unlock the queue
	p_server->unlock_write( write_pos );

	// Release the server
	p_server->release();
}

// Get the current instantenous queue depth on a remote server
const DWORD FC3::utilities::queue_depth( const wchar_t server_name[] )
{	// Get the server pointer
	implementation::server *p_server = implementation::server_cache::get_cache().get_server( server_name );
	if ( !p_server ) return 0;

	// Get the queue depth
	const DWORD ret = p_server->queue_depth();

	// Release the server
	p_server->release();

	// Return the result
	return ret;
}

// This will recover the heart-beat time for a particular server.
// This represents the last ::QueryPeformanceCounter time that the server in
// question was considered alive.
const __int64 FC3::utilities::heart_beat( const wchar_t server_name[] )
{	// Get the server pointer
	implementation::server *p_server = implementation::server_cache::get_cache().get_server( server_name );
	if ( !p_server ) return 0;

	// Get the queue depth
	const __int64 ret = p_server->heart_beat();

	// Release the server
	p_server->release();

	// Return the result
	return ret;
}

bool FC3::utilities::multiwait::all( const DWORD time_out, const trigger& t0 )
{	// Return whether it timed out
	return t0.wait( time_out );
}

bool FC3::utilities::multiwait::all( const DWORD time_out, const trigger& t0, const trigger& t1 )
{	// Get the objects to wait on
	HANDLE objs[] = { get_event_handle( t0 ), get_event_handle( t1 ) };
	return ::WaitForMultipleObjects( sizeof(objs)/sizeof(objs[0]), objs, TRUE, time_out ) != WAIT_TIMEOUT;
}

bool FC3::utilities::multiwait::all( const DWORD time_out, const trigger& t0, const trigger& t1, const trigger& t2 )
{	// Get the objects to wait on
	HANDLE objs[] = { get_event_handle( t0 ), get_event_handle( t1 ), get_event_handle( t2 ) };
	return ::WaitForMultipleObjects( sizeof(objs)/sizeof(objs[0]), objs, TRUE, time_out ) != WAIT_TIMEOUT;
}

bool FC3::utilities::multiwait::all( const DWORD time_out, const trigger& t0, const trigger& t1, const trigger& t2, const trigger& t3 )
{	// Get the objects to wait on
	HANDLE objs[] = { get_event_handle( t0 ), get_event_handle( t1 ), get_event_handle( t2 ), get_event_handle( t3 ) };
	return ::WaitForMultipleObjects( sizeof(objs)/sizeof(objs[0]), objs, TRUE, time_out ) != WAIT_TIMEOUT;
}

int FC3::utilities::multiwait::any( const DWORD time_out, const trigger& t0 )
{	// Return whether it timed out
	return t0.wait( time_out ) ? 0 : -1;
}

int FC3::utilities::multiwait::any( const DWORD time_out, const trigger& t0, const trigger& t1 )
{	// Get the objects to wait on
	HANDLE objs[] = { get_event_handle( t0 ), get_event_handle( t1 ) };
	switch( ::WaitForMultipleObjects( sizeof(objs)/sizeof(objs[0]), objs, FALSE, time_out ) ) 
	{	case WAIT_OBJECT_0 + 0:		return 0;
		case WAIT_OBJECT_0 + 1:		return 1;
		default:					return -1;
	}
}

int FC3::utilities::multiwait::any( const DWORD time_out, const trigger& t0, const trigger& t1, const trigger& t2 )
{	// Get the objects to wait on
	HANDLE objs[] = { get_event_handle( t0 ), get_event_handle( t1 ), get_event_handle( t2 ) };
	switch( ::WaitForMultipleObjects( sizeof(objs)/sizeof(objs[0]), objs, FALSE, time_out ) ) 
	{	case WAIT_OBJECT_0 + 0:		return 0;
		case WAIT_OBJECT_0 + 1:		return 1;
		case WAIT_OBJECT_0 + 2:		return 2;
		default:					return -1;
	}
}

int FC3::utilities::multiwait::any( const DWORD time_out, const trigger& t0, const trigger& t1, const trigger& t2, const trigger& t3 )
{	// Get the objects to wait on
	HANDLE objs[] = { get_event_handle( t0 ), get_event_handle( t1 ), get_event_handle( t2 ), get_event_handle( t3 ) };
	switch( ::WaitForMultipleObjects( sizeof(objs)/sizeof(objs[0]), objs, FALSE, time_out ) ) 
	{	case WAIT_OBJECT_0 + 0:		return  0;
		case WAIT_OBJECT_0 + 1:		return  1;
		case WAIT_OBJECT_0 + 2:		return  2;
		case WAIT_OBJECT_0 + 3:		return  3;
		default:					return -1;
	}
}


// This is a utility that will only send messaegs to a destination if it is currently running.
// This helps avoid queue depth problems in most cases. 
const bool FC3::utilities::safe_send_message( const FC3::implementation::message &msg, const wchar_t server_name[], const int allow_messages_on_queue )
{	// We first ping the destination
	const __int64 dst_heart_beat = FC3::utilities::heart_beat( server_name );

	// This server is not yet running
	if ( !dst_heart_beat )
	{	// Frames cannot be added to queue
		if ( allow_messages_on_queue < 0 ) return false;

		// Get the server pointer
		implementation::server *p_server = implementation::server_cache::get_cache().get_server( server_name );
		if ( p_server )
		{	// We first need to lock the queue
			const DWORD write_pos = p_server->lock_write();

			// Remove items on queue
			while( (int)p_server->queue_depth() > std::max( 0, allow_messages_on_queue-1 ) )
			{	// While the queue is not empty
				DWORD	block_id, addr;
				if ( !p_server->get_message( 0, block_id, addr ) ) break;

				// We just instantiate a message on the stack so that it is freed
				struct null_message : public message { null_message( const DWORD block_id, const DWORD addr ) : message( block_id, addr ) {} } a_message( block_id, addr );
			}

			// Unlock the queue
			p_server->unlock_write( write_pos );

			// Release the server reference
			p_server->release();
		}
	}

	// Send the message
	return msg.send( server_name );
}