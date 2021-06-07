#include "logItem.hpp"

QString logItem_c::threadId_f() const
{
    return threadId_pri;
}

void logItem_c::setTranslated_f(const bool translated_par_con)
{
    message_pri.setTranslated_f(translated_par_con);
}

QString logItem_c::reference_f() const
{
    return reference_pri;
}

logItem_c::logItem_c(
        const text_c& message_par_con
        , QString reference_par
        , const messageType_ec type_par_con
        //, const QDateTime datetime_par_con
        , const QString& sourceFileName_par_con
        , const QString& sourceFunctionName_par_con
        , const int_fast32_t sourceLineNumber_par_con
        , const QString& threadId_par_con)
    : message_pri(message_par_con)
    , reference_pri(reference_par.remove('\t').remove('\n'))
  , type_pri(type_par_con)
  //, datetime_pri(datetime_par_con)
  , sourceFileName_pri(sourceFileName_par_con)
  , sourceFunctionName_pri(sourceFunctionName_par_con)
  , sourceLineNumber_pri(sourceLineNumber_par_con)
  , threadId_pri(threadId_par_con)
{}

//void logItem_c::write_f(QJsonObject& json_par) const
//{
//    json_par["message"] = message_pri;
//    json_par["type"] = logTypeToStrUMap_ext_con.at(type_pri);
//    json_par["datetime"] = datetime_pri.toString("yyyy-MM-dd hh:mm:ss.zzz");
//    json_par["sourceFile"] = sourceFile_pri;
//    json_par["sourceFunction"] = sourceFunction_pri;
//    json_par["sourceLineNumber"] = QString::number(sourceLineNumber_pri);
//}

//void logItem_c::read_f(const QJsonObject& json_par_con)
//{
//    message_pri = json_par_con["message"].toString();
//    type_pri = strTologTypeMap_ext_con.value(json_par_con["type"].toString().toLower());
//    datetime_pri = QDateTime::fromString(json_par_con["datetime"].toString(), "yyyy-MM-dd hh:mm:ss.zzz");
//    sourceFile_pri = json_par_con["sourceFile"].toString();
//    sourceFunction_pri = json_par_con["sourceFunction"].toString();
//    sourceLineNumber_pri = json_par_con["sourceLineNumber"].toString("0").toLongLong();
//}

//logItem_c::logItem_c(const QJsonObject& json_par_con)
//{
//    read_f(json_par_con);
//}

bool logItem_c::isValid_f() const
{
    return not message_pri.empty_f()
            and type_pri not_eq messageType_ec::empty
    //        and not datetime_pri.isNull()
            and not sourceFileName_pri.isEmpty()
            and not sourceFunctionName_pri.isEmpty()
            and sourceLineNumber_pri > 0;
}

text_c logItem_c::message_f() const
{
    return message_pri;
}

messageType_ec logItem_c::type_f() const
{
    return type_pri;
}

//QDateTime logItem_c::datetime_f() const
//{
//    return datetime_pri;
//}

QString logItem_c::sourceFileName_f() const
{
    return sourceFileName_pri;
}

QString logItem_c::sourceFunctionName_f() const
{
    return sourceFunctionName_pri;
}

int_fast32_t logItem_c::sourceLineNumber_f() const
{
    return sourceLineNumber_pri;
}
