#ifndef LOGQTSO_LOGITEMSTRMAPPING_H_
#define LOGQTSO_LOGITEMSTRMAPPING_H_

#include "logItem.hpp"

#include <QString>
#include <QMap>

#include <unordered_map>

//keys are lower case
extern const QMap<QString, logItem_c::type_ec> strTologTypeMap_glo_sta_con;
//values are camelcase
extern const std::unordered_map<logItem_c::type_ec, QString> logTypeToStrUMap_glo_sta_con;

#endif // LOGQTSO_LOGITEMSTRMAPPING_H_
