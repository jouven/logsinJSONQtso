//library to manage logs
//by default logs are stored in memory
//there is a setting to save them also on a json file.
//This library uses less memory when the messages are the same,
//i.e. the same error happening repeatedly
//the weak point of this library is that it uses json
//when saving the log entris in a file
//which saves the entire json file every time an entry is added,
//there is no option to append values in json, the log entries in memory need to be serialized every time
//which is pretty heavy for a logging library, still
//since the OS (*nix or windows) normally caches every thing it can and if the log file doesn't grow too much
//it should be fast enough anyway
//for the previous reason the maximum log file size is 2MB by default

//all the functions that modify the containers where
//log objects are saved are thread safe,
//AKA inserting logs from different threads is fine,
//everything else... probably not

#ifndef LOGSINJSONQTSO_LOGDATAHUB_HPP
#define LOGSINJSONQTSO_LOGDATAHUB_HPP

#include "baseClassQtso/baseClassQt.hpp"
#include "logItem.hpp"
#include "crossPlatformMacros.hpp"

#include <QString>
#include <QDateTime>
#include <QJsonObject>
#include <QMutex>
#include <QObject>
#include <QMap>

#include <vector>
#include <unordered_map>
#include <map>
#include <unordered_set>

#define MACRO_FILENAME (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)
#define MACRO_ADDMESSAGE(VAR, MESSAGE, TYPE) VAR.addMessage_f(MESSAGE, TYPE, MACRO_FILENAME, __func__, __LINE__)

class EXPIMP_LOGSINJSONQTSO logFilter_c
{
    QString messageContains_pri;
    bool messageContainsSet_pri = false;

    std::vector<logItem_c::type_ec> types_pri;
    bool typesSet_pri = false;

    QDateTime dateTimeFrom_pri;
    bool dateTimeFromSet_pri = false;
    QDateTime dateTimeTo_pri;
    bool dateTimeToSet_pri = false;

    QString sourceFileNameContains_pri;
    bool sourceFileNameContainsSet_pri = false;
    QString functionNameContains_pri;
    bool functionNameContainsSet_pri = false;

public:
    logFilter_c() = default;

    QString messageContains_f() const;
    void setMessageContains_f(const QString& messageContains_par_con);
    std::vector<logItem_c::type_ec> types_f() const;
    void setTypes_f(const std::vector<logItem_c::type_ec>& types_par_con);
    QDateTime dateTimeFrom_f() const;
    void setDatetimeFrom_f(const QDateTime& dateTimeFrom_par_con);
    QDateTime dateTimeTo_f() const;
    void setDatetimeTo_f(const QDateTime& dateTimeTo_par_con);
    QString sourceFileNameContains_f() const;
    void setSourceFileNameContains_f(const QString& fileNameContains_par_con);
    QString sourceFunctionNameContains_f() const;
    void setSourceFunctionNameContains_f(const QString& functionNameContains_par_con);

    bool messageContainsSet_f() const;
    void unsetMessageContains_f();
    bool typesSet_f() const;
    void unsetTypes_f();
    bool dateTimeFromSet_f() const;
    void unsetDateTimeFrom_f();
    bool dateTimeToSet_f() const;
    void unsetDateTimeTo_f();
    bool sourceFileNameContainsSet_f() const;
    void unsetSourceFileNameContains_f();
    bool sourceFunctionNameContainsSet_f() const;
    void unsetSourceFunctionNameContains_f();

    bool anythingSet_f() const;
};


class EXPIMP_LOGSINJSONQTSO logDataHub_c : public QObject, public eines::baseClassQt_c
{
    Q_OBJECT

    //std::map<uint_fast64_t, logItem_c*> indexToLogItemPtrMap_pri;

    //std::map<uint_fast64_t, QDateTime> indexToDatetimeMap_pri;

    QMap<QDateTime, logItem_c*> dateTimeToLogItemPtrMap_pri;

    //UMap --> key = (message + type) size, value = (UMap --> key = (message + type) hash, value = logItem object)
    std::unordered_map<int_fast64_t, std::unordered_map<uint_fast64_t, logItem_c>> messageSizeUMap_hashElementUMap_pri;

    bool loggingEnabled_pri = true;
    bool echoStdoutOnAddMessage_pri = false;
    bool echoStderrOnError_pri = false;

    QMutex addMessageMutex_pri;

    uint_fast64_t maxMessages_pri = 100 * 1000;
    uint_fast64_t maxUniqueMessages_pri = 10 * 1000;
    // 1024 * 1024 * 2 = 2MB
    int_fast64_t maxLogFileByteSize_pri = 1024 * 1024 * 2;

    //because this class hold all the logs
    //when saving log files, not all the log items are saved
    //this has the starting index for the current log file
    //and when the files changes, this index changes too
    QMap<QDateTime, logItem_c*>::ConstIterator startLogIteratorToSave_pri;
    bool startLogIteratorToSaveSet_pri = false;

    //clear the log containers when:
    //the maxMessage amount is reached
    bool clearWhenMaxMessages_pri = false;
    //the maxUniqueMessage amount is reached
    bool clearWhenMaxUniqueMessages_pri = false;
    //same time as the log file is "rotated" (saveLogFiles_pri must be true)
    bool clearWhenMaxLogFileSize_pri = false;

    QString logSaveDirectoryPath_pri;
    bool isValidLogPathBaseName_pri = false;
    //if logSaveDirectoryPath_pri is not set and valid it won't save
    bool saveLogFiles_pri = false;

    QString currentLogFileName_pri;

    //"yyyyMMdd_hhmmsszzz" for maximum time precission
    QString logFileDatetimeFormat_pri = "yyyyMMdd";
    QString currentDateTimeValue_pri;

    //save them all by default
    std::unordered_set<logItem_c::type_ec> logTypesToSave_pri =
    {
        logItem_c::type_ec::debug
        , logItem_c::type_ec::error
        , logItem_c::type_ec::warning
        , logItem_c::type_ec::info
    };

    //save them all by default, above setting limits this one too
    std::unordered_set<logItem_c::type_ec> logTypesToSaveFile_pri =
    {
        logItem_c::type_ec::debug
        , logItem_c::type_ec::error
        , logItem_c::type_ec::warning
        , logItem_c::type_ec::info
    };

    //returns true if the message was inserted
    //still will "append" an error if
    //file saving fails
    bool addMessageInternal_f(
            const QString& message_par_con
            , const logItem_c::type_ec type_par_con
            , const QString& sourceFile_par_con
            , const QString& sourceFunction_par_con
            , const int_fast32_t sourceLineNumber_par_con
            , const QDateTime& dateTime_par_con = QDateTime()
    );

    void write_f(QJsonObject& json_par) const;
    void read_f(
            const QJsonObject& json_par_con
            , const logFilter_c& logFilter_par_con
    );

    QString getNextFilePath_f();

    bool readLogFile_f(
            const QString& path_par_con
            , const logFilter_c& logFilter_par_con
    );
    bool filterMatch_f(
            const logFilter_c& logFilter_par_con
            , const QString& message_par_con
            , const logItem_c::type_ec typeStr_par_con
            , const QDateTime& datetime_par_con
            , const QString& sourceFile_par_con
            , const QString& sourceFunction_par_con
    ) const;
public:
    logDataHub_c();

    //returns true if the message was inserted
    //still will "append" an error if
    //file saving fails
    bool addMessage_f(
            const QString& message_par_con
            , const logItem_c::type_ec type_par_con
            , const QString& sourceFile_par_con
            , const QString& sourceFunction_par_con
            , const int_fast32_t sourceLineNumber_par_con
    );

    //when inserting a log show the values on stdout
    void setEchoToStdoutOnAddMessage_f(const bool echoOnStdoutOnAddMessage_par_con);
    bool echoToStdoutOnAddMessage_f() const;

    //this is for this class errors, not the logs
    bool echoStderrOnError_f() const;
    void setEchoStderrOnError_f(const bool echoStderrOnError_par_con);

    std::vector<std::pair<const logItem_c* const, const QDateTime* const> > filter_f(const logFilter_c& logFilter_par_con = logFilter_c()) const;

    //removes all log messages from this class log containers,
    //log files aren't affected BUT it will "rotate" the current log filename
    //(otherwise it would replace the current log file the first message after the clear, losing the previous entries)
    void clearLogs_f();

    //because some infinite loop might happen on any program having logs, stop logging after certain number of messages
    //a uint_fast64_t is used but there is a hidden? limit from QHash max size, which as of 20181204, uses an int do the the size() function
    uint_fast64_t maxMessages_f() const;
    void setMaxMessages_f(const uint_fast64_t& maxMessages_par_con);

    //because some infinite loop might happen on any program having logs, stop logging after certain number of messages
    uint_fast64_t maxUniqueMessages_f() const;
    void setMaxUniqueMessages_f(const uint_fast64_t& maxUniqueMessages_par_con);

    //log entries
    uint_fast64_t messageCount_f() const;
    //unique log entries
    uint_fast64_t uniqueMessageCount_f() const;

    QString logPathBaseName_f() const;
    //logSaveDirectoryPath_par_con is the "directory" path where to save the logs,
    //if empty, the path from where the process was called will be used
    //the log files will be named using QCoreApplication::applicationName()
    //if QCoreApplication::applicationName() returns "appA" -> "*logSaveDirectoryPath_par_con*/appA_%date%_%time%_UTC.log"
    //when a log file reaches a certain size (see maxLogFileSize_f)
    //a new one will be created
    //will "append" an error if the file already exists
    //or can't open-write the "new" file
    void setLogSaveDirectoryPath_f(
            const QString& logSaveDirectoryPath_par_con = QString()
            , const bool keepOldLogPathOnError_par_con = false
    );
    bool isValidLogPathBaseName_f() const;

    bool saveLogFiles_f() const;
    //if a valid "logPathBaseName" hasn't been set saving will be ignored
    void setSaveLogFiles_f(const bool saveLogFiles_par_con);

    int_fast64_t maxLogFileByteSize_f() const;
    void setMaxLogFileByteSize_f(const int_fast64_t maxLogFileByteSize_par_con);

    //loads a previously saved log file/s
    //this can be a single file or a "baseName", "basename" will load all the files with the same basename
    //if logPathBaseName_par_con is empty, applicationName will be used
    //keepUsingLastLogFile_par_con will set logSaveDirectoryPath to the loaded file directory
    //returns true if logs were loaded
    bool loadLogFiles_f(
            const QString& logPathBaseName_par_con = QString()
            , const logFilter_c& logFilter_par_con = logFilter_c()
            , const bool onlyLoadLastLogFile_par_con = false
            , const bool keepUsingLastLogFile_par_con = false
    );

    QString currentLogFileName_f() const;

    bool loggingEnabled_f() const;
    //disables addMessage_f, logs will not be inserted
    //use if logging is slow in certain code sections
    void setLoggingEnabled_f(const bool loggingEnabled_par_con);

    std::unordered_set<logItem_c::type_ec> logTypesToSave_f() const;
    void setLogTypesToSave_f(const std::unordered_set<logItem_c::type_ec>& logTypesToSave_par_con);

    std::unordered_set<logItem_c::type_ec> logTypesToSaveFile_f() const;
    void setLogTypesToSaveFile_f(const std::unordered_set<logItem_c::type_ec>& logTypesToSaveFile_par_con);

    QString logFileDatetimeFormat_f() const;
    //this applies the next time a log file is "rotated" or when setting the log files save directory
    void setLogFileDatetimeFormat_f(const QString& logFileDatetimeFormat_par_con);

    bool clearWhenMaxMessages_f() const;
    void setClearWhenMaxMessages_f(const bool clearWhenMaxMessages_ar_con);

    bool clearWhenMaxUniqueMessages_f() const;
    void setClearWhenMaxUniqueMessages_f(const bool clearWhenMaxUniqueMessages_par_con);

    bool clearWhenMaxLogFileSize_f() const;
    //will only work is saveLogFiles = true
    void setClearWhenMaxLogFileSize_f(const bool clearWhenMaxLogFileSize_par_con);

Q_SIGNALS:
    //using an int because map size is as int
    void messageAdded_signal(const int index_par_con, const logItem_c* logItem_par_con, const QDateTime* datetime_par_con);
    //emmited when clearLogs is called
    void clearLogs_signal();
};

#endif // LOGSINJSONQTSO_LOGDATAHUB_HPP
