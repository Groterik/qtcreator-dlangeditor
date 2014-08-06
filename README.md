#About
The Qt Creator plugin for basic support of [D programming language](http://dlang.org/).

The plugin provides some features for comfortable code editing and programming using D language such as code completion, indention, syntax highlighting, etc.


#Building
1. clone the project.
1. cd to the project directory
1. run ```qmake && make``` or open dlangeditor.pro in the Qt Creator and build the project from IDE. May be you should set QTC_SOURCE and/or QTC_BUILD variables for qmake for QtCreator's sources and libraries correspondingly. It can be done with the following syntax: ```qmake "QTC_SOURCE=$MY_QTCREATOR_SOURCE_PATH" "QTC_BUILD=$MY_QTCREATOR_LIB_PATH"```
1. if successful, the plugin shared library will appear in local plugin directory:
	* $HOME/.local/share/data/QtProject/qtcreator/plugins/$QTCREATOR_VERSION/cleem/ on Linux
	* %LOCALAPPDATA%\QtProject\qtcreator\plugins\$QTCREATOR_VERSION\cleem\
	* ~/Library/Application Support/QtProject/Qt Creator/plugins/$QTCREATOR_VERSION/cleem/ on Mac

#Build requirements
1. Qt >= 4.8.0
1. Qt Creator >= 3.1.0 (with sources for plugins building, of course)

#Usage requirements
1. [DCD](https://github.com/Hackerpilot/DCD) binaries (server and client).

#Implementation notes
Code completion is supported via DCD (D Completion Daemon by Brian Schott). For each project one instance of DCD server is executed (with a choice of listening port in customizable range). Each instance can include personal import paths from project settings. A completion request from Qt Creator leads to executing DCD client for providing completion proposals.
