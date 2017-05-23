/**
 * @file ssuurlresolver.cpp
 * @copyright 2012 Jolla Ltd.
 * @author Bernd Wachter <bernd.wachter@jollamobile.com>
 * @date 2012
 */

#include "ssuurlresolver.h"

#include <QCoreApplication>
#include <QFileInfo>
#include <QStringList>
#include <QUrl>
#include <systemd/sd-journal.h>

#include "libssu/sandbox_p.h"
#include "libssu/ssulog_p.h"

SsuUrlResolver::SsuUrlResolver()
    : QObject()
{
    QObject::connect(this, SIGNAL(done()),
                     QCoreApplication::instance(), SLOT(quit()),
                     Qt::QueuedConnection);
}

void SsuUrlResolver::error(const QString &message)
{
    SsuLog *ssuLog = SsuLog::instance();
    ssuLog->print(LOG_WARNING, message);

    PluginFrame out("ERROR");
    out.setBody(message.toStdString());
    out.writeTo(std::cout);
    QCoreApplication::exit(1);
}

bool SsuUrlResolver::writeZyppCredentialsIfNeeded(const QString &credentialsScope)
{
    QString filePath = Sandbox::map("/etc/zypp/credentials.d/" + credentialsScope);
    QFileInfo credentialsFileInfo(filePath);

    /// @TODO: add scope to lastCredentialsUpdate() to allow scope specific update
    ///        tracking
    if (credentialsFileInfo.exists() &&
            credentialsFileInfo.lastModified() > ssu.lastCredentialsUpdate() &&
            credentialsScope != "store") {
        // zypp credentials up to date
        return true;
    }

    QFile credentialsFile(filePath);
    QPair<QString, QString> credentials = ssu.credentials(credentialsScope);
    SsuLog *ssuLog = SsuLog::instance();

    if (credentials.first.isEmpty() || credentials.second.isEmpty()) {
        ssuLog->print(LOG_WARNING, "Returned credentials are empty, skip writing");
        return false;
    }

    if (!credentialsFile.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        ssuLog->print(LOG_WARNING, "Unable to open credentials file for writing");
        return false;
    }

    QTextStream out(&credentialsFile);
    out << "[" << ssu.credentialsUrl(credentialsScope) << "]\n";
    out << "username=" << credentials.first << "\n";
    out << "password=" << credentials.second << "\n";
    out.flush();
    credentialsFile.close();
    return true;
}

void SsuUrlResolver::run()
{
    QHash<QString, QString> repoParameters;
    QString repo;
    bool isRnd = false;
    SsuLog *ssuLog = SsuLog::instance();

    PluginFrame in(std::cin);

    if (in.headerEmpty()) {
        error("Received empty header list. Most likely your ssu setup is broken");
    }

    PluginFrame::HeaderListIterator it;
    QStringList headerList;

    /*
       hasKey() for some reason makes zypper run into timeouts, so we have
       to handle special keys here
    */
    for (it = in.headerBegin(); it != in.headerEnd(); it++) {
        QString first = QString::fromStdString((*it).first);
        QString second = QString::fromStdString((*it).second);

        if (first == "repo") {
            repo = second;
        } else if (first == "rnd") {
            isRnd = true;
        } else if (first == "deviceFamily") {
            repoParameters.insert(first, second);
        } else if (first == "arch") {
            repoParameters.insert(first, second);
        } else if (first == "debug") {
            repoParameters.insert("debugSplit", "debug");
        } else {
            if ((*it).second.empty())
                headerList.append(first);
            else
                headerList.append(QString("%1=%2")
                                  .arg(first)
                                  .arg(second));
        }
    }

    if (!ssu.useSslVerify())
        headerList.append("ssl_verify=no");

    if (ssu.isRegistered()) {
        SignalWait w;
        connect(&ssu, SIGNAL(done()), &w, SLOT(finished()));
        ssu.updateCredentials();
        w.sleep();

        // error can be found in ssu.log, so just exit
        // TODO: figure out if there's better eror handling for
        //       zypper plugins than 'blow up'
        if (ssu.error()) {
            error(ssu.lastError());
        }
    } else {
        ssuLog->print(LOG_DEBUG, "Device not registered -- skipping credential update");
    }

    // resolve base url
    QString resolvedUrl = ssu.repoUrl(repo, isRnd, repoParameters);

    QString credentialsScope = ssu.credentialsScope(repo, isRnd);
    // only do credentials magic on secure connections
    if (resolvedUrl.startsWith("https://") && !credentialsScope.isEmpty() &&
            (ssu.isRegistered() || credentialsScope == "store")) {
        // TODO: check for credentials scope required for repository; check if the file exists;
        //       compare with configuration, and dump credentials to file if necessary
        ssuLog->print(LOG_DEBUG, QString("Requesting credentials for '%1' with RND status %2...").arg(repo).arg(isRnd));

        // personal store repositories as well as the ones listed in the
        // store-auth-repos domain setting use store credentials. Refresh
        // here, as we only know after checking scope if we need to have
        // store credentials at all
        if (credentialsScope == "store") {
            ssu.updateStoreCredentials();
            if (ssu.error())
                error (ssu.lastError());
        }
        headerList.append(QString("credentials=%1").arg(credentialsScope));
        writeZyppCredentialsIfNeeded(credentialsScope);
    } else {
        ssuLog->print(LOG_DEBUG, QString("Skipping credential for %1 with scope %2").arg(repo).arg(credentialsScope));
    }

    if (!headerList.isEmpty() && !resolvedUrl.isEmpty()) {
        QUrl url(resolvedUrl);

        if (url.hasQuery()) {
            url.setQuery(url.query() + "&" + headerList.join("&"));
        } else
            url.setQuery(headerList.join("&"));

        resolvedUrl = url.toString();
    }

    // TODO, we should bail out here if the configuration specifies that the repo
    //       is protected, but device is not registered and/or we don't have credentials
    ssuLog->print(LOG_INFO, QString("%1 resolved to %2").arg(repo).arg(resolvedUrl));

    if (resolvedUrl.isEmpty()) {
        error("URL for repository is not set.");
    } else if (resolvedUrl.indexOf(QRegExp("[a-z]*://", Qt::CaseInsensitive)) != 0) {
        error("URL for repository is invalid.");
    } else {
        PluginFrame out("RESOLVEDURL");
        out.setBody(resolvedUrl.toStdString());
        out.writeTo(std::cout);
    }

    emit done();
}
