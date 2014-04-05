// *********************************************************************
//  created:    1014/03/27 - 11:37
//  filename:   DbtPlySickLDMRSManager.cpp
//
//  authors:     Gerald Dherbomez, Cyril Fougeray    
//              Copyright Heudiasyc UMR UTC/CNRS 6599
// 
//  version:    $Id$
//
//  purpose:    
// *********************************************************************

#include "DbtPlySickLDMRSManager.h"

#include <boost/assert.hpp>
#include <iostream>
#include <string>

#include "kernel/Log.h"
#include "PacpusTools/ShMem.h"


#define UTC_MAGIC_WORD  0x55544300


namespace pacpus {

using namespace std;

DECLARE_STATIC_LOGGER("pacpus.base.DbtPlySickLDMRSManager");

/// Construction de la fabrique de composant DbtPlySickLDMRSManager
static ComponentFactory<DbtPlySickLDMRSManager> sFactory("DbtPlySickLDMRSManager");

static const char * kSickMemoryName = "sickLDMRS";

//////////////////////////////////////////////////////////////////////////
/// Constructor.
DbtPlySickLDMRSManager::DbtPlySickLDMRSManager(QString name)
    : DbtPlyFileManager(name)
{
    LOG_TRACE("constructor(" << name << ")");

    // mShMem = new ShMem(kAlaskaMemoryName, sizeof(ScanSickData));
}

//////////////////////////////////////////////////////////////////////////
/// Destructor.
DbtPlySickLDMRSManager::~DbtPlySickLDMRSManager()
{
    LOG_TRACE("destructor");
    // delete mShMem;
}

//////////////////////////////////////////////////////////////////////////
/// Configure the component.
ComponentBase::COMPONENT_CONFIGURATION DbtPlySickLDMRSManager::configureComponent(XmlComponentConfig config)
{
    DbtPlyFileManager::configureComponent(config);

    mDataFilename = param.getProperty("binFile");

    return ComponentBase::CONFIGURED_OK;
}

//////////////////////////////////////////////////////////////////////////
/// Starts the component.
void DbtPlySickLDMRSManager::startActivity()
{
    // QString dataDir = mEngine->getDataDir();

    // for (int i = 0; i < mDataFilenameList.size(); ++i) {
    //     QString file = mDataFilenameList.at(i);
        // mDataFilenameList[i] = dataDir + file;

    LOG_TRACE("DbtPlySickLDMRSManager component is starting.");

    mDataFilename = mEngine->getDataDir() + mDataFilename;

    LOG_TRACE("Opening "<< mDataFilename);

    mDataFile.open(mDataFilename.toLatin1().data(),std::ios_base::in|std::ios_base::binary);
    if (!mDataFile) {
        LOG_ERROR("cannot open file '" << mDataFilename << "'");
        return;
    }
    // }
    DbtPlyFileManager::startActivity();
}

//////////////////////////////////////////////////////////////////////////
/// Stops the component.
void DbtPlySickLDMRSManager::stopActivity()
{
    DbtPlyFileManager::stopActivity();
    mDataFile.close();
}

//////////////////////////////////////////////////////////////////////////
/// processData
void DbtPlySickLDMRSManager::processData(road_time_t t, road_timerange_t tr, void * buffer)
{
    if (!buffer) {
        LOG_DEBUG("no data available: NULL buffer");
        return;
    }

    LOG_TRACE("sizeof(sickLDMRS_dbt) = " << sizeof(SickLDMRS_dbt));
    // BOOST_ASSERT(88 == sizeof(SickLMS_dbt));
    SickLDMRS_dbt * sickLDMRS_dbt = static_cast<SickLDMRS_dbt *>(buffer);

    // // copy the values contained in the dbt file
    mSickDbt.timeStartFromSensor = sickLDMRS_dbt->timeStartFromSensor;
    mSickDbt.hScan = sickLDMRS_dbt->hScan;
    mSickDbt.dataPos = sickLDMRS_dbt->dataPos;
    mSickDbt.time = t;
    mSickDbt.timerange = tr;

    LOG_TRACE("Number of points " << mSickDbt.hScan.numPoints);

    LOG_TRACE("Reading UTC file ... ");

    mDataFile.seekg(mSickDbt.dataPos); // set the get pointer to the correct place
    
    ScanPoint* points = (ScanPoint*) malloc(mSickDbt.hScan.numPoints * sizeof(ScanPoint));

    // then copy the data contained in the binary file
    for (unsigned int i = 0 ; i < mSickDbt.hScan.numPoints ; ++i) {
        mDataFile.read(reinterpret_cast<char *>(&(points[i])), sizeof(ScanPoint));
    }


    // verify that the last value is the UTC magic word
    int32_t utcMagicWord = 0;
    mDataFile.read(reinterpret_cast<char *>(&(utcMagicWord)), sizeof(int32_t));
    if (UTC_MAGIC_WORD != utcMagicWord) {
        LOG_WARN("corrupted data, do not use them!");
        LOG_DEBUG("wrong magic word: EXPECTED=" << UTC_MAGIC_WORD << ", ACTUAL=" << utcMagicWord);
    } else {
        LOG_TRACE("writing scan ");
        LOG_WARN("NOT YET IMPLEMENTED");
        // mShMem->write(&mSickDbt, sizeof(SickLMS_dbt));

        /**********************************/
        /*      TODO : Send data !        */
        /**********************************/
    }

    if (mVerbose) {
        cout << "[SICK LDMRS]:\t"
             << "numPoints=" << mSickDbt.hScan.numPoints << "\t"
             << "time=" << t << endl
                ;
    }
    if (mVerbose >= 2)  {
        cout << "[SICK LDMRS]:\t"
             << "startAngle=" << mSickDbt.hScan.startAngle << "\t" 
             << "endAngle=" << mSickDbt.hScan.endAngle << std::endl ;
    }
    free(points);
}

//////////////////////////////////////////////////////////////////////////
/// Displays the graphical user interface (GUI)
void DbtPlySickLDMRSManager::displayUI()
{
    LOG_WARN("GUI not implemented");

    // TODO
}

} // namespace pacpus
