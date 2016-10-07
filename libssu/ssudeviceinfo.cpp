/**
 * @file ssudeviceinfo.cpp
 * @copyright 2013 2013 Jolla Ltd.
 * @author Bernd Wachter <bwachter@lart.info>
 * @date 2013
 */


#include <QTextStream>
#include <QDir>

#include <QDBusMessage>
#include <QDBusConnection>
#include <QDBusArgument>

#include <sys/utsname.h>

#include "sandbox_p.h"
#include "ssudeviceinfo.h"
#include "ssucoreconfig_p.h"
#include "ssulog_p.h"
#include "ssuvariables_p.h"

#include "../constants.h"

SsuDeviceInfo::SsuDeviceInfo(QString model): QObject()
{

    boardMappings = new SsuSettings(SSU_BOARD_MAPPING_CONFIGURATION, SSU_BOARD_MAPPING_CONFIGURATION_DIR);
    if (!model.isEmpty())
        cachedModel = model;
}

QStringList SsuDeviceInfo::adaptationRepos()
{
    QStringList result;

    QString model = deviceVariant(true);

    if (boardMappings->contains(model + "/adaptation-repos"))
        result = boardMappings->value(model + "/adaptation-repos").toStringList();

    return result;
}

QString SsuDeviceInfo::adaptationVariables(const QString &adaptationName, QHash<QString, QString> *storageHash)
{
    SsuLog *ssuLog = SsuLog::instance();
    QStringList adaptationRepoList = adaptationRepos();
    // special handling for adaptation-repositories
    // - check if repo is in right format (adaptation\d*)
    // - check if the configuration has that many adaptation repos
    // - export the entry in the adaptation list as %(adaptation)
    // - look up variables for that adaptation, and export matching
    //   adaptation variable
    QRegExp regex("adaptation\\\d*", Qt::CaseSensitive, QRegExp::RegExp2);
    if (regex.exactMatch(adaptationName)) {
        regex.setPattern("\\\d*");
        regex.lastIndexIn(adaptationName);
        int n = regex.cap().toInt();

        if (!adaptationRepoList.isEmpty()) {
            if (adaptationRepoList.size() <= n) {
                ssuLog->print(LOG_INFO, "Note: repo index out of bounds, substituting 0" + adaptationName);
                n = 0;
            }

            QString adaptationRepo = adaptationRepoList.at(n);
            storageHash->insert("adaptation", adaptationRepo);
            ssuLog->print(LOG_DEBUG, "Found first adaptation " + adaptationName);

            QString model = deviceVariant(true);
            QHash<QString, QString> h;

            // add global variables for this model
            if (boardMappings->contains(model + "/variables")) {
                QStringList sections = boardMappings->value(model + "/variables").toStringList();
                foreach (const QString &section, sections)
                    variableSection(section, &h);
            }

            // override with variables specific to this repository
            variableSection(adaptationRepo, &h);

            QHash<QString, QString>::const_iterator i = h.constBegin();
            while (i != h.constEnd()) {
                storageHash->insert(i.key(), i.value());
                i++;
            }
        } else
            ssuLog->print(LOG_INFO, "Note: adaptation repo for invalid repo requested " + adaptationName);

        return "adaptation";
    }
    return adaptationName;
}

void SsuDeviceInfo::clearCache()
{
    cachedFamily = "";
    cachedModel = "";
    cachedVariant = "";
}

bool SsuDeviceInfo::contains(const QString &model)
{
    QString oldModel = deviceModel();
    bool found = false;

    if (!model.isEmpty()) {
        clearCache();
        setDeviceModel(model);
    }

    if (!deviceVariant(false).isEmpty())
        found = true;
    if (boardMappings->childGroups().contains(deviceModel()))
        found = true;

    if (!model.isEmpty()) {
        clearCache();
        setDeviceModel(oldModel);
    }
    return found;
}

QString SsuDeviceInfo::deviceFamily()
{
    if (!cachedFamily.isEmpty())
        return cachedFamily;

    QString model = deviceVariant(true);

    cachedFamily = "UNKNOWN";

    if (boardMappings->contains(model + "/family"))
        cachedFamily = boardMappings->value(model + "/family").toString();

    return cachedFamily;
}

QString SsuDeviceInfo::deviceVariant(bool fallback)
{
    if (!cachedVariant.isEmpty())
        return cachedVariant;

    cachedVariant = "";

    if (boardMappings->contains("variants/" + deviceModel())) {
        cachedVariant = boardMappings->value("variants/" + deviceModel()).toString();
    }

    if (cachedVariant == "" && fallback)
        return deviceModel();

    return cachedVariant;
}

QString SsuDeviceInfo::deviceModel()
{
    QDir dir;
    QFile procCpuinfo;
    QStringList keys;

    if (!cachedModel.isEmpty())
        return cachedModel;

    boardMappings->beginGroup("file.exists");
    keys = boardMappings->allKeys();

    // check if the device can be identified by testing for a file
    foreach (const QString &key, keys) {
        QString value = boardMappings->value(key).toString();
        if (dir.exists(Sandbox::map(value))) {
            cachedModel = key;
            break;
        }
    }
    boardMappings->endGroup();
    if (!cachedModel.isEmpty()) return cachedModel;

    // check if the device can be identified by a string in /proc/cpuinfo
    procCpuinfo.setFileName(Sandbox::map("/proc/cpuinfo"));
    procCpuinfo.open(QIODevice::ReadOnly | QIODevice::Text);
    if (procCpuinfo.isOpen()) {
        QTextStream in(&procCpuinfo);
        QString cpuinfo = in.readAll();
        boardMappings->beginGroup("cpuinfo.contains");
        keys = boardMappings->allKeys();

        foreach (const QString &key, keys) {
            QString value = boardMappings->value(key).toString();
            if (cpuinfo.contains(value)) {
                cachedModel = key;
                break;
            }
        }
        boardMappings->endGroup();
    }
    if (!cachedModel.isEmpty()) return cachedModel;

    // mer-hybris adaptations: /etc/hw-release MER_HA_DEVICE variable
    QString hwReleaseDevice = hwRelease()["MER_HA_DEVICE"];
    if (!hwReleaseDevice.isEmpty()) {
        boardMappings->beginGroup("hwrelease.device");
        keys = boardMappings->allKeys();

        foreach (const QString &key, keys) {
            QString value = boardMappings->value(key).toString();
            if (hwReleaseDevice == value) {
                cachedModel = key;
                break;
            }
        }
        boardMappings->endGroup();
    }
    if (!cachedModel.isEmpty()) return cachedModel;

    // check if the device can be identified by the kernel version string
    struct utsname buf;
    if (!uname(&buf)) {
        QString utsRelease(buf.release);
        boardMappings->beginGroup("uname-release.contains");
        keys = boardMappings->allKeys();

        foreach (const QString &key, keys) {
            QString value = boardMappings->value(key).toString();
            if (utsRelease.contains(value)) {
                cachedModel = key;
                break;
            }
        }
        boardMappings->endGroup();
    }
    if (!cachedModel.isEmpty()) return cachedModel;

    // check if there's a match on arch of generic fallback. This probably
    // only makes sense for x86
    boardMappings->beginGroup("arch.equals");
    keys = boardMappings->allKeys();

    SsuCoreConfig *settings = SsuCoreConfig::instance();
    foreach (const QString &key, keys) {
        QString value = boardMappings->value(key).toString();
        if (settings->value("arch").toString() == value) {
            cachedModel = key;
            break;
        }
    }
    boardMappings->endGroup();
    if (cachedModel.isEmpty()) cachedModel = "UNKNOWN";

    return cachedModel;
}

static QStringList
ofonoGetImeis()
{
    QStringList result;

    QDBusMessage reply = QDBusConnection::systemBus().call(
                             QDBusMessage::createMethodCall("org.ofono", "/",
                                                            "org.nemomobile.ofono.ModemManager", "GetIMEI"));

    QList<QVariant> arguments = reply.arguments();
    if (arguments.count() > 0) {
        result = arguments.at(0).toStringList();
    }

    return result;
}

static QStringList
getWlanMacs()
{
    // Based on QtSystems' qnetworkinfo_linux.cpp
    QStringList result;

    QStringList dirs = QDir(QLatin1String("/sys/class/net/"))
                       .entryList(QStringList() << QLatin1String("wlan*"));
    foreach (const QString &dir, dirs) {
        QFile carrier(QString("/sys/class/net/%1/address").arg(dir));
        if (carrier.open(QIODevice::ReadOnly)) {
            result.append(QString::fromLatin1(carrier.readAll().simplified().data()));
        }
    }
    return result;
}

static QString
normalizeUid(const QString &uid)
{
    // Normalize by stripping colons, dashes and making it lowercase
    return uid.trimmed().replace(":", "").replace("-", "").toLower();
}

QString SsuDeviceInfo::deviceUid()
{
    SsuLog *ssuLog = SsuLog::instance();
    QStringList imeis = ofonoGetImeis();
    if (imeis.size() > 0) {
        return imeis[0];
    }

    QStringList wlanMacs = getWlanMacs();
    if (wlanMacs.size() > 0) {
        return normalizeUid(wlanMacs[0]);
    }

    ssuLog->print(LOG_WARNING, "Could not get IMEI(s) from ofono, nor WLAN mac, trying fallback");

    // The fallback list is taken from QtSystems' qdeviceinfo_linux.cpp
    QStringList fallbackFiles;
    fallbackFiles << "/sys/devices/virtual/dmi/id/product_uuid";
    fallbackFiles << "/etc/machine-id";
    fallbackFiles << "/etc/unique-id";
    fallbackFiles << "/var/lib/dbus/machine-id";

    foreach (const QString &filename, fallbackFiles) {
        QFile machineId(filename);
        if (machineId.open(QFile::ReadOnly | QFile::Text)) {
            QTextStream in(&machineId);
            return normalizeUid(in.readAll());
        }
    }

    ssuLog->print(LOG_CRIT, "Could not read fallback UID - returning empty string");
    return "";
}

QStringList SsuDeviceInfo::disabledRepos()
{
    QStringList result;

    QString model = deviceVariant(true);

    if (boardMappings->contains(model + "/disabled-repos"))
        result = boardMappings->value(model + "/disabled-repos").toStringList();

    return result;
}

QString SsuDeviceInfo::displayName(const int type)
{
    QString model = deviceModel();
    QString variant = deviceVariant(false);
    QString value, key;


    switch (type) {
    case Ssu::DeviceManufacturer:
        key = "/deviceManufacturer";
        break;
    case Ssu::DeviceModel:
        key = "/prettyModel";
        break;
    case Ssu::DeviceDesignation:
        key = "/deviceDesignation";
        break;
    default:
        return "";
    }

    /*
     * Go through different levels of fallbacks:
     * 1. model specific setting
     * 2. variant specific setting
     * 3. global setting
     * 4. return model name, or "UNKNOWN" in case query was for manufacturer
     */

    if (boardMappings->contains(model + key))
        value = boardMappings->value(model + key).toString();
    else if (variant != "" && boardMappings->contains(variant + key))
        value = boardMappings->value(variant + key).toString();
    else if (boardMappings->contains(key))
        value = boardMappings->value(key).toString();
    else if (type != Ssu::DeviceManufacturer)
        value = model;
    else
        value = "UNKNOWN";

    return value;
}

// this half belongs into repo-manager, as it not only handles board-specific
// repositories. Right now this one looks like the better place due to the
// connection to board specific stuff, though
QStringList SsuDeviceInfo::repos(bool rnd, int filter)
{
    int adaptationCount = adaptationRepos().size();
    QStringList result;


    ///@TODO move this to a hash, containing repo and enabled|disabled
    ///      write repos with enabled/disabled to disks
    ///      for the compat functions providing a stringlist, do the filtering
    ///      run only when creating the list, based on the enabled|disabled flags
    if (filter == Ssu::NoFilter ||
            filter == Ssu::BoardFilter ||
            filter == Ssu::BoardFilterUserBlacklist) {
        // for repo names we have adaptation0, adaptation1, ..., adaptationN
        for (int i = 0; i < adaptationCount; i++)
            result.append(QString("adaptation%1").arg(i));

        // now read the release/rnd repos
        SsuSettings repoSettings(SSU_REPO_CONFIGURATION, QSettings::IniFormat);
        QString repoKey = (rnd ? "default-repos/rnd" : "default-repos/release");
        if (repoSettings.contains(repoKey))
            result.append(repoSettings.value(repoKey).toStringList());

        // TODO: add specific repos (developer, sdk, ..)

        // add device configured repos
        if (boardMappings->contains(deviceVariant(true) + "/repos"))
            result.append(boardMappings->value(deviceVariant(true) + "/repos").toStringList());

        // add device configured repos only valid for rnd and/or release
        repoKey = (rnd ? "/repos-rnd" : "/repos-release");
        if (boardMappings->contains(deviceVariant(true) + repoKey))
            result.append(boardMappings->value(deviceVariant(true) + repoKey).toStringList());

        // read the disabled repositories for this device
        // user can override repositories disabled here in the user configuration
        foreach (const QString &key, disabledRepos())
            result.removeAll(key);
    }

    result.removeDuplicates();
    return result;
}

QVariant SsuDeviceInfo::variable(QString section, const QString &key)
{
    /// @todo compat-setting as ssudeviceinfo guaranteed to prepend sections with var-;
    ///       SsuVariables does not have this guarantee. Remove from here as well.
    if (!section.startsWith("var-"))
        section = "var-" + section;

    return SsuVariables::variable(boardMappings, section, key);
}

void SsuDeviceInfo::variableSection(QString section, QHash<QString, QString> *storageHash)
{
    if (!section.startsWith("var-"))
        section = "var-" + section;

    SsuVariables::variableSection(boardMappings, section, storageHash);
}

void SsuDeviceInfo::setDeviceModel(QString model)
{
    if (model == "")
        cachedModel = "";
    else
        cachedModel = model;

    cachedFamily = "";
    cachedVariant = "";
}

QVariant SsuDeviceInfo::value(const QString &key, const QVariant &value)
{
    if (boardMappings->contains(deviceModel() + "/" + key)) {
        return boardMappings->value(deviceModel() + "/" + key);
    } else if (boardMappings->contains(deviceVariant() + "/" + key)) {
        return boardMappings->value(deviceVariant() + "/" + key);
    }

    return value;
}

QMap<QString, QString> SsuDeviceInfo::hwRelease()
{
    QMap<QString, QString> result;

    // Specification of the format, encoding is similar to /etc/os-release
    // http://www.freedesktop.org/software/systemd/man/os-release.html

    QFile hwRelease("/etc/hw-release");
    if (hwRelease.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&hwRelease);

        // "All strings should be in UTF-8 format, and non-printable characters
        // should not be used."
        in.setCodec("UTF-8");

        while (!in.atEnd()) {
            QString line = in.readLine();

            // "Lines beginning with "#" shall be ignored as comments."
            if (line.startsWith('#')) {
                continue;
            }

            QString key = line.section('=', 0, 0);
            QString value = line.section('=', 1);

            // Remove trailing whitespace in value
            value = value.trimmed();

            // POSIX.1-2001 says uppercase, digits and underscores.
            //
            // Bash uses "[a-zA-Z_]+[a-zA-Z0-9_]*", so we'll use that too,
            // as we can safely assume that "shell-compatible variable
            // assignments" means it should be compatible with bash.
            //
            // see http://stackoverflow.com/a/2821183
            // and http://stackoverflow.com/a/2821201
            if (!QRegExp("[a-zA-Z_]+[a-zA-Z0-9_]*").exactMatch(key)) {
                qWarning("Invalid key in input line: '%s'", qPrintable(line));
                continue;
            }

            // "Variable assignment values should be enclosed in double or
            // single quotes if they include spaces, semicolons or other
            // special characters outside of A-Z, a-z, 0-9."
            if (((value.at(0) == '\'') || (value.at(0) == '"'))) {
                if (value.at(0) != value.at(value.size() - 1)) {
                    qWarning("Quoting error in input line: '%s'", qPrintable(line));
                    continue;
                }

                // Remove the quotes
                value = value.mid(1, value.size() - 2);
            }

            // "If double or single quotes or backslashes are to be used within
            // variable assignments, they should be escaped with backslashes,
            // following shell style."
            value = value.replace("\\\"", "\"");
            value = value.replace("\\'", "'");
            value = value.replace("\\\\", "\\");

            result[key] = value;
        }

        hwRelease.close();
    }

    return result;
}
