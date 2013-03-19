/**
 * @file ssudeviceinfo.cpp
 * @copyright 2013 2013 Jolla Ltd.
 * @author Bernd Wachter <bwachter@lart.info>
 * @date 2013
 */

#include <QSystemDeviceInfo>
#include <QTextStream>
#include <QDir>

#include "ssudeviceinfo.h"
#include "../constants.h"

QTM_USE_NAMESPACE

SsuDeviceInfo::SsuDeviceInfo(): QObject(){

    boardMappings = new QSettings(SSU_BOARD_MAPPING_CONFIGURATION, QSettings::IniFormat);
}

QString SsuDeviceInfo::deviceFamily(){
  QString model = deviceModel();

  if (!cachedFamily.isEmpty())
    return cachedFamily;

  cachedFamily = "UNKNOWN";

  if (boardMappings->contains("variants/" + model)) {
    model = boardMappings->value("variants/" + model).toString();
    cachedVariant = model;
  }

  if (boardMappings->contains(model + "/family"))
    cachedFamily = boardMappings->value(model + "/family").toString();

  return cachedFamily;
}

QString SsuDeviceInfo::deviceVariant(){
  if (!cachedVariant.isEmpty())
    return cachedVariant;

  cachedVariant = "";

  if (boardMappings->contains("variants/" + deviceModel())) {
    cachedVariant = boardMappings->value("variants/" + deviceModel()).toString();
  }

  return cachedVariant;
}

QString SsuDeviceInfo::deviceModel(){
  QDir dir;
  QFile procCpuinfo;
  QStringList keys;

  if (!cachedModel.isEmpty())
    return cachedModel;

  boardMappings->beginGroup("file.exists");
  keys = boardMappings->allKeys();

  // check if the device can be identified by testing for a file
  foreach (const QString &key, keys){
    QString value = boardMappings->value(key).toString();
    if (dir.exists(value)){
      cachedModel = key;
      break;
    }
  }
  boardMappings->endGroup();
  if (!cachedModel.isEmpty()) return cachedModel;

  // check if the QSystemInfo model is useful
  QSystemDeviceInfo devInfo;
  QString model = devInfo.model();
  boardMappings->beginGroup("systeminfo.equals");
  keys = boardMappings->allKeys();
  foreach (const QString &key, keys){
    QString value = boardMappings->value(key).toString();
    if (model == value){
      cachedModel = key;
      break;
    }
  }
  boardMappings->endGroup();
  if (!cachedModel.isEmpty()) return cachedModel;

  // check if the device can be identified by a string in /proc/cpuinfo
  procCpuinfo.setFileName("/proc/cpuinfo");
  procCpuinfo.open(QIODevice::ReadOnly | QIODevice::Text);
  if (procCpuinfo.isOpen()){
    QTextStream in(&procCpuinfo);
    QString cpuinfo = in.readAll();
    boardMappings->beginGroup("cpuinfo.contains");
    keys = boardMappings->allKeys();

    foreach (const QString &key, keys){
      QString value = boardMappings->value(key).toString();
      if (cpuinfo.contains(value)){
        cachedModel = key;
        break;
      }
    }
    boardMappings->endGroup();
  }
  if (!cachedModel.isEmpty()) return cachedModel;


  // check if there's a match on arch ofr generic fallback. This probably
  // only makes sense for x86
  boardMappings->beginGroup("arch.equals");
  keys = boardMappings->allKeys();

  QSettings settings(SSU_CONFIGURATION, QSettings::IniFormat);
  foreach (const QString &key, keys){
    QString value = boardMappings->value(key).toString();
    if (settings.value("arch").toString() == value){
      cachedModel = key;
      break;
    }
  }
  boardMappings->endGroup();
  if (cachedModel.isEmpty()) cachedModel = "UNKNOWN";

  return cachedModel;
}

QString SsuDeviceInfo::deviceUid(){
  QString IMEI;
  QSystemDeviceInfo devInfo;

  IMEI = devInfo.imei();

  // this might not be completely unique (or might change on reflash), but works for now
  if (IMEI == ""){
      IMEI = devInfo.uniqueDeviceID();
  }

  return IMEI;
}

bool SsuDeviceInfo::getValue(const QString& key, QString& value){
  if (boardMappings->contains(deviceVariant()+"/"+key)){
    value = boardMappings->value(deviceVariant()+"/"+key).toString();
    return true;
  }
  else if (boardMappings->contains(deviceModel()+"/"+key)){
    value = boardMappings->value(deviceModel()+"/"+key).toString();
    return true;
  }
  return false;
}
