/*********************************************************************
//  created:    2014/02/02
//  filename:  SickSocket.cpp
//
//  author:     Cyril Fougeray
//              Copyright Heudiasyc UMR UTC/CNRS 6599
// 
//  version:    $Id: $
//
//  purpose:    Management of the socket connection with Sick sensors
*********************************************************************/

#include "SickSocket.h"

#include <QCoreApplication>
#include <QDebug>


#include "Pacpus/kernel/Log.h"

namespace pacpus {

DECLARE_STATIC_LOGGER("pacpus.base.SickSocket");


SickSocket::SickSocket(AbstractSickSensor * parent)
    : myParent(parent)
{
    socket = new QTcpSocket(this);
    connect( socket, SIGNAL(connected()), this,SLOT(socketConnected()) );
    connect( socket, SIGNAL(disconnected()),this, SLOT(socketConnectionClosed()) );
    connect( socket, SIGNAL(readyRead()),this, SLOT(socketReadyRead()) );
    connect( socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(socketError(QAbstractSocket::SocketError)) );
}


SickSocket::~SickSocket()
{
    delete socket;
}


void SickSocket::connectToServer(QString host, int port)
{
  qDebug("trying to connect to server");
  socket->connectToHost(host,port);
}


int SickSocket::socketConnected()
{ 
  qDebug() << "we are connected to the server " << socket->peerName().toLatin1() 
    << " at the port " << socket->peerPort() << " via the socket " << socket->socketDescriptor();
  emit configuration(); 
  
 return 1;
}

//////////////////////////////////////////////////////////////////////////
/// protected slot
void SickSocket::socketConnectionClosed()
{
  qDebug("the connection was closed");
}


void SickSocket::socketReadyRead()
{   
    SickFrame * frame = new SickFrame();
  frame->time = road_time(); 
  frame->size = socket->bytesAvailable(); 
  frame->msg = new char[frame->size]; 

  if (!frame->msg) {
      LOG_FATAL("cannot allocate memory of size " << frame->size
                << " - file: " << __FILE__
                << " line: " << __LINE__
                );
  }

  frame->size = socket->read(frame->msg, (qint64) frame->size);

  SickFrameEvent *e = new SickFrameEvent;
  e->frame = frame; 
  QCoreApplication::postEvent((QObject*)myParent, e);
}


void SickSocket::socketError(QAbstractSocket::SocketError e)
{
  qWarning("socket error number %d",e); 
}


void SickSocket::sendToServer(QString data) //a adapter aux données binaires
{
  mutex.lock();
  QTextStream os(socket);
  os << data.toAscii() << endl;
  mutex.unlock();
  qDebug() << "data sent to server: " << data.toAscii() ; //a adapter aussi
}

} // namespace pacpus
