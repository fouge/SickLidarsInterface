// *********************************************************************
//  created:    2014-02-11 - 16:51
//  filename:   DbtPlySickLMSManager.h
//
//  author:     Cyril Fougeray
//              Copyright Heudiasyc UMR UTC/CNRS 6599
// 
//  version:    $Id: $
//
//  purpose:    
// *********************************************************************

#ifndef DBTPLYSICKLMSMANAGER_H
#define DBTPLYSICKLMSMANAGER_H

#include <fstream>

#include "Pacpus/DbitePlayer/DbtPlyFileManager.h"
#include "../Sick/SickLMSData.h"

// Export macro for DbtPlySick DLL for Windows only
#ifdef WIN32
#   ifdef DBTPLYSICK_EXPORTS
        // make DLL
#       define DBTPLYSICK_API __declspec(dllexport)
#   else
        // use DLL
#       define DBTPLYSICK_API __declspec(dllimport)
#   endif
#else
    // On other platforms, simply ignore this 
#   define DBTPLYSICK_API 
#endif

namespace pacpus {
    
class ShMem;

class DBTPLYSICK_API DbtPlySickLMSManager
        : public DbtPlyFileManager
{
public:
    DbtPlySickLMSManager(QString name);
    ~DbtPlySickLMSManager();

protected:
    void processData(road_time_t t, road_timerange_t tr, void * buffer);
    void displayUI();

    virtual ComponentBase::COMPONENT_CONFIGURATION configureComponent(XmlComponentConfig config);
    virtual void startActivity();
    virtual void stopActivity();

private:
   // ShMem * mShMem;
   SickLMS_dbt mSickDbt;

    std::ifstream mDataFile;
    QString mDataFilename;

    QStringList mDataFilenameList;
};

} // namespace pacpus

#endif 