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
	framework_work_fcn_name_cpp_ret
#else	framework_work_ret
	framework_work_fcn_name_cpp
#endif	framework_work_ret
				( 
#ifdef framework_work_use_thread_param
				  HANDLE framework_work_use_apc,
#endif framework_work_use_thread_param
#ifdef framework_work_ret
				  return_type &ret, 
#endif framework_work_ret
				  base_type *p_this
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
{	return 
#ifdef	framework_work_ret
	framework_work_fcn_name_cpp_ret_ex
#else	framework_work_ret
	framework_work_fcn_name_cpp_ex
#endif	framework_work_ret
				( 0, 
#ifdef framework_work_use_thread_param
				  framework_work_use_apc,
#endif framework_work_use_thread_param
#ifdef framework_work_ret
				  ret, 
#endif framework_work_ret
				  p_this
				, fcn
#if ( framework_work_no_params >= 1 )
				, param1
#endif
#if ( framework_work_no_params >= 2 )
				, param2
#endif
#if ( framework_work_no_params >= 3 )
				, param3
#endif
#if ( framework_work_no_params >= 4 )
				, param4
#endif
#if ( framework_work_no_params >= 5 )
				, param5
#endif
#if ( framework_work_no_params >= 6 )
				, param6
#endif
#if ( framework_work_no_params >= 7 )
				, param7
#endif
#if ( framework_work_no_params >= 8 )
				, param8 
#endif
#if ( framework_work_no_params > 8 )
#pragma error( "Not yet supported, add more parameters to templates." )
#endif
			 );
}