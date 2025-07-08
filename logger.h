#ifndef LOGGER_H
#define LOGGER_H

#include <QString>
#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <QMutex>

class Logger {
public:
    enum LogLevel { INFO, WARNING, ERROR };

    static Logger* instance();
    void log(const QString& message, LogLevel level);

private:
    Logger(); // Private constructor
    ~Logger();
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    QString logLevelToString(LogLevel level);
    QFile m_logFile;
    QTextStream m_logStream;
    QMutex m_mutex;

    static Logger* m_instance;
};

#endif // LOGGER_H
