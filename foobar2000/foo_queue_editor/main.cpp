#include "stdafx.h"

// Declaration of your component's version information
// Since foobar2000 v1.0 having at least one of these in your DLL is mandatory to let the troubleshooter tell different versions of your component apart.
// Note that you can declare multiple components within one DLL.
DECLARE_COMPONENT_VERSION(
	// component name
	COMPONENT_NAME_H,
	// component version
	FOO_COMPONENT_VERSION,
	// about text, use \n to separate multiple lines
	// If you don't want any about text, set this parameter to NULL.
	COMPONENT_NAME_H ": A " COMPONENT_DESC
	"\n\n"
	"Version: "FOO_COMPONENT_VERSION"\n"
	"Compiled: "__DATE__ "\n"
	"fb2k SDK: "PLUGIN_FB2K_SDK"\n\n"
	"Copyright (c) 2005 Holger Stenger"
	"\n"
	"Copyright (c) 2011-2023 Sami Salonen"
	"\n"
	"Copyright (c) 2023 Da yuyu"
	"\n\n"
	"Contributors:\n\n"
	"Holger Stenger (Original author of foo_locktest)\n"
	"Sami Salonen (Original author of Queue Contents Editor up to v0.5.2)\n"
	"Da yuyu (Queue Editor from v1.0) \n\n"
	"Queue Editor enables the user to edit and view queue contents through an ui element. Both the Default User Interface (DUI) and Columns User Interface (CUI/uie) are supported. As a legacy option, queue contents can also be viewed and edited by a special queue playlist, which is updated automatically with queue contents."
	"\n\n"
	"For more information, please see the Wiki page: " COMPONENT_WIKIPAGEURL
	//"\n\n"
	//"Please use this thread on hydrogenaudio forums to provide feedback, or to report any bugs you might have found: " FORUMURL 
);

// This will prevent users from renaming your component around (important for proper troubleshooter behaviors) or loading multiple instances of it.
VALIDATE_COMPONENT_FILENAME("foo_queue_editor.dll");
