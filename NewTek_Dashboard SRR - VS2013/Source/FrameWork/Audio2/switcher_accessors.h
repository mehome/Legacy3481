#pragma once

#define		PSWIT_referenced_property( type, name )													\
							struct s__##name														\
							{	s__##name( void ) : m_p_value( NULL ) {}							\
								operator	   type& ( void )		{ return *m_p_value; }			\
								operator const type& ( void ) const { return *m_p_value; }			\
								void operator= ( const type& from ) { m_p_value = (type*)&from; }	\
		private:				mutable type *m_p_value;											\
							};																		\
		private:			s__##name m__##name;													\
		public:				__forceinline const s__##name& name( void ) const { return m__##name; }	\
							__forceinline	    s__##name& name( void )		  { return m__##name; }

#define		PSWIT_indexed_property( type, name )													\
		private:			mutable std::vector< type > m__##name;									\
		public:				__forceinline type&	name( const int idx )								\
								{	assert( idx >= 0 ) ;											\
									if ( idx >= (int)m__##name.size() ) m__##name.resize( idx + 1 );\
									return m__##name[ idx ];										\
								}																	\
							__forceinline const type&	name( const int idx ) const					\
								{	assert( idx >= 0 ) ;											\
									if ( idx >= (int)m__##name.size() ) m__##name.resize( idx + 1 );\
									return m__##name[ idx ];										\
								}																	\
							__forceinline const int size_##name( void ) const						\
								{	return (int)m__##name.size();									\
								}																	\
							__forceinline const std::vector< type >& list_##name( void ) const		\
								{	return m__##name;												\
								}																	\
							__forceinline std::vector< type >& list_##name( void )					\
								{	return m__##name;												\
								}

#define		PSWIT_array_property( type, name, size )												\
		private:			struct s__##name														\
							{	type m_val[ size ];													\
							}	m__##name;															\
		public:				__forceinline type&	name( const int idx )								\
								{	assert( idx>=0 && idx<(size) );									\
									return m__##name . m_val[ idx ];								\
								}																	\
							__forceinline const type& name( const int idx ) const					\
								{	assert( idx>=0 && idx<(size) );									\
									return m__##name . m_val[ idx ];								\
								}																	\
							__forceinline const s__##name& name( void ) const						\
								{	return m__##name;												\
								}																	\
							__forceinline s__##name& name( void )									\
								{	return m__##name;												\
								}																	\
							__forceinline const int size_##name( void ) const						\
								{	return (int)(size);												\
								}							

#define		PSWIT_array_property_default( type, name, size, default_val )								\
		private:			struct s__##name															\
							{	type m_val[ size ];														\
								s__##name( void ) { for( int i=0; i<size; i++ ) m_val[i]=default_val; }	\
							}	m__##name;																\
		public:				__forceinline type&	name( const int idx )									\
								{	assert( idx>=0 && idx<(size) );										\
									return m__##name . m_val[ idx ];									\
								}																		\
							__forceinline const type& name( const int idx ) const						\
								{	assert( idx>=0 && idx<(size) );										\
									return m__##name . m_val[ idx ];									\
								}																		\
							__forceinline const s__##name& name( void ) const							\
								{	return m__##name;													\
								}																		\
							__forceinline s__##name& name( void )										\
								{	return m__##name;													\
								}																		\
							__forceinline const int size_##name( void ) const							\
								{	return (int)(size);													\
								}

#define		PSWIT_indexed_ptr( type, name )															\
		private:			mutable std::vector< type* > m__##name;									\
		public:				__forceinline type* &name( const int idx )								\
								{	assert( idx >= 0 ) ;											\
									if ( idx >= (int)m__##name.size() ) m__##name.resize( idx + 1, NULL ); \
									return m__##name[ idx ];										\
								}																	\
							__forceinline const type*	name( const int idx ) const					\
								{	assert( idx >= 0 ) ;											\
									if ( idx >= (int)m__##name.size() ) m__##name.resize( idx + 1, NULL ); \
									return m__##name[ idx ];										\
								}																	\
							__forceinline const int size_##name( void ) const						\
								{	return (int)m__##name.size();									\
								}																	\
							__forceinline const std::vector< type* >& list_##name( void ) const		\
								{	return m__##name;												\
								}																	\
							__forceinline std::vector< type* >& list_##name( void )					\
								{	return m__##name;												\
								}

#define		PSWIT_property_default( type, name, default_value )										\
		private:			struct s__##name														\
							{	type m_value;														\
								__forceinline s__##name( void ) : m_value( default_value ) {}		\
							};																		\
							s__##name m__##name;													\
		public:				__forceinline type& name( void )										\
							{	return m__##name.m_value;											\
							}																		\
							__forceinline const type& name( void ) const							\
							{	return m__##name.m_value;											\
							}																		\
							static __forceinline const type default_##name( void )					\
							{	return default_value;												\
							}

#define		PSWIT_property_default_indexed_fixed( type, name, default_value, N )					\
		private:			struct s__##name														\
							{	type m_value;														\
								__forceinline s__##name( void ) : m_value( default_value ) {}		\
							};																		\
							s__##name m__##name[ N ];												\
		public:				__forceinline type& name( const int i )									\
							{	assert( i>=0 && i<(N) ); return m__##name[i].m_value;				\
							}																		\
							__forceinline const type&	name( const int i ) const					\
							{	assert( i>=0 && i<(N) ); return m__##name[i].m_value;				\
							}																		\
							__forceinline const int size_##name( void ) const						\
							{	return (int)(N);													\
							}																		\
							static __forceinline const type default_##name( void )					\
							{	return default_value;												\
							}

#define		PSWIT_property_default_with_changed( type, name, default_value )										\
		private:			struct s__##name																		\
							{	type m_value;																		\
								mutable bool m_changed;																\
								__forceinline s__##name( void ) : m_value( default_value ), m_changed( 1 ) {}		\
								__forceinline operator type ( void ) { return m_value; }							\
								__forceinline operator const type ( void ) const { return m_value; }				\
								__forceinline void operator= ( const type x ) { if ( x != m_value ) { m_value = x; m_changed = true; } } \
								__forceinline void operator= ( const s__##name& x ) { if ( x.m_value != m_value ) { m_value = x.m_value; m_changed = true; } } \
								__forceinline bool changed( const bool reset=1 ) const { const bool ret = m_changed; if ( reset ) m_changed = 0; return ret; } \
							};																						\
							s__##name m__##name;																	\
		public:				__forceinline s__##name& name( void )													\
							{	return m__##name;																	\
							}																						\
							__forceinline const s__##name& name( void ) const										\
							{	return m__##name;																	\
							}																						\
							static __forceinline const type default_##name( void ) 									\
							{	return default_value;																\
							}

#define		PSWIT_property_ptr( type, name )														\
		private:			struct s__##name														\
							{	type* m_p_value;													\
								__forceinline s__##name( void ) : m_p_value( NULL ) {}				\
							};																		\
							s__##name m__##name;													\
		public:				__forceinline type* &name( void )										\
							{	return m__##name.m_p_value;											\
							}																		\
							__forceinline const type*	name( void ) const							\
							{	return m__##name.m_p_value;											\
							}

#define		PSWIT_property_const_ptr( type, name )													\
		private:			typedef const type* const_ptr__##name;									\
							struct s__##name														\
							{	const_ptr__##name m_p_value;										\
								__forceinline s__##name( void ) : m_p_value( NULL ) {}				\
							};																		\
							s__##name m__##name;													\
		public:				__forceinline const_ptr__##name &name( void )							\
							{	return m__##name.m_p_value;											\
							}																		\
							__forceinline const_ptr__##name name( void ) const						\
							{	return m__##name.m_p_value;											\
							}

#define		PSWIT_property( type, name )															\
		private:			type m__##name;															\
		public:				__forceinline type&	name( void )										\
							{	return m__##name;													\
							}																		\
							__forceinline const type&	name( void ) const							\
							{	return m__##name;													\
							}