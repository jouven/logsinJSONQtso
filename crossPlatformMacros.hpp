//because windows sucks...

#ifndef LOGSINJSONQTSO_CROSSPLATFORMMACROS_HPP
#define LOGSINJSONQTSO_CROSSPLATFORMMACROS_HPP

#include <QtCore/QtGlobal>

//remember to define this variable in the .pro file
#if defined(LOGSINJSONQTSO_LIB)
#  define EXPIMP_LOGSINJSONQTSO Q_DECL_EXPORT
#else
#  define EXPIMP_LOGSINJSONQTSO Q_DECL_IMPORT
#endif

#endif // LOGSINJSONQTSO_CROSSPLATFORMMACROS_HPP
