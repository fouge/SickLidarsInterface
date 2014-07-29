/*********************************************************************
//  created:    2014/02/02 - 10:48
//  filename:   SickLDMRSSensor.h
//
//  author:     Cyril Fougeray
//              Copyright Heudiasyc UMR UTC/CNRS 6599
// 
//  version:    $Id: $
//
//  purpose:    Definition of the SickLDMRSSensor class
*********************************************************************/




#ifndef SICKLDMRSSENSOR_H
#define SICKLDMRSSENSOR_H

#include "Pacpus/kernel/ComponentBase.h"
#include "Pacpus/kernel/DbiteFile.h"

#include "AbstractSickSensor.h"
#include "SickLDMRSData.h"

#include <fstream>
#include <string>

#define BODY_MAX_SIZE   10000

// Export macro for SickLDMRS DLL for Windows only
#ifdef WIN32
#   ifdef SICKLDMRS_EXPORTS
        // make DLL
#       define SICKLDMRS_API __declspec(dllexport)
#   else
        // use DLL
#       define SICKLDMRS_API __declspec(dllimport)
#   endif
#else
    // On other platforms, simply ignore this 
#   define SICKLDMRS_API
#endif

class QEvent;

namespace pacpus {
    
class ShMem;
class SickComponent;



/// The class carrying Sick LDMRS message.
/** This class is used so that we can store every information sent by a Sick LDMRS sensor.
 * First, the raw data is stored in \c body.
 * These data are then decoded and general information about the message is stored in DataHeader and ScanHeader
 * (Object data decoding is not implemented yet).
 * Then, the body field is replaced by a ScanPoint or ScanObject array in order to be stored in DBT/UTC files.
 */
class SICKLDMRS_API MessageLDMRS
{
public:
    /// Constructor.
    MessageLDMRS()
    {
        time = 0;
        timerange = 0;
        memset(&hData,0,sizeof(hData));
        memset(&hScan,0,sizeof(hScan));
    }

    /// Destructor.
    ~MessageLDMRS(){}

    //! An instance of DataHeader.
    DataHeader hData;

    //! An instance of ScanHeader (if data type is scan points).
    ScanHeader hScan;

    //! An array of characters : raw data then array of points or objects, depending on data type.
    /** This array pointer points to allocated in memory (basically, in heap (malloc)) and then must be freed (free) when the whole message is decoded and stored. */
    char body[BODY_MAX_SIZE];

    //! Time when the message is received.
    road_time_t time;

    //! Timerange : roughly, time between measurement of a point and the processing step (not implemented).
    road_timerange_t timerange;
};



//! The class implenting receiving, decoding and storing process of Sick LD-MRS data.
/**
 * This class can be used as a particular thread to acquire data from Sick LDMRS sensors.
 * The Ethernet interface is used to get data from the sensor. Thus, the goal of this class is to
 * get packets and decode them. Also, it offers the possibility to store all relevant information in
 * two files (.dbt and .utc).
 * It can be managed by SickComponent objects.
 */
class SickLDMRSSensor : public AbstractSickSensor
{
    Q_OBJECT
public:
    /// Constructor
    SickLDMRSSensor(QObject *parent);

    /**
     * @brief SickLDMRSSensor constructor.
     * @param parent Basically, a SickComponent object.
     * @param name Name of the sensor in order to write on .dbt and .utc files and to recognize every sensors used.
     * @param ip The IP address of the remote Sick LDMRS sensor.
     * @param port The port of the remote Sick LDMRS sensor.
     * @param recording If \c true, data is recorded into dbt + utc files. Data is not recorded otherwise.
     */
    SickLDMRSSensor(QObject *parent, QString name, QString ip, int port, int recording);

    /// Destructor
    ~SickLDMRSSensor();

    void run() {}

    void stopActivity(); /*!< To stop the processing thread */
    void startActivity(); /*!< To start the processing thread */

    /**
     * @brief splitPacket reconstitute incoming data and find messages.
     * @param packet Raw data coming from the sensor.
     * @param length Length of the data.
     * @param time Time of the last received data.
     *
     * Analyse the ethernet packet received from the Sick sensor and try to find a
     * complete message (scan data message or object message)
     * If a message has been found it is added at the end of the message list
     * else the pending bytes are stored to be analyzed by further incoming data.
     */
    void splitPacket(const char * packet, const int length, road_time_t time);


    /**
     * @brief Process/decode a message.
     * @param msg The message is encapsulated into a MessageLDMRS
     * @return Type of the message
     *
     * Process the raw data of the message and update the MessageLDMRS object passed : it fills the 2 headers (message and scan)
     * and replace the body field of the MessageLDRMS object by an array of ScanPoint.
     * - @b Warning : the process of object data type is not implemented yet !
     */
    unsigned long processMessage(MessageLDMRS & msg);

    //! Find the position of the magic word into the array and returns this index.
    /*!
     * \param message Array of characters, raw data received from sensor.
     * \param length Length of the array.
     * \return
     * - @b -1 if no magic word is found
     * - @b position of the magic word otherwise
     */
    u_int32_t findMagicWord(const char * message, const unsigned length);

    /**
    * @brief getMessageSize get the message size of the entire message.
    * @param message Raw data of the message.
    * @param length Length of the raw data received.
    * @param magicWordIndex First element of the message, used to get the size of the message.
    * @return The @b size of the whole message.
    *
    * The size of the message is found inside the message thanks to an offset after the index of the Magic Word.
    * - The first header of the message that contains the size of the message is in \b Big \b Endian format !
    *
    */
    u_int32_t getMessageSize(const char * message, const unsigned length, const long magicWordIndex);

    /**
     * @brief isMessageComplete compare the size of the message read into the message and the length of the received data.
     * @param length Length of the received data.
     * @param size Size of the message read. See getMessageSize.
     * @return @b true if the message is complete, @b false otherwise
     */
    bool isMessageComplete(const unsigned length, const long size);

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

    /// List of messages to process.
    std::list<MessageLDMRS> msgList;

    /// Received raw data is appended into this MessagePacket.
    MessagePacket pendingBytes;

    /// Append new data into the private MessagePacket @b pendingBytes.
    void storePendingBytes(road_time_t time);

    /**
     * @brief fillDataHeader fills the message header of the message
     * @param msg
     * - @b Warning:
     *          - the body field of the message have to be completed before
     *          - Data header format is @b Big @b Endian
     */
    void fillDataHeader(MessageLDMRS& msg);


    /**
     * @brief fillScanHeader fill the scan header of the message
     * @param msg Raw data must be stored into the body field of the message
     * - @b Warning
     *          - the body field of the message have to be completed before
     *          - Scan header format is @b Little @b Endian
     */
    void fillScanHeader(MessageLDMRS& msg);

    /**
     * @brief Write data into .dbt + .utc files.
     * @param msg
     *
     * SickLDMRS_dbt structure is filled and stored into the DBT file.
     * The array of ScanPoint is then stored into the UTC file.
     */
    void writeData(MessageLDMRS &msg);

    std::string kSickDbtFileName; //!< Name of the DBT file.
    std::string kSickUtcFileName; //!< Name of the UTC file.

    pacpus::DbiteFile dbtFile_; //!< DBT file.
    std::ofstream dataFile_; //!< UTC file.


#ifdef SICKLDMRS_SH_MEM
    ShMem * shmem_;
    SickLDMRS_shMem sickData;
#endif

};

} // namespace pacpus

#endif // SICKLDMRSSENSOR_H
