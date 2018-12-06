
#ifndef LOGQTSO_LOG_HPP
#define LOGQTSO_LOG_HPP

#include <QString>

class logItem_c
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

    //save always in UTC, show on UTC or local
    //QDateTime datetime_pri;

    QString sourceFileName_pri;
    QString sourceFunctionName_pri;
    int_fast32_t sourceLineNumber_pri = 0;

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
    );

  //  void write_f(QJsonObject& json_par) const;
    bool isValid_f() const;

    QString message_f() const;
    type_ec type_f() const;
    //QDateTime datetime_f() const;
    QString sourceFileName_f() const;
    QString sourceFunctionName_f() const;
    int_fast32_t sourceLineNumber_f() const;
};

#endif // LOGQTSO_LOG_HPP
