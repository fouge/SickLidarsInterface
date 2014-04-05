/*********************************************************************
//  created:    2014/02/02
//  filename:   SickSocket.h
//
//  author:     Gerald Dherbomez
//              Modified by Cyril Fougeray
//              Copyright Heudiasyc UMR UTC/CNRS 6599
// 
//  version:    $Id: $
//
//  purpose:    Defintion of the SickSocket class
//              Management of the socket connection with Sick sensors
*********************************************************************/

#ifndef _SICKSOCKET_H_
#define _SICKSOCKET_H_

#include <cmath>
#include <QObject>
#include <QTcpSocket>
#include <QMutex>
#include <QEvent>

#include "Pacpus/kernel/road_time.h"

namespace pacpus {

class AbstractSickSensor;

// Sensors classes :
class SickLDMRSSensor;
class SickLMSSensor;

/**
 * @brief The SickFrame class
 */
class SickFrame
{
public:
    /**
     * @brief SickFrame constructor
     */
    SickFrame()
    {
        size = 0;
        time = 0;
        msg = NULL;
    }
    
    /// Destructor
    ~SickFrame()
    {
        delete[] msg; // check for NULL is not necessary
    }
    
    qint64 size; //!< Size of incoming packet.
    road_time_t time; //!< Time when packet is received.
    char * msg; //!< Packet (raw data).
};

/**
 * @brief The SickFrameEvent class
 * QEvent that encapsulates packets.
 */
class SickFrameEvent
        : public QEvent
{
public:
    /// Constructor
    SickFrameEvent()
            : QEvent((QEvent::Type)(QEvent::User + 522))
    {}
    
    /// Destructor
    ~SickFrameEvent()
    {}
    
    /// Packet data
    SickFrame * frame;
};

/**
 * @brief The SickSocket class
 * Handles the ethernet connection with the remote sensor.
 */
class SickSocket
        : public QObject
{
  Q_OBJECT 
 
public:
    /// Constructor
    SickSocket(AbstractSickSensor * parent);

    /// Destructor
    ~SickSocket();

public Q_SLOTS:
    /// Enable the connection to the server
    void connectToServer(QString host, int port);

    /// Warns about connection of the socket and launch configuration of the socket.
    int socketConnected();

    /** Called when incoming data is received. Create an event and send it to the sensor's handler. @see AbstractSickComponent */
    void socketReadyRead();  

    /// Close the connection with the server
    void closeSocket() { socket->close(); }

    /**
     * @brief sendToServer Sends data to the remote lidar.
     * @param data Data to be sent, translated in ASCII.
     */
    void sendToServer(QString data);
Q_SIGNALS:
    /// Asked for configuring sensor.
    void configuration(); 

protected slots:
    /// Says to the user the connection is closed.
    void socketConnectionClosed();

    /// Warns the user an error occured
    void socketError(QAbstractSocket::SocketError e); 

private:
    /// Socket
    QTcpSocket *socket; 

    /// Mutex to use socket resource.
    QMutex mutex;

    /// Parent (contains slots to connect to in order to pass the received data).
    AbstractSickSensor *myParent;
};

} // namespace pacpus

#endif // _SICKSOCKET_H_
