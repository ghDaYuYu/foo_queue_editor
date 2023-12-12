Prerequisite: Visual Studio 2015 Community
1. Extract foo_queuecontents folder under <root>\SDK\foobar2000\
2. Install WTL 9.0 under <WTLRoot>\WTL90. Available from http://sourceforge.net/projects/wtl/
3. Have an environmental variable WTL_HOME pointing to <WTLRoot>\WTL90

As the newest SDK of columns_ui-sdk doesn't work with foobar 1.x SDK, I've modified it (according to various forum posts); the modified CUI SDK is also attached in this zip file. Accompaniying diff file is also included.

All dependencies (ATL helpers, component_client, sdk_helpers, sdk, and uie sdk) should be linked statically to the component (Multi-threaded (/MT) does this, multithreaded dll is dynamic so don't use it!). This means user doesn't have to have c++ runtime libraries, they are bundled with component.

You must configure WTL_HOME environmental variable to "Additional Include Directories" of the foobar2000_ATL_helpers project. Here's a step by step guide:
1. Right click the project in solution explorer
2. Click properties
3. Select "All configuration" from the top left
4. From "Configuration Properties  /  C/C++  /  General", type the following to Additional Include Directories:  $(WTL_HOME)\include\
   

Platform toolset should be (for all projects): Visual Studio 2015 (v140(