/**
 * @file ssuvariables.h
 * @copyright 2013 Jolla Ltd.
 * @author Bernd Wachter <bwachter@lart.info>
 * @date 2013
 */

#ifndef _SSUVARIABLES_H
#define _SSUVARIABLES_H

#include <QObject>
#include <QSettings>
#include <QHash>

class SsuVariables: public QObject {
    Q_OBJECT

  public:
    SsuVariables();
    /**
     * Look up all variables in the specified configuration file section,
     * run them through the variable expander, and add them to the supplied
     * QHash
     */
    void resolveSection(QSettings *settings, QString section, QHash<QString, QString> *storageHash);
    /**
     * Resolve a whole string, containing several variables. Variables inside variables are allowed
     */
    QString resolveString(QString pattern, QHash<QString, QString> *variables, int recursionDepth=0);
    /**
     * Resolve variables; variable can be passed as %(var) or var
     */
    QString resolveVariable(QString variable, QHash<QString, QString> *variables);
};

#endif
