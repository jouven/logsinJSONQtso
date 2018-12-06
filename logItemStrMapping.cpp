#include "logItemStrMapping.hpp"

const QMap<QString, logItem_c::type_ec> strTologTypeMap_glo_sta_con(
{
//Keys are lower case because this way when reading from json we can "lower" w/e is
//there and compare, allowing to ignore case
    {	"info", logItem_c::type_ec::info}
    , {	"warning", logItem_c::type_ec::warning}
    , {	"error", logItem_c::type_ec::error}
    , {	"debug", logItem_c::type_ec::debug}

});

const std::unordered_map<logItem_c::type_ec, QString> logTypeToStrUMap_glo_sta_con(
{
    {	logItem_c::type_ec::info, "info" }
    , {	logItem_c::type_ec::warning, "warning" }
    , {	logItem_c::type_ec::error, "error" }
    , {	logItem_c::type_ec::debug, "debug" }

});
