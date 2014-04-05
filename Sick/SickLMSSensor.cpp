/*********************************************************************
//  created:    2014/02/11 - 12:08
//  filename:   SickLMSSensor.cpp
//
//  author:     Cyril Fougeray
//              Copyright Heudiasyc UMR UTC/CNRS 6599
// 
//  version:    $Id: $
//
//  purpose:    The acquisition class of the SickLMS sensor
//
*********************************************************************/

#include "SickLMSSensor.h"

#include "SickSocket.h"
// #include "Pacpus/kernel/ComponentFactory.h"
#include "Pacpus/kernel/DbiteException.h"
#include "Pacpus/kernel/DbiteFileTypes.h"
#include "Pacpus/kernel/Log.h"
#include "Pacpus/PacpusTools/ShMem.h"

#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>

#include <iostream>
#include <QTcpSocket>
#include <string>
#include <vector>

using namespace std;

// #define SICKLMS_SH_MEM

namespace pacpus {

static const int STX_CHAR = 0x02;
static const int ETX_CHAR = 0x03;

DECLARE_STATIC_LOGGER("pacpus.base.SickLMSSensor");

// Commands which can be taken into account
static const std::string    SICKLMS_SCANDATA_MESSAGE = "LMDscandata";
static const std::string    SICKLMS_SCANCONFIG_MESSAGE = "LMPscancfg";
static const int            MINIMUM_PARAMETERS_MSG = 6;                 // ScanConfig message is the smallest relevant message

//////////////////////////////////////////////////////////////////////////
/// Constructor.
SickLMSSensor::SickLMSSensor(QObject *parent)
    : ipaddr_("192.168.0.50"),
      port_(1211),
      name_("SickLMSDefault"),
      recording_(false)
{
    LOG_TRACE("constructor ("<<this->name_ << "), default settings");

    S_socket = new SickSocket(this);

    connect(S_socket, SIGNAL(configuration()), this, SLOT(configure()) );

    pendingBytes.time = 0;
    pendingBytes.previousData = false;

    this->kSickDbtFileName = name_.toStdString()+".dbt";
    this->kSickUtcFileName = name_.toStdString()+"_data.utc";
}

SickLMSSensor::SickLMSSensor(QObject *parent, QString name, QString ip, int port, int recording)
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


SickLMSSensor::~SickLMSSensor()
{
    LOG_TRACE("destructor ("<< this->name_ << ")");
    delete S_socket;
}


void SickLMSSensor::startActivity()
{

    LOG_TRACE("Start Activity ("<< this->name_ <<")");

    S_socket->connectToServer(ipaddr_, port_);

    // config can be computed for each measure
    // askScanCfg();


    char buf[100];

//    ///LOGIN
//    sprintf(buf, "%c%s%c", 0x02, "sMN SetAccessMode 03 F4724744", 0x03);
//    S_socket->sendToServer(QString(buf));


//    sprintf(buf, "%c%s%c", 0x02, "sWN LMDscandatacfg 03 00 1 1 0 00 00 0 0 0 0 1 1", 0x03);
//    S_socket->sendToServer(QString(buf));

////    sprintf(buf, "%c%s%c", 0x02, "sWN FREchoFilter 1", 0x03);
////    S_socket->sendToServer(QString(buf));

/////// SAVE PERMANENT
//    sprintf(buf, "%c%s%c", 0x02, "sMN mEEwriteall", 0x03);
//    S_socket->sendToServer(QString(buf));

////    /// LOGOUT
//    sprintf(buf, "%c%s%c", 0x02, "sMN Run", 0x03);
//    S_socket->sendToServer(QString(buf));


    // Start measurement
    sprintf(buf, "%c%s%c", 0x02, "sEN LMDscandata 1", 0x03);
    S_socket->sendToServer(QString(buf));

    /// TODO get response from sensor and analyse it to know if measuring has started
    /// See p23 telegram
    if(0)
        LOG_INFO("(Measuring) Data sent permanently");


    if (recording_) {
        LOG_INFO("(Recording) Recording is on.");

        try {
            dbtFile_.open(kSickDbtFileName, WriteMode, TELEM_SICK_LMS, sizeof(SickLMS_dbt));
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
        LOG_INFO("(Recording) Not recording.");

#ifdef SICKLMS_SH_MEM
    shmem_ = new ShMem(kSickMemoryName.c_str(), sizeof(ScanSickData));
#endif

}



void SickLMSSensor::stopActivity()
{
    LOG_TRACE("destructor (" << this->name_ << ")");

    // Stop measurement. Not necessary : when socket is closed, data is no longer sent from sensor
    char buf[64];
    sprintf(buf, "%c%s%c", 0x02, "sEN LMDscandata 0", 0x03);
    S_socket->sendToServer(QString(buf));

    S_socket->closeSocket();

    if (recording_) {
        LOG_TRACE("(Recording) Recording stopped");
        dbtFile_.close();
        dataFile_.close();
    }

#ifdef SICKLMS_SH_MEM
    delete shmem_;
#endif
    // delete generator;

}


void SickLMSSensor::configure(){
    // Start measuring
    // S_socket->sendToServer(QString((u_int32_t)0x0020));

    // LOG_TRACE(this->name_ +" configured.");
}




/* Data must be parsed in slot customEvent */
void SickLMSSensor::askScanCfg(){
    char buf[100];
    sprintf(buf, "%c%s%c", 0x02, "sRN LMPscancfg", 0x03);

    S_socket->sendToServer(QString(buf));
}




int SickLMSSensor::isMessageComplete(const char* packets, unsigned int size)
{
    for(int i = 0; i < size; ++i){
        if(packets[i] == ETX_CHAR)
            return i;
    }
    return -1;
}



int SickLMSSensor::findSTX(const char* packets, const unsigned int size ){
    int i = 0;
    while(i < size){
        if(packets[i] == STX_CHAR)
            return i;
        i++;
    }
    return -1;
}


void SickLMSSensor::storePendingBytes(road_time_t time)
{
    if (!pendingBytes.previousData)
    {
        pendingBytes.time = time;
        pendingBytes.previousData = true;
    }
}


void SickLMSSensor::reconstituteMessage(const char * packet, const int length, road_time_t time)
{ 
    long indexSTX = 0;
    long indexETX = 0;
    long msgSize = 0;

    // we are working on the previous not decoded data + the actual incoming packet
    pendingBytes.data.append(packet,length);
    LOG_TRACE("(Packet reconstitution) Pending bytes : " << pendingBytes.data.size() );


    while (pendingBytes.data.size() > 0)
    {
        // we are looking for the start of frame <STX> (= 0x02)
        indexSTX = findSTX(pendingBytes.data.c_str() , pendingBytes.data.size() );
        LOG_TRACE("(Packet reconstitution) Start of text index : " << indexSTX );
        if (indexSTX == -1)
        {
            storePendingBytes(time);
            // exit the while loop
            break;
        }

        // we are verifying if the message is complete
        indexETX = isMessageComplete(pendingBytes.data.c_str(), pendingBytes.data.size());
        LOG_TRACE("(Packet reconstitution) End of text index : " << indexETX );
        if (indexETX == -1)
        {
            storePendingBytes(time);
            // exit the while loop
            break;
        }

        // we have a complete message available that we can add to the list
        MessageLMS msg;

        // we copy the bytes in the body message
        msgSize = indexETX - indexSTX + 1;
        char* messageData = (char*)malloc(msgSize+1);
        memcpy(messageData, pendingBytes.data.c_str() + indexSTX, msgSize);

        msg.body = messageData;
        msg.msgSize = msgSize;

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


int SickLMSSensor::splitMessage(MessageLMS* message){

    message->splitMessage = new std::vector<std::string>();

    for(int i; i < message->msgSize; ++i){
        std::string* str = new std::string();
        while(message->body[i] != ' ' && i < message->msgSize){
            str->push_back(message->body[i]);
            ++i;
        }
        message->splitMessage->push_back(*str);
        delete str;
    }
    LOG_TRACE("(splitMessage) Number of parameters into the message : "<< (int)message->splitMessage->size());

    return (int) message->splitMessage->size();
}



long SickLMSSensor::xstol(std::string str){
    long ret = 0;
    for (std::string::iterator it=str.begin(); it!=str.end(); ++it){
        if(*it >= 'A' && *it <= 'F'){
            ret *= 16;
            ret += (*it - 'A' + 10);
        }
        else if (*it >= '0' && *it <= '9'){
            ret *= 16;
            ret += (*it - '0');
        }
        else
            LOG_WARN("(conversion) String is not a hex value");
    }
    return ret;
}


int SickLMSSensor::processScanData(MessageLMS* msg)
{
    // just non-empty arrays will be saved in DBT
    // so first, we initialize these values to 0 and then fill them if message contains corresponding data
    msg->data.dist_len1 = 0;
    msg->data.dist_len2 = 0;
    msg->data.dist_len3 = 0;
    msg->data.dist_len4 = 0;
    msg->data.dist_len5 = 0;
    msg->data.rssi_len1 = 0;
    msg->data.rssi_len2 = 0;


//    for(std::vector<std::string>::iterator it = msg->splitMessage->begin(); it != msg->splitMessage->end(); it++){
//        printf("%s ", (*it).c_str());
//    }
//    printf("\n");

    //    0 //Type of command
    //    1 //Command
    //    2 //VersionNumber
    //    3 //DeviceNumber
    //    4 //Serial number
    //    5-6 //DeviceStatus
    //          00 00 OK
    //          00 01 Error
    //          00 02 Pollution Warning
    //          00 04 Pollution Error

    //    7 //MessageCounter

    LOG_TRACE("(Parsing) Message number 0x"<< msg->splitMessage->at(7).c_str());

    //    8 //ScanCounter
    //    9 //PowerUpDuration
    //    10 //TransmissionDuration
    //    11-12 //InputStatus
    //    13-14 //OutputStatus
    //    15 //ReservedByteA
    //    16 //ScanningFrequency
    msg->data.scanFrequency = xstol(msg->splitMessage->at(16));
    LOG_TRACE("(Parsing) Scan frequency "<< msg->data.scanFrequency <<" [1/100 Hz]");

    //    17 //MeasurementFrequency

    //    18 //NumberEncoders
    int NumberEncoders = xstol(msg->splitMessage->at(18));
    LOG_TRACE("(Parsing) Number Encoders "<< NumberEncoders);

        for (int i = 0; i < NumberEncoders; i++) {
    //         //EncoderPosition
    //         //EncoderSpeed
        }


    //    18+NumberEncoders*2+1 //NumberChannels16Bit
        int NumberChannels16Bit = xstol(msg->splitMessage->at(18+NumberEncoders*2+1));
        LOG_TRACE("(Parsing) Number channels 16Bit : "<<NumberChannels16Bit);

        int totalData16 = 0;

        for (int i = 0; i < NumberChannels16Bit; i++) {
            int type = -1;  // 0 DIST1 1 DIST2 2 (LMS1xx & LMS5xx)
                            // RSSI1 3 RSSI2 (LMS1xx)
                            // 4 DIST3 5 DIST4 6 DIST5 (LMS5xx)


            int NumberData;

            std::string content = msg->splitMessage->at(19+NumberEncoders*2+i*6+totalData16+1);
    //        19+NumberEncoders*2+i*6+totalData16+1 //MeasuredDataContent

            LOG_TRACE("(Parsing 16bit channel #"<<i<<") Measured Data Content : " << content);
            if (content == "DIST1") {
                type = 0;
            } else if (content == "DIST2") {
                type = 1;
            } else if (content == "RSSI1") {
                type = 2;
            } else if (content == "RSSI2") {
                type = 3;
            } else if (content == "DIST3") {
                type = 4;
            } else if (content == "DIST4") {
                type = 5;
            } else if (content == "DIST5") {
                type = 6;
            }

//            19+NumberEncoders*2+i*6+totalData16+2 //ScalingFactor
            int scalingFactor = 1;
            if(msg->splitMessage->at(19+NumberEncoders*2+i*6+totalData16+2) == "40000000")
                scalingFactor = 2;
            LOG_TRACE("(Parsing 16bit channel #"<<i<<") Scaling factor x"<< scalingFactor);

//            19+NumberEncoders*2+i*6+totalData16+3 //ScalingOffset
//            19+NumberEncoders*2+i*6+totalData16+4 //Starting angle
            msg->data.startAngle = xstol(msg->splitMessage->at(19+NumberEncoders*2+i*6+totalData16+4));
            LOG_TRACE("(Parsing 16bit channel #"<<i<<") Start angle "<< msg->data.startAngle << " [1/10000 degree]");

//            19+NumberEncoders*2+i*6+totalData16+5 //Angular step width
            msg->data.angleResolution = xstol(msg->splitMessage->at(19+NumberEncoders*2+i*6+totalData16+5));
            LOG_TRACE("(Parsing 16bit channel #"<<i<<") Angular step width "<< msg->data.angleResolution<<" [1/10000 degree]");

//            19+NumberEncoders*2+i*6+totalData16+6 //NumberData
            NumberData = xstol(msg->splitMessage->at(19+NumberEncoders*2+i*6+totalData16+6));

            LOG_TRACE("(Parsing 16bit channel #"<<i<<") Number Data for "<<content<<" : "<<NumberData);

            LOG_TRACE("(Parsing 16bit channel #"<<i<<") First data "<<content<<" (index "<<(19+NumberEncoders*2+i*6+totalData16+7)<<") : "<<xstol(msg->splitMessage->at(19+NumberEncoders*2+i*6+totalData16+7)));
            LOG_TRACE("(Parsing 16bit channel #"<<i<<") Last data "<<content<<" (index "<<(19+NumberEncoders*2+i*6+totalData16+7+NumberData-1)<<") : "<<xstol(msg->splitMessage->at(19+NumberEncoders*2+i*6+totalData16+7+NumberData-1)));

            uint16_t* distPoints = (uint16_t*) malloc(NumberData * sizeof(uint16_t));
            for (int j = 0; j < NumberData; j++) {
                distPoints[j] = scalingFactor * xstol(msg->splitMessage->at(19+NumberEncoders*2+i*6+totalData16+7+j));
            }
            if (type == 0) {
                msg->data.dist1 = distPoints;
                msg->data.dist_len1 = NumberData;
            } else if (type == 1) {
                msg->data.dist2 = distPoints;
                msg->data.dist_len2 = NumberData;
            } else if (type == 2) {
                msg->data.rssi1 = distPoints;
                msg->data.rssi_len1 = NumberData;
            } else if (type == 3) {
                msg->data.rssi2 = distPoints;
                msg->data.rssi_len2 = NumberData;
            } else if (type == 4) {
                msg->data.dist3 = distPoints;
                msg->data.dist_len3 = NumberData;
            } else if (type == 5) {
                msg->data.dist4 = distPoints;
                msg->data.dist_len4 = NumberData;
            } else if (type == 6) {
                msg->data.dist5 = distPoints;
                msg->data.dist_len5 = NumberData;
            }
            totalData16 += NumberData;

        }


//        19+NumberEncoders*2+NumberChannels16Bit*6+totalData16+1 //NumberChannels8Bit
        int NumberChannels8Bit = xstol(msg->splitMessage->at(19+NumberEncoders*2+NumberChannels16Bit*6+totalData16+1));
        LOG_TRACE("(Processing) Number channels 8Bit : "<<NumberChannels8Bit);

        LOG_INFO("(Parsing) 8bit channel not implemented yet !");

/*
//        int totalData8 = 0;
//        for (int i = 0; i < NumberChannels8Bit; i++) {

//            int type = -1;

//            std::string content = msg->splitMessage->at(21+NumberEncoders*2+NumberChannels16Bit*6+totalData16+totalData8+i*6);
//            LOG_TRACE("(Parsing 8bit channel) Measured Data Content : " << content);
//            if (content == "DIST1") {
//                type = 0;
//            } else if (content=="DIST2") {
//                type = 1;
//            } else if (content=="RSSI1") {
//                type = 2;
//            } else if (content == "RSSI2") {
//                type = 3;
//            }

//            21+NumberEncoders*2+NumberChannels16Bit*6+totalData16+i*6+1 //ScalingFactor
//            21+NumberEncoders*2+NumberChannels16Bit*6+totalData16+i*6+2 //ScalingOffset
//            21+NumberEncoders*2+NumberChannels16Bit*6+totalData16+i*6+3 //Starting angle
//            21+NumberEncoders*2+NumberChannels16Bit*6+totalData16+i*6+4 //Angular step width
//            21+NumberEncoders*2+NumberChannels16Bit*6+totalData16+i*6+5 //NumberData

//            int NumberData = xstol(msg->splitMessage->at(21+NumberEncoders*2+NumberChannels16Bit*6+totalData16+totalData8+i*6+5));
//            LOG_TRACE("(Parsing 16bit channel) Number Data for "<<content<<" : "<<NumberData);

//            sscanf(tok, "%X", &NumberData);

//            LOG_TRACE("(Processing) Number data : "<< NumberData);

//            if (type == 0) {
//                msg->data.dist_len1 = NumberData;
//            } else if (type == 1) {
//                msg->data.dist_len2 = NumberData;
//            } else if (type == 2) {
//                msg->data.rssi_len1 = NumberData;
//            } else if (type == 3) {
//                msg->data.rssi_len2 = NumberData;
//            }
//            for (int i = 0; i < NumberData; i++) {
//                int dat;
//                tok = strtok(NULL, " "); //data
//                sscanf(tok, "%X", &dat);

//                if (type == 0) {
//                    msg->data.dist1[i] = dat;
//                } else if (type == 1) {
//                    msg->data.dist2[i] = dat;
//                } else if (type == 2) {
//                    msg->data.rssi1[i] = dat;
//                } else if (type == 3) {
//                    msg->data.rssi2[i] = dat;
//                }
//            }
        } // 8bit channel
*/

}


void SickLMSSensor::writeData(MessageLMS &msg)
{
    SickLMS_dbt entry;
    entry.angleResolution = msg.data.angleResolution;
    entry.scanNumber = xstol(msg.splitMessage->at(7));
    entry.scannerStatus = xstol(msg.splitMessage->at(6));
    /* time in SickLMS_dbt is used uniquely when DBT file read */
        //    entry.time = msg.time;
        //    entry.timerange = msg.timerange;
    entry.scanFrequency = msg.data.scanFrequency;
    entry.angleResolution = msg.data.angleResolution;
    entry.startAngle = msg.data.startAngle;

    // Initialisation
    entry.dist_len1 = entry.dataPos_dist1 = 0;
    entry.dist_len2 = entry.dataPos_dist2 = 0;
    entry.dist_len3 = entry.dataPos_dist3 = 0;
    entry.dist_len4 = entry.dataPos_dist4 = 0;
    entry.dist_len5 = entry.dataPos_dist5 = 0;
    entry.rssi_len1 = entry.dataPos_rssi1 = 0;
    entry.rssi_len2 = entry.dataPos_rssi2 = 0;

    if(msg.data.dist_len1){
        entry.dist_len1 = msg.data.dist_len1;
        entry.dataPos_dist1 = dataFile_.tellp();

        for (unsigned int i = 0 ; i < msg.data.dist_len1; ++i) {
            dataFile_.write(reinterpret_cast<char*>(&(msg.data.dist1[i])), sizeof(u_int16_t));
        }

        free(msg.data.dist1);
    }
    if(msg.data.dist_len2){
        entry.dist_len2 = msg.data.dist_len2;
        entry.dataPos_dist2 = dataFile_.tellp();

        for (unsigned int i = 0 ; i < msg.data.dist_len2; ++i) {
            dataFile_.write(reinterpret_cast<char*>(&(msg.data.dist2[i])), sizeof(u_int16_t));
        }

        free(msg.data.dist2);
    }
    if(msg.data.dist_len3){
        entry.dist_len3 = msg.data.dist_len3;
        entry.dataPos_dist3 = dataFile_.tellp();

        for (unsigned int i = 0 ; i < msg.data.dist_len3; ++i) {
            dataFile_.write(reinterpret_cast<char*>(&(msg.data.dist3[i])), sizeof(u_int16_t));
        }

        free(msg.data.dist3);
    }
    if(msg.data.dist_len4){
        entry.dist_len4 = msg.data.dist_len4;
        entry.dataPos_dist4 = dataFile_.tellp();

        for (unsigned int i = 0 ; i < msg.data.dist_len4; ++i) {
            dataFile_.write(reinterpret_cast<char*>(&(msg.data.dist4[i])), sizeof(u_int16_t));
        }

        free(msg.data.dist4);
    }
    if(msg.data.dist_len5){
        entry.dist_len5 = msg.data.dist_len5;
        entry.dataPos_dist5 = dataFile_.tellp();

        for (unsigned int i = 0 ; i < msg.data.dist_len5; ++i) {
            dataFile_.write(reinterpret_cast<char*>(&(msg.data.dist5[i])), sizeof(u_int16_t));
        }

        free(msg.data.dist5);
    }
    if(msg.data.rssi_len1){
        entry.rssi_len1 = msg.data.rssi_len1;
        entry.dataPos_rssi1 = dataFile_.tellp();

        for (unsigned int i = 0 ; i < msg.data.rssi_len1; ++i) {
            dataFile_.write(reinterpret_cast<char*>(&(msg.data.rssi1[i])), sizeof(u_int16_t));
        }

        free(msg.data.rssi1);
    }
    if(msg.data.rssi_len2){
        entry.rssi_len2 = msg.data.rssi_len2;
        entry.dataPos_rssi2 = dataFile_.tellp();

        for (unsigned int i = 0 ; i < msg.data.rssi_len2; ++i) {
            dataFile_.write(reinterpret_cast<char*>(&(msg.data.rssi2[i])), sizeof(u_int16_t));
        }

        free(msg.data.rssi2);
    }
    // add a magic word to delimit the block of data
    int32_t utcMagicWord = UTC_MAGIC_WORD;
    dataFile_.write(reinterpret_cast<char*>(&(utcMagicWord)), sizeof(int32_t));

    // write DBT
    try {
        dbtFile_.writeRecord(msg.time, msg.timerange, (char *) &entry, sizeof(SickLMS_dbt));
    } catch (DbiteException & e) {
        cerr << "error writing data: " << e.what() << endl;
        return;
    }

}




void SickLMSSensor::customEvent(QEvent * e)
{
    char answerMsg1[10];
    sprintf(answerMsg1, "%c%s", 0x02, "sRA");
    char answerMsg2[10];
    sprintf(answerMsg2, "%c%s", 0x02, "sSN");

    SickFrame * frame = ((SickFrameEvent*)e)->frame;

    // we try to find some messages in the current packet + the pending bytes of the previous incoming data
    reconstituteMessage(frame->msg, frame->size, frame->time);

    // we delete the heap variable
    delete frame;

    // we test if we have some messages to decode
    while ( !msgList.empty() )
    {
        // get the first (the eldest) message and process it
        MessageLMS* msgToProcess = &(msgList.front());

        // verify if the message is worth to be decoded
        if(!strncmp(answerMsg1, msgToProcess->body, 4) || !strncmp(answerMsg2, msgToProcess->body, 4)){

            if(splitMessage(msgToProcess) >= MINIMUM_PARAMETERS_MSG){

                std::string type = msgToProcess->splitMessage->at(1);

                LOG_TRACE("(Message type) "<< type);

                if (type == SICKLMS_SCANDATA_MESSAGE)
                {
                    LOG_TRACE("Scan data message !");

                    processScanData(msgToProcess);

                    // write data on the disk (dbt + utc)
                    if (recording_)
                        writeData(msgList.front());


        #ifdef SICKLMS_SH_MEM
                    /// push data in shared memory
                    // First the scan info
                    SickLMS_shMem toWrite;
                    toWrite.time = msgToProcess.time;
                    toWrite.timerange = msgToProcess.timerange;
                    toWrite.scanInfo = msgToProcess.hScan;
                    shmem_->write(toWrite, sizeof(SickLMS_shMem));

                    // Then, the points
                    for (unsigned int i = 0 ; i < msgToProcess.hScan.numPoints ; ++i) {
                        shmem_->write((msg.body + i*sizeof(ScanPoint)), sizeof(ScanPoint));
                    }
        #endif
                }
                else if (type == SICKLMS_SCANCONFIG_MESSAGE){
                    LOG_TRACE("Scan configuration message !");

                    // get the values as (unsigned or signed) integer
                    mainCfg.scaningFrequency = xstol(msgToProcess->splitMessage->at(2));
                    mainCfg.angleResolution = xstol(msgToProcess->splitMessage->at(4));
                    mainCfg.startAngle = xstol(msgToProcess->splitMessage->at(5));
                    mainCfg.stopAngle = xstol(msgToProcess->splitMessage->at(6));
                    LOG_TRACE("(Scan config) Frequency : "<<mainCfg.scaningFrequency<<", Resolution : "<<mainCfg.angleResolution);
                    LOG_TRACE("(Scan config) Start angle : "<<mainCfg.startAngle<<", Stop angle : "<<mainCfg.stopAngle);
                }
            }

            delete msgToProcess->splitMessage;
        }


        // (malloced memory) raw data no longer needed
        free(msgToProcess->body);


        // removes the processed item of the list
        msgList.pop_front();
    }
}

} // namespace pacpus
