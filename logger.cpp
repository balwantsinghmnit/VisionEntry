#include "logger.h"
#include <QDebug>

Logger* Logger::m_instance = nullptr;

Logger::Logger()
    : m_logFile("application.log")
{
    if (m_logFile.open(QIODevice::Append | QIODevice::Text)) {
        m_logStream.setDevice(&m_logFile);
    } else {
        qWarning() << "Unable to open log file!";
    }
}

Logger::~Logger() {
    m_logFile.close();
}

Logger* Logger::instance() {
    if (!m_instance) {
        m_instance = new Logger();
    }
    return m_instance;
}

QString Logger::logLevelToString(LogLevel level) {
    switch (level) {
    case INFO: return "INFO";
    case WARNING: return "WARNING";
    case ERROR: return "ERROR";
    default: return "UNKNOWN";
    }
}

void Logger::log(const QString& message, LogLevel level) {
    QMutexLocker locker(&m_mutex); // thread-safe

    QString timeStamp = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    QString fullMessage = QString("[%1] [%2] %3").arg(timeStamp, logLevelToString(level), message);

    // Write to log file
    if (m_logStream.device()) {
        m_logStream << fullMessage << "\n";
        m_logStream.flush();
    }

    // Also print to console
    switch (level) {
    case INFO:    qInfo().noquote() << fullMessage; break;
    case WARNING: qWarning().noquote() << fullMessage; break;
    case ERROR:   qCritical().noquote() << fullMessage; break;
    }
}
