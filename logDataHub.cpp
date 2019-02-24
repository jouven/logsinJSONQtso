#include "logDataHub.hpp"

#include "logItemStrMapping.hpp"

#include "cryptoQtso/hashQt.hpp"
#include "essentialQtso/essentialQt.hpp"

#include <QJsonArray>
#include <QJsonDocument>
#include <QFile>
#include <QFileInfo>
#include <QDir>
//#include <QCollator>

uint_fast64_t logDataHub_c::maxMessages_f() const
{
    return maxMessages_pri;
}

void logDataHub_c::setMaxMessages_f(const uint_fast64_t& maxMessages_par_con)
{
    QMutexLocker lockerTmp(&addMessageMutex_pri);
    uint_fast64_t oldValueTmp(maxMessages_pri);
    maxMessages_pri = maxMessages_par_con;
    if (maxMessages_pri < 1)
    {
        maxMessages_pri = oldValueTmp;
    }
}

uint_fast64_t logDataHub_c::maxUniqueMessages_f() const
{
    return maxUniqueMessages_pri;
}

void logDataHub_c::setMaxUniqueMessages_f(const uint_fast64_t& maxUniqueMessages_par_con)
{
    QMutexLocker lockerTmp(&addMessageMutex_pri);
    uint_fast64_t oldValueTmp(maxUniqueMessages_pri);
    maxUniqueMessages_pri = maxUniqueMessages_par_con;
    if (maxUniqueMessages_pri < 1)
    {
        maxUniqueMessages_pri = oldValueTmp;
    }
}

uint_fast64_t logDataHub_c::messageCount_f() const
{
    return dateTimeToLogItemPtrMap_pri.size();
}

uint_fast64_t logDataHub_c::uniqueMessageCount_f() const
{
    uint_fast64_t countTmp(0);
    for (const std::pair<const int_fast64_t, std::unordered_map<uint_fast64_t, logItem_c>>& pair_ite_con : messageSizeUMap_hashElementUMap_pri)
    {
        countTmp = countTmp + pair_ite_con.second.size();
    }
    return countTmp;
}

bool logDataHub_c::echoStderrOnError_f() const
{
    return echoStderrOnError_pri;
}

void logDataHub_c::setEchoStderrOnError_f(const bool echoStderrOnError_par_con)
{
    echoStderrOnError_pri = echoStderrOnError_par_con;
}

bool logDataHub_c::isValidLogPathBaseName_f() const
{
    return isValidLogPathBaseName_pri;
}

QString logDataHub_c::currentLogFileName_f() const
{
    return currentLogFileName_pri;
}

bool logDataHub_c::loggingEnabled_f() const
{
    return loggingEnabled_pri;
}

void logDataHub_c::setLoggingEnabled_f(const bool loggingEnabled_par_con)
{
    loggingEnabled_pri = loggingEnabled_par_con;
}

std::unordered_set<logItem_c::type_ec> logDataHub_c::logTypesToSave_f() const
{
    return logTypesToSave_pri;
}

void logDataHub_c::setLogTypesToSave_f(const std::unordered_set<logItem_c::type_ec>& logTypesToSave_par_con)
{
    logTypesToSave_pri = logTypesToSave_par_con;
}

QString logDataHub_c::logFileDatetimeFormat_f() const
{
    return logFileDatetimeFormat_pri;
}

void logDataHub_c::setLogFileDatetimeFormat_f(const QString& logFileDatetimeFormat_par_con)
{
    logFileDatetimeFormat_pri = logFileDatetimeFormat_par_con;
}

bool logDataHub_c::clearWhenMaxMessages_f() const
{
    return clearWhenMaxMessages_pri;
}

void logDataHub_c::setClearWhenMaxMessages_f(const bool clearWhenMaxMessages_ar_con)
{
    clearWhenMaxMessages_pri = clearWhenMaxMessages_ar_con;
}

bool logDataHub_c::clearWhenMaxUniqueMessages_f() const
{
    return clearWhenMaxUniqueMessages_pri;
}

void logDataHub_c::setClearWhenMaxUniqueMessages_f(const bool clearWhenMaxUniqueMessages_par_con)
{
    clearWhenMaxUniqueMessages_pri = clearWhenMaxUniqueMessages_par_con;
}

bool logDataHub_c::clearWhenMaxLogFileSize_f() const
{
    return clearWhenMaxLogFileSize_pri;
}

void logDataHub_c::setClearWhenMaxLogFileSize_f(const bool clearWhenMaxLogFileSize_par_con)
{
    clearWhenMaxLogFileSize_pri = clearWhenMaxLogFileSize_par_con;
}

std::unordered_set<logItem_c::type_ec> logDataHub_c::logTypesToSaveFile_f() const
{
    return logTypesToSaveFile_pri;
}

void logDataHub_c::setLogTypesToSaveFile_f(const std::unordered_set<logItem_c::type_ec>& logTypesToSaveFile_par_con)
{
    logTypesToSaveFile_pri = logTypesToSaveFile_par_con;
}

bool logDataHub_c::addMessageInternal_f(
        const QString& message_par_con
        , const logItem_c::type_ec type_par_con
        , const QString& sourceFile_par_con
        , const QString& sourceFunction_par_con
        , const int_fast32_t sourceLineNumber_par_con
        , const QDateTime& dateTime_par_con
)
{
    bool resultTmp(false);
    while (true)
    {
        if (message_par_con.isEmpty())
        {
            appendError_f("Empy message");
            break;
        }

        //IMPORTANT if the datetime variable creation happens before the mutex, more than one log entries to be added
        //can get the same datetime, which spells disaster dealing with index calculation,
        //because the second time the same datetime is "added" (it won't, it will be updated on the QMap)
        //the index calculation will return an already used one, which will mess any log array/table/lists
        //trying to add the same index again.
        //The issue boils down to multiple threads being able to get the same datetime, but that's not
        //a case that this logging class/algorithm can't deal with so geting the datetime after
        //locking the mutex solves this, but if someday in the FAR future comp speeds become really fast
        //and datetime doesn't get more data "resolution" this might be a problem
        //but I don't think this will be a problem since there is storage access happening
        //WARNING this can still fail when loading logs from a file if the file is rigged to have
        //more than one try with the same datetime
        //but a log file created by this class can't be like this
        QMutexLocker lockerTmp(&addMessageMutex_pri);
        const QDateTime currentDateTimeTmp(dateTime_par_con.isNull() ? QDateTime::currentDateTimeUtc() : dateTime_par_con);

        bool maxMessagesReachedTmp(dateTimeToLogItemPtrMap_pri.size() >= maxMessages_pri);
        bool maxUniqueMessagesReachedTmp(uniqueMessageCount_f() >= maxUniqueMessages_pri);
        bool maxLogFileSizeReachedTmp(false);

        if (saveLogFiles_pri)
        {
            QFileInfo currentLogFileInfo(currentLogFileName_pri);
            if (currentLogFileInfo.size() >= maxLogFileByteSize_pri)
            {
                maxLogFileSizeReachedTmp = true;
            }
        }

        if (
            (clearWhenMaxMessages_pri and maxMessagesReachedTmp)
            or (clearWhenMaxUniqueMessages_pri and maxUniqueMessagesReachedTmp)
            or (clearWhenMaxLogFileSize_pri and maxLogFileSizeReachedTmp)
        )
        {
            clearLogs_f();
        }
        else
        {
            if (maxMessagesReachedTmp)
            {
                appendError_f("Max message limit reached");
                break;
            }

            if (maxUniqueMessagesReachedTmp)
            {
                appendError_f("Max unique message limit reached");
                break;
            }
        }

        QString typeStrTmp(logTypeToStrUMap_glo_sta_con.at(type_par_con));

        if (echoStdoutOnAddMessage_pri)
        {
            qtOutRef_ext() << "\nMessage: " << message_par_con << '\n'
                      << "Type: " << typeStrTmp << '\n'
                      << "DateTime: " << currentDateTimeTmp.toLocalTime().toString("yyyy-MM-dd hh:mm:ss.zzz") << '\n'
                      << "Source file: " << sourceFile_par_con << '\n'
                      << "Source function: " << sourceFunction_par_con << '\n'
                      << "Source line number: " << sourceLineNumber_par_con;
        }

        QString messageToHashTmp(message_par_con + typeStrTmp + sourceFile_par_con + sourceFunction_par_con + QString::number(sourceLineNumber_par_con));
        eines::hasher_c hasherTmp(
                    eines::hasher_c::inputType_ec::string
                    , messageToHashTmp
                    , eines::hasher_c::outputType_ec::unsignedXbitInteger
                    , eines::hasher_c::hashType_ec::XXHASH64
        );
        hasherTmp.generateHash_f();
        uint_fast64_t hashResultTmp(hasherTmp.hash64BitNumberResult_f());

        logItem_c* logItemPtrTmp(nullptr);

        //search for a the message size
        int_fast64_t messageSizeTmp(message_par_con.size() + typeStrTmp.size());
        std::unordered_map<int_fast64_t, std::unordered_map<uint_fast64_t, logItem_c>>::iterator findSizeResultTmp(messageSizeUMap_hashElementUMap_pri.find(messageSizeTmp));
        if (findSizeResultTmp not_eq messageSizeUMap_hashElementUMap_pri.end())
        {
            //search for the hash
            std::unordered_map<uint_fast64_t, logItem_c>::iterator findHashResultTmp(findSizeResultTmp->second.find(hashResultTmp));
            if (findHashResultTmp not_eq findSizeResultTmp->second.end())
            {
                if (message_par_con not_eq findHashResultTmp->second.message_f())
                {
                    qtErrRef_ext() << "\nDifferent message, same size, same hash";
                }
                logItemPtrTmp = std::addressof(findHashResultTmp->second);
            }
            else
            {
                std::pair<std::unordered_map<uint_fast64_t, logItem_c>::iterator, bool> emplaceResultTmp(
                            findSizeResultTmp->second.emplace(hashResultTmp, logItem_c(message_par_con, type_par_con, sourceFile_par_con, sourceFunction_par_con, sourceLineNumber_par_con))
                );
                logItemPtrTmp = std::addressof(emplaceResultTmp.first->second);
            }
        }
        else
        {
            std::unordered_map<uint_fast64_t, logItem_c> hashElementUMapTmp;
            //std::pair<std::unordered_map<uint_fast64_t, logItem_c>::iterator, bool> emplaceResultTmp(
                        hashElementUMapTmp.emplace(hashResultTmp, logItem_c(message_par_con, type_par_con, sourceFile_par_con, sourceFunction_par_con, sourceLineNumber_par_con));
            //);
            messageSizeUMap_hashElementUMap_pri.emplace(messageSizeTmp, hashElementUMapTmp);
            logItemPtrTmp = std::addressof(messageSizeUMap_hashElementUMap_pri.at(messageSizeTmp).at(hashResultTmp));
        }

        QMap<QDateTime, logItem_c*>::iterator dateTimeToLogItemPtrMapIteratorTmp(dateTimeToLogItemPtrMap_pri.insert(currentDateTimeTmp, logItemPtrTmp));
        resultTmp = true;

        int indexTmp(0);
        while (true)
        {
            //use 0/first index
            if (dateTimeToLogItemPtrMap_pri.firstKey() > currentDateTimeTmp)
            {
                break;
            }
            //use the size/last index+1
            if (dateTimeToLogItemPtrMap_pri.lastKey() < currentDateTimeTmp)
            {
                indexTmp = dateTimeToLogItemPtrMap_pri.size();
                break;
            }
            //get the index using distance to the iterator
            indexTmp = std::distance(dateTimeToLogItemPtrMap_pri.begin(), dateTimeToLogItemPtrMapIteratorTmp);
            break;
        }
#ifdef DEBUGJOUVEN
        //QOUT_TS("indexTmp " << QString::number(indexTmp) << endl);
#endif

        Q_EMIT messageAdded_signal(indexTmp, logItemPtrTmp, std::addressof(dateTimeToLogItemPtrMapIteratorTmp.key()));

        if (saveLogFiles_pri and isValidLogPathBaseName_pri and logTypesToSaveFile_pri.count(type_par_con) == 1)
        {
            QFile logFileTmp(currentLogFileName_pri);
            if (logFileTmp.size() >= maxLogFileByteSize_pri)
            {
                currentLogFileName_pri = getNextFilePath_f(); //logSaveDirectoryPath_pri + "/" + generateNewLogFileName_f();
                logFileTmp.setFileName(currentLogFileName_pri);
                startLogIteratorToSave_pri = dateTimeToLogItemPtrMapIteratorTmp;
                startLogIteratorToSaveSet_pri = true;
            }

            if (logFileTmp.open(QIODevice::WriteOnly))
            {
                QJsonObject jsonToSaveTmp;
                write_f(jsonToSaveTmp);
                QJsonDocument somethingJsonD(jsonToSaveTmp);
                logFileTmp.write(somethingJsonD.toJson(QJsonDocument::Indented));
            }
            else
            {
                QString errorMsgTmp("\nCould not save log entry, file " + logFileTmp.fileName() + " could not be opened");
                appendError_f(errorMsgTmp);
                if (echoStderrOnError_pri)
                {
                    qtErrRef_ext() << errorMsgTmp;
                }
            }
        }
        break;
    }
    return resultTmp;
}

int_fast64_t logDataHub_c::maxLogFileByteSize_f() const
{
    return maxLogFileByteSize_pri;
}

void logDataHub_c::setMaxLogFileByteSize_f(const int_fast64_t maxLogFileByteSize_par_con)
{
    maxLogFileByteSize_pri = maxLogFileByteSize_par_con;
}

bool logDataHub_c::loadLogFiles_f(
        const QString& logPathBaseName_par_con
        , const logFilter_c& logFilter_par_con
        , const bool onlyLoadLastLogFile_par_con
        , const bool keepUsingLastLogFile_par_con)
{
    bool resultTmp(false);
    auto currentSizeTmp(dateTimeToLogItemPtrMap_pri.size());
    while (true)
    {
        QString pathNameTmp;
        bool defaultCaseTmp(false);
        if (logPathBaseName_par_con.isEmpty())
        {
            //default case use application directory
            pathNameTmp = appDirectoryPath_f();
            defaultCaseTmp = true;
        }
        else
        {
            pathNameTmp = logPathBaseName_par_con;
        }
#ifdef DEBUGJOUVEN
        //qtOutRef_ext() << "pathNameTmp " << pathNameTmp << endl;
#endif

        QFileInfo fileInfoTmp(pathNameTmp);
        //single file case
        if (not defaultCaseTmp and fileInfoTmp.isFile() and fileInfoTmp.exists())
        {
            readLogFile_f(pathNameTmp, logFilter_par_con);
            break;
        }

        QStringList entriesTmp;
        //directory case
        if (defaultCaseTmp)
        {
            QDir parentTmp(pathNameTmp);
            entriesTmp = parentTmp.entryList({QCoreApplication::applicationName() + "*_UTC*.log"}, QDir::Files, QDir::Time);
        }
        else
        {
            QDir parentTmp(fileInfoTmp.path());
            entriesTmp = parentTmp.entryList({fileInfoTmp.fileName() + "*_UTC*.log"}, QDir::Files, QDir::Time);
        }

        if (entriesTmp.isEmpty())
        {
            appendError_f("No log files found with file base name: " + pathNameTmp);
            break;
        }

//        QCollator collatorTmp;
//        collatorTmp.setNumericMode(true);
//        std::sort(entriesTmp.begin(), entriesTmp.end(), collatorTmp);

//#ifdef DEBUGJOUVEN
//        for (const QString& filename_ite_con : entriesTmp)
//        {
//            qtOutRef_ext() << "filename_ite_con " << filename_ite_con << endl;
//        }
//#endif

        bool readLogResultTmp(false);
        if (onlyLoadLastLogFile_par_con)
        {
            readLogResultTmp = readLogFile_f(entriesTmp.first(), logFilter_par_con);
        }
        else
        {
            for (const QString& filename_ite_con : entriesTmp)
            {
                readLogResultTmp = readLogFile_f(filename_ite_con, logFilter_par_con);
            }
        }

        //keep logging in the last loaded log, use it's directory too
        if (keepUsingLastLogFile_par_con and readLogResultTmp)
        {
//#ifdef DEBUGJOUVEN
//            qtOutRef_ext() << "entriesTmp.first() " << entriesTmp.first() << endl;
//#endif
            currentLogFileName_pri = entriesTmp.first();
            QFileInfo fileInfoCurrentLogFileTmp(currentLogFileName_pri);
            logSaveDirectoryPath_pri = fileInfoCurrentLogFileTmp.path();
            isValidLogPathBaseName_pri = true;
        }
        break;
    }
    auto newSizeTmp(dateTimeToLogItemPtrMap_pri.size());
    resultTmp = newSizeTmp > currentSizeTmp;
    return resultTmp;
}

bool logDataHub_c::saveLogFiles_f() const
{
    return saveLogFiles_pri;
}

void logDataHub_c::setSaveLogFiles_f(const bool saveLogFiles_par_con)
{
    QMutexLocker lockerTmp(&addMessageMutex_pri);
    saveLogFiles_pri = saveLogFiles_par_con;
}

QString logDataHub_c::logPathBaseName_f() const
{
    return logSaveDirectoryPath_pri;
}

QString logDataHub_c::getNextFilePath_f()
{
    QString resultTmp;
    if (
        currentDateTimeValue_pri.isEmpty()
        or (currentDateTimeValue_pri not_eq QDateTime::currentDateTimeUtc().toString(logFileDatetimeFormat_pri))
    )
    {
        currentDateTimeValue_pri = QDateTime::currentDateTimeUtc().toString(logFileDatetimeFormat_pri);
    }

    resultTmp = logSaveDirectoryPath_pri + "/" + QCoreApplication::applicationName() + "_" + currentDateTimeValue_pri + "_UTC.log";
    if (QFileInfo::exists(resultTmp))
    {
        int_fast32_t indexTmp(0);
        QString commonStringTmp(logSaveDirectoryPath_pri + "/" + QCoreApplication::applicationName() + "_" + currentDateTimeValue_pri + "_UTC_");
        do
        {
            indexTmp = indexTmp + 1;
            resultTmp = commonStringTmp + QString::number(indexTmp) + ".log";
        }
        while (QFileInfo::exists(resultTmp));
    }
    return resultTmp;
}

bool logDataHub_c::readLogFile_f(
        const QString& path_par_con
        , const logFilter_c& logFilter_par_con
)
{
    bool resultTmp(false);
    while (true)
    {
        QFile fileTmp(path_par_con);
        if (not fileTmp.open(QIODevice::ReadOnly))
        {
            break;
        }
        QByteArray jsonByteArrayTmp(fileTmp.readAll());
        QJsonDocument jsonDocObjTmp(QJsonDocument::fromJson(jsonByteArrayTmp));
        if (jsonDocObjTmp.isNull())
        {
            break;
        }
        read_f(jsonDocObjTmp.object(), logFilter_par_con);
        resultTmp = true;
        break;
    }
    return resultTmp;
}

void logDataHub_c::setLogSaveDirectoryPath_f(
        const QString& logSaveDirectoryPath_par_con
        , const bool keepOldLogPathOnError_par_con
)
{
    bool errorTmp(false);
    bool isValidOldTmp(isValidLogPathBaseName_pri);
    while (true)
    {
        QString logSaveDirectoryPathTmp;
        if (logSaveDirectoryPath_par_con.isNull())
        {
            logSaveDirectoryPathTmp = "./";
        }
        else
        {
            logSaveDirectoryPathTmp = logSaveDirectoryPath_par_con;
        }

        QDir logSaveDirectoryPathDirTmp(logSaveDirectoryPathTmp);
        if (not logSaveDirectoryPathDirTmp.exists())
        {
            errorTmp = true;
            //append/print error
            QString errorMsgTmp("\nDirectory " + logSaveDirectoryPathTmp + " doesn't exists");
            appendError_f(errorMsgTmp);
            if (echoStderrOnError_pri)
            {
                qtErrRef_ext() << errorMsgTmp;
            }
            break;
        }
        else
        {
            QMutexLocker lockerTmp(&addMessageMutex_pri);
            isValidLogPathBaseName_pri = true;
            logSaveDirectoryPath_pri = logSaveDirectoryPathTmp;
        }

        //getNextFilePath_f() should solve this issue
//        QString logFilePathTmp(logSaveDirectoryPathDirTmp.path() + "/" + generateNewLogFileName_f());
//        if (QFileInfo::exists(logFilePathTmp))
//        {
//            errorTmp = true;
//            //append/print error
//            QString errorMsgTmp("\nFile " + logFilePathTmp + " already exists");
//            appendError_f(errorMsgTmp);
//            if (echoStderrOnError_pri)
//            {
//                qtErrRef_ext() << errorMsgTmp;
//            }
//            break;
//        }

        QString currentLogFileNameTmp(getNextFilePath_f());
        QFile testFileTmp(currentLogFileNameTmp);
        if (testFileTmp.open(QIODevice::WriteOnly))
        {
            QMutexLocker lockerTmp(&addMessageMutex_pri);
            currentLogFileName_pri = currentLogFileNameTmp;
            testFileTmp.remove();
        }
        else
        {
            errorTmp = true;
            //append/print error
            QString errorMsgTmp("\nCould not save log entry, file " + currentLogFileNameTmp + " could not be opened");
            appendError_f(errorMsgTmp);
            if (echoStderrOnError_pri)
            {
                qtErrRef_ext() << errorMsgTmp;
            }
        }
        break;
    }
    if (errorTmp)
    {
        if (keepOldLogPathOnError_par_con and isValidOldTmp)
        {
        }
        else
        {
            isValidLogPathBaseName_pri = false;
        }
    }
}

//logDataHub_c::logDataHub_c()
//{}

bool logDataHub_c::addMessage_f(
        const QString& message_par_con
        , const logItem_c::type_ec type_par_con
        , const QString& sourceFile_par_con
        , const QString& sourceFunction_par_con
        , const int_fast32_t sourceLineNumber_par_con)
{
    bool resultTmp(false);
    if (loggingEnabled_pri and logTypesToSave_pri.count(type_par_con) == 1)
    {
        resultTmp = addMessageInternal_f
        (
                    message_par_con
                    , type_par_con
                    , sourceFile_par_con
                    , sourceFunction_par_con
                    , sourceLineNumber_par_con
        );
    }
    return resultTmp;
}

void logDataHub_c::setEchoToStdoutOnAddMessage_f(const bool echoOnStdoutOnAddMessage_par_con)
{
    echoStdoutOnAddMessage_pri = echoOnStdoutOnAddMessage_par_con;
}

bool logDataHub_c::echoToStdoutOnAddMessage_f() const
{
    return echoStdoutOnAddMessage_pri;
}

void logDataHub_c::write_f(QJsonObject& json_par) const
{
    QJsonArray logItemArrayTmp;
    QMap<QDateTime, logItem_c*>::ConstIterator startingIteratorTmp(startLogIteratorToSaveSet_pri ? startLogIteratorToSave_pri : dateTimeToLogItemPtrMap_pri.begin());
    while (startingIteratorTmp not_eq dateTimeToLogItemPtrMap_pri.end())
    {
        logItem_c* logItemPtrTmp(startingIteratorTmp.value());
        QDateTime datetimeTmp(startingIteratorTmp.key());

        QJsonObject jsonObjectTmp;
        jsonObjectTmp["message"] = logItemPtrTmp->message_f();
        jsonObjectTmp["type"] = logTypeToStrUMap_glo_sta_con.at(logItemPtrTmp->type_f());
        jsonObjectTmp["datetime"] = datetimeTmp.toString("yyyy-MM-dd hh:mm:ss.zzz");
        jsonObjectTmp["sourceFile"] = logItemPtrTmp->sourceFileName_f();
        jsonObjectTmp["sourceFunction"] = logItemPtrTmp->sourceFunctionName_f();
        jsonObjectTmp["sourceLineNumber"] = QString::number(logItemPtrTmp->sourceLineNumber_f());

        logItemArrayTmp.append(jsonObjectTmp);
        ++startingIteratorTmp;
    }

    json_par["logItems"] = logItemArrayTmp;
}

bool logDataHub_c::filterMatch_f(
        const logFilter_c& logFilter_par_con
        , const QString& message_par_con
        , const logItem_c::type_ec typeStr_par_con
        , const QDateTime& datetime_par_con
        , const QString& sourceFile_par_con
        , const QString& sourceFunction_par_con
) const
{
    bool resultTmp(true);

    while (logFilter_par_con.anythingSet_f())
    {
        //message
        if (not logFilter_par_con.messageContainsSet_f()
            or message_par_con.contains(logFilter_par_con.messageContains_f())
        )
        {

        }
        else
        {
            resultTmp = false;
            break;
        }

        //types
        if (not logFilter_par_con.typesSet_f()
            or (
                not logFilter_par_con.types_f().empty()
                and std::find(
                    logFilter_par_con.types_f().begin()
                    , logFilter_par_con.types_f().end()
                    , typeStr_par_con) not_eq logFilter_par_con.types_f().end()
                )
        )
        {

        }
        else
        {
            resultTmp = false;
            break;
        }

        //datetime from
        if (not logFilter_par_con.dateTimeFromSet_f()
            or (
                logFilter_par_con.dateTimeFrom_f().isValid()
                and datetime_par_con >= logFilter_par_con.dateTimeFrom_f()
                )
        )
        {

        }
        else
        {
            resultTmp = false;
            break;
        }

        //datetime to
        if (not logFilter_par_con.dateTimeToSet_f()
            or (
                logFilter_par_con.dateTimeTo_f().isValid()
                and datetime_par_con <= logFilter_par_con.dateTimeTo_f()
                )
        )
        {

        }
        else
        {
            resultTmp = false;
            break;
        }

        //source file name
        if (not logFilter_par_con.sourceFileNameContainsSet_f()
            or sourceFile_par_con.contains(logFilter_par_con.sourceFileNameContains_f())
        )
        {

        }
        else
        {
            resultTmp = false;
            break;
        }

        //source function name
        if (not logFilter_par_con.sourceFunctionNameContainsSet_f()
            or sourceFunction_par_con.contains(logFilter_par_con.sourceFunctionNameContains_f())
        )
        {

        }
        else
        {
            resultTmp = false;
            break;
        }
        break;
    }

    return resultTmp;
}

logDataHub_c::logDataHub_c()
{
    qRegisterMetaType<const logItem_c*>("const logItem_c*");
}

void logDataHub_c::read_f(
        const QJsonObject& json_par_con
        , const logFilter_c& logFilter_par_con
)
{
    QJsonArray arrayTmp(json_par_con["logItems"].toArray());
    bool saveLogFilesValueTmp(saveLogFiles_pri);
    //disable saving while reading from a json file
    saveLogFiles_pri = false;
    for (const auto item_ite_con : arrayTmp)
    {
        QJsonObject actionDataJsonObject(item_ite_con.toObject());

        QString messageTmp(actionDataJsonObject["message"].toString());
        QString typeStrTmp(actionDataJsonObject["type"].toString().toLower());
        logItem_c::type_ec typeTmp(strTologTypeMap_glo_sta_con.value(typeStrTmp));
        QDateTime datetimeTmp(QDateTime::fromString(actionDataJsonObject["datetime"].toString(), "yyyy-MM-dd hh:mm:ss.zzz"));
        datetimeTmp.setTimeSpec(Qt::UTC);
        QString sourceFileTmp(actionDataJsonObject["sourceFile"].toString());
        QString sourceFunctionTmp(actionDataJsonObject["sourceFunction"].toString());
        int_fast32_t sourceLineNumberTmp(actionDataJsonObject["sourceLineNumber"].toString("0").toLongLong());

        if (
            filterMatch_f(
                logFilter_par_con
                , messageTmp
                , typeTmp
                , datetimeTmp
                , sourceFileTmp
                , sourceFunctionTmp
                )
            )
        {
            addMessageInternal_f(messageTmp, typeTmp, sourceFileTmp, sourceFunctionTmp, sourceLineNumberTmp, datetimeTmp);
        }
    }
    saveLogFiles_pri = saveLogFilesValueTmp;
}

std::vector<std::pair<const logItem_c* const, const QDateTime* const>> logDataHub_c::filter_f(
        const logFilter_c& logFilter_par_con
) const
{
    std::vector<std::pair<const logItem_c* const, const QDateTime* const>> resultTmp;
    //resultTmp.reserve(dateTimeToLogItemPtrMap_pri.size());
    QMap<QDateTime, logItem_c*>::ConstIterator startingIteratorTmp(dateTimeToLogItemPtrMap_pri.cbegin());
    while (startingIteratorTmp not_eq dateTimeToLogItemPtrMap_pri.cend())
    {
        //line index begins at 1
        logItem_c* const logItemPtrTmp(startingIteratorTmp.value());
        const QDateTime& datetimeTmp(startingIteratorTmp.key());

        if (
            filterMatch_f(
                logFilter_par_con
                , logItemPtrTmp->message_f()
                , logItemPtrTmp->type_f()
                , datetimeTmp
                , logItemPtrTmp->sourceFileName_f()
                , logItemPtrTmp->sourceFunctionName_f()
                )
            )
        {
            resultTmp.emplace_back(std::make_pair(logItemPtrTmp, std::addressof(datetimeTmp)));
        }
        ++startingIteratorTmp;
    }

    return resultTmp;
}

void logDataHub_c::clearLogs_f()
{
    QMutexLocker lockerTmp(&addMessageMutex_pri);
    dateTimeToLogItemPtrMap_pri.clear();
    messageSizeUMap_hashElementUMap_pri.clear();
    startLogIteratorToSave_pri = dateTimeToLogItemPtrMap_pri.cbegin();
    if (saveLogFiles_pri)
    {
        currentLogFileName_pri = getNextFilePath_f();
    }
    Q_EMIT clearLogs_signal();
}

std::vector<logItem_c::type_ec> logFilter_c::types_f() const
{
    return types_pri;
}

void logFilter_c::setTypes_f(const std::vector<logItem_c::type_ec>& types_par_con)
{
    types_pri = types_par_con;
}

QDateTime logFilter_c::dateTimeFrom_f() const
{
    return dateTimeFrom_pri;
}

void logFilter_c::setDatetimeFrom_f(const QDateTime& dateTimeFrom_par_con)
{
    dateTimeFrom_pri = dateTimeFrom_par_con;
}

QDateTime logFilter_c::dateTimeTo_f() const
{
    return dateTimeTo_pri;
}

void logFilter_c::setDatetimeTo_f(const QDateTime& dateTimeTo_par_con)
{
    dateTimeTo_pri = dateTimeTo_par_con;
}

QString logFilter_c::sourceFileNameContains_f() const
{
    return sourceFileNameContains_pri;
}

void logFilter_c::setSourceFileNameContains_f(const QString& fileNameContains_par_con)
{
    sourceFileNameContains_pri = fileNameContains_par_con;
}

QString logFilter_c::sourceFunctionNameContains_f() const
{
    return functionNameContains_pri;
}

void logFilter_c::setSourceFunctionNameContains_f(const QString& functionNameContains_par_con)
{
    functionNameContains_pri = functionNameContains_par_con;
}

bool logFilter_c::messageContainsSet_f() const
{
    return messageContainsSet_pri;
}

void logFilter_c::unsetMessageContains_f()
{
    messageContains_pri.clear();
    messageContainsSet_pri = false;
}

bool logFilter_c::typesSet_f() const
{
    return typesSet_pri;
}

void logFilter_c::unsetTypes_f()
{
    types_pri.clear();
    typesSet_pri = false;
}

bool logFilter_c::dateTimeFromSet_f() const
{
    return dateTimeFromSet_pri;
}

void logFilter_c::unsetDateTimeFrom_f()
{
    dateTimeFrom_pri = QDateTime();
    dateTimeFromSet_pri = false;
}

bool logFilter_c::dateTimeToSet_f() const
{
    return dateTimeToSet_pri;
}

void logFilter_c::unsetDateTimeTo_f()
{
    dateTimeTo_pri = QDateTime();
    dateTimeToSet_pri = false;
}

bool logFilter_c::sourceFileNameContainsSet_f() const
{
    return sourceFileNameContainsSet_pri;
}

void logFilter_c::unsetSourceFileNameContains_f()
{
    sourceFileNameContains_pri.clear();
    sourceFileNameContainsSet_pri = false;
}

bool logFilter_c::sourceFunctionNameContainsSet_f() const
{
    return functionNameContainsSet_pri;
}

void logFilter_c::unsetSourceFunctionNameContains_f()
{
    functionNameContains_pri.clear();
    functionNameContainsSet_pri = false;
}

bool logFilter_c::anythingSet_f() const
{
    return messageContainsSet_pri
            or typesSet_pri
            or dateTimeFromSet_pri
            or dateTimeToSet_pri
            or sourceFileNameContainsSet_pri
            or functionNameContainsSet_pri;
}

QString logFilter_c::messageContains_f() const
{
    return messageContains_pri;
}

void logFilter_c::setMessageContains_f(const QString& messageContains_par_con)
{
    messageContains_pri = messageContains_par_con;
    messageContainsSet_pri = true;
}
