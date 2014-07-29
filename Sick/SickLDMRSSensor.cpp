/*********************************************************************
//  created:    2014/02/02 - 12:08
//  filename:   SickLDMRSSensor.cpp
//
//  author:     Cyril Fougeray
//              Copyright Heudiasyc UMR UTC/CNRS 6599
// 
//  version:    $Id: $
//
//  purpose:    The acquisition class of the SickLDMRS sensor
//
*********************************************************************/

#include "SickLDMRSSensor.h"

#include "SickSocket.h"
// #include "Pacpus/kernel/ComponentFactory.h"
#include "Pacpus/kernel/DbiteException.h"
#include "Pacpus/kernel/DbiteFileTypes.h"
#include "Pacpus/kernel/Log.h"
#include "Pacpus/PacpusTools/ShMem.h"

#include <iostream>
#include <QTcpSocket>
#include <string>

using namespace std;

// #define SICKLDMRS_SH_MEM

namespace pacpus {

DECLARE_STATIC_LOGGER("pacpus.base.SickLDMRSSensor");


static const string kSickMemoryName = "SickLDMRS";


static const long MagicWord = 0xAFFEC0C2; // Sick LDMRS


static const int SICKLDMRS_SCANDATA_TYPE    = 0x2202;
static const int SICKLDMRS_OBJECTDATA_TYPE  = 0x2221;


SickLDMRSSensor::SickLDMRSSensor(QObject *parent)
    : ipaddr_("192.168.0.1"),
      port_(12002),
      name_("SickLDMRSDefault")
{
    LOG_TRACE("constructor ("<< this->name_ << "), default settings");

    S_socket = new SickSocket(this);

    connect(S_socket, SIGNAL(configuration()), this, SLOT(configure()) );

    pendingBytes.time = 0;
    pendingBytes.previousData = false;

    this->kSickDbtFileName = name_.toStdString()+".dbt";
    this->kSickUtcFileName = name_.toStdString()+"_data.utc";
}

SickLDMRSSensor::SickLDMRSSensor(QObject *parent, QString name, QString ip, int port, int recording)
    : ipaddr_(ip),
      port_(port),
      name_(name),
      recording_((bool)recording)
{   

    LOG_TRACE("constructor ("<< this->name_ << "), configuration : " << this->ipaddr_<<":"<<this->port_);

    S_socket = new SickSocket(this);

    connect(S_socket, SIGNAL(configuration()), this, SLOT(configure()) );

    pendingBytes.time = 0;
    pendingBytes.previousData = false;

    this->kSickDbtFileName = name_.toStdString()+".dbt";
    this->kSickUtcFileName = name_.toStdString()+"_data.utc";
}


SickLDMRSSensor::~SickLDMRSSensor()
{
    LOG_TRACE("destructor ("<< this->name_ << ")");
    delete S_socket;
}


void SickLDMRSSensor::startActivity()
{

    LOG_TRACE("Start Activity ("<< this->name_ <<")");

    S_socket->connectToServer(ipaddr_, port_);

    if (recording_) {
        LOG_TRACE("(Recording) Starting recording...");

        try {
            dbtFile_.open(kSickDbtFileName, WriteMode, TELEM_SICK_LDMRS, sizeof(SickLDMRS_dbt));
        } catch (DbiteException & e) {
            cerr << "error opening dbt file: "<< kSickDbtFileName << ", " << e.what() << endl;
            return;
        }

        // FIXME: use ofstream
        // open the file with C function to be sure that it will exist
        FILE * stream;
        if (NULL == (stream = fopen(kSickUtcFileName.c_str(), "a+"))) {
            LOG_FATAL("cannot open file '" << kSickUtcFileName.c_str() << "'");
            ::exit(-1);
        } else {
            fclose(stream);
        }

        dataFile_.open(kSickUtcFileName.c_str(), ios_base::out|ios_base::binary|ios_base::app);
        if (!dataFile_) {
            LOG_FATAL("cannot open file '" << kSickUtcFileName.c_str() << "'");
            ::exit(-1);
        }
    }
    else
        LOG_TRACE("(Recording) Not recording...");


#ifdef SICKLDMRS_SH_MEM
    shmem_ = new ShMem(kSickMemoryName.c_str(), sizeof(ScanSickData));
#endif

}


void SickLDMRSSensor::stopActivity()
{
    LOG_TRACE("destructor (" << this->name_ << ")");

    S_socket->sendToServer(QString((u_int32_t)0x0021));

    S_socket->closeSocket();

    if (recording_) {
        LOG_TRACE("(Recording) Recording stopped");
        dbtFile_.close();
        dataFile_.close();
    }

#ifdef SICKLDMRS_SH_MEM
    delete shmem_;
#endif
    // delete generator;

}



u_int32_t SickLDMRSSensor::findMagicWord(const char * message, const unsigned length)
{
    if (length < 4) {
        return -1;
    }

    unsigned long i = 0;
    while(*((u_int32_t*)(message+i)) != 0xC2C0FEAF){ // BigE
        if (i == length) {
            return -1;
        }
        ++i;
    }
    return i;

}


u_int32_t SickLDMRSSensor::getMessageSize(const char * message, const unsigned length, const long magicWordIndex)
{

    // we need at least 12 bytes
    if (length < 12) {
        return 0;
    }
    return ((*(message+magicWordIndex+11))&0x000000FF)
            + ((*(message+magicWordIndex+10)<<8)&0x0000FF00)
            + ((*(message+magicWordIndex+9)<<16)&0x00FF0000)
            + ((*(message+magicWordIndex+8)<<24)&0xFF000000)
            + 24 ;
}


bool SickLDMRSSensor::isMessageComplete(const unsigned length, const long size)
{
    if (size <= length) {
        return true;
    }
    return false;
}


void SickLDMRSSensor::fillScanHeader( MessageLDMRS &msg )
{
    // address = base + Data header size (24-byte long) + offset
    msg.hScan.scanNumber = *((u_int16_t*)(msg.body+24));
    msg.hScan.scannerStatus = *((u_int16_t*)(msg.body+24+2));
    msg.hScan.phaseOffset = *((u_int16_t*)(msg.body+24+4));
    msg.hScan.startNtpTime = *((u_int64_t*)(msg.body+24+6));
    msg.hScan.endNtpTime = *((u_int64_t*)(msg.body+24+14));
    msg.hScan.ticksPerRot= *((u_int16_t*)(msg.body+24+22)); // needed to compute angle (°)
    msg.hScan.startAngle = *((int16_t*)(msg.body+24+24));
    msg.hScan.endAngle = *((int16_t*)(msg.body+24+26));
    msg.hScan.numPoints = *((u_int16_t*)(msg.body+24+28));

//    msg.hScan.mountingYawAngle = *((int16_t*)(msg.body+24+30));
//    msg.hScan.mountingPitchAngle = *((int16_t*)(msg.body+24+32));
//    msg.hScan.mountingRollAngle = *((int16_t*)(msg.body+24+34));
//    msg.hScan.mountingX = *((int16_t*)(msg.body+24+36));
//    msg.hScan.mountingY = *((int16_t*)(msg.body+24+38));
//    msg.hScan.mountingZ = *((int16_t*)(msg.body+24+40));


    LOG_TRACE("(hScan) Scan header data parsed, scan number " << msg.hScan.scanNumber);
    LOG_TRACE("(hScan) Number of scanned points : " << msg.hScan.numPoints);
}


void SickLDMRSSensor::fillDataHeader(MessageLDMRS &msg)
{

    msg.hData.magicWord = ((*(msg.body+3))&0x000000FF) +
            ((*(msg.body+2)<<8)&0x0000FF00) +
            ((*(msg.body+1)<<16)&0x00FF0000)+
            ((*(msg.body)<<24)&0xFF000000);
    LOG_TRACE("(hData) Magic word read "<<msg.hData.magicWord);
    // TODO check if OK

    msg.hData.sizePreviousMessage = ((*(msg.body+7))&0x000000FF)+
            ((*(msg.body+6)<<8)&0x0000FF00)+
            ((*(msg.body+5)<<16)&0x00FF0000)+
            ((*(msg.body+4)<<24)&0xFF000000);
    LOG_TRACE("(hData) previous message size : "<<msg.hData.sizePreviousMessage);

    msg.hData.sizeCurrentMessage =((*(msg.body+11))&0x000000FF)+
            ((*(msg.body+10)<<8)&0x0000FF00)+
            ((*(msg.body+9)<<16)&0x00FF0000)+
            ((*(msg.body+8)<<24)&0xFF000000);

    LOG_TRACE("(hData) current message size : "<<msg.hData.sizeCurrentMessage);


    msg.hData.deviceId = *(msg.body+13);

    msg.hData.dataType =  ((*(msg.body+15))&0x000000FF)+((*(msg.body+14)<<8)&0x0000FF00);
    LOG_TRACE("(hData) Data type : " << msg.hData.dataType);


    msg.hData.ntpTime = ((*(msg.body+15))&0x000000FF)+
            ((*(msg.body+14)<<8)&0x0000FF00)+
            ((*(msg.body+13)<<16)&0x00FF0000)+
            ((*(msg.body+12)<<24)&0xFF000000)+
            ((*(msg.body+11))&0x000000FF)+
            ((*(msg.body+10)<<8)&0x0000FF00)+
            ((*(msg.body+9)<<16)&0x00FF0000)+
            ((*(msg.body+8)<<24)&0xFF000000);
}


void SickLDMRSSensor::storePendingBytes(road_time_t time)
{
    if (!pendingBytes.previousData)
    {
        pendingBytes.time = time;
        pendingBytes.previousData = true;
    }
}


void SickLDMRSSensor:: splitPacket(const char * packet, const int length, road_time_t time)
{ 
    long index = 0;
    long msgSize = 0;
    bool msgComplete = false;

    // we are working on the previous not decoded data + the actual incoming packet
    pendingBytes.data.append(packet,length);
    LOG_TRACE("(Packet reconstitution) Pending bytes : " << pendingBytes.data.size() );


    while (pendingBytes.data.size() > 0)
    {
        // we are looking for the MagicWord
        index = findMagicWord(pendingBytes.data.c_str() , pendingBytes.data.size() );
        LOG_TRACE("(Packet reconstitution) MagicWord index : " << index );
        if (index == -1)
        {
            storePendingBytes(time);
            // exit the while loop
            break;
        }


        // we are looking for the size of the message (scan data + scan points)
        msgSize = getMessageSize(pendingBytes.data.c_str(), pendingBytes.data.size(), index);
        if (msgSize == 0)
        {
            storePendingBytes(time);
            // exit the while loop
            break;
        }

        LOG_TRACE("(Packet reconstitution) Message size (data header(24) + size of the message) : " << msgSize );

        // we are verifying if the message is complete
        msgComplete = isMessageComplete( pendingBytes.data.size() , msgSize );
        if (msgComplete == false)
        {
            storePendingBytes(time);
            // exit the while loop
            break;
        }

        LOG_TRACE("(Packet reconstitution) Message complete ! ");
        // we have a complete message available that we can add to the list
        MessageLDMRS msg;

        // we copy the bytes in the body message
//        char* messageData = (char*)malloc(msgSize);
//        if(messageData == NULL){
//            LOG_FATAL("(Packet reconstitution) Malloc FAILED. Packet lost.");
//            return;
//        }
        memcpy(msg.body, pendingBytes.data.c_str() + index, msgSize);

//        msg->body = messageData;

        // we set the timestamp of the message
        if (pendingBytes.previousData)
        {
            // the timestamp is the one of the previous packet
            msg.time = pendingBytes.time;
            pendingBytes.previousData = false;
        }
        else
        {
            // the timestamp is the one of the actual received packet
            msg.time = time;
        }

        // we add the message to the list
        msgList.push_back(msg);
        // and we suppress the processed bytes of the pending data
        pendingBytes.data.erase(0, msgSize);
    }
}


unsigned long SickLDMRSSensor::processMessage(MessageLDMRS &msg)
{ 
    fillDataHeader(msg);
    if (SICKLDMRS_SCANDATA_TYPE == msg.hData.dataType) {
        LOG_TRACE("(Process Message) Scan Data Type!");
        fillScanHeader(msg);

        int index = 24 + 44; // data header + scan header

        if(sizeof(ScanPoint) * msg.hScan.numPoints > BODY_MAX_SIZE){
            LOG_FATAL("Size of the message is too long !");
            return 0;
        }

        ScanPoint scanPoints[msg.hScan.numPoints];

        // replace memory with structured data
        for (int i = 0; i < msg.hScan.numPoints; ++i) {
            scanPoints[i].layerEcho = *((uchar*)(msg.body + index));
            scanPoints[i].flags = *((uchar*)(msg.body + index + 1));
            scanPoints[i].angle = *((u_int16_t*)(msg.body + index + 2));
            scanPoints[i].distance = *((u_int16_t*)(msg.body + index + 4));
            scanPoints[i].echoPulseWidth = *((u_int16_t*)(msg.body + index + 6));
        }

        memcpy(msg.body, scanPoints, sizeof(ScanPoint) * msg.hScan.numPoints);
    }
    else if (msg.hData.dataType == SICKLDMRS_OBJECTDATA_TYPE){
        LOG_TRACE("(Process Message) Object Data Type!");

        // TODO
    }
    else {// irrelevant data type
        // TODO
    }


    return msg.hData.dataType;
}


void SickLDMRSSensor::writeData(MessageLDMRS &msg)
{
    // record the data in a dbt file. The dbt data is only the structure DataHeader
    // scan data are recorded in the sickldmrs_data.utc file

    SickLDMRS_dbt entry;
    entry.timeStartFromSensor = msg.hData.ntpTime;
    entry.hScan = msg.hScan;
    entry.dataPos = dataFile_.tellp(); // absolute position of pointer in UTC file
//    entry.time = msg.time;
//    entry.timerange = msg.timerange;

    LOG_TRACE("Writing into DBT + UTC files..");

    // write DBT
    try {
        dbtFile_.writeRecord(msg.time, msg.timerange, (char *) &entry, sizeof(SickLDMRS_dbt));
    } catch (DbiteException & e) {
        cerr << "error writing data: " << e.what() << endl;
        return;
    }

    // record the scan data in a binary file sickldmrs_data.utc with "UTC\0" to separate the data
    for (unsigned int i = 0 ; i < msg.hScan.numPoints ; ++i) {
        dataFile_.write((msg.body + i*sizeof(ScanPoint)), sizeof(ScanPoint));
    }
    // add a magic word to delimit the block of data
    int32_t utcMagicWord = UTC_MAGIC_WORD;
    dataFile_.write(reinterpret_cast<char*>(&(utcMagicWord)), sizeof(int32_t));

    LOG_TRACE("Writing done !");

}



void SickLDMRSSensor::configure(){
    // Start measuring
    // S_socket->sendToServer(QString((u_int32_t)0x0020));

    // LOG_TRACE(this->name_ +" configured.");
}


void SickLDMRSSensor::customEvent(QEvent * e)
{   
    SickFrame * frame = ((SickFrameEvent*)e)->frame;

    // we try to find some messages in the current packet + the pending bytes of the previous incoming data
    splitPacket(frame->msg, frame->size, frame->time);

    // we delete the heap variable
    delete frame;

    // we test if we have some messages to decode
    while ( !msgList.empty() )
    {
        LOG_TRACE("Message waiting");
        // get the first (the eldest) message and process it
        MessageLDMRS msgToProcess = msgList.front();
        unsigned long type = processMessage(msgToProcess);
        LOG_TRACE("Message processed !");

        if (type == SICKLDMRS_SCANDATA_TYPE)
        {
//            setState(ComponentBase::MONITOR_OK);
            // write data on the disk
            if (recording_)
                writeData(msgToProcess);

#ifdef SICKLDMRS_SH_MEM
            /// NOT TESTED !
            /// push data in shared memory
            // First the scan info
            SickLDMRS_shMem toWrite;
            toWrite.time = msgToProcess.time;
            toWrite.timerange = msgToProcess.timerange;
            toWrite.scanInfo = msgToProcess.hScan;
            shmem_->write(toWrite, sizeof(SickLDMRS_shMem));

            // Then, the points
            for (unsigned int i = 0 ; i < msgToProcess.hScan.numPoints ; ++i) {
                shmem_->write((msg.body + i*sizeof(ScanPoint)), sizeof(ScanPoint));
            }
#endif

        }
        else if (type == SICKLDMRS_OBJECTDATA_TYPE)
        {
            // Handled via CAN bus ?
        }

        // removes the processed item of the list
        // free(msgList.front().body);
        msgList.pop_front();
    }
}

} // namespace pacpus
