/**
 * @file ssuurlresolver.cpp
 * @copyright 2012 Jolla Ltd.
 * @author Bernd Wachter <bernd.wachter@jollamobile.com>
 * @date 2012
 */

#include <QCoreApplication>
#include <systemd/sd-journal.h>

#include "ssuurlresolver.h"
#include "ssulog.h"

SsuUrlResolver::SsuUrlResolver(): QObject(){
  QObject::connect(this,SIGNAL(done()),
                   QCoreApplication::instance(),SLOT(quit()),
                   Qt::QueuedConnection);
}

bool SsuUrlResolver::writeCredentials(QString filePath, QString credentialsScope){
  QFile credentialsFile(filePath);
  QPair<QString, QString> credentials = ssu.credentials(credentialsScope);
  SsuLog *ssuLog = SsuLog::instance();

  if (credentials.first == "" || credentials.second == ""){
    ssuLog->print(LOG_WARNING, "Returned credentials are empty, skip writing");
    return false;
  }

  if (!credentialsFile.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)){
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

void SsuUrlResolver::run(){
  QHash<QString, QString> repoParameters;
  QString resolvedUrl, repo;
  bool isRnd = false;
  SsuLog *ssuLog = SsuLog::instance();

  PluginFrame in(std::cin);

  if (in.headerEmpty()){
    // FIXME, do something; we need at least repo header
    ssuLog->print(LOG_WARNING, "Received empty header list. Most likely your ssu setup is broken");
  }

  PluginFrame::HeaderListIterator it;
  QStringList headerList;

  /*
     hasKey() for some reason makes zypper run into timeouts, so we have
     to handle special keys here
  */
  for (it=in.headerBegin();it!=in.headerEnd();it++){
    QString first = QString::fromStdString((*it).first);
    QString second = QString::fromStdString((*it).second);

    if (first == "repo"){
      repo = second;
    } else if (first == "rnd"){
      isRnd = true;
    } else if (first == "deviceFamily"){
      repoParameters.insert(first, second);
    } else if (first == "arch"){
      repoParameters.insert(first, second);
    } else if (first == "debug"){
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

  if (isRnd || ssu.isRegistered()){
    SignalWait w;
    connect(&ssu, SIGNAL(done()), &w, SLOT(finished()));
    ssu.updateCredentials();
    w.sleep();

    // error can be found in ssu.log, so just exit
    // TODO: figure out if there's better eror handling for
    //       zypper plugins than 'blow up'
    if (ssu.error()){
      emit done();
      return;
    }
  } else
    ssuLog->print(LOG_DEBUG, "No RnD repository, and device not registered -- skipping credential update");

  // TODO: check for credentials scope required for repository; check if the file exists;
  //       compare with configuration, and dump credentials to file if necessary
  ssuLog->print(LOG_DEBUG, QString("Requesting credentials for '%1' with RND status %2...").arg(repo).arg(isRnd));
  QString credentialsScope = ssu.credentialsScope(repo, isRnd);
  if (!credentialsScope.isEmpty()){
    headerList.append(QString("credentials=%1").arg(credentialsScope));

    QFileInfo credentialsFileInfo("/etc/zypp/credentials.d/" + credentialsScope);
    if (!credentialsFileInfo.exists() ||
        credentialsFileInfo.lastModified() <= ssu.lastCredentialsUpdate()){
      writeCredentials(credentialsFileInfo.filePath(), credentialsScope);
    }
  } else
    ssuLog->print(LOG_DEBUG, "Skipping credential update due to missing credentials scope");

  if (headerList.isEmpty()){
    resolvedUrl = ssu.repoUrl(repo, isRnd, repoParameters);
  } else {
    resolvedUrl = QString("%1?%2")
      .arg(ssu.repoUrl(repo, isRnd, repoParameters))
      .arg(headerList.join("&"));
  }

  ssuLog->print(LOG_INFO, QString("%1 resolved to %2").arg(repo).arg(resolvedUrl));

  PluginFrame out("RESOLVEDURL");
  out.setBody(resolvedUrl.toStdString());
  out.writeTo(std::cout);

  emit done();
}
