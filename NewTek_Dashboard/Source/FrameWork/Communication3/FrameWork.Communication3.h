#pragma once

// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the FRAMEWORKCOMMUNICATION3_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// FRAMEWORKCOMMUNICATION3_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#ifdef FRAMEWORKCOMMUNICATION3_EXPORTS
#define FRAMEWORKCOMMUNICATION3_API 
#else
#define FRAMEWORKCOMMUNICATION3_API 
#endif

// STL includes
#include <vector>
#include <queue>


// Bitmaps
#include "..\FrameWork.h"
#include "..\XML\FrameWork.XML.h"
#include "..\Audio2\FrameWork.Audio2.h"

// The namespace
namespace FrameWork
{
	namespace Communication3
	{
		namespace config
		{	// There is a FC3.5 version that handles memory very differently. In general this is much more space
			// efficient, and handles general purpose use much better than previous versions. In general this 
			// works better on typical memory usuages than previous versions.
			#define	FC3_VERSION_3_5
			
			// The windows handle names
			static const wchar_t	name_trigger[]             = L"ntk_fc3_tgr_";
			static const wchar_t	name_memory_map[]          = L"ntk_fc3_mmp_";
			static const wchar_t	name_memory_map_event[]    = L"ntk_fc3_met_";
			static const wchar_t	name_message_queue_map[]   = L"ntk_fc3_que_";
			static const wchar_t	name_message_queue_event[] = L"ntk_fc3_evt_";

			// is there a server running
			static const wchar_t	name_remote_server[] = L"ntk_fc3_remote_server";

			// The default block size
			#ifdef	FC3_VERSION_3_5
			// In FC3.5 there is no real down-size to having a larger number of blocks, but the flip
			// side of having more items in the pool is less thread contention, which is advantageous.
			// ADJC : I have increased this because video frames can get so huge so quickly.
			static const DWORD		memory_block_size     = 64 * 1024 * 1024;
			#else	FC3_VERSION_3_5
			static const DWORD		memory_block_size     = 64 * 1024 * 1024;
			static const DWORD		memory_max_alloc_size = 64 * 1024 * 1024;
			#endif	FC3_VERSION_3_5

			// A new implementation of the 

			// This is the length of the message message_queue. This must be a power of two.
			#ifdef	FC3_VERSION_3_5
			// Because we do not free old blocks that are in flight, we need to have shorter
			// queue lengths, which has some inherant scary side-effects.
			static const DWORD		message_queue_length  = 16;
			#else	FC3_VERSION_3_5
			static const DWORD		message_queue_length  = 512;
			#endif	FC3_VERSION_3_5

			// This is how old we allow unreferenced triggers to exist for. This is not necessarily the number
			// of items in the cache itself. It represents the number that have not been accessed since this
			// number have been created. This does serve to keep the number of triggers at a reasonable number
			// while also ensuring that we do keep triggers around forever.
			static const DWORD		trigger_cache_history = 32;
			static const DWORD		server_cache_history  = 32;			

			// We always keep blocks for at least 250ms beyond when messages are send from them.
			#ifndef	FC3_VERSION_3_5
			static const DWORD		memory_cache_history = 4;
			static const DWORD		memory_cache_time	 = 250;
			#endif	FC3_VERSION_3_5

			// The number of times we retry allocations
			// I have bumped this number up because on startup there are a lot of frame allocations at the same time that tend
			// to block with each-other and can potentiall cause it to take slightly longer than one might like.
			static const DWORD		memory_cache_alloc_retries = 16;

			// We implement a very complex inter-process non locking way of recycling memory from free buffers in
			// particular block sizes. This is implemented in such a way that we can often avoid allocating new
			// memory maps with the associated overhead of the OS clearing the entire memory contents. This does
			// impose a slight increase in the required memory size on average, but this is offset by the ability
			// to keep memory maps for much longer and in general I see this have a massive performance improvement.
			// On random allocations this makes the allocator over 10x faster.
			static const bool		memory_block_recycling = true;

			// The data alignment. Probably risky to mess with these. It might be possible to increase them, but
			// decreasing them sounds quite risky.
			static const int		alignment = 16;
			#define					fc3_size_align( a )	(((a)+(FrameWork::Communication3::config::alignment-1))&(~(FrameWork::Communication3::config::alignment-1)))

			// Debugging tools
			static const wchar_t*	debug_category = L"FC3";

			// This can no longer be enabled because it causes recursion in debug output messages which also create objects
			static const bool		debug_object_creation  = true;
			static const bool		debug_receive_creation = true;

			// When running as a server, what port number do we work on
			static const int		remote_port_number = 5950;
		};

		#include "FC3i_predecl.h"

		namespace utilities
		{
			#include "FC3_utilities.h"			
		};

		namespace implementation
		{
			#include "FC3i_predecl.h"

			#include "FC3i_memory_allocator.h"

			#include "FC3i_rwlock.h"
			#include "FC3i_critical_section.h"
			#include "FC3i_thread_name.h"

			#include "FC3i_spinhelp.h"

			#include "FC3i_event.h"

			#include "FC3i_mapped_file.h"

			#include "FC3i_trigger.h"
			#include "FC3i_trigger_event.h"
			#include "FC3i_trigger_cache.h"

			#include "FC3i_message.h"
						
			#include "FC3i_memory_block.h"			
			#include "FC3i_memory_cache.h"			

			#include "FC3i_message_queue.h"
			#include "FC3i_message_event.h"

			#include "FC3i_server.h"
			#include "FC3i_server_cache.h"

			#include "FC3i_receive.h"

			#include "FC3i_memory_pool.h"

			namespace remote
			{
				#include "FC3i_remote.h"
				#include "FC3i_remote_server.h"
				#include "FC3i_remote_client.h"
				#include "FC3i_remote_client_cache.h"
			};
		};

		namespace xml
		{
			#include "FC3_xml_message.h"
			#include "FC3_xml_message_printf.h"
			#include "FC3_xml_receive.h"
			#include "FC3_xml_pull.h"
		};

		namespace audio_video
		{
			#include "FC3_audio_video_receive.h"
		};		

		namespace video
		{
			#include "FC3_video_message.h"
			#include "FC3_video_receive.h"
			#include "FC3_video_pull.h"
		};

		namespace audio
		{
			#include "FC3_audio_message.h"
			#include "FC3_audio_receive.h"
			#include "FC3_audio_pull.h"
		};

		namespace debug
		{
			#include "FC3_debug.h"
			#include "FC3_debug_config.h"
			#include "FC3_debug_message.h"
			#include "FC3_debug_receive.h"
			#include "FC3_debug_pull.h"
		};

		namespace audio_video_xml
		{
			#include "FC3_audio_video_xml_receive.h"
		};

		namespace utilities
		{
			#include "FC3_master_clock.h"
		};

		namespace remote
		{
			#include "FC3_remote.h"
		};

		typedef FrameWork::Communication3::implementation::trigger	trigger;
	};
};

// A useful define
namespace FC3 = FrameWork::Communication3;