// *********************************************************************
//  created:    1014/03/13 - 16:49
//  filename:   DbtPlySickLMSManager.cpp
//
//  authors:     Gerald Dherbomez, Cyril Fougeray    
//              Copyright Heudiasyc UMR UTC/CNRS 6599
// 
//  version:    $Id$
//
//  purpose:    
// *********************************************************************

#include "DbtPlySickLMSManager.h"

#include <boost/assert.hpp>
#include <iostream>
#include <string>

#include "kernel/Log.h"
#include "PacpusTools/ShMem.h"


#define UTC_MAGIC_WORD  0x55544300


namespace pacpus {

using namespace std;

DECLARE_STATIC_LOGGER("pacpus.base.DbtPlySickLMSManager");

/// Construction de la fabrique de composant DbtPlySickLMSManager
static ComponentFactory<DbtPlySickLMSManager> sFactory("DbtPlySickLMSManager");

static const char * kSickMemoryName = "sickLMS";

//////////////////////////////////////////////////////////////////////////
/// Constructor.
DbtPlySickLMSManager::DbtPlySickLMSManager(QString name)
    : DbtPlyFileManager(name)
{
    LOG_TRACE("constructor(" << name << ")");

    // mShMem = new ShMem(kAlaskaMemoryName, sizeof(ScanSickData));
}

//////////////////////////////////////////////////////////////////////////
/// Destructor.
DbtPlySickLMSManager::~DbtPlySickLMSManager()
{
    LOG_TRACE("destructor");
    // delete mShMem;
}

//////////////////////////////////////////////////////////////////////////
/// Configure the component.
ComponentBase::COMPONENT_CONFIGURATION DbtPlySickLMSManager::configureComponent(XmlComponentConfig config)
{
    DbtPlyFileManager::configureComponent(config);

    mDataFilename = param.getProperty("binFile");

    return ComponentBase::CONFIGURED_OK;
}

//////////////////////////////////////////////////////////////////////////
/// Starts the component.
void DbtPlySickLMSManager::startActivity()
{
    // QString dataDir = mEngine->getDataDir();

    // for (int i = 0; i < mDataFilenameList.size(); ++i) {
    //     QString file = mDataFilenameList.at(i);
        // mDataFilenameList[i] = dataDir + file;

    LOG_TRACE("DbtPlySickLMSManager component is starting.");

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
void DbtPlySickLMSManager::stopActivity()
{
    DbtPlyFileManager::stopActivity();
    mDataFile.close();
}

//////////////////////////////////////////////////////////////////////////
/// processData
void DbtPlySickLMSManager::processData(road_time_t t, road_timerange_t tr, void * buffer)
{
    if (!buffer) {
        LOG_DEBUG("no data available: NULL buffer");
        return;
    }

    LOG_TRACE("sizeof(SickLMS_dbt) = " << sizeof(SickLMS_dbt));
    // BOOST_ASSERT(88 == sizeof(SickLMS_dbt));
    SickLMS_dbt * sickLMS_dbt = static_cast<SickLMS_dbt *>(buffer);

    // // copy the values contained in the dbt file
    mSickDbt.scanNumber = sickLMS_dbt->scanNumber;
    mSickDbt.scannerStatus = sickLMS_dbt->scannerStatus;
    mSickDbt.scanFrequency = sickLMS_dbt->scanFrequency;
    mSickDbt.angleResolution = sickLMS_dbt->angleResolution;
    mSickDbt.startAngle = sickLMS_dbt->startAngle;
    mSickDbt.time = t;
    mSickDbt.timerange = tr;


    int sizes[7];
    sizes[0] = mSickDbt.dist_len1 = sickLMS_dbt->dist_len1;
    sizes[1] = mSickDbt.dist_len2 = sickLMS_dbt->dist_len2;
    sizes[2] = mSickDbt.dist_len3 = sickLMS_dbt->dist_len3;
    sizes[3] = mSickDbt.dist_len4 = sickLMS_dbt->dist_len4;
    sizes[4] = mSickDbt.dist_len5 = sickLMS_dbt->dist_len5;
    sizes[5] = mSickDbt.rssi_len1 = sickLMS_dbt->rssi_len1;
    sizes[6] = mSickDbt.rssi_len2 = sickLMS_dbt->rssi_len2;

    uint32_t pos[7];
    pos[0] = mSickDbt.dataPos_dist1 = sickLMS_dbt->dataPos_dist1;
    pos[1] = mSickDbt.dataPos_dist2 = sickLMS_dbt->dataPos_dist2;
    pos[2] = mSickDbt.dataPos_dist3 = sickLMS_dbt->dataPos_dist3;
    pos[3] = mSickDbt.dataPos_dist4 = sickLMS_dbt->dataPos_dist4;
    pos[4] = mSickDbt.dataPos_dist5 = sickLMS_dbt->dataPos_dist5;
    pos[5] = mSickDbt.dataPos_rssi1 = sickLMS_dbt->dataPos_rssi1;
    pos[6] = mSickDbt.dataPos_rssi2 = sickLMS_dbt->dataPos_rssi2;

    LOG_TRACE("(ScanPointsInfo) dist1 points\tSize : " << mSickDbt.dist_len1 <<"\tPos. : "<< mSickDbt.dataPos_dist1);
    LOG_TRACE("(ScanPointsInfo) dist2 points\tSize : " << mSickDbt.dist_len2 <<"\tPos. : "<< mSickDbt.dataPos_dist2);
    LOG_TRACE("(ScanPointsInfo) dist3 points\tSize : " << mSickDbt.dist_len3 <<"\tPos. : "<< mSickDbt.dataPos_dist3);
    LOG_TRACE("(ScanPointsInfo) dist4 points\tSize : " << mSickDbt.dist_len4 <<"\tPos. : "<< mSickDbt.dataPos_dist4);
    LOG_TRACE("(ScanPointsInfo) dist5 points\tSize : " << mSickDbt.dist_len5 <<"\tPos. : "<< mSickDbt.dataPos_dist5);
    LOG_TRACE("(ScanPointsInfo) rssi1 points\tSize : " << mSickDbt.rssi_len1 <<"\tPos. : "<< mSickDbt.dataPos_rssi1);
    LOG_TRACE("(ScanPointsInfo) rssi2 points\tSize : " << mSickDbt.rssi_len2 <<"\tPos. : "<< mSickDbt.dataPos_rssi2);


    uint16_t* data[7];

    int sumSizes = 0;

    LOG_TRACE("Reading UTC file ... ");
    for(int i = 0; i<7; ++i){

        if(sizes[i]){
            mDataFile.seekg(pos[i]); // set the get pointer to the correct place
            // // then copy the data contained in the binary file
            data[i] = (uint16_t*) malloc(sizes[i]*sizeof(uint16_t));
            for (size_t j = 0; j < sizes[i]; ++j) {
                mDataFile.read(reinterpret_cast<char*>(data[i]+j), sizeof(uint16_t));
                if(j%500==0){
                    LOG_TRACE("Data : " << (uint16_t) *(data[i] + j));
                }
            }
        }
        sumSizes += sizes[i];
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
        cout << "[SICK LMS]:\t"
             << "dataSize=" << sumSizes << "\t"
             << "time=" << t << endl
                ;
    }
    if (mVerbose >= 2)  {
        cout << "[SICK LMS]:\t"
             << "startAngle=" << mSickDbt.startAngle << "\t" 
             << "angleResolution=" << mSickDbt.angleResolution << std::endl ;
    }


    for(int i=0; i<7; ++i){
        free(data[i]);
    }
}

//////////////////////////////////////////////////////////////////////////
/// Displays the graphical user interface (GUI)
void DbtPlySickLMSManager::displayUI()
{
    LOG_WARN("GUI not implemented");

    // TODO
}

} // namespace pacpus
