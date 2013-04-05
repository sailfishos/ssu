/**
 * @file ssuks.cpp
 * @copyright 2013 Jolla Ltd.
 * @author Bernd Wachter <bernd.wachter@jollamobile.com>
 * @date 2013
 */

#include <QCoreApplication>
#include <QTranslator>
#include <QLocale>
#include <QLibraryInfo>
#include <QTimer>
#include <QStringList>

#include "ssukickstarter.h"
#include "constants.h"
#include "libssu/sandbox_p.h"

#include "ssuks.h"

void SsuKs::run(){
  QStringList arguments = QCoreApplication::arguments();
  // get rid of the binary name
  arguments.removeFirst();

  QTextStream qout(stdout);
  QHash<QString, QString> repoParameters;

  QString fileName;
  if (arguments.count() >= 1 && !arguments.at(0).contains("=")){
    fileName = arguments.at(0);
    arguments.removeFirst();
  }

  if (arguments.count() >= 1){
    for (int i=0; i<arguments.count(); i++){
      if (arguments.at(i).count("=") != 1){
        qout << "Invalid flag: " << arguments.at(i) << endl;
        return;
      }
      QStringList split = arguments.at(i).split("=");
      repoParameters.insert(split.at(0), split.at(1));
    }


    if (repoParameters.contains("sandbox")){
      QString sandbox = repoParameters.value("sandbox");
      repoParameters.remove("sandbox");

      qout << "Using sandbox at " << sandbox << endl;
      Sandbox *sb = new Sandbox(sandbox, Sandbox::UseAsSkeleton, Sandbox::ThisProcess);
      sb->addWorldFiles(SSU_BOARD_MAPPING_CONFIGURATION_DIR);
    }

    SsuKickstarter kickstarter;
    kickstarter.setRepoParameters(repoParameters);
    kickstarter.write(fileName);
  } else
    usage();

  QCoreApplication::exit(0);
}

void SsuKs::usage(){
  QTextStream qout(stdout);
  qout << "\nUsage: ssuks <filename> <flags>" << endl
       << endl
       << "Flags are in the form key=value. 'model', 'rnd' and 'sandbox' keys have special meanings." << endl
       << "To do a kickstart for N9 do 'ssuks model=N9'" << endl
       << endl;
  qout.flush();
  QCoreApplication::exit(1);
}

int main(int argc, char** argv){
  QCoreApplication app(argc, argv);
  QCoreApplication::setOrganizationName("Jolla");
  QCoreApplication::setOrganizationDomain("http://www.jollamobile.com");
  QCoreApplication::setApplicationName("ssuks");

  QTranslator qtTranslator;
  qtTranslator.load("qt_" + QLocale::system().name(),
                    QLibraryInfo::location(QLibraryInfo::TranslationsPath));
  app.installTranslator(&qtTranslator);

  SsuKs mw;
  QTimer::singleShot(0, &mw, SLOT(run()));

  return app.exec();
}
