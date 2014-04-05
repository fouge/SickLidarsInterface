/*********************************************************************
//  created:    2014-02-02
//  filename:   AbstractSickSensor.h
//
//  author:     Cyril Fougeray
//              Copyright Heudiasyc UMR UTC/CNRS 6599
//
//  version:    $Id: $
//
//  purpose:    Abstract class to implement Sick sensors threads
//
*********************************************************************/

#ifndef ABSTRACTSICKSENSOR_H
#define ABSTRACTSICKSENSOR_H

#include <QThread>
#include "SickSocket.h"

namespace pacpus {

/// To separate data in UTC file = "UTC\0"
static const int32_t UTC_MAGIC_WORD = 0x55544300;

/**
 * \brief Structure used to stored Sick data between several decoding processes
 *
 * Note that data coming from sensors are split in IP packets. In order to get the
 * whole message in one block, MessagePacket is used to reconstitue the raw data.
 * As we want to reconstitute the message, it is useful to know if another packet
 * belonging to the message was previously received. Also, the time of the reception of
 * the first packet is carried into the structure in order to trace scans.
 */
struct MessagePacket {
    road_time_t time;
    std::string data;
    bool previousData;
};


/**
 * @brief The AbstractSickSensor class
 *
 * The AbstractSickSensor class provides the abstract model for implementing sensor
 * interfaces using SickComponent, used with the PACPUS Framework.
 */
class AbstractSickSensor : public QThread
{
    Q_OBJECT
public:
    void run(){}

    virtual void stopActivity() = 0 ; /*!< to stop the processing thread */
    virtual void startActivity() = 0; /*!< to start the processing thread */

    SickSocket * S_socket;

signals:

public slots:
    /**
     * @brief customEvent is a slot called when connected to a signal.
     * @param e QEvent that carries information from sensor.
     *
     * Sick sensor interface implementation uses this slot to get data from the remote sensor,
     * using IP packets. As long as Sick sensors use TCP/IP interface, it is advised to use this
     * slot to get the packets from the device.
     * @see SickFrameEvent
     */
    virtual void customEvent(QEvent * e) = 0;

protected:
    /// The SickLDMRS IP or hostname
    QString host_;

    /// The SickLDMRS port
    int port_;

    /// If data need to be recorded, set this member to true.
    bool recording;
};

}

#endif // ABSTRACTSICKSENSOR_H
