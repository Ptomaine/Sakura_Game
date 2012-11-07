#ifndef VERSION_H
#define VERSION_H

namespace AutoVersion{
	
	//Date Version Types
	static const char DATE[] = "21";
	static const char MONTH[] = "05";
	static const char YEAR[] = "2008";
	static const double UBUNTU_VERSION_STYLE = 8.05;
	
	//Software Status
	static const char STATUS[] = "Alpha";
	static const char STATUS_SHORT[] = "a";
	
	//Standard Version Type
	static const long MAJOR = 0;
	static const long MINOR = 7;
	static const long BUILD = 194;
	static const long REVISION = 845;
	
	//Miscellaneous Version Types
	static const long BUILDS_COUNT = 178;
	#define RC_FILEVERSION 0,7,194,845
	#define RC_FILEVERSION_STRING "0, 7, 194, 845\0"
	static const char FULLVERSION_STRING[] = "0.7.194.845";
	
	//These values are to keep track of your versioning state, don't modify them.
	static const long BUILD_HISTORY = 50;
	

}
#endif //VERSION_H
