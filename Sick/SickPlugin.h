// Autogenerated file by PacpusPlugin.cmake
// DO NOT EDIT!!! ALL CHANGES WOULD BE REMOVED BY THE NEXT CALL OF CMAKE

#ifndef __SICKPLUGIN_H__
#define __SICKPLUGIN_H__

#include <QObject>
#include <qplugin.h>

#include <Pacpus/kernel/PacpusPluginInterface.h>

/// Auto-generated plugin class
class SickPlugin
    : public QObject
    , public PacpusPluginInterface
{
    Q_OBJECT
    Q_INTERFACES(PacpusPluginInterface)

public:
    SickPlugin();
    ~SickPlugin();

protected:
    QString name();
};

#endif // __SICKPLUGIN_H__
