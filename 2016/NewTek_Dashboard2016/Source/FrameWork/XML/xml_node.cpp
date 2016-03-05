#include "StdAfx.h"
#include "FrameWork.XML.h"

using namespace FrameWork::xml;

node *node::p_create_child( const wchar_t type[], const int no_parameters, const parameter *p_parameters )
{	return NULL;
}

// When a node has finished being created
void node::create_child_end( const wchar_t type[], node *p_child )
{
}

// This item is starting to be created
void node::start( const wchar_t type[], const int no_parameters, const parameter *p_parameters )
{	
}

// Add text to this item
void node::add_text( const wchar_t text[], const int no_chars )
{
}

// This item has finished being created
void node::end( const wchar_t type[] )
{
}