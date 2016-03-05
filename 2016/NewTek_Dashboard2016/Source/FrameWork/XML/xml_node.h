#pragma once

struct FRAMEWORKXML_API node 
	{		// This is called when a child is created. The type is passed, as are the 
			// child parameters. You should return your child type. This can return NULL
			// if this child type is not supported or known.
			struct parameter
			{	wchar_t	*m_p_name;
				wchar_t *m_p_value;
			};

			virtual node *p_create_child( const wchar_t type[], const int no_parameters, const parameter *p_parameters );

			// When a node has finished being created
			virtual void create_child_end( const wchar_t type[], node *p_child );

			// This item is starting to be created
			virtual void start( const wchar_t type[], const int no_parameters, const parameter *p_parameters );

			// Add text to this item
			virtual void add_text( const wchar_t text[], const int no_chars );

			// This item has finished being created
			virtual void end( const wchar_t type[] );
};