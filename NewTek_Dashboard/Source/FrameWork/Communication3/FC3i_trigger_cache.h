#pragma once

struct trigger_cache
{			// Get the global cache
			static trigger_cache& get_cache( void );

protected:	// Constructor
			trigger_cache( void );

			// Destructor
			~trigger_cache( void );

private:	// This will get a event of known id
			bool new_trigger( trigger *p_dst );

			// This will get a new event
			bool ref_trigger( trigger *p_dst, const DWORD event_id );

			// This will clean up the cache of items that are no longer accessed and are outdated.
			void clean( void );
			
			// Read and write locks
			read_write_lock	m_cache_lock;

			// This is the current cache time-stamp
			DWORD m_time_stamp;

			// A global
			static DWORD g_next_event_id;

			// We maintain a last accessed 
			struct block_desc
			{	DWORD m_time_stamp;
				trigger_event* m_p_trigger_event;
				const bool operator== ( const DWORD id ) const { return m_p_trigger_event->trigger_id() == id; }
			};

			typedef std::vector< block_desc >::iterator iterator;
			std::vector< block_desc > m_events;

			// This is who creates trigger items
			friend trigger;
};