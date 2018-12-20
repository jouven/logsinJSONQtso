#ifndef LOGSINJSONQTSO_LOGITEMSTRMAPPING_H_
#define LOGSINJSONQTSO_LOGITEMSTRMAPPING_H_

#include "logItem.hpp"

#include "crossPlatformMacros.hpp"

#include <QString>
#include <QMap>

#include <unordered_map>

//keys are lower case
extern EXPIMP_LOGSINJSONQTSO const QMap<QString, logItem_c::type_ec> strTologTypeMap_glo_sta_con;
//values are camelcase
extern EXPIMP_LOGSINJSONQTSO const std::unordered_map<logItem_c::type_ec, QString> logTypeToStrUMap_glo_sta_con;

#endif // LOGSINJSONQTSO_LOGITEMSTRMAPPING_H_
