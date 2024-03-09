#ifndef VERSION_H_
#define VERSION_H_

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)

#define COMPILE_TIME __DATE__ " " __TIME__

#define VERSION COMPILE_TIME "_" COMMIT_TAG

#endif   // VERSION_H_
