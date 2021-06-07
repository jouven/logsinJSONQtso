
#ifndef LOGSINJSONQTSO_LOG_HPP
#define LOGSINJSONQTSO_LOG_HPP

#include "crossPlatformMacros.hpp"

#include "textQtso/text.hpp"
#include "essentialQtso/enumClass.hpp"

#include <QString>
#include <QMap>

#include <unordered_map>

class EXPIMP_LOGSINJSONQTSO logItem_c
{
    //QString message_pri;
    text_c message_pri;
    //object/ids context, when objects of the same class/structs pass by the same functions/instructions
    //it's hard to tell which instance of the object is affected by the log message, also this automates adding the ids every time in the "log line"
    //since they can be added later
    //not translated, tabs and newlines will be removed when inserting
    QString reference_pri;

    messageType_ec type_pri = messageType_ec::empty;

    //DON'T USE THIS,
    //it makes log entries in different indexes have the same time
    //QDateTime datetime_pri;

    QString sourceFileName_pri;
    QString sourceFunctionName_pri;
    int_fast32_t sourceLineNumber_pri = 0;

    QString threadId_pri;

//    void read_f(const QJsonObject& json_par_con);
public:
    logItem_c() = default;
    //logItem_c(const QJsonObject& json_par_con);
    logItem_c(const text_c& message_par_con
            , QString reference_par
            , const messageType_ec type_par_con
            , const QString& sourceFileName_par_con
            , const QString& sourceFunctionName_par_con
            , const int_fast32_t sourceLineNumber_par_con
            , const QString& threadId_par_con = QString()
    );

  //  void write_f(QJsonObject& json_par) const;
    bool isValid_f() const;

    text_c message_f() const;
    messageType_ec type_f() const;
    //QDateTime datetime_f() const;
    QString sourceFileName_f() const;
    QString sourceFunctionName_f() const;
    int_fast32_t sourceLineNumber_f() const;
    QString threadId_f() const;
    QString reference_f() const;

    void setTranslated_f(const bool translated_par_con);
};

#endif // LOGSINJSONQTSO_LOG_HPP
