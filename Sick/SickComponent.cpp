/*********************************************************************
//  created:    2014-02-01 - 12:08
//  filename:   SickComponent.cpp
//
//  author:     Cyril Fougeray
//              Copyright Heudiasyc UMR UTC/CNRS 6599
// 
//  version:    $Id: $
//
//  purpose:    The acquisition component of the Sick sensors (parent class)
//
*********************************************************************/

#include "SickComponent.h"

//#include "AlascaDataGenerator.h"
#include "SickSocket.h"
#include "Pacpus/kernel/ComponentFactory.h"
#include "Pacpus/kernel/DbiteException.h"
#include "Pacpus/kernel/DbiteFileTypes.h"
#include "Pacpus/kernel/Log.h"
#include "Pacpus/PacpusTools/ShMem.h"

#include <iostream>
#include <QTcpSocket>
#include <string>

using namespace std;


namespace pacpus {

DECLARE_STATIC_LOGGER("pacpus.base.SickComponent");

// Construct the factory
static ComponentFactory<SickComponent> sFactory("SickComponent");


SickComponent::SickComponent(QString name)
    : ComponentBase(name)
{   
    LOG_TRACE("constructor(" << name << ")");

    S_sensors = new std::vector<AbstractSickSensor*>();
}

SickComponent::~SickComponent()
{
    LOG_TRACE("destructor");
}


void SickComponent::startActivity()
{
    for(std::vector<AbstractSickSensor*>::iterator it = S_sensors->begin(); it != S_sensors->end(); ++it){
        (*it)->startActivity();
    }
}


void SickComponent::stopActivity()
{
    for(std::vector<AbstractSickSensor*>::iterator it = S_sensors->begin(); it != S_sensors->end(); ++it){
        (*it)->stopActivity();
    }
}


ComponentBase::COMPONENT_CONFIGURATION SickComponent::configureComponent(XmlComponentConfig config)
{
    /*
     * <Sick sickldmrs_0="192.168.0.1:2111" sicklms151_0="192.168.0.10:2111" sicklms511_0="192.168.1.50:2111">
     */
    // Sick LD-MRS
    int num = 0;
    while (param.getProperty("sickldmrs_"+QString::number(num)) != QString::null){
        QString information = param.getProperty("sickldmrs_"+QString::number(num));
        QStringList list = information.split(":");
        int recording = 0;
        if (param.getProperty("sickldmrs_"+QString::number(num)+"_recording") != QString::null)
        {
             recording = param.getProperty("sickldmrs_"+QString::number(num)+"_recording").toInt();
        }

        S_sensors->push_back(new SickLDMRSSensor(this, "sickldmrs_"+QString::number(num), list.at(0), list.at(1).toInt(), recording));
        ++num;
    }

    // Sick LMS 151
       num = 0;
     while (param.getProperty("sicklms151_"+QString::number(num)) != QString::null){
        QString information = param.getProperty("sicklms151_"+QString::number(num));
        QStringList list = information.split(":");
        int recording = 0;
        if (param.getProperty("sicklms151_"+QString::number(num)+"_recording") != QString::null)
        {
             recording = param.getProperty("sicklms151_"+QString::number(num)+"_recording").toInt();
        }

        S_sensors->push_back(new SickLMSSensor(this, "sicklms151_"+QString::number(num), list.at(0), list.at(1).toInt(), recording));
        ++num;
    }

     // Sick LMS 511
     num = 0;
   while (param.getProperty("sicklms511_"+QString::number(num)) != QString::null){
      QString information = param.getProperty("sicklms511_"+QString::number(num));
      QStringList list = information.split(":");
      int recording = 0;
      if (param.getProperty("sicklms511_"+QString::number(num)+"_recording") != QString::null)
      {
           recording = param.getProperty("sicklms511_"+QString::number(num)+"_recording").toInt();
      }

      S_sensors->push_back(new SickLMSSensor(this, "sicklms511_"+QString::number(num), list.at(0), list.at(1).toInt(), recording));
      ++num;
  }

    return ComponentBase::CONFIGURED_OK;
}


} // namespace pacpus
