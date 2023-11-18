#pragma once

#ifndef WINVER
#define WINVER 0x601
#endif
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x601
#endif
#ifndef _WIN32_WINDOWS
#define _WIN32_WINDOWS 0x601
#endif

#ifndef _WIN32_IE               // Specifies that the minimum required platform is Internet Explorer 6.0.
#define _WIN32_IE 0x0601       // Change this to the appropriate value to target other versions of IE.
#endif
