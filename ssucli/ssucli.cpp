/**
 * @file ssucli.cpp
 * @copyright 2012 - 2019 Jolla Ltd.
 * @copyright 2019 Open Mobile Platform LLC.
 * @copyright LGPLv2+
 * @date 2012 - 2019
 */

/*
 *  This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, see <http://www.gnu.org/licenses/>.
 *
 */
#include <QCoreApplication>

#include <termios.h>
#include <unistd.h>

#include "libssu/ssudeviceinfo.h"
#include "libssu/ssurepomanager.h"
#include "libssu/ssucoreconfig_p.h"
#include "libssu/ssuvariables_p.h"

#include <QDebug>

#include "ssucli.h"

SsuCli::SsuCli()
    : QObject()
{
    qDBusRegisterMetaType<SsuRepo>();

    connect(this, SIGNAL(done()),
            QCoreApplication::instance(), SLOT(quit()), Qt::DirectConnection);
    connect(&ssu, SIGNAL(done()),
            this, SLOT(handleResponse()));

    ssuProxy = new SsuProxy();

    connect(ssuProxy, SIGNAL(done()),
            this, SLOT(handleDBusResponse()));

    state = Idle;
}

SsuCli::~SsuCli()
{
    ssuProxy->quit();
}

void SsuCli::handleResponse()
{
    QTextStream qout(stdout);

    if (ssu.error()) {
        qout << "Last operation failed: \n" << ssu.lastError() << endl;
        QCoreApplication::exit(1);
    } else {
        qout << "Operation successful (direct)" << endl;
        QCoreApplication::exit(0);
    }
}

void SsuCli::handleDBusResponse()
{
    QTextStream qout(stdout);

    if (ssuProxy->error()) {
        qout << "Last operation failed: \n" << ssuProxy->lastError() << endl;
        QCoreApplication::exit(1);
    } else {
        qout << "Operation successful" << endl;
        QCoreApplication::exit(0);
    }
}

void SsuCli::optBrand(QStringList opt)
{
    QTextStream qout(stdout);

    if (opt.count() == 3 && opt.at(2) == "-s") {
        qout << ssu.brand();
        state = Idle;
    } else if (opt.count() == 2) {
        qout << "Device brand is: " << ssu.brand() << endl;
        state = Idle;
    }
}

void SsuCli::optDomain(QStringList opt)
{
    QTextStream qout(stdout);
    QTextStream qerr(stderr);

    if (opt.count() == 3 && opt.at(2) == "-s") {
        qout << ssu.domain();
        state = Idle;
    } else if (opt.count() > 2 && opt.at(2) == "-c") {
        if (opt.count() == 3) { // dump all domain config
            QVariantMap config = ssu.getDomainConfig(ssu.domain());
            for (QVariantMap::iterator i = config.begin(); i != config.end(); i++) {
                qout << i.key() << ": " << i.value().toString() << endl;
            }
            state = Idle;
        } else if (opt.count() == 4) { // dump one domain config value
            QVariantMap config = ssu.getDomainConfig(ssu.domain());
            qout << config.value(opt.at(3)).toString() << endl;
            state = Idle;
        } else if (opt.count() == 5) { // set one domain config value
                QVariantMap config = ssu.getDomainConfig(ssu.domain());
                config.insert(opt.at(3), opt.at(4));
                QDBusPendingReply<> reply = ssuProxy->setDomainConfig(ssu.domain(), config);
                reply.waitForFinished();
                if (reply.isError()) {
                    qerr << fallingBackToDirectUse(reply.error()) << endl;
                    ssu.setDomainConfig(ssu.domain(), config);
                }
        }
    } else if (opt.count() == 3) {
        if (ssu.listDomains().contains(opt.at(2))) {
            qout << "Changing domain from " << ssu.domain()
                             << " to " << opt.at(2) << endl;
            ssu.setDomain(opt.at(2));
        }
        else {
            qout << "Domain " << opt.at(2) << " does not exist" << endl;
            state = Idle;
        }

        state = Idle;
    } else if (opt.count() == 2) {
        qout << "Device domain is currently: " << ssu.domain() << endl;
        state = Idle;
    }
}

void SsuCli::optFlavour(QStringList opt)
{
    QTextStream qout(stdout);
    QTextStream qerr(stderr);

    if (opt.count() == 3 && opt.at(2) == "-s") {
        qout << ssu.flavour();
        state = Idle;
    } else if (opt.count() == 3) {
        qout << "Changing flavour from " << ssu.flavour()
             << " to " << opt.at(2) << endl;

        QDBusPendingReply<> reply = ssuProxy->setFlavour(opt.at(2));
        reply.waitForFinished();
        if (reply.isError()) {
            qerr << fallingBackToDirectUse(reply.error()) << endl;
            ssu.setFlavour(opt.at(2));

            SsuRepoManager repoManager;
            repoManager.update();
            uidWarning();
        }

        state = Idle;
    } else if (opt.count() == 2) {
        qout << "Device flavour is currently: " << ssu.flavour() << endl;
        state = Idle;
    }
}

QString SsuCli::getModeString(int mode) {
    QStringList modeList;

    if ((mode & Ssu::DisableRepoManager) == Ssu::DisableRepoManager)
        modeList.append("DisableRepoManager");
    if ((mode & Ssu::RndMode) == Ssu::RndMode)
        modeList.append("RndMode");
    if ((mode & Ssu::ReleaseMode) == Ssu::ReleaseMode)
        modeList.append("ReleaseMode");
    if ((mode & Ssu::LenientMode) == Ssu::LenientMode)
        modeList.append("LenientMode");
    if ((mode & Ssu::UpdateMode) == Ssu::UpdateMode)
        modeList.append("UpdateMode");
    if ((mode & Ssu::AppInstallMode) == Ssu::AppInstallMode)
        modeList.append("AppInstallMode");

    return modeList.join(" | ");
}

void SsuCli::optMode(QStringList opt)
{
    QTextStream qout(stdout);
    QTextStream qerr(stderr);

    int deviceMode = ssu.deviceMode();

    if (opt.count() == 2) {
        qout << "Device mode is: " << deviceMode
             << " (" << getModeString(deviceMode) << ")" << endl;

        if ((deviceMode & Ssu::RndMode) == Ssu::RndMode &&
                (deviceMode & Ssu::ReleaseMode) == Ssu::ReleaseMode)
            qout << "Both Release and RnD mode set, device is in RnD mode" << endl;

        state = Idle;
    } else if (opt.count() == 3 && opt.at(2) == "-s") {
        qout << deviceMode;
        state = Idle;
        return;
    } else if (opt.count() >= 3) {
        bool isInt;
        int newMode = opt.at(2).toInt(&isInt);

        if (!isInt) {
            for (int i=2; i<opt.size(); i++) {
                if (opt.at(i) == "DisableRepoManager")
                    newMode |= Ssu::DisableRepoManager;
                else if (opt.at(i) == "RndMode")
                    newMode |= Ssu::RndMode;
                else if (opt.at(i) == "ReleaseMode")
                    newMode |= Ssu::ReleaseMode;
                else if (opt.at(i) == "LenientMode")
                    newMode |= Ssu::LenientMode;
                else if (opt.at(i) == "UpdateMode")
                    newMode |= Ssu::UpdateMode;
                else if (opt.at(i) == "AppInstallMode")
                    newMode |= Ssu::AppInstallMode;
                else {
                    qout << "Unknown mode: " << opt.at(i) << endl;
                    state = Idle;
                    return;
                }
            }
        }

        qout << "Setting device mode from " << deviceMode
             << " (" << getModeString(deviceMode)
             << ") to " << newMode
             << " (" << getModeString(newMode) << ")" << endl;

        QDBusPendingReply<> reply = ssuProxy->setDeviceMode(newMode);
        reply.waitForFinished();
        if (reply.isError()) {
            qerr << fallingBackToDirectUse(reply.error()) << endl;
            ssu.setDeviceMode(Ssu::DeviceModeFlags(newMode));

            SsuRepoManager repoManager;
            repoManager.update();
            uidWarning();
        }
    }

    state = Idle;
}

void SsuCli::optModel(QStringList opt)
{
    QTextStream qout(stdout);
    SsuDeviceInfo deviceInfo;

    if (opt.count() == 3 && opt.at(2) == "-s") {
        qout << deviceInfo.deviceModel();
        state = Idle;
    } else if (opt.count() == 2) {
        qout << "Device model is: " << deviceInfo.deviceModel() << endl;
        state = Idle;
    }
}

void SsuCli::optModifyRepo(enum Actions action, QStringList opt)
{
    SsuRepoManager repoManager;
    QTextStream qerr(stderr);

    if (opt.count() == 3) {
        QDBusPendingReply<> reply = ssuProxy->modifyRepo(action, opt.at(2));
        reply.waitForFinished();
        if (reply.isError()) {
            qerr << fallingBackToDirectUse(reply.error()) << endl;

            switch (action) {
            case Add:
                repoManager.add(opt.at(2));
                repoManager.update();
                uidWarning();
                break;
            case Remove:
                repoManager.remove(opt.at(2));
                repoManager.update();
                uidWarning();
                break;
            case Disable:
                repoManager.disable(opt.at(2));
                repoManager.update();
                uidWarning();
                break;
            case Enable:
                repoManager.enable(opt.at(2));
                repoManager.update();
                uidWarning();
                break;
            }
        }
    } else if (opt.count() == 4 && action == Add) {
        QString url, repo;

        if (opt.at(2).indexOf(QRegExp("[a-z]*://", Qt::CaseInsensitive)) == 0) {
            url = opt.at(2);
            repo = opt.at(3);
        } else if (opt.at(3).indexOf(QRegExp("[a-z]*://", Qt::CaseInsensitive)) == 0) {
            url = opt.at(3);
            repo = opt.at(2);
        } else {
            qerr << "Invalid parameters for 'ssu ar': URL required." << endl;
            return;
        }

        QDBusPendingReply<> reply = ssuProxy->addRepo(repo, url);
        reply.waitForFinished();
        if (reply.isError()) {
            qerr << fallingBackToDirectUse(reply.error()) << endl;
            repoManager.add(repo, url);
            repoManager.update();
            uidWarning();
        }
    }
}

void SsuCli::optRegister(QStringList opt)
{
    /*
     * register a new device
     */

    QString username, password;
    QTextStream qin(stdin);
    QTextStream qout(stdout);
    QTextStream qerr(stderr);
    SsuCoreConfig *ssuSettings = SsuCoreConfig::instance();

    struct termios termNew, termOld;

    qout << "Username: " << flush;
    username = qin.readLine();

    tcgetattr(STDIN_FILENO, &termNew);
    termOld = termNew;
    termNew.c_lflag &= ~ECHO;
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &termNew) == -1)
        qout << "WARNING: Unable to disable echo on your terminal, password will echo!" << endl;

    qout << "Password: " << flush;
    password = qin.readLine();
    qout << endl;

    tcsetattr(STDIN_FILENO, TCSANOW, &termOld);

    if (opt.count() == 3 && opt.at(2) == "-h")
        ssuSettings->setValue("repository-url-variables/user", username);

    QDBusPendingReply<> reply = ssuProxy->registerDevice(username, password);
    reply.waitForFinished();
    if (reply.isError()) {
        qerr << fallingBackToDirectUse(reply.error()) << endl;
        ssu.sendRegistration(username, password);
    }

    state = Busy;
}

void SsuCli::optRelease(QStringList opt)
{
    QTextStream qout(stdout);
    QTextStream qerr(stderr);

    if (opt.count() == 3) {
        if (opt.at(2) == "-r") {
            qout << "Device release (RnD) is currently: " << ssu.release(true) << endl;
            state = Idle;
        } else {
            qout << "Changing release from " << ssu.release()
                 << " to " << opt.at(2) << endl;
            qout << "Your device is now in release mode!" << endl;

            QDBusPendingReply<> reply = ssuProxy->setRelease(opt.at(2), false);
            reply.waitForFinished();
            if (reply.isError()) {
                qerr << fallingBackToDirectUse(reply.error()) << endl;
                ssu.setRelease(opt.at(2));

                SsuRepoManager repoManager;
                repoManager.update();
                uidWarning();
            }

            state = Idle;
        }
    } else if (opt.count() == 2) {
        qout << "Device release is currently: " << ssu.release() << endl;
        state = Idle;
    } else if (opt.count() == 4 && opt.at(2) == "-r") {
        qout << "Changing release (RnD) from " << ssu.release(true)
             << " to " << opt.at(3) << endl;
        qout << "Your device is now in RnD mode!" << endl;

        QDBusPendingReply<> reply = ssuProxy->setRelease(opt.at(3), true);
        reply.waitForFinished();
        if (reply.isError()) {
            qerr << fallingBackToDirectUse(reply.error()) << endl;
            ssu.setRelease(opt.at(3), true);

            SsuRepoManager repoManager;
            repoManager.update();
            uidWarning();
        }

        state = Idle;
    }
}

void SsuCli::optRepos(QStringList opt)
{
    QTextStream qout(stdout);
    SsuRepoManager repoManager;
    SsuDeviceInfo deviceInfo;
    QHash<QString, QString> repoParameters, repoOverride;
    QString device;
    bool rndRepo = false;
    int micMode = 0, flagStart = 0;

    if ((ssu.deviceMode() & Ssu::RndMode) == Ssu::RndMode)
        rndRepo = true;

    if (opt.count() >= 3 && opt.at(2) == "-m") {
        micMode = 1;
        // TODO: read the default mic override variables from some config
        /*
        repoOverride.insert("release", "@RELEASE@");
        repoOverride.insert("rndRelease", "@RNDRELEASE@");
        repoOverride.insert("flavour", "@FLAVOUR@");
        repoOverride.insert("arch", "@ARCH@");
        */
    }

    if (opt.count() >= 3 + micMode) {
        // first argument is flag
        if (opt.at(2 + micMode).contains("=")) {
            flagStart = 2 + micMode;
        } else if (!opt.at(2 + micMode).contains("=") &&
                   opt.count() == 3 + micMode) {
            // first (and only) argument is device)
            device = opt.at(2 + micMode);
        } else if (!opt.at(2 + micMode).contains("=") &&
                   opt.count() > 3 + micMode &&
                   opt.at(3 + micMode).contains("=")) {
            // first argument is device, second flag
            device = opt.at(2 + micMode);
            flagStart = 3 + micMode;
        } else {
            state = UserError;
            return;
        }
    }

    if (flagStart != 0) {
        for (int i = flagStart; i < opt.count(); i++) {
            if (opt.at(i).count("=") != 1) {
                qout << "Invalid flag: " << opt.at(i) << endl;
                state = Idle;
                return;
            }
            QStringList split = opt.at(i).split("=");
            repoOverride.insert(split.at(0), split.at(1));
        }
    }

    if (repoOverride.contains("rnd")) {
        if (repoOverride.value("rnd") == "true")
            rndRepo = true;
        else if (repoOverride.value("rnd") == "false")
            rndRepo = false;
    }

    if (!device.isEmpty()) {
        deviceInfo.setDeviceModel(device);
        repoOverride.insert("model", device);
    }

    // TODO: rnd mode override needs to be implemented
    QStringList repos;

    // micMode? handle it and return, as it's a lot simpler than full mode
    if (micMode) {
        repos = repoManager.repos(rndRepo, deviceInfo, Ssu::BoardFilter);
        foreach (const QString &repo, repos) {
            QString repoName = repo;
            if (repo.endsWith("-debuginfo")) {
                repoName = repo.left(repo.size() - 10);
                repoParameters.insert("debugSplit", "debug");
            } else if (repoParameters.value("debugSplit") == "debug") {
                repoParameters.remove("debugSplit");
            }

            QString repoUrl = ssu.repoUrl(repoName, rndRepo, repoParameters, repoOverride);
            qout << "repo --name=" << repo << "-"
                 << (rndRepo ? repoOverride.value("rndRelease")
                     : repoOverride.value("release"))
                 << " --baseurl=" << repoUrl << endl;
        }
        state = Idle;
        return;
    }

    if (device.isEmpty()) {
        repos = repoManager.repos(rndRepo, deviceInfo, Ssu::BoardFilterUserBlacklist);
    } else {
        qout << "Printing repository configuration for '" << device << "'" << endl << endl;
        repos = repoManager.repos(rndRepo, deviceInfo, Ssu::BoardFilter);
    }

    SsuCoreConfig *ssuSettings = SsuCoreConfig::instance();

    qout << "Enabled repositories (global): " << endl;
    for (int i = 0; i <= 3; i++) {
        // for each repository, print repo and resolve url
        int longestField = 0;
        foreach (const QString &repo, repos)
            if (repo.length() > longestField)
                longestField = repo.length();

        qout.setFieldAlignment(QTextStream::AlignLeft);

        foreach (const QString &repo, repos) {
            QString repoName = repo;
            if (repo.endsWith("-debuginfo")) {
                repoName = repo.left(repo.size() - 10);
                repoParameters.insert("debugSplit", "debug");
            } else if (repoParameters.value("debugSplit") == "debug")
                repoParameters.remove("debugSplit");

            QString repoUrl = ssu.repoUrl(repoName, rndRepo, repoParameters, repoOverride);
            qout << " - " << qSetFieldWidth(longestField) << repo << qSetFieldWidth(0) << " ... " << repoUrl << endl;
        }

        if (i == 0) {
            if (!device.isEmpty()) {
                repos.clear();
                continue;
            }
            repos = repoManager.repos(rndRepo, deviceInfo, Ssu::UserFilter);
            qout << endl << "Enabled repositories (user): " << endl;
        } else if (i == 1) {
            repos = deviceInfo.disabledRepos();
            if (device.isEmpty())
                qout << endl << "Disabled repositories (global, might be overridden by user config): " << endl;
            else
                qout << endl << "Disabled repositories (global): " << endl;
        } else if (i == 2) {
            repos.clear();
            if (!device.isEmpty())
                continue;
            if (ssuSettings->contains("disabled-repos"))
                repos.append(ssuSettings->value("disabled-repos").toStringList());
            qout << endl << "Disabled repositories (user): " << endl;
        }
    }

    state = Idle;
}

void SsuCli::optSet(QStringList opt)
{
    QTextStream qout(stdout);
    SsuVariables var;
    SsuCoreConfig *settings = SsuCoreConfig::instance();
    QHash<QString, QString> storageHash;

    // set repository specific variable
    if (opt.count() == 5 && opt.at(2) == "-r") {
        settings->setValue("repository-url-variables/" + opt.at(3), opt.at(4));
        // clear repo specific variable
    } else if (opt.count() == 4 && opt.at(2) == "-r") {
        settings->remove("repository-url-variables/" + opt.at(3));
        // list repo specific variables
    } else if (opt.count() == 3 && opt.at(2) == "-r") {
        qout << "Repository specific variables:" << endl << endl;
        var.variableSection(settings, "repository-url-variables", &storageHash);
        // set global variable
    } else if (opt.count() == 4) {
        settings->setValue("global-variables/" + opt.at(2), opt.at(3));
        // clear global variable
    } else if (opt.count() == 3) {
        settings->remove("global-variables/" + opt.at(2));
        // list global variables
    } else if (opt.count() == 2) {
        qout << "Global variables:" << endl << endl;
        var.variableSection(settings, "global-variables", &storageHash);
    }

    settings->sync();

    if (!storageHash.isEmpty()) {
        QHash<QString, QString>::const_iterator i = storageHash.constBegin();
        while (i != storageHash.constEnd()) {
            qout << i.key() << "=" << i.value() << endl;
            i++;
        }
    }

    state = Idle;
}

void SsuCli::optStatus(QStringList opt)
{
    Q_UNUSED(opt)

    QTextStream qout(stdout);
    QTextStream qerr(stderr);
    SsuDeviceInfo deviceInfo;

    /*
     * print device information and registration status
     */

    QString deviceUid;

    QDBusPendingReply<QString> reply = ssuProxy->deviceUid();
    reply.waitForFinished();
    if (reply.isError()) {
        qerr << "DBus unavailable, UUID not necessarily connected to reality." << endl;
        deviceUid = deviceInfo.deviceUid();
    } else
        deviceUid = reply.value();

    qout << "Device registration status: "
         << (ssu.isRegistered() ? "registered" : "not registered") << endl;
    qout << "Device model: " << deviceInfo.displayName(Ssu::DeviceModel) << " ("
         << deviceInfo.deviceModel() << " / "
         << deviceInfo.displayName(Ssu::DeviceDesignation) << ")" << endl;
    if (!deviceInfo.deviceVariant().isEmpty())
        qout << "Device variant: " << deviceInfo.deviceVariant() << endl;
    qout << "Device UID: " << deviceUid << endl;
    if ((ssu.deviceMode() & Ssu::RndMode) == Ssu::RndMode)
        qout << "Release (rnd): " << ssu.release(true) << " (" << ssu.flavour() << ")" << endl;
    else
        qout << "Release: " << ssu.release() << endl;
    qout << "Domain: " <<  ssu.domain() << endl;
    qout << "Brand: " << (ssu.brand().isEmpty() ? "N/A" : ssu.brand()) << endl;

    SsuCoreConfig *settings = SsuCoreConfig::instance();
    QHash<QString, QString> storageHash;
    SsuVariables var;
    var.variableSection(settings, "global-variables", &storageHash);
    if (!storageHash.isEmpty()) {
        qout << "Global variables: " << endl;
        QHash<QString, QString>::const_iterator i = storageHash.constBegin();
        while (i != storageHash.constEnd()) {
            qout << i.key() << "=" << i.value() << endl;
            i++;
        }
    }
}

void SsuCli::optUpdateCredentials(QStringList opt)
{
    QTextStream qout(stdout);
    /*
     * update the credentials
     * optional argument: -f
     */
    bool force = false;
    if (opt.count() == 3 && opt.at(2) == "-f")
        force = true;

    if (!ssu.isRegistered()) {
        qout << "Device is not registered, can't update credentials" << endl;
        state = Idle;
        QCoreApplication::exit(1);
    } else {
        ssu.updateCredentials(force);
        state = Busy;
    }
}

void SsuCli::optUpdateRepos(QStringList opt)
{
    Q_UNUSED(opt)

    QTextStream qerr(stdout);

    QDBusPendingReply<> reply = ssuProxy->updateRepos();
    reply.waitForFinished();
    if (reply.isError()) {
        qerr << fallingBackToDirectUse(reply.error()) << endl;
        SsuRepoManager repoManager;
        repoManager.update();
        uidWarning();
    }
}

void SsuCli::run()
{
    QTextStream qerr(stderr);

    QStringList arguments = QCoreApplication::arguments();

    SsuCoreConfig *ssuSettings = SsuCoreConfig::instance();
    if (!ssuSettings->isWritable())
        qerr << "WARNING: ssu.ini does not seem to be writable. Setting values might not work." << endl;

    // make sure there's a first argument to parse
    if (arguments.count() < 2) {
        usage();
        return;
    }

    struct {
        const char *longopt; // long option name
        const char *shortopt; // option shortcut name
        int minargs; // minimum number of required args
        int maxargs; // -1 for "function will handle max args"
        void (SsuCli::*handler)(QStringList opt); // handler function
    } handlers[] = {
        // functions accepting no additional arguments
        "status", "s", 0, 0, &SsuCli::optStatus,
        "updaterepos", "ur", 0, 0, &SsuCli::optUpdateRepos,

        // functions requiring at least one argument
        "addrepo", "ar", 1, 2, &SsuCli::optAddRepo,
        "removerepo", "rr", 1, 1, &SsuCli::optRemoveRepo,
        "enablerepo", "er", 1, 1, &SsuCli::optEnableRepo,
        "disablerepo", "dr", 1, 1, &SsuCli::optDisableRepo,

        // functions accepting 0 or more arguments
        // those need to set state to Idle on success
        "register", "r", 0, -1, &SsuCli::optRegister,
        "repos", "lr", 0, -1, &SsuCli::optRepos,
        "brand", "b", 0, -1, &SsuCli::optBrand,
        "flavour", "fl", 0, -1, &SsuCli::optFlavour,
        "mode", "m", 0, -1, &SsuCli::optMode,
        "model", "mo", 0, -1, &SsuCli::optModel,
        "release", "re", 0, -1, &SsuCli::optRelease,
        "update", "up", 0, -1, &SsuCli::optUpdateCredentials,
        "domain", "do", 0, -1, &SsuCli::optDomain,
        "set", "set", 0, -1, &SsuCli::optSet,
    };

    bool found = false;
    int argc = arguments.count() - 2;

    for (unsigned int i = 0; i < sizeof(handlers) / sizeof(handlers[0]); i++) {
        if ((arguments.at(1) != handlers[i].longopt) &&
                (arguments.at(1) != handlers[i].shortopt)) {
            continue;
        }

        if (argc < handlers[i].minargs) {
            usage(QString("%1: Too few arguments").arg(handlers[i].longopt));
            return;
        }

        if (handlers[i].maxargs != -1 && argc > handlers[i].maxargs) {
            usage(QString("%1: Too many arguments").arg(handlers[i].longopt));
            return;
        }

        // Call option handler
        (this->*(handlers[i].handler))(arguments);

        found = true;
        break;
    }

    if (!found)
        state = UserError;

    // functions that need to wait for a response from ssu should set a flag so
    // we can do default exit catchall here
    if (state == Idle)
        QCoreApplication::exit(0);
    else if (state == UserError)
        usage();
}

void SsuCli::uidWarning()
{
    if (geteuid() != 0) {
        QTextStream qout(stderr);
        qout << "You're not root. Run 'ssu ur' as root to recreate repository files" << endl;
    }
}

void SsuCli::usage(const QString &message)
{
    QTextStream qout(stderr);
    qout << "\nUsage: ssu <command> [-command-options] [arguments]" << endl
         << endl
         << "Repository management:" << endl
         << "\tmode, m                \tShow repository mode information." << endl
         << "\t           <int>       \tSet repository mode as an integer (see docs)." << endl
         << "\t           <modes>     \tCombo of modes to set from: DisableRepoManager RndMode " << endl
         << "\t                       \tReleaseMode LenientMode UpdateMode AppInstallMode" << endl
         << "\tupdaterepos, ur        \tupdate repository files" << endl
         << "\trepos, lr              \tlist configured repositories" << endl
         << "\t           [-m]        \tformat output suitable for kickstart" << endl
         << "\t           [device]    \tuse repos for 'device'" << endl
         << "\t           [flags]     \tadditional flags" << endl
         << "\t           rnd=<bool>  \tset rnd or release mode (default: take from host)" << endl
         << "\taddrepo, ar <repo>     \tadd this repository" << endl
         << "\t           [url]       \tspecify URL, if not configured" << endl
         << "\tremoverepo, rr <repo>  \tremove this repository from configuration" << endl
         << "\tenablerepo, er <repo>  \tenable this repository" << endl
         << "\tdisablerepo, dr <repo> \tdisable this repository" << endl
         << endl
         << "Configuration management:" << endl
         << "\tflavour, fl          \tdisplay flavour used (RnD only)" << endl
         << "\t  [newflavour]       \tset new flavour" << endl
         << "\trelease, re          \tdisplay release used" << endl
         << "\t  [-r]               \tuse RnD release" << endl
         << "\t  [newrelease]       \tset new (RnD)release" << endl
         << "\tset                  \tdisplay global variables" << endl
         << "\t  [-r]               \toperate on repository only variables" << endl
         << "\t  <variable>         \tdisplay value of <variable>" << endl
         << "\t  <variable> <value> \tset value of <variable> to <value>" << endl
         << "\tdomain do            \tdisplay current device domain" << endl
         << "\t  [newdomain]        \tset new domain" << endl
         << "\t  [-c]               \tshow domain configuration" << endl
         << "\t  -c <variable>      \tshow single domain variable" << endl
         << "\t  -c <variable> <val>\tset single domain variable" << endl
         << endl
         << "Device management:" << endl
         << "\tstatus, s     \tprint registration status and device information" << endl
         << "\tregister, r   \tregister this device" << endl
         << "\t      [-h]    \tconfigure user for OBS home" << endl
         << "\tupdate, up    \tupdate repository credentials" << endl
         << "\t      [-f]    \tforce update" << endl
         << "\tmodel, mo     \tprint name of device model (like N9)" << endl
         << "\tbrand, b      \tprint brand of device model" << endl
         << endl;
    if (!message.isEmpty())
        qout << message << endl;
    qout.flush();
    QCoreApplication::exit(1);
}

QString SsuCli::fallingBackToDirectUse(const QDBusError &reason) const
{
    if (reason.type() == QDBusError::Disconnected)
        return QStringLiteral("DBus unavailable, falling back to libssu");
    else
        return QStringLiteral("WARNING: DBus call failed, falling back to libssu: ") + reason.message();
}
