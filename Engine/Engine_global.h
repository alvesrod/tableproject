#ifndef ENGINE_GLOBAL_H
#define ENGINE_GLOBAL_H

#include <QtCore/qglobal.h>

/* =========================================== */

#define ENGINE_VERSION "0.3b"

/* =========================================== */

#if defined(ENGINE_LIBRARY)
#  define ENGINESHARED_EXPORT Q_DECL_EXPORT
#else
#  define ENGINESHARED_EXPORT Q_DECL_IMPORT
#endif

/* SOME IMPORTANT ENGINE CONSTANTS: */

#define LOCAL_IP "127.0.0.1"
#define DEFAULT_LOCATION "Unknown" ///If the user doesn't set a location
#define INVALID_ID -1 //Default user id

#include "engine.h"

#endif // ENGINE_GLOBAL_H
