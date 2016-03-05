// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the FRAMEWORKXML_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// FRAMEWORKXML_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#ifdef FRAMEWORKXML_EXPORTS
#define FRAMEWORKXML_API 
#else
#define FRAMEWORKXML_API 
#endif

// We need std::queue implementations
#include <stack>
#include <vector>

namespace FrameWork
{
	namespace xml
	{
		#include "xml_node.h"
		#include "xml_parser.h"
		#include "xml_file.h"
		#include "xml_string.h"
		#include "xml_tree.h"
		#include "xml_tree_2.h"

		namespace utilities
		{
			#include "xml_string_utility.h"
		}
	};
};

namespace FXML = FrameWork::xml;