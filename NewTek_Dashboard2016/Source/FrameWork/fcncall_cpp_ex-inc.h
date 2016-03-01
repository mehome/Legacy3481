#define def_cpp_fcn_call_param2( fcn_name )					param_##fcn_name
#define def_cpp_fcn_call_param1( fcn_name, no_params )		def_cpp_fcn_call_param2( fcn_name )##no_params

#ifdef	framework_work_ret
#define cpp_fcn_call_param									def_cpp_fcn_call_param1( framework_work_fcn_name_cpp_ret_ex, framework_work_no_params )
#else	framework_work_ret
#define cpp_fcn_call_param									def_cpp_fcn_call_param1( framework_work_fcn_name_cpp_ex, framework_work_no_params )
#endif	framework_work_ret

template< 
#ifdef framework_work_ret
		    typename return_type, 
#endif
			typename base_type,
			typename fcn_type
#if ( framework_work_no_params >= 1 )
		  , typename param1_type
#endif
#if ( framework_work_no_params >= 2 )
		  , typename param2_type
#endif
#if ( framework_work_no_params >= 3 )
		  , typename param3_type
#endif
#if ( framework_work_no_params >= 4 )
          , typename param4_type
#endif
#if ( framework_work_no_params >= 5 )
		  , typename param5_type
#endif
#if ( framework_work_no_params >= 6 )
		  , typename param6_type
#endif
#if ( framework_work_no_params >= 7 )
		  , typename param7_type
#endif
#if ( framework_work_no_params >= 8 )
		  , typename param8_type	
#endif
#if ( framework_work_no_params > 8 )
#pragma error( "Not yet supported, add more parameters to templates." )
#endif
        >
struct cpp_fcn_call_param
{	base_type	*m_p_this;
	fcn_type	m_p_fcn;
#ifdef framework_work_ret
	return_type *m_p_ret;
#endif 
#if ( framework_work_no_params >= 1 )
	param1_type	m_param1;
#endif
#if ( framework_work_no_params >= 2 )
	param2_type	m_param2;
#endif
#if ( framework_work_no_params >= 3 )
	param3_type	m_param3;
#endif
#if ( framework_work_no_params >= 4 )
	param4_type	m_param4;
#endif
#if ( framework_work_no_params >= 5 )
	param5_type	m_param5;
#endif
#if ( framework_work_no_params >= 6 )
	param6_type	m_param6;
#endif
#if ( framework_work_no_params >= 7 )
	param7_type	m_param7;
#endif
#if ( framework_work_no_params >= 8 )
	param8_type	m_param8;
#endif
#if ( framework_work_no_params > 8 )
#pragma error( "Not yet supported, add more parameters to templates." )
#endif
	static DWORD WINAPI thread_proc( void* p_val )
	{	wait_item* p_wait_item = (wait_item*)p_val;
		cpp_fcn_call_param* p_this = (cpp_fcn_call_param*)&p_wait_item->m_data[ 0 ];		
#ifdef framework_work_ret
		*p_this->m_p_ret = 
#endif
		((*(p_this->m_p_this)).*(p_this->m_p_fcn))(		// Check this out.
#if ( framework_work_no_params >= 1 )
								  p_this->m_param1
#endif
#if ( framework_work_no_params >= 2 )
								, p_this->m_param2
#endif
#if ( framework_work_no_params >= 3 )
								, p_this->m_param3
#endif
#if ( framework_work_no_params >= 4 )
								, p_this->m_param4
#endif
#if ( framework_work_no_params >= 5 )
								, p_this->m_param5 
#endif
#if ( framework_work_no_params >= 6 )
								, p_this->m_param6
#endif
#if ( framework_work_no_params >= 7 )
								, p_this->m_param7 
#endif
#if ( framework_work_no_params >= 8 )
								, p_this->m_param8 
#endif
#if ( framework_work_no_params > 8 )
#pragma error( "Not yet supported, add more parameters to templates." )
#endif
										);
		return 0;
	}
};
		
template< 
#ifdef framework_work_ret
		    typename return_type, 
#endif
			typename base_type,
			typename fcn_type
#if ( framework_work_no_params >= 1 )
		  , typename param1_type
#endif
#if ( framework_work_no_params >= 2 )
		  , typename param2_type
#endif
#if ( framework_work_no_params >= 3 )
		  , typename param3_type
#endif
#if ( framework_work_no_params >= 4 )
          , typename param4_type
#endif
#if ( framework_work_no_params >= 5 )
		  , typename param5_type
#endif
#if ( framework_work_no_params >= 6 )
		  , typename param6_type
#endif
#if ( framework_work_no_params >= 7 )
		  , typename param7_type
#endif
#if ( framework_work_no_params >= 8 )
		  , typename param8_type	
#endif
#if ( framework_work_no_params > 8 )
#pragma error( "Not yet supported, add more parameters to templates." )
#endif
        > __forceinline
wait 
#ifdef	framework_work_ret
	framework_work_fcn_name_cpp_ret_ex
#else	framework_work_ret
	framework_work_fcn_name_cpp_ex
#endif	framework_work_ret
			  ( const work_flags flags
#ifdef framework_work_use_thread_param
			  , HANDLE framework_work_use_apc
#endif framework_work_use_thread_param
#ifdef framework_work_ret
		      , return_type &ret
#endif
			  , base_type *p_this
			  , const fcn_type fcn
#if ( framework_work_no_params >= 1 )
			  , param1_type param1
#endif
#if ( framework_work_no_params >= 2 )
				, param2_type param2
#endif
#if ( framework_work_no_params >= 3 )
				, param3_type param3
#endif
#if ( framework_work_no_params >= 4 )
				, param4_type param4
#endif
#if ( framework_work_no_params >= 5 )
				, param5_type param5
#endif
#if ( framework_work_no_params >= 6 )
				, param6_type param6
#endif
#if ( framework_work_no_params >= 7 )
				, param7_type param7
#endif
#if ( framework_work_no_params >= 8 )
				, param8_type param8 
#endif
#if ( framework_work_no_params > 8 )
#pragma error( "Not yet supported, add more parameters to templates." )
#endif
			 )
{	
	typedef cpp_fcn_call_param
		< 
#ifdef framework_work_ret
		    typename return_type, 
#endif
			typename base_type,
			typename fcn_type
#if ( framework_work_no_params >= 1 )
		  , typename param1_type
#endif
#if ( framework_work_no_params >= 2 )
		  , typename param2_type
#endif
#if ( framework_work_no_params >= 3 )
		  , typename param3_type
#endif
#if ( framework_work_no_params >= 4 )
          , typename param4_type
#endif
#if ( framework_work_no_params >= 5 )
		  , typename param5_type
#endif
#if ( framework_work_no_params >= 6 )
		  , typename param6_type
#endif
#if ( framework_work_no_params >= 7 )
		  , typename param7_type
#endif
#if ( framework_work_no_params >= 8 )
		  , typename param8_type	
#endif
#if ( framework_work_no_params > 8 )
#pragma error( "Not yet supported, add more parameters to templates." )
#endif
        >	fcn_call_type;

	assert( sizeof(fcn_call_type) <= wait_item::maximum_block_size );
	wait_item	*p_wait_item = new wait_item;	
	fcn_call_type *p_l_fcn = (fcn_call_type*)p_wait_item->m_data;
	p_l_fcn->m_p_this = p_this;
	p_l_fcn->m_p_fcn  = fcn;	
#ifdef framework_work_ret
	p_l_fcn->m_p_ret = &ret;
#endif
#if ( framework_work_no_params >= 1 )
	::memcpy( &p_l_fcn->m_param1, &param1, sizeof( param1 ) );
#endif
#if ( framework_work_no_params >= 2 )
	::memcpy( &p_l_fcn->m_param2, &param2, sizeof( param2 ) );
#endif
#if ( framework_work_no_params >= 3 )
	::memcpy( &p_l_fcn->m_param3, &param3, sizeof( param3 ) );
#endif
#if ( framework_work_no_params >= 4 )
	::memcpy( &p_l_fcn->m_param4, &param4, sizeof( param4 ) );
#endif
#if ( framework_work_no_params >= 5 )
	::memcpy( &p_l_fcn->m_param5, &param5, sizeof( param5 ) );
#endif
#if ( framework_work_no_params >= 6 )
	::memcpy( &p_l_fcn->m_param6, &param6, sizeof( param6 ) );
#endif
#if ( framework_work_no_params >= 7 )
	::memcpy( &p_l_fcn->m_param7, &param7, sizeof( param7 ) );
#endif
#if ( framework_work_no_params >= 8 )
	::memcpy( &p_l_fcn->m_param8, &param8, sizeof( param8 ) );
#endif
#if ( framework_work_no_params > 8 )
#pragma error( "Not yet supported, add more parameters to templates." )
#endif	
	p_wait_item->m_p_thread_proc = fcn_call_type::thread_proc;
	p_wait_item->m_flags = flags;
#ifdef framework_work_use_apc
	p_wait_item->m_run_thread = framework_work_use_apc;
#else framework_work_use_apc
	p_wait_item->m_run_thread = NULL;
#endif framework_work_use_apc
#ifdef framework_work_queue_work
	framework_work_queue_work( p_wait_item );
#else framework_work_queue_work
	p_wait_item->launch();
#endif framework_work_queue_work
	return wait( p_wait_item );
};

#undef	def_cpp_fcn_call_param2
#undef	def_cpp_fcn_call_param1
#undef	cpp_fcn_call_param