#pragma once

// This is a utility that allows you to flush the queue on a remote source. This is obviously slightly dangerous
FRAMEWORKCOMMUNICATION3_API void flush_queue( const wchar_t server_name[] );

// Get the current instantenous queue depth on a remote server
FRAMEWORKCOMMUNICATION3_API const DWORD queue_depth( const wchar_t server_name[] );

// This will recover the heart-beat time for a particular server.
// This represents the last ::QueryPeformanceCounter time that the server in
// question was considered alive.
FRAMEWORKCOMMUNICATION3_API const __int64 heart_beat( const wchar_t server_name[] );

// This is a utility that will only send messaegs to a destination if it is currently running.
// This helps avoid queue depth problems in most cases. The allow_messages_on_queue allows some
// finite number of messages to build up on the queue even when the server is not running and
// messages on the queue are flushed FIFO style to keep this number constant. If allow_messages_on_queue 
// is -1, then this message is not added unless the server is running.
// The purpose of allow_messages_on_queue is to allow you to for instance ensure that when the switcher
// starts up with the last frame that you sent and not a blank one, that a single frame is kept around
// but that they are not queued.
FRAMEWORKCOMMUNICATION3_API const bool safe_send_message( const FrameWork::Communication3::implementation::message &msg, const wchar_t server_name[],
														  const int allow_messages_on_queue = -1 );

// This is a function that allows you to wait on increasing number of triggers. 
namespace multiwait
{
	typedef FrameWork::Communication3::implementation::trigger	trigger;

	// Returns whether it was triggered or not
	FRAMEWORKCOMMUNICATION3_API bool all( const DWORD time_out, const trigger& t0 );
	FRAMEWORKCOMMUNICATION3_API bool all( const DWORD time_out, const trigger& t0, const trigger& t1 );
	FRAMEWORKCOMMUNICATION3_API bool all( const DWORD time_out, const trigger& t0, const trigger& t1, const trigger& t2 );
	FRAMEWORKCOMMUNICATION3_API bool all( const DWORD time_out, const trigger& t0, const trigger& t1, const trigger& t2, const trigger& t3 );

	// The return value is the index of the trigger that got hit. -1 means that time_out expired.
	FRAMEWORKCOMMUNICATION3_API int any( const DWORD time_out, const trigger& t0 );
	FRAMEWORKCOMMUNICATION3_API int any( const DWORD time_out, const trigger& t0, const trigger& t1 );
	FRAMEWORKCOMMUNICATION3_API int any( const DWORD time_out, const trigger& t0, const trigger& t1, const trigger& t2 );
	FRAMEWORKCOMMUNICATION3_API int any( const DWORD time_out, const trigger& t0, const trigger& t1, const trigger& t2, const trigger& t3 );
};