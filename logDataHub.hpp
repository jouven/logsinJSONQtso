//library to manage logs
//by default logs are stored in memory
//there is a setting to save them also on a text/json file.
//This library uses less memory when the messages are the same,
//i.e. the same error happening repeatedly

//the weak point mentioned below can be dodged using "text mode" instead of json
//The weak point of this library is that it uses json (20191019 in the first versions, not anymore!)
//when saving the log entris in a file
//which saves the entire json file every time an entry is added,
//there is no option to append values in json, the log entries in memory need to be serialized every time
//which is pretty heavy for a logging library, still
//since the OS (*nix and windows both do this)
//normally caches everything and if the log file doesn't grow too much
//it should be fast enough anyway...?
//for the previous reason the maximum log file size is 2MB by default


//all the functions that modify the containers where
//log objects are saved are thread safe,
//AKA inserting/deleting logs from different threads is fine,
//everything else... probably not

//20210217 regarding translations the log library won't translate anything
//the only translation related functionality is text_c which can hint if a log message is already translated or not
//translations should be done when outputing (in a program) or just grabbing the log entries and translating them, then they can be saved, translated, separately
//still messages can be translated just before inserting the log entry
//the only point of no return is when log entries are saved in a file, because of the value replacing, the message placeholders are replaced with the values
//that were a separate element until then, this creates an alternate message to translate

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
#include <QFile>
#include <QTimer>

#include <atomic>
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

    QString referenceContains_pri;
    bool referenceContainsSet_pri = false;

    std::vector<messageType_ec> types_pri;
    bool typesSet_pri = false;

    QDateTime dateTimeFrom_pri;
    bool dateTimeFromSet_pri = false;
    QDateTime dateTimeTo_pri;
    bool dateTimeToSet_pri = false;

    QString sourceFileNameContains_pri;
    bool sourceFileNameContainsSet_pri = false;
    QString functionNameContains_pri;
    bool functionNameContainsSet_pri = false;

    QString threadIdContains_pri;
    bool threadIdContainsSet_pri = false;
public:
    logFilter_c() = default;

    QString messageContains_f() const;
    void setMessageContains_f(const QString& messageContains_par_con);
    QString referenceContains_f() const;
    void setReferenceContains_f(const QString& referenceContains_par_con);

    std::vector<messageType_ec> types_f() const;
    void setTypes_f(const std::vector<messageType_ec>& types_par_con);
    QDateTime dateTimeFrom_f() const;
    void setDatetimeFrom_f(const QDateTime& dateTimeFrom_par_con);
    QDateTime dateTimeTo_f() const;
    void setDatetimeTo_f(const QDateTime& dateTimeTo_par_con);
    QString sourceFileNameContains_f() const;
    void setSourceFileNameContains_f(const QString& fileNameContains_par_con);
    QString sourceFunctionNameContains_f() const;
    void setSourceFunctionNameContains_f(const QString& functionNameContains_par_con);
    QString threadIdContains_f() const;
    void setThreadIdContains_f(const QString& threadIdContains_par_con);

    bool messageContainsSet_f() const;
    void unsetMessageContains_f();
    bool referenceContainsSet_f() const;
    void unsetReferenceContains_f();
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
    bool threadIdContainsSet_f() const;
    void unsetThreadIdContains_f();

    bool anythingSet_f() const;
};


class EXPIMP_LOGSINJSONQTSO logDataHub_c : public QObject, public baseClassQt_c
{
    Q_OBJECT

    //this variable below is the absolute true order of log insertion
    //because more than one log can have the same datetime to the millisecond (which is the maximum resolution of QDatetime)
    //and how do you sort when 2 or more values are the same?

    std::atomic<int_fast64_t> atomicLogIndex_pri = {0};
    int_fast64_t getNewLogIndex_f();
    //std::map<uint_fast64_t, logItem_c*> indexToLogItemPtrMap_pri;

    //std::map<uint_fast64_t, QDateTime> indexToDatetimeMap_pri;

    //because an order is needed
    //Key index of insertion, value pair with a ptr to the logItem object and the datetime of the log insertion
    QMap<int_fast64_t, std::pair<logItem_c*, QDateTime>> indexToLogItemPtrAndDatetimeMap_pri;

    //the nested UMap thing is to have less hash collisions,
    //it should? be less probable to have the same hash within messages of the same size
    //and is two smaller UMaps faster than one big UMap?
    //Outer UMap key = it's the SIZE value of (message + type + source File + source Function + source Line number)
    //Outer UMap value = (nested UMap key = (message + type + source File + source Function + source Line number) hash, nested UMap value = logItem object)
    std::unordered_map<int_fast64_t, std::unordered_map<uint_fast64_t, logItem_c>> messageSizeUMap_hashElementUMap_pri;

    //NOTODO? now that logItem_c uses text_c instead of QString, there is a further possible optimization
    //if the text is the same and the replacement values change and they are "big", the replacement values could be hashed and stored
    //in another container similar to messageSizeUMap_hashElementUMap_pri but only for replacement values, indexToLogItemPtrAndDatetimeMap_pri would need
    //another pointer in the value "class" to reference an element to this new container
    //but as of 20191227 there is no case of the same text repeating a lot but with different replacement values, also no big replacement values in general

    bool loggingEnabled_pri = true;
    //bool echoStdoutOnAddMessage_pri = false;
    //bool echoStderrOnError_pri = false;

    QMutex addMessageMutex_pri;
    QTimer saveFileFlushDebounce_pri;

    //memory log limits, I know this is not the appropiate solution because it doesn't measure how much is inside the containers
    uint_fast64_t maxMessages_pri = 100 * 1000;
    uint_fast64_t maxUniqueMessages_pri = 10 * 1000;
    //file size limit 1024 * 1024 * 2 = 2MB
    //it's signed because qt size() functions are signed
    int_fast64_t maxLogFileByteSize_pri = 1024 * 1024 * 2;

    //this only applies for json logging
    //unlike text log, JSON log has a file saving issue, when the memory log is partially cleared because it has hit the limit (or fully cleared)
    //the log file must be rotated too because the "old" logs can't be saved anymore because json saving works replacing all the current logs in the current log file
    //and that would remove the old ones on the next save after, same situation when the file log size limit is reached but in this case it would duplicate logs on the new file,
    //so an extra variable index must be used to point to an index from where the logs will be "written from"
    int_fast64_t currentIndexToSaveFrom_pri = 0;

    //the log containers are cleared when the maxMessage / maxUniqueMessages is reached
    //this below makes the clear partial instead total to leave some context
    bool partialClearWhenMaxMessages_pri = false;
    bool partialClearWhenMaxUniqueMessages_pri = false;

    //rotate log file when maxLogFileByteSize_pri is reached (saveLogFiles_pri must be true)
    bool rotateWhenMaxLogFileSize_pri = false;

    QString logSaveDirectoryPath_pri;
    bool isValidLogPathBaseName_pri = false;
    //logSaveDirectoryPath_pri must set and valid to really save
    bool saveLogFiles_pri = false;
    //current log base filename, default it executable name
    QString logBaseFilename_pri;
    //current log filename, this is basename + datetime + extension
    QString currentLogFileName_pri;


    //"yyyyMMdd_hhmmsszzz" for maximum time precission
    QString logFileDatetimeFormat_pri = "yyyyMMdd";
    QString currentDateTimeValue_pri;

    //log types to log, all by default
    std::unordered_set<messageType_ec> logTypesToSave_pri =
    {
        messageType_ec::debug
        , messageType_ec::error
        , messageType_ec::warning
        , messageType_ec::information
        , messageType_ec::question
    };

    //log types to file save, all by default, above setting limits this one too
    std::unordered_set<messageType_ec> logTypesToSaveFile_pri =
    {
        messageType_ec::debug
        , messageType_ec::error
        , messageType_ec::warning
        , messageType_ec::information
        , messageType_ec::question
    };

    //file save log type, true is text, one log entry per line, false is json type
    bool fileSaveLogTypeText_pri = true;
    //to keep the file open
    QFile logFile_pri;

    //returns true if the message was inserted
    //still will "append" an error if
    //file saving fails
    bool addMessageInternal_f(const text_c& message_par_con
            , const QString& reference_par_con
            , const messageType_ec type_par_con
            , const QString& sourceFile_par_con
            , const QString& sourceFunction_par_con
            , const int_fast32_t sourceLineNumber_par_con
            , const QDateTime& dateTime_par_con = QDateTime()
            , const QString& threadId_par_con = QString()
    );

    void write_f(QJsonObject& json_par) const;
    void readLogsFromJsonObject_f(
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
            , const QString& reference_par_con
            , const messageType_ec typeStr_par_con
            , const QDateTime& datetime_par_con
            , const QString& sourceFile_par_con
            , const QString& sourceFunction_par_con
            , const QString& threadId_par_con
    ) const;

    QString generateTextLine_f(logItem_c* logItem_par, const QDateTime& datetime_par_con) const;

    //parses a text log line and (if successfully parsed) adds it to log containers/objects of this class
    //retuns true on success
    bool readTextLine_f(QString line_par, const logFilter_c& logFilter_par_con = logFilter_c());

    //if a translator ptr is set, log entries will be translated when written in the log file/s
    //also filtering will be done against the translations
    //translator_c* translator_pri = nullptr;

    //when the containers reach the maxMessage limits this functions clears them partially
    //so there is at least some logs left as a possible context
    void partialMemoryLogClear_f();

public:
    //no translation errors are reported
    //care must be taken that the translator_c object pointed by translatorPtr_par
    //has its config properly set
    logDataHub_c();

    //returns true if the message was inserted
    //will "append" an error if file saving fails
    bool addMessage_f(
            const text_c& message_par_con
            , const messageType_ec type_par_con
            , const QString& sourceFile_par_con
            , const QString& sourceFunction_par_con
            , const int_fast32_t sourceLineNumber_par_con
    );
    bool addMessage_f(
            const text_c& message_par_con
            , const QString& reference_par_con
            , const messageType_ec type_par_con
            , const QString& sourceFile_par_con
            , const QString& sourceFunction_par_con
            , const int_fast32_t sourceLineNumber_par_con
    );
    std::vector<bool> addMessage_f(
            const textCompilation_c& message_par_con
            , const QString& reference_par_con
            , const messageType_ec type_par_con
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

    std::vector<std::pair<const logItem_c* const, const QDateTime* const>> filter_f(const logFilter_c& logFilter_par_con = logFilter_c()) const;

    //removes all log messages from this class log containers,
    //log files aren't affected BUT it will "rotate" the current log filename
    //(otherwise it would replace the current log file the first message after the clear, losing the previous entries)
    //thread-safe
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

    QString logSaveDirectoryPath_f() const;

    //IMPORTANT to save log files setLogSaveDirectoryPath_f or loadLogFiles_f must be called first

    //logSaveDirectoryPath_par_con is a "directory" path where to save the logs,
    //if empty the path from where the process was called will be used
    //the log files will be named using logBaseFilename_par_con or
    //if it's null, the executable name, i.e., logBaseFilename_par_con = "test" -> "*logSaveDirectoryPath_par_con*/test_%date%_%time%_UTC.log"
    //for executable name "appA" -> "*logSaveDirectoryPath_par_con*/appA_%date%_%time%_UTC.log"
    //when a log file reaches a certain size (see maxLogFileSize_f)
    //a new one will be created
    //will "append" an error if the file already exists
    //or can't open-write the "new" file
    void setLogSaveDirectoryPath_f(
            const QString& logSaveDirectoryPath_par_con = QString()
            , const QString& logBaseFilename_par_con = QString()
            , const bool keepOldLogPathOnError_par_con = false
    );
    bool isValidLogPathBaseName_f() const;

    bool saveLogFiles_f() const;
    //if a valid "logPathBaseName" hasn't been set saving will be ignored
    void setSaveLogFiles_f(const bool saveLogFiles_par_con);

    int_fast64_t maxLogFileByteSize_f() const;
    void setMaxLogFileByteSize_f(const int_fast64_t maxLogFileByteSize_par_con);

    //loads a previously saved log file/s
    //logFilePath_par_con can be a single file path or a partial file path name which will load all the files that match the partial
    //if logFilePath_par_con is empty, executableName will be used
    //keepUsingLastLogFile_par_con will set logSaveDirectoryPath to the loaded file/s directory
    //returns true if any log entries were loaded
    //writing log lines to a file is disabled while loading the log file/s
    //WARNING duplicates can be inserted this way, because loaded logs are appended, they won't replace existing indexes,
    //even if the index order doesn't make sense chronologically
    bool loadLogFiles_f(
            const QString& logFilePath_par_con = QString()
            , const logFilter_c& logFilter_par_con = logFilter_c()
            //log files need to be properly sorted, by name, for this to work
            , const bool onlyLoadLastLogFile_par_con = false
            //append an error if no log file to load is found
            , const bool keepUsingLoadedLogFilePath_par_con = true
            , const bool errorWhenNoneFound_par_con = false
    );

    //creates a new log file in the logFilePath, stops using the current log file path
    //won't replace/use existing files
    bool changeLogFilePath_f(
            const QString& logFilePath_par_con
            , const bool createParentDirectories_par_con = true
    );

    QString currentLogFileName_f() const;

    bool loggingEnabled_f() const;
    //disables addMessage_f, logs will not be inserted
    //use if logging is slow in certain code sections
    void setLoggingEnabled_f(const bool loggingEnabled_par_con);

    std::unordered_set<messageType_ec> logTypesToSave_f() const;
    void setLogTypesToSave_f(const std::unordered_set<messageType_ec>& logTypesToSave_par_con);

    std::unordered_set<messageType_ec> logTypesToSaveFile_f() const;
    void setLogTypesToSaveFile_f(const std::unordered_set<messageType_ec>& logTypesToSaveFile_par_con);

    QString logFileDatetimeFormat_f() const;
    //this applies the next time a log file is "rotated" or when setting the log files save directory
    void setLogFileDatetimeFormat_f(const QString& logFileDatetimeFormat_par_con);

    bool partialClearWhenMaxMessages_f() const;
    void setPartialClearWhenMaxMessages_f(const bool partialClearWhenMaxMessages_par_con);

    bool partialClearWhenMaxUniqueMessages_f() const;
    void setClearWhenMaxUniqueMessages_f(const bool partialClearWhenMaxUniqueMessages_par_con);

    bool rotateWhenMaxLogFileSize_f() const;
    //will only work is saveLogFiles = true
    void setRotateWhenMaxLogFileSize_f(const bool rotateWhenMaxLogFileSize_par_con);

    bool fileSaveLogTypeText_f() const;
    //true for text logs (1 log entry = 1 text line), false for json
    void setFileSaveLogTypeText_f(const bool fileSaveLogTypeText_par_con);

    //translator_c* translatorPtr_f() const;
    //will translate errors if set to a translator object
    //void setTranslatorPtr_f(translator_c* translatorPtr_par);
Q_SIGNALS:
    //using an int because QMap size is as int
    void messageAdded_signal(const int index_par_con, const logItem_c* logItem_par_con, const QDateTime* datetime_par_con);
    //emmited when clearLogs is called
    void clearLogs_signal();
    void stopFlushTimer_signal();
    void startFlushTimer_signal();
private Q_SLOTS:
    void stopFlushTimer_f();
    void startFlushTimer_f();
    void flushSaveFile_f();
};

#endif // LOGSINJSONQTSO_LOGDATAHUB_HPP
