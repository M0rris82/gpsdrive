

#ifndef _CONFIG_H
#define _CONFIG_H


#define _USE_MATH_DEFINES
#define _CRT_SECURE_NO_WARNINGS


#include <Winsock2.h>
#include <math.h>
#include <io.h>
#include <stdlib.h>
#include <locale.h>
#include <direct.h>

#include <glib.h>
#include <glib/gstdio.h>
#include "gettext.h"

#define VERSION "0.0.1 (Win32 pre-alpha)"
#define PACKAGE_VERSION "0.0.1"

extern char gpsdr_datadir[];
extern char gpsdr_localedir[];
extern char gpsdr_gettext_package[];

#define PSIZE gpsdr_psize
#define DATADIR gpsdr_datadir
#define LOCALEDIR gpsdr_localedir

#define GETTEXT_PACKAGE gpsdr_gettext_package

typedef int socklen_t;

extern int rint (double x);

#define finite _finite
#define __finite _finite
#define strdup _strdup
#define open _open
#define write _write
#define read _read
#define close _close
#define tzset _tzset
#define unlink _unlink
#define lseek _lseek
#define mkdir(dirname,access) _mkdir(dirname)

// disable: warning C4244: 'function' : conversion from 'x' to 'y', possible loss of data
#pragma warning(disable:4244)
// disable: warning C4267: 'function' : conversion from 'size_t' to 'int', possible loss of data
#pragma warning(disable:4267)
// disable: warning C4305: 'function' : truncation from 'double' to 'gfloat'
#pragma warning(disable:4305)
// disable: warning C4311: 'type cast' : pointer truncation from 'gpointer' to 'long'
#pragma warning(disable:4311)
// disable: warning C4305: 'initializing' : truncation from 'double' to 'float'
#pragma warning(disable:4305)
// disable: warning C4312: 'type cast' : conversion from 'glong' to 'gpointer' of greater size
#pragma warning(disable:4312)
// disable: warning C4090: 'function' : different 'const' qualifiers
#pragma warning(disable:4090)

#endif /* _CONFIG_H */


#ifndef _CONFIG_H
#define _CONFIG_H


#define _USE_MATH_DEFINES
#define _CRT_SECURE_NO_WARNINGS


#include <Winsock2.h>
#include <math.h>
#include <io.h>
#include <stdlib.h>
#include <locale.h>
#include <direct.h>

#include <glib.h>
#include <glib/gstdio.h>
#include "gettext.h"

#define VERSION "0.0.1 (Win32 pre-alpha)"
#define PACKAGE_VERSION "0.0.1"

extern char gpsdr_datadir[];
extern char gpsdr_localedir[];
extern char gpsdr_gettext_package[];

#define PSIZE gpsdr_psize
#define DATADIR gpsdr_datadir
#define LOCALEDIR gpsdr_localedir

#define GETTEXT_PACKAGE gpsdr_gettext_package

typedef int socklen_t;

extern int rint (double x);

#define finite _finite
#define __finite _finite
#define strdup _strdup
#define open _open
#define write _write
#define read _read
#define close _close
#define tzset _tzset
#define unlink _unlink
#define lseek _lseek
#define mkdir(dirname,access) _mkdir(dirname)

// disable: warning C4244: 'function' : conversion from 'x' to 'y', possible loss of data
#pragma warning(disable:4244)
// disable: warning C4267: 'function' : conversion from 'size_t' to 'int', possible loss of data
#pragma warning(disable:4267)
// disable: warning C4305: 'function' : truncation from 'double' to 'gfloat'
#pragma warning(disable:4305)
// disable: warning C4311: 'type cast' : pointer truncation from 'gpointer' to 'long'
#pragma warning(disable:4311)
// disable: warning C4305: 'initializing' : truncation from 'double' to 'float'
#pragma warning(disable:4305)
// disable: warning C4312: 'type cast' : conversion from 'glong' to 'gpointer' of greater size
#pragma warning(disable:4312)
// disable: warning C4090: 'function' : different 'const' qualifiers
#pragma warning(disable:4090)

#endif /* _CONFIG_H */


#ifndef _CONFIG_H
#define _CONFIG_H


#define _USE_MATH_DEFINES
#define _CRT_SECURE_NO_WARNINGS


#include <Winsock2.h>
#include <math.h>
#include <io.h>
#include <stdlib.h>
#include <locale.h>
#include <direct.h>

#include <glib.h>
#include <glib/gstdio.h>
#include "gettext.h"

#define VERSION "0.0.1 (Win32 pre-alpha)"
#define PACKAGE_VERSION "0.0.1"

extern char gpsdr_datadir[];
extern char gpsdr_localedir[];
extern char gpsdr_gettext_package[];

#define PSIZE gpsdr_psize
#define DATADIR gpsdr_datadir
#define LOCALEDIR gpsdr_localedir

#define GETTEXT_PACKAGE gpsdr_gettext_package

typedef int socklen_t;

extern int rint (double x);

#define finite _finite
#define __finite _finite
#define strdup _strdup
#define open _open
#define write _write
#define read _read
#define close _close
#define tzset _tzset
#define unlink _unlink
#define lseek _lseek
#define mkdir(dirname,access) _mkdir(dirname)

// disable: warning C4244: 'function' : conversion from 'x' to 'y', possible loss of data
#pragma warning(disable:4244)
// disable: warning C4267: 'function' : conversion from 'size_t' to 'int', possible loss of data
#pragma warning(disable:4267)
// disable: warning C4305: 'function' : truncation from 'double' to 'gfloat'
#pragma warning(disable:4305)
// disable: warning C4311: 'type cast' : pointer truncation from 'gpointer' to 'long'
#pragma warning(disable:4311)
// disable: warning C4305: 'initializing' : truncation from 'double' to 'float'
#pragma warning(disable:4305)
// disable: warning C4312: 'type cast' : conversion from 'glong' to 'gpointer' of greater size
#pragma warning(disable:4312)
// disable: warning C4090: 'function' : different 'const' qualifiers
#pragma warning(disable:4090)

#endif /* _CONFIG_H */


#ifndef _CONFIG_H
#define _CONFIG_H


#define _USE_MATH_DEFINES
#define _CRT_SECURE_NO_WARNINGS


#include <Winsock2.h>
#include <math.h>
#include <io.h>
#include <stdlib.h>
#include <locale.h>
#include <direct.h>

#include <glib.h>
#include <glib/gstdio.h>
#include "gettext.h"

#define VERSION "0.0.1 (Win32 pre-alpha)"
#define PACKAGE_VERSION "0.0.1"

extern char gpsdr_datadir[];
extern char gpsdr_localedir[];
extern char gpsdr_gettext_package[];

#define PSIZE gpsdr_psize
#define DATADIR gpsdr_datadir
#define LOCALEDIR gpsdr_localedir

#define GETTEXT_PACKAGE gpsdr_gettext_package

typedef int socklen_t;

extern int rint (double x);

#define finite _finite
#define __finite _finite
#define strdup _strdup
#define open _open
#define write _write
#define read _read
#define close _close
#define tzset _tzset
#define unlink _unlink
#define lseek _lseek
#define mkdir(dirname,access) _mkdir(dirname)

// disable: warning C4244: 'function' : conversion from 'x' to 'y', possible loss of data
#pragma warning(disable:4244)
// disable: warning C4267: 'function' : conversion from 'size_t' to 'int', possible loss of data
#pragma warning(disable:4267)
// disable: warning C4305: 'function' : truncation from 'double' to 'gfloat'
#pragma warning(disable:4305)
// disable: warning C4311: 'type cast' : pointer truncation from 'gpointer' to 'long'
#pragma warning(disable:4311)
// disable: warning C4305: 'initializing' : truncation from 'double' to 'float'
#pragma warning(disable:4305)
// disable: warning C4312: 'type cast' : conversion from 'glong' to 'gpointer' of greater size
#pragma warning(disable:4312)
// disable: warning C4090: 'function' : different 'const' qualifiers
#pragma warning(disable:4090)

#endif /* _CONFIG_H */
