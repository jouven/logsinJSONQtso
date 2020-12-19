#include "logDataHub.hpp"

#include "cryptoQtso/hashQt.hpp"
#include "essentialQtso/essentialQt.hpp"
#include "translatorJSONQtso/translator.hpp"

#include <QJsonArray>
#include <QJsonDocument>
#include <QFileInfo>
#include <QDir>
#include <QVector>
#include <QThread>

//#include <QCollator>

uint_fast64_t logDataHub_c::maxMessages_f() const
{
    return maxMessages_pri;
}

void logDataHub_c::setMaxMessages_f(const uint_fast64_t& maxMessages_par_con)
{
    QMutexLocker lockerTmp(&addMessageMutex_pri);
    if (maxMessages_par_con > 0)
    {
        maxMessages_pri = maxMessages_par_con;
    }
}

uint_fast64_t logDataHub_c::maxUniqueMessages_f() const
{
    return maxUniqueMessages_pri;
}

void logDataHub_c::setMaxUniqueMessages_f(const uint_fast64_t& maxUniqueMessages_par_con)
{
    QMutexLocker lockerTmp(&addMessageMutex_pri);
    if (maxUniqueMessages_par_con > 0)
    {
        maxUniqueMessages_pri = maxUniqueMessages_par_con;
    }
}

uint_fast64_t logDataHub_c::messageCount_f() const
{
    return indexToLogItemPtrAndDatetimeMap_pri.size();
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

bool logDataHub_c::fileSaveLogTypeText_f() const
{
    return fileSaveLogTypeText_pri;
}

void logDataHub_c::setFileSaveLogTypeText_f(const bool fileSaveLogTypeText_par_con)
{
    fileSaveLogTypeText_pri = fileSaveLogTypeText_par_con;
}

translator_c*logDataHub_c::translatorPtr_f() const
{
    return translator_pri;
}

void logDataHub_c::setTranslatorPtr_f(translator_c* translatorPtr_par)
{
    translator_pri = translatorPtr_par;
}

void logDataHub_c::stopFlushTimer_f()
{
    saveFileFlushDebounce_pri.stop();
}

void logDataHub_c::startFlushTimer_f()
{
    saveFileFlushDebounce_pri.start();
}

int_fast64_t logDataHub_c::getNewLogIndex_f()
{
    return ++atomicLogIndex_pri;
}

bool logDataHub_c::addMessageInternal_f(
        const text_c& message_par_con
        , const QString& reference_par_con
        , const logItem_c::type_ec type_par_con
        , const QString& sourceFile_par_con
        , const QString& sourceFunction_par_con
        , const int_fast32_t sourceLineNumber_par_con
        , const QDateTime& dateTime_par_con
        , const QString& threadId_par_con)
{
    bool resultTmp(false);
    while (true)
    {
        if (message_par_con.empty_f())
        {
            appendError_f("Empy message");
            break;
        }

        //20191017 I guess I was being stupid, this can be solved easily using an index with an atomic integer
        //and even in a multithread situation it will come well sorted (no need for a thread name even, but it's already done so...)

        //UPDATE after creating the regular, log per text line when saving into a file feature, which is way faster than JSON,
        //tho keeping the save file open helps too, which I didn't previously, still many log entries can get the same datetime (to the millisecond)
        //and the mutex locker isn't slow enough to make a millisecond difference, even on debug.
        //As I explained before below how do you deal with multiple logs with the same datetimes?
        //which goes first or later when writing the log file? So, my solution is... add the thread name into the logs too for context

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
        const QDateTime currentDateTimeTmp(dateTime_par_con.isNull() ? QDateTime::currentDateTimeUtc() : dateTime_par_con);
        const int_fast64_t indexTmp_con(getNewLogIndex_f());

        //ONLY 1 THREAD AT TIME FROM HERE
        QMutexLocker lockerTmp(std::addressof(addMessageMutex_pri));
        //compiler complains unsigned vs signed because Qt uses signed for size... sigh
        bool maxMessagesReachedTmp(indexToLogItemPtrAndDatetimeMap_pri.size() >= maxMessages_pri);
        bool maxUniqueMessagesReachedTmp(uniqueMessageCount_f() >= maxUniqueMessages_pri);
        bool maxLogFileSizeReachedTmp(false);

        //threads should do a setObjectName prior for this to work, threadFunctionQtso does it
        //for all the threads generated from that library
        //even for "empty named" threads, it gets the internal threadId and sets it as the object name
        QString threadIdTmp(threadId_par_con.isNull() ? QThread::currentThread()->objectName() : threadId_par_con);
        if (threadIdTmp.isEmpty())
        {
            threadIdTmp = "main";
        }

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

        const QString& typeStrTmp(logItem_c::logTypeToStrUMap_pub_sta_con.at(type_par_con));

        if (echoStdoutOnAddMessage_pri)
        {
            qtStdout_f() << "\nMessage: " << message_par_con.rawReplace_f() << '\n'
                      << "Reference: " << reference_par_con << '\n'
                      << "Type: " << typeStrTmp << '\n'
                      << "DateTime: " << currentDateTimeTmp.toLocalTime().toString("yyyy-MM-dd hh:mm:ss.zzz") << '\n'
                      << "Source file: " << sourceFile_par_con << '\n'
                      << "Source function: " << sourceFunction_par_con << '\n'
                      << "Source line number: " << sourceLineNumber_par_con;
        }

        QString messageToHashTmp(message_par_con.rawReplace_f() + reference_par_con + typeStrTmp + sourceFile_par_con + sourceFunction_par_con + QString::number(sourceLineNumber_par_con));
        //minor warning, with the addition of text_c, a log message could be inserted already translated or not,
        //this means "memory saving" won't be as effective as with simple string and no translation
        //although in theory inserted message aren't translated (actonQtso and actonQtg do this),
        //it's only when fecthing log entries that translations happen (if the translator obj is set)
        hasher_c hasherTmp(
                    hasher_c::inputType_ec::string
                    , messageToHashTmp
                    , hasher_c::outputType_ec::unsignedXbitInteger
                    , hasher_c::hashType_ec::XXHASH64
        );
        hasherTmp.generateHash_f();
        uint_fast64_t hashResultTmp(hasherTmp.hash64BitNumberResult_f());

        logItem_c* logItemPtrTmp(nullptr);

        //search for a the message size
        int_fast64_t messageSizeTmp(messageToHashTmp.size());
        auto findSizeResultTmp(messageSizeUMap_hashElementUMap_pri.find(messageSizeTmp));
        if (findSizeResultTmp not_eq messageSizeUMap_hashElementUMap_pri.end())
        {
            //search for the hash
            auto findHashResultTmp(findSizeResultTmp->second.find(hashResultTmp));
            if (findHashResultTmp not_eq findSizeResultTmp->second.end())
            {
                //here is the "slower insertion less memory usage tradeoff",
                //when inserting a message that already exists in memory
                //it will always be "full compared", "worst case" it's one string comparison
                //plus two integer-keyed unordered_maps finds which are fast
                if (message_par_con == findHashResultTmp->second.message_f())
                {
                   //message being inserted has the same text as the one found, which is OK
                }
                else
                {
                    qtErrLine_f("\nDifferent message but same size and same hash, message to insert\n"
                                   + message_par_con.rawReplace_f()
                                   + "\nmessage found\n"
                                   + findHashResultTmp->second.message_f().rawReplace_f());
                    //throw;
                }
                logItemPtrTmp = std::addressof(findHashResultTmp->second);
            }
            else
            {
                std::pair<std::unordered_map<uint_fast64_t, logItem_c>::iterator, bool> emplaceResultTmp(
                            findSizeResultTmp->second.emplace(hashResultTmp, logItem_c(
                                                                  message_par_con
                                                                  , reference_par_con
                                                                  , type_par_con
                                                                  , sourceFile_par_con
                                                                  , sourceFunction_par_con
                                                                  , sourceLineNumber_par_con
                                                                  , threadIdTmp
                                                                  ))
                );
                logItemPtrTmp = std::addressof(emplaceResultTmp.first->second);
            }
        }
        else
        {
            std::unordered_map<uint_fast64_t, logItem_c> hashElementUMapTmp;
            hashElementUMapTmp.emplace(hashResultTmp, logItem_c(
                                           message_par_con
                                           , reference_par_con
                                           , type_par_con
                                           , sourceFile_par_con
                                           , sourceFunction_par_con
                                           , sourceLineNumber_par_con
                                           , threadIdTmp
            ));

            messageSizeUMap_hashElementUMap_pri.emplace(messageSizeTmp, hashElementUMapTmp);
            logItemPtrTmp = std::addressof(messageSizeUMap_hashElementUMap_pri.at(messageSizeTmp).at(hashResultTmp));
        }

        QMap<int_fast64_t, std::pair<logItem_c*, QDateTime>>::const_iterator dateTimeToLogItemPtrMapIteratorTmp(
                    indexToLogItemPtrAndDatetimeMap_pri.insert(indexTmp_con, {logItemPtrTmp, currentDateTimeTmp})
        );
        resultTmp = true;


//        //use a counter that's initialized when the class is ctored?
//        //and ignore this below
//        int indexTmp(0);
//        while (true)
//        {
//            //use 0/first index
//            if (dateTimeToLogItemPtrMap_pri.firstKey() > currentDateTimeTmp)
//            {
//                break;
//            }
//            //use the size/last index+1
//            if (dateTimeToLogItemPtrMap_pri.lastKey() <= currentDateTimeTmp)
//            {
//                indexTmp = dateTimeToLogItemPtrMap_pri.size();
//                break;
//            }
//            //get the index using distance to the iterator
//            indexTmp = std::distance(dateTimeToLogItemPtrMap_pri.begin(), dateTimeToLogItemPtrMapIteratorTmp);
//            break;
//        }
#ifdef DEBUGJOUVEN
        //QOUT_TS("logIndex_pri " << QString::number(logIndex_pri) << endl);
#endif

        Q_EMIT messageAdded_signal(indexTmp_con, logItemPtrTmp, std::addressof(dateTimeToLogItemPtrMapIteratorTmp.value().second));

        if (saveLogFiles_pri and isValidLogPathBaseName_pri and logTypesToSaveFile_pri.count(type_par_con) == 1)
        {
            if (logFile_pri.fileName().isEmpty())
            {
                logFile_pri.setFileName(currentLogFileName_pri);
            }
            else
            {

            }

            if (logFile_pri.size() >= maxLogFileByteSize_pri)
            {
                currentLogFileName_pri = getNextFilePath_f(); //logSaveDirectoryPath_pri + "/" + generateNewLogFileName_f();
                logFile_pri.setFileName(currentLogFileName_pri);
                startLogIteratorToSave_pri = dateTimeToLogItemPtrMapIteratorTmp;
                startLogIteratorToSaveSet_pri = true;
            }

            if (logFile_pri.isOpen())
            {
            }
            else
            {
                bool openErrorTmp(false);
                if (fileSaveLogTypeText_f())
                {
                    if (logFile_pri.open(QIODevice::Append))
                    {
                    }
                    else
                    {
                        openErrorTmp = true;
                    }
                }
                else
                {
                    if (logFile_pri.open(QIODevice::WriteOnly))
                    {
                    }
                    else
                    {
                        openErrorTmp = true;
                    }
                }
                if (openErrorTmp)
                {
                    QString errorMsgTmp("\nCould not save log entry, file " + logFile_pri.fileName() + " could not be opened");
                    appendError_f(errorMsgTmp);
                    if (echoStderrOnError_pri)
                    {
                        qtStderr_f() << errorMsgTmp;
                    }
                    break;
                }
            }

            if (fileSaveLogTypeText_f())
            {
                QString lineTmp(generateTextLine_f(logItemPtrTmp, currentDateTimeTmp));
                lineTmp.append('\n');
                logFile_pri.write(lineTmp.toUtf8());
            }
            else
            {
                QJsonObject jsonToSaveTmp;
                write_f(jsonToSaveTmp);
                QJsonDocument somethingJsonD(jsonToSaveTmp);

                QByteArray jsonByteArrayTmp(somethingJsonD.toJson(QJsonDocument::Indented));
                logFile_pri.resize(jsonByteArrayTmp.size());
                logFile_pri.reset();
                logFile_pri.write(jsonByteArrayTmp);
            }

            //this will debounce the flush if many logs are inserted close to each other time-wise
            Q_EMIT stopFlushTimer_signal();
            Q_EMIT startFlushTimer_signal();
            //saveFileFlushDebounce_pri.stop();
            //saveFileFlushDebounce_pri.start();
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
    auto currentSizeTmp(indexToLogItemPtrAndDatetimeMap_pri.size());
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
    auto newSizeTmp(indexToLogItemPtrAndDatetimeMap_pri.size());
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
    if (saveLogFiles_pri)
    {
    }
    else
    {
        //when disabling the log file save
        //close the file
        if (logFile_pri.isOpen())
        {
            logFile_pri.close();
        }
    }
}

QString logDataHub_c::logSaveDirectoryPath_f() const
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
        if (fileTmp.size() == 0)
        {
            resultTmp = true;
            break;
        }
        if (not fileTmp.open(QIODevice::ReadOnly))
        {
            break;
        }
        QByteArray byteArrayTmp(fileTmp.readAll());
        bool saveLogFilesValueTmp(saveLogFiles_pri);
        //disable saving while reading a log file
        saveLogFiles_pri = false;
        if (fileSaveLogTypeText_pri)
        {
            QList<QByteArray> linesTmp(byteArrayTmp.split('\n'));
            for (const QByteArray& lineTmp_ite_con : linesTmp)
            {
                //20191114 read log lines, ignore the ones that aren't read successfully
                if (readTextLine_f(lineTmp_ite_con, logFilter_par_con))
                {
                }
//                else
//                {
//                    break;
//                }
            }
        }
        else
        {
            QJsonDocument jsonDocObjTmp(QJsonDocument::fromJson(byteArrayTmp));
            if (jsonDocObjTmp.isNull())
            {
                break;
            }
            readLogsFromJsonObject_f(jsonDocObjTmp.object(), logFilter_par_con);
        }
        saveLogFiles_pri = saveLogFilesValueTmp;

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
            QString errorMsgTmp("Directory " + logSaveDirectoryPathTmp + " doesn't exist");
            appendError_f(errorMsgTmp);
            if (echoStderrOnError_pri)
            {
                qtErrLine_f(errorMsgTmp);
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
            QString errorMsgTmp("Could not save log entry, file " + currentLogFileNameTmp + " could not be opened");
            appendError_f(errorMsgTmp);
            if (echoStderrOnError_pri)
            {
                qtErrLine_f(errorMsgTmp);
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
        const text_c& message_par_con
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
                    , QString()
                    , type_par_con
                    , sourceFile_par_con
                    , sourceFunction_par_con
                    , sourceLineNumber_par_con
        );
    }
    return resultTmp;
}

bool logDataHub_c::addMessage_f(
        const text_c& message_par_con
        , const QString& reference_par_con
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
                    , reference_par_con
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

//field order (index wise) 0DateTime 1threadId 2Type 3message 4translated 5reference 6File 7Function 8Line 9Tab-indexes 10Newline-indexes
//the resulting QString doesn't include newline at the end
QString logDataHub_c::generateTextLine_f(logItem_c* logItem_par, const QDateTime& datetime_par_con) const
{
    QString resultTmp;
    resultTmp.append(datetime_par_con.toString("yyyy-MM-dd hh:mm:ss.zzz")).append('\t');
    resultTmp.append(logItem_par->threadId_f()).append('\t');
    resultTmp.append(logItem_c::logTypeToStrUMap_pub_sta_con.at(logItem_par->type_f())).append('\t');

    QString messageTmp;
    bool translatedTmp(false);
    while (true)
    {
        //this function is used to output the log to storage
        //so it must to try to translate

        //1 the message is already translated, just replace
        if (logItem_par->message_f().translated_f())
        {
            messageTmp = logItem_par->message_f().rawReplace_f();
            //resultTmp.append("T\t");
            translatedTmp = true;
            break;
        }
        //there is a translator set and the message is not translated
        //translate the message BUT don't set it as translated in memory and
        //don't replace the original message with the translation in memory
        if (translator_pri not_eq nullptr and not logItem_par->message_f().translated_f())
        {
            bool translationFoundTmp(false);
            QString translatedMessageTmp(translator_pri->translateAndReplace_f(logItem_par->message_f(), std::addressof(translationFoundTmp)));
            if (translationFoundTmp)
            {
                messageTmp = translatedMessageTmp;
                //this wouldn't allow future translation
                //i.e. if the translation language is changed, the message is already translated
                //although if I improve the translation library it could be possible chaining translations
                //logItem_par->setTranslated_f(translationFoundTmp);
                //resultTmp.append("T\t");
                translatedTmp = true;
                break;
            }
        }

        messageTmp = logItem_par->message_f().rawReplace_f();
        //resultTmp.append("U\t");
        translatedTmp = false;

        break;
    }

#ifdef DEBUGJOUVEN
    //qDebug() << "messageTmp " <<  messageTmp;
#endif

    std::vector<int_fast64_t> tabIndexsTmp;
    {
        //if there are tabs
        int newLineCountTmp(messageTmp.count('\t'));
        if (newLineCountTmp > 0)
        {
            tabIndexsTmp.reserve(newLineCountTmp);
            int lastTabIndexTmp(messageTmp.lastIndexOf('\t'));
            while (lastTabIndexTmp not_eq -1)
            {
                tabIndexsTmp.emplace_back(lastTabIndexTmp);
                messageTmp.replace(lastTabIndexTmp, 1, "HT");
                lastTabIndexTmp = messageTmp.lastIndexOf('\t');
            }
        }
    }

    std::vector<int_fast64_t> newLineIndexs;
    {
        //if there are new lines
        int newLineCountTmp(messageTmp.count('\n'));
        if (newLineCountTmp > 0)
        {
            newLineIndexs.reserve(newLineCountTmp);
            int lastNewlineIndexTmp(messageTmp.lastIndexOf('\n'));
            while (lastNewlineIndexTmp not_eq -1)
            {
                newLineIndexs.emplace_back(lastNewlineIndexTmp);
                messageTmp.replace(lastNewlineIndexTmp, 1, "LF");
                lastNewlineIndexTmp = messageTmp.lastIndexOf('\n');
            }
        }
    }
    resultTmp.append(messageTmp).append('\t');
    if (translatedTmp)
    {
        resultTmp.append("T\t");
    }
    else
    {
        resultTmp.append("U\t");
    }
    QString referenceTmp(logItem_par->reference_f());
    referenceTmp.remove('\n').remove('\t');
    resultTmp.append(referenceTmp).append('\t');
    resultTmp.append(logItem_par->sourceFileName_f()).append('\t');
    resultTmp.append(logItem_par->sourceFunctionName_f()).append('\t');
    resultTmp.append(QString::number(logItem_par->sourceLineNumber_f())).append('\t');

    //tabs first
    if (tabIndexsTmp.empty())
    {

    }
    else
    {
        for (const int tabPos_ite_con : tabIndexsTmp)
        {
            resultTmp.append(QString::number(tabPos_ite_con)).append(',');
        }
        //remove last comma
        resultTmp.chop(1);
    }
    resultTmp.append('\t');

    //newlines second
    if (newLineIndexs.empty())
    {

    }
    else
    {
        for (const int newlinePos_ite_con : newLineIndexs)
        {
            resultTmp.append(QString::number(newlinePos_ite_con)).append(',');
        }
        //remove last comma
        resultTmp.chop(1);
    }
    return resultTmp;
}

bool logDataHub_c::readTextLine_f(
        QString line_par
        , const logFilter_c& logFilter_par_con
)
{
    bool resultTmp(false);
    while (true)
    {
        //bye carriage returns
        line_par.remove('\r');
        QStringList fieldsTmp(line_par.split('\t'));
        //it should have 10 fields at least
        if (fieldsTmp.size() < 11)
        {
            appendError_f("Less than 11 fields per log line");
            break;
        }

        //field order (index wise) 0DateTime 1threadId 2Type 3message 4translated 5reference 6File 7Function 8Line 9Tab-indexes 10Newline-indexes

        const QString& typeStrTmp(fieldsTmp.at(2));
        logItem_c::type_ec typeTmp(logItem_c::strTologTypeMap_pub_sta_con.value(typeStrTmp));

        QString messageTmp(fieldsTmp.at(3));
        if (messageTmp.isEmpty())
        {
            appendError_f("Empy message log line");
            break;
        }
        //IMPORTANT order, in the writeTextLine function, message "field",
        //first the tabs are "serialized" and then the "newlines"
        //when reading it has to be done in reverse, first the newlines and then the tabs

        //add newlines
        const QString& newLinePositonsFieldStr(fieldsTmp.at(10));
        if (newLinePositonsFieldStr.isEmpty())
        {
            //message had no newlines
        }
        else
        {
            QVector<QStringRef> newLinePositionsStrVectorTmp(newLinePositonsFieldStr.splitRef(','));
            std::vector<int_fast64_t> newLinePositionsVectorTmp;
            newLinePositionsVectorTmp.reserve(newLinePositionsStrVectorTmp.size());
            for (const QStringRef& newlinePosStrRef_ite_con : newLinePositionsStrVectorTmp)
            {
                bool conversionResultTmp(false);
                int_fast64_t conversionValueTmp(newlinePosStrRef_ite_con.toLongLong(std::addressof(conversionResultTmp)));
                if (conversionResultTmp)
                {
                    newLinePositionsVectorTmp.emplace_back(conversionValueTmp);
                }
            }
            for (const int_fast64_t newlinePosition_ite_con : newLinePositionsVectorTmp)
            {
                //newline is written as LF, 2 chars, in the log file
                messageTmp.replace(newlinePosition_ite_con, 2, '\n');
            }
        }

        //add tabs
        const QString& tabPositonsFieldStr(fieldsTmp.at(9));
        if (tabPositonsFieldStr.isEmpty())
        {
            //message had no tabs
        }
        else
        {
            QVector<QStringRef> tabPositionsStrVectorTmp(tabPositonsFieldStr.splitRef(','));
            std::vector<int_fast64_t> tabPositionsVectorTmp;
            tabPositionsVectorTmp.reserve(tabPositionsStrVectorTmp.size());
            for (const QStringRef& tabPosStrRef_ite_con : tabPositionsStrVectorTmp)
            {
                bool conversionResultTmp(false);
                int_fast64_t conversionValueTmp(tabPosStrRef_ite_con.toLongLong(std::addressof(conversionResultTmp)));
                if (conversionResultTmp)
                {
                    tabPositionsVectorTmp.emplace_back(conversionValueTmp);
                }
            }
            for (const int_fast64_t tabPosition_ite_con : tabPositionsVectorTmp)
            {
                //tab is written as HT, 2 chars, in the log file
                messageTmp.replace(tabPosition_ite_con, 2, '\t');
            }
        }

        QDateTime datetimeTmp(QDateTime::fromString(fieldsTmp.at(0), "yyyy-MM-dd hh:mm:ss.zzz"));
        const QString& threadIdTmp(fieldsTmp.at(1));
        datetimeTmp.setTimeSpec(Qt::UTC);
        const QString& referenceTmp(fieldsTmp.at(5));
        const QString& sourceFileTmp(fieldsTmp.at(6));
        const QString& sourceFunctionTmp(fieldsTmp.at(7));

        if (
            filterMatch_f(
                logFilter_par_con
                , messageTmp
                , referenceTmp
                , typeTmp
                , datetimeTmp
                , sourceFileTmp
                , sourceFunctionTmp
                , threadIdTmp
                )
            )
        {
            int_fast32_t sourceLineNumberTmp(fieldsTmp.at(8).toLongLong());
            text_c textMessageTmp(messageTmp);
            if (fieldsTmp.at(4) == "T")
            {
                textMessageTmp.setTranslated_f(true);
            }
            else
            {
                //not translated
            }
            addMessageInternal_f(textMessageTmp, referenceTmp, typeTmp, sourceFileTmp, sourceFunctionTmp, sourceLineNumberTmp, datetimeTmp, threadIdTmp);
            resultTmp = true;
        }
        break;
    }
    return resultTmp;
}

void logDataHub_c::flushSaveFile_f()
{
    logFile_pri.flush();
}

void logDataHub_c::write_f(QJsonObject& json_par) const
{
    QJsonArray logItemArrayTmp;
    QMap<int_fast64_t, std::pair<logItem_c*, QDateTime>>::ConstIterator startingIteratorTmp(startLogIteratorToSaveSet_pri ? startLogIteratorToSave_pri : indexToLogItemPtrAndDatetimeMap_pri.begin());
    while (startingIteratorTmp not_eq indexToLogItemPtrAndDatetimeMap_pri.end())
    {
        logItem_c* const logItemPtrTmp(startingIteratorTmp.value().first);
        QDateTime datetimeTmp(startingIteratorTmp.value().second);

        QJsonObject jsonObjectTmp;

        QString messageTmp;
        while (true)
        {
            if (logItemPtrTmp->message_f().translated_f())
            {
                messageTmp = logItemPtrTmp->message_f().rawReplace_f();
                break;
            }
            if (translator_pri not_eq nullptr and not logItemPtrTmp->message_f().translated_f())
            {
                bool translationFoundTmp(false);
                QString translatedMessageTmp(translator_pri->translateAndReplace_f(logItemPtrTmp->message_f(), std::addressof(translationFoundTmp)));
                jsonObjectTmp["translated"] = translationFoundTmp;
                if (translationFoundTmp)
                {
                    messageTmp = translatedMessageTmp;
                    //logItemPtrTmp->setTranslated_f(translationFoundTmp);
                }
                break;
            }

            jsonObjectTmp["translated"] = false;
            messageTmp = logItemPtrTmp->message_f().rawReplace_f();

            break;
        }

        jsonObjectTmp["message"] = messageTmp;
        jsonObjectTmp["reference"] = logItemPtrTmp->reference_f();
        jsonObjectTmp["type"] = logItem_c::logTypeToStrUMap_pub_sta_con.at(logItemPtrTmp->type_f());
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
        , const QString& reference_par_con
        , const logItem_c::type_ec typeStr_par_con
        , const QDateTime& datetime_par_con
        , const QString& sourceFile_par_con
        , const QString& sourceFunction_par_con
        , const QString& threadId_par_con) const
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

        //reference
        if (not logFilter_par_con.referenceContainsSet_f()
            or reference_par_con.contains(logFilter_par_con.referenceContains_f())
        )
        {

        }
        else
        {
            resultTmp = false;
            break;
        }

        //20191107 to prevent using logFilter_par_con.types_f().cbegin() (temporary)
        auto typesTmp(logFilter_par_con.types_f());
        //types
        if (not logFilter_par_con.typesSet_f()
            or (
                not typesTmp.empty()
                and std::find(
                    typesTmp.cbegin()
                    , typesTmp.cend()
                    , typeStr_par_con) not_eq typesTmp.cend()
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

        //threadId
        if (not logFilter_par_con.threadIdContainsSet_f()
            or threadId_par_con.contains(logFilter_par_con.threadIdContains_f())
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

logDataHub_c::logDataHub_c(
        translator_c* translatorPtr_par
) : translator_pri(translatorPtr_par)
{
    qRegisterMetaType<const logItem_c*>("const logItem_c*");
    saveFileFlushDebounce_pri.setSingleShot(true);
    saveFileFlushDebounce_pri.setInterval(1000);
    QObject::connect(std::addressof(saveFileFlushDebounce_pri), &QTimer::timeout, this, &logDataHub_c::flushSaveFile_f);
    QObject::connect(this, &logDataHub_c::startFlushTimer_signal, this, &logDataHub_c::startFlushTimer_f);
    QObject::connect(this, &logDataHub_c::stopFlushTimer_signal, this, &logDataHub_c::stopFlushTimer_f);
}

void logDataHub_c::readLogsFromJsonObject_f(
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
        if (messageTmp.isEmpty())
        {
            //ignore empty message
            continue;
        }

        QString referenceTmp(actionDataJsonObject["reference"].toString());
        QString threadIdTmp(actionDataJsonObject["threadId"].toString());
        QString typeStrTmp(actionDataJsonObject["type"].toString().toLower());
        logItem_c::type_ec typeTmp(logItem_c::strTologTypeMap_pub_sta_con.value(typeStrTmp));
        QDateTime datetimeTmp(QDateTime::fromString(actionDataJsonObject["datetime"].toString(), "yyyy-MM-dd hh:mm:ss.zzz"));
        datetimeTmp.setTimeSpec(Qt::UTC);
        QString sourceFileTmp(actionDataJsonObject["sourceFile"].toString());
        QString sourceFunctionTmp(actionDataJsonObject["sourceFunction"].toString());
        int_fast32_t sourceLineNumberTmp(actionDataJsonObject["sourceLineNumber"].toString("0").toLongLong());
        bool translatedTmp(actionDataJsonObject["translated"].toBool());


        if (
            filterMatch_f(
                logFilter_par_con
                , messageTmp
                , referenceTmp
                , typeTmp
                , datetimeTmp
                , sourceFileTmp
                , sourceFunctionTmp
                , threadIdTmp
                )
            )
        {
            text_c textMessageTmp(messageTmp);
            if (translatedTmp)
            {
                textMessageTmp.setTranslated_f(true);
            }
            addMessageInternal_f(textMessageTmp, referenceTmp, typeTmp, sourceFileTmp, sourceFunctionTmp, sourceLineNumberTmp, datetimeTmp, threadIdTmp);
        }
    }
    saveLogFiles_pri = saveLogFilesValueTmp;
}

std::vector<std::pair<const logItem_c* const, const QDateTime* const> > logDataHub_c::filter_f(
        const logFilter_c& logFilter_par_con
) const
{
    std::vector<std::pair<const logItem_c* const, const QDateTime* const>> resultTmp;
    if (logFilter_par_con.anythingSet_f())
    {
        //filtering makes the result size initially unknown
    }
    else
    {
        resultTmp.reserve(indexToLogItemPtrAndDatetimeMap_pri.size());
    }
    QMap<int_fast64_t, std::pair<logItem_c*, QDateTime>>::ConstIterator startingIteratorTmp(indexToLogItemPtrAndDatetimeMap_pri.cbegin());
    while (startingIteratorTmp not_eq indexToLogItemPtrAndDatetimeMap_pri.cend())
    {
        logItem_c* const logItemPtrTmp(startingIteratorTmp.value().first);
        const QDateTime& datetimeTmp(startingIteratorTmp.value().second);

        QString messageTmp;
        while (true)
        {
            if (logItemPtrTmp->message_f().translated_f())
            {
                messageTmp = logItemPtrTmp->message_f().rawReplace_f();
                break;
            }
            if (translator_pri not_eq nullptr and not logItemPtrTmp->message_f().translated_f())
            {
                bool translationFoundTmp(false);
                QString translatedMessageTmp(translator_pri->translateAndReplace_f(logItemPtrTmp->message_f(), std::addressof(translationFoundTmp)));
                if (translationFoundTmp)
                {
                    messageTmp = translatedMessageTmp;
                    //logItemPtrTmp->setTranslated_f(translationFoundTmp);
                    break;
                }
            }

            messageTmp = logItemPtrTmp->message_f().rawReplace_f();
            break;
        }

        if (
            filterMatch_f(
                logFilter_par_con
                , messageTmp
                , logItemPtrTmp->reference_f()
                , logItemPtrTmp->type_f()
                , datetimeTmp
                , logItemPtrTmp->sourceFileName_f()
                , logItemPtrTmp->sourceFunctionName_f()
                , logItemPtrTmp->threadId_f()
                )
            )
        {
            resultTmp.emplace_back(logItemPtrTmp, std::addressof(datetimeTmp));
        }
        ++startingIteratorTmp;
    }

    return resultTmp;
}

void logDataHub_c::clearLogs_f()
{
    QMutexLocker lockerTmp(&addMessageMutex_pri);
    indexToLogItemPtrAndDatetimeMap_pri.clear();
    messageSizeUMap_hashElementUMap_pri.clear();
    startLogIteratorToSave_pri = indexToLogItemPtrAndDatetimeMap_pri.cbegin();
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

QString logFilter_c::threadIdContains_f() const
{
    return threadIdContains_pri;
}

void logFilter_c::setThreadIdContains_f(const QString& threadIdContains_par_con)
{
    threadIdContains_pri = threadIdContains_par_con;
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

bool logFilter_c::threadIdContainsSet_f() const
{
    return threadIdContainsSet_pri;
}

void logFilter_c::unsetThreadIdContains_f()
{
    threadIdContains_pri.clear();
    threadIdContainsSet_pri = false;
}

bool logFilter_c::anythingSet_f() const
{
    return messageContainsSet_pri
            or referenceContainsSet_pri
            or typesSet_pri
            or dateTimeFromSet_pri
            or dateTimeToSet_pri
            or sourceFileNameContainsSet_pri
            or functionNameContainsSet_pri
            or threadIdContainsSet_pri;
}

QString logFilter_c::referenceContains_f() const
{
    return referenceContains_pri;
}

void logFilter_c::setReferenceContains_f(const QString& referenceContains_par_con)
{
    referenceContains_pri = referenceContains_par_con;
}

bool logFilter_c::referenceContainsSet_f() const
{
    return referenceContainsSet_pri;
}

void logFilter_c::unsetReferenceContains_f()
{
    referenceContains_pri.clear();
    referenceContainsSet_pri = false;
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
