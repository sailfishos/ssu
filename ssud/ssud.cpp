/**
 * @file ssud.cpp
 * @copyright 2013 - 2019 Jolla Ltd.
 * @copyright 2019 Open Mobile Platform LLC.
 * @copyright LGPLv2+
 * @date 2013 - 2019
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

#include "ssud.h"
#include "ssu_adaptor.h"

#include "libssu/ssudeviceinfo.h"
#include "libssu/ssurepomanager.h"

#include <QDBusConnection>

const char *Ssud::SERVICE_NAME = "org.nemo.ssu";
const char *Ssud::OBJECT_PATH = "/org/nemo/ssu";

// prepare for controlled suicide on boredom
static const int AUTOCLOSE_TIMEOUT_MS = 180 * 1000;
static const int SHORT_AUTOCLOSE_TIMEOUT_MS = 5 * 1000;

Ssud::Ssud(QObject *parent)
    : QObject(parent)
{
    qDBusRegisterMetaType<SsuRepo>();
    qDBusRegisterMetaType<QList<SsuRepo>>();
    QDBusConnection connection = QDBusConnection::systemBus();
    if (!connection.registerObject(OBJECT_PATH, this)) {
        qFatal("Cannot register object at %s", OBJECT_PATH);
    }

    if (!connection.registerService(SERVICE_NAME)) {
        qFatal("Cannot register D-Bus service at %s", SERVICE_NAME);
    }

    autoclose.setSingleShot(true);

    connect(&autoclose, &QTimer::timeout,
            QCoreApplication::instance(), &QCoreApplication::quit);

    new SsuAdaptor(this);

    connect(&ssu, SIGNAL(done()),
            this, SIGNAL(done()));
    connect(&ssu, SIGNAL(credentialsChanged()),
            this, SIGNAL(credentialsChanged()));
    connect(&ssu, SIGNAL(registrationStatusChanged()),
            this, SIGNAL(registrationStatusChanged()));

    // a cry for help everytime we do something to prevent suicide
    startAutoclose();
}

Ssud::~Ssud()
{
}

QString Ssud::brand()
{
    startAutoclose();
    return ssu.brand();
}

QString Ssud::deviceModel()
{
    SsuDeviceInfo deviceInfo;

    startAutoclose();
    return deviceInfo.deviceModel();
}

QString Ssud::deviceFamily()
{
    SsuDeviceInfo deviceInfo;

    startAutoclose();
    return deviceInfo.deviceFamily();
}

QString Ssud::deviceUid()
{
    SsuDeviceInfo deviceInfo;

    startAutoclose();
    return deviceInfo.deviceUid();
}

QString Ssud::deviceVariant()
{
    SsuDeviceInfo deviceInfo;

    startAutoclose();
    return deviceInfo.deviceVariant();
}

QString Ssud::displayName(int type)
{
    SsuDeviceInfo deviceInfo;

    startAutoclose();
    return deviceInfo.displayName(type);
}

bool Ssud::error()
{
    startAutoclose();
    return ssu.error();
}

QString Ssud::lastError()
{
    startAutoclose();
    return ssu.lastError();
}

void Ssud::quit()
{
    if (!autoclose.isActive() || autoclose.remainingTime() > SHORT_AUTOCLOSE_TIMEOUT_MS) {
        autoclose.setInterval(SHORT_AUTOCLOSE_TIMEOUT_MS);
        autoclose.start();
    }
}

bool Ssud::isRegistered()
{
    startAutoclose();
    return ssu.isRegistered();
}

void Ssud::registerDevice(const QString &username, const QString &password)
{
    autoclose.stop();
    ssu.sendRegistration(username, password);
    startAutoclose();
}

void Ssud::unregisterDevice()
{
    startAutoclose();
    ssu.unregister();
};

QString Ssud::domain()
{
    startAutoclose();
    return ssu.domain();
}

// called by DBus Adaptor, return integer instead of enum Ssu::DeviceModeFlags
int Ssud::deviceMode()
{
    startAutoclose();
    return ssu.deviceMode();
}

void Ssud::setDeviceMode(int mode)
{
    setDeviceMode(mode, Ssu::Replace);
}

void Ssud::setDeviceMode(int mode, int editMode)
{
    ssu.setDeviceMode(
        Ssu::DeviceModeFlags(mode),
        Ssu::EditMode(editMode)
    );

    SsuRepoManager repoManager;
    repoManager.update();
    startAutoclose();
}

QString Ssud::flavour()
{
    startAutoclose();
    return ssu.flavour();
}

void Ssud::setFlavour(const QString &flavour)
{
    ssu.setFlavour(flavour);

    SsuRepoManager repoManager;
    repoManager.update();
    startAutoclose();
}

QString Ssud::release(bool rnd)
{
    startAutoclose();
    return ssu.release(rnd);
}

void Ssud::setRelease(const QString &release, bool rnd)
{
    ssu.setRelease(release, rnd);

    SsuRepoManager repoManager;
    repoManager.update();
    startAutoclose();
}

void Ssud::modifyRepo(int action, const QString &repo)
{
    SsuRepoManager repoManager;

    autoclose.stop();

    switch (action) {
    case Add:
        repoManager.add(repo);
        break;
    case Remove:
        repoManager.remove(repo);
        break;
    case Disable:
        repoManager.disable(repo);
        break;
    case Enable:
        repoManager.enable(repo);
        break;
    }

    repoManager.update();
    startAutoclose();
}

void Ssud::addRepo(const QString &repo, const QString &url)
{
    SsuRepoManager repoManager;
    repoManager.add(repo, url);
    repoManager.update();
    startAutoclose();
}

void Ssud::updateRepos()
{
    SsuRepoManager repoManager;
    autoclose.stop();
    repoManager.update();
    startAutoclose();
}

QList<SsuRepo> Ssud::listRepos(bool rnd)
{
    QList<SsuRepo> reposList;
    SsuRepoManager repoManager;

    for (const QString &repo : repoManager.repos(rnd)) {
        const QString repoUrl = ssu.repoUrl(repo, rnd);

        SsuRepo ssuRepo;
        ssuRepo.name = repo;
        ssuRepo.url = repoUrl;

        reposList.append(ssuRepo);
    }
    startAutoclose();
    return reposList;
}

QStringList Ssud::listDomains()
{
    startAutoclose();
    return ssu.listDomains();
}

void Ssud::setDomainConfig(const QString &domain, QVariantMap config)
{
    ssu.setDomainConfig(domain, config);
    startAutoclose();
}

QVariantMap Ssud::getDomainConfig(const QString &domain)
{
    startAutoclose();
    return ssu.getDomainConfig(domain);
}

void Ssud::startAutoclose()
{
    autoclose.setInterval(AUTOCLOSE_TIMEOUT_MS);
    autoclose.start();
}
