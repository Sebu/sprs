#ifndef EPICORE_GLOBAL_H
#define EPICORE_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(EPICORE_LIBRARY)
#  define EPICORESHARED_EXPORT Q_DECL_EXPORT
#else
#  define EPICORESHARED_EXPORT Q_DECL_IMPORT
#endif

#endif // EPICORE_GLOBAL_H
