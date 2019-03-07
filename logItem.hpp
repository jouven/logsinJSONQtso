
#ifndef LOGSINJSONQTSO_LOG_HPP
#define LOGSINJSONQTSO_LOG_HPP

#include "crossPlatformMacros.hpp"

#include <QString>

class EXPIMP_LOGSINJSONQTSO logItem_c
{
    QString message_pri;
public:
    enum class type_ec
    {
        empty
        , info
        , warning
        , error
        , debug
    };
private:
    type_ec type_pri = type_ec::empty;

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
    logItem_c(
            const QString& message_par_con
            , const type_ec type_par_con
            //, const QDateTime datetime_par_con
            , const QString& sourceFileName_par_con
            , const QString& sourceFunctionName_par_con
            , const int_fast32_t sourceLineNumber_par_con
            , const QString& threadId_par_con = QString()
    );

  //  void write_f(QJsonObject& json_par) const;
    bool isValid_f() const;

    QString message_f() const;
    type_ec type_f() const;
    //QDateTime datetime_f() const;
    QString sourceFileName_f() const;
    QString sourceFunctionName_f() const;
    int_fast32_t sourceLineNumber_f() const;
    QString threadId_f() const;
};

#endif // LOGSINJSONQTSO_LOG_HPP
