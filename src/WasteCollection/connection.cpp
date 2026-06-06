#include "connection.h"

#include <QSqlError>
#include <QDebug>
#include <QFile>
#include <QTextStream>
#include <QDir>
#include <QFileInfo>
#include <QCoreApplication>
#include <QStringList>
#include <QByteArray>
#include <utility>

Connection::Connection()
{
    db = QSqlDatabase::addDatabase("QODBC");
}

Connection::~Connection()
{
    if (db.isOpen()) {
        db.close();
    }
}

Connection& Connection::createInstance()
{
    static Connection instance;
    return instance;
}

static QString findEnvFile()
{
    QStringList startPaths;
    startPaths << QDir::currentPath();
    startPaths << QCoreApplication::applicationDirPath();

    for (const QString& startPath : std::as_const(startPaths)) {
        QDir dir(startPath);

        for (int i = 0; i < 8; ++i) {
            QString envPath = dir.filePath(".env");

            if (QFileInfo::exists(envPath)) {
                return envPath;
            }

            if (!dir.cdUp()) {
                break;
            }
        }
    }

    return QString();
}

static bool loadEnvFile(const QString& filePath)
{
    QFile file(filePath);

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "Unable to open .env file:" << filePath;
        return false;
    }

    QTextStream in(&file);

    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();

        // Ignore empty lines and comments
        if (line.isEmpty() || line.startsWith("#")) {
            continue;
        }

        int equalPosition = line.indexOf("=");

        if (equalPosition == -1) {
            continue;
        }

        QString key = line.left(equalPosition).trimmed();
        QString value = line.mid(equalPosition + 1).trimmed();

        // Remove optional quotes
        if ((value.startsWith("\"") && value.endsWith("\"")) ||
            (value.startsWith("'") && value.endsWith("'"))) {
            value = value.mid(1, value.length() - 2);
        }

        QByteArray keyBytes = key.toUtf8();
        QByteArray valueBytes = value.toUtf8();

        qputenv(keyBytes.constData(), valueBytes);
    }

    file.close();
    return true;
}

bool Connection::createconnect()
{
    if (db.isOpen()) {
        return true;
    }

    static bool envLoaded = false;

    if (!envLoaded) {
        QString envPath = findEnvFile();

        if (envPath.isEmpty()) {
            qDebug() << ".env file not found.";
            return false;
        }

        if (!loadEnvFile(envPath)) {
            qDebug() << "Failed to load .env file.";
            return false;
        }

        envLoaded = true;
    }

    QString odbcSourceName = qEnvironmentVariable("ODBC_SOURCE_NAME");
    QString dbUser = qEnvironmentVariable("DB_USER");
    QString dbPassword = qEnvironmentVariable("DB_PASSWORD");

    if (odbcSourceName.isEmpty() || dbUser.isEmpty() || dbPassword.isEmpty()) {
        qDebug() << "Missing database configuration in .env file.";
        qDebug() << "Required variables: ODBC_SOURCE_NAME, DB_USER, DB_PASSWORD";
        return false;
    }

    db.setDatabaseName(odbcSourceName);
    db.setUserName(dbUser);
    db.setPassword(dbPassword);

    if (db.open()) {
        qDebug() << "Database connection established successfully.";
        return true;
    } else {
        qDebug() << "Database connection failed:" << db.lastError().text();
        return false;
    }
}
