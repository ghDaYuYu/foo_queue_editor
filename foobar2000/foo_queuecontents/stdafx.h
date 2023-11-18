// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "version.h"

#define FOOBAR2000_HAVE_CFG_VAR_LEGACY
#define COMPONENTCONFIGVERSION 4

// Changelog in config versions
/*
v4:
- unsigned long m_border introduced in ui_element_settings
*/

#define TITLEFORMAT_WIKIPAGEURL "http://wiki.hydrogenaudio.org/index.php?title=Foobar2000:Titleformat_Reference"
#define COMPONENT_WIKIPAGEURL "http://wiki.hydrogenaudio.org/index.php?title=Foobar2000:Components/Queue_Editor_(foo_queue_editor)"
#define FORUMURL "http://www.hydrogenaudio.org/forums/index.php?showtopic=73648"

#include <helpers/foobar2000+atl.h>
#include <memory>

#pragma warning(push, 1)
#pragma warning(disable : 4068)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Weverything"

#include "targetver.h"

#include <helpers/foobar2000+atl.h>
#include "libPPUI/win32_op.h"

#include "resource.h"
#include "guids.h"

#ifndef _DEBUG 

#include <ostream>
#include <cstdio>
#include <ios>

struct debug_nullstream : std::ostream {
	struct nullbuf: std::streambuf {
	int overflow(int c) { return traits_type::not_eof(c); }
	} m_sbuf;
	debug_nullstream() : std::ios(&m_sbuf), std::ostream(&m_sbuf) {}
};

#define DEBUG_PRINT debug_nullstream()
#endif

#ifdef _DEBUG
#define DEBUG_PRINT console::formatter() << COMPONENTNAME ": " 
#endif