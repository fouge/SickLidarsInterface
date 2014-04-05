/*********************************************************************
//  created:    2014/02/11 - 10:48
//  filename:   SickLMSSensor.h
//
//  author:     Cyril Fougeray
//              Copyright Heudiasyc UMR UTC/CNRS 6599
// 
//  version:    $Id: $
//
//  purpose:    Definition of the SickLMSSensor class
*********************************************************************/

#ifndef SICKLMSSENSOR_H
#define SICKLMSSENSOR_H

#include "Pacpus/kernel/ComponentBase.h"
#include "Pacpus/kernel/DbiteFile.h"

#include "AbstractSickSensor.h"
#include "SickLMSData.h"

#include <fstream>
#include <string>

// Export macro for SickLMS DLL for Windows only
#ifdef WIN32
#   ifdef SICKLMS_EXPORTS
        // make DLL
#       define SICKLMS_API __declspec(dllexport)
#   else
        // use DLL
#       define SICKLMS_API __declspec(dllimport)
#   endif
#else
    // On other platforms, simply ignore this 
#   define SICKLMS_API
#endif

class QEvent;

namespace pacpus {
    
class ShMem;
class SickComponent;


/// The class carrying Sick LMS message.
/** This class is used so that we can store every information sent by a Sick LMS sensor.
 * First, the raw data is stored in \c body. Then, if the message is relevant, \c splitMessage is instanciated in order
 * to parse easily information from the sensor.
 * These data are then decoded and stored in the scanData structure.
 */
class SICKLMS_API MessageLMS
{
public:
    /// Constructor.
    MessageLMS()
    {
        time = 0;
        timerange = 0;
        memset(&data,0,sizeof(data));
    }

    /// Destructor.
    ~MessageLMS(){}

    scanData data; //!< Every needed information about the scan (general info + scan points).

    long msgSize; //!< Size of the message

    std::vector<std::string>* splitMessage; //!< The message is split into an array of string in order to be processed easily.
    char* body;                             //!< Raw data
    road_time_t time;                       //!< Time when the first packet of the message is received.
    road_timerange_t timerange;             //!< Timerange : roughly, time between measurement of a point and the processing step (not implemented).
};


//! The class implenting receiving, decoding and storing process of Sick LMS data.
/**
 * This class can be used as a particular thread to acquire data from Sick LDMRS sensors.
 * The Ethernet interface is used to get data from the sensor. Thus, the goal of this class is to
 * get packets and decode them. Also, it offers the possibility to store all relevant information in
 * two files (.dbt and .utc).
 * It can be managed by SickComponent objects.
 */
class SickLMSSensor : public AbstractSickSensor
{
    Q_OBJECT
public:
    /// Constructor
    SickLMSSensor(QObject *parent);

    /**
     * @brief SickLMSSensor constructor.
     * @param parent Basically, a SickComponent object.
     * @param name Name of the sensor in order to write on .dbt and .utc files and to recognize every sensors used.
     * @param ip The IP address of the remote Sick LMS sensor.
     * @param port The port of the remote Sick LMS sensor.
     * @param recording If \c true, data is recorded into dbt + utc files. Data is not recorded otherwise.
     */
    SickLMSSensor(QObject *parent, QString name, QString ip, int port, int recording);

    /// Destructor
    ~SickLMSSensor();

    void run() {}

    void stopActivity(); /*!< To stop the processing thread. */
    void startActivity(); /*!< To start the processing thread. */

    /**
     * @brief reconstituteMessage reconstitute a complete message from received packets
     * @param packet Raw data coming from the sensor.
     * @param length Length of the raw data received.
     * @param time Time of the last received data.
     *
     * A message starts with a <STX> (0x02 in ASCII) char and ends with <ETX> (0x03 in ASCII).
     * This function stores packets until a complete message is received. In this case, the
     * message is added to the list of MessageLMS, msgList.
     */
    void reconstituteMessage(const char * packet, const int length, road_time_t time);

    /**
     * @brief processScanData Parse information and process every needed values.
     * @param msg Carries a message. splitMessage field of MessageLMS must be filled.
     * @return Not used for the moment.
     */
    int processScanData(MessageLMS *msg);

    /**
     * @brief isMessageComplete find the <ETX> character (corresponding to the end of a message).
     * @param packets Raw data.
     * @param size Size of raw data.
     * @return The index of the <ETX> character.
     */
    int isMessageComplete(const char* packets, unsigned int size);

public Q_SLOTS:  
    /**
     * @brief customEvent allows to receive the incoming data and store them into known structures.
     * @param e Event that carries the Ethernet packets and receiving time.
     */
    void customEvent(QEvent * e);

    /**
     * @brief Configure the object, not used for the moment.
     */
    void configure();
    
public:
    /**
     * @brief S_socket, used to receive and send data to the remote sensor.
     */
    SickSocket * S_socket;

private:
    /// Name is used to recognize between several sensors and store data into .dbt and utc files.
    QString name_;

    /// The IP address of the remote Sick LDMRS sensor we are connected to.
    QString ipaddr_;

    /// The SickLDMRS port
    int port_;

    /// Enable storing in DBT + UTC files
    bool recording_;

    scanCfg mainCfg;

    /// Append new data into the private MessagePacket @b pendingBytes.
    void storePendingBytes(road_time_t time);

    /** Ask for scan configuration (frequency and angular resolution) . [See §5.2 of the documentation : telegrams]{TL_LMS1xx_5xx_TiM3xx_JEF300_JEF500_en_8014631_20120508.pdf}
        Data is parsed in customEvent slot.
    */
    void askScanCfg();

    /**
     * @brief findSTX looks for the first character of the message <STX>.
     * @param packets Raw data received.
     * @param size Size of the raw data array (\c packets).
     * @return Index of the <STX> character in \c packets.
     */
    int findSTX(const char* packets, const unsigned int size );

    /**
     * @brief splitMessage Split message into arrays of string to parse.
     * @param message Contains raw data and is modified to store the vector of strings.
     * @return Size of the vector.
     */
    int splitMessage(MessageLMS* message);

    /**
     * @brief xstol convert hexadecimal values coded in ASCII to integer.
     * @param str String to convert (Hexadecimal value)
     * @return long integer value (to cast as needed)
     */
    long xstol(std::string str);

    std::string kSickDbtFileName; //!< Name of the DBT file.
    std::string kSickUtcFileName; //!< Name of the UTC file.

    /// List of messages to process.
    std::list<MessageLMS> msgList;

    /// Received raw data is appended into this MessagePacket.
    MessagePacket pendingBytes;

    /**
     * @brief Write data into .dbt + .utc files.
     * @param msg
     *
     * SickLMS_dbt structure is filled and stored into the DBT file.
     * The arrays of points' distance is then stored into the UTC file, depending on data received.
     */
    void writeData(MessageLMS &msg);

    pacpus::DbiteFile dbtFile_; //!< DBT file.
    std::ofstream dataFile_; //!< UTC file.

#ifdef SickLMS_SH_MEM
    ShMem * shmem_;
    SickLMS_shMem sickData;
#endif

};

} // namespace pacpus

#endif // SICKLMSSENSOR_H
