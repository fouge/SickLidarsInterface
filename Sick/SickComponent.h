/*********************************************************************
//  created:    2014-11-11 - 10:48
//  filename:   SickComponent.h
//
//  author:     Cyril Fougeray
//              Copyright Heudiasyc UMR UTC/CNRS 6599
// 
//  version:    $Id: $
//
//  purpose:    Declaration of the SickComponent class
*********************************************************************/

#ifndef SICKCOMPONENT_H
#define SICKCOMPONENT_H

#include "Pacpus/kernel/ComponentBase.h"
#include "Pacpus/kernel/DbiteFile.h"

#include <fstream>
#include <QThread>
#include <string>

#include "SickLDMRSSensor.h"
#include "SickLMSSensor.h"

// Export macro for Sick DLL for Windows only
#ifdef WIN32
#   ifdef SICK_EXPORTS
        // make DLL
#       define SICK_API __declspec(dllexport)
#   else
        // use DLL
#       define SICK_API __declspec(dllimport)
#   endif
#else
    // On other platforms, simply ignore this 
#   define SICK_API
#endif


class QEvent;

namespace pacpus {
    
/**
 * @brief The SickComponent class
 *
 * This class defines a PACPUS component used to acquire Sick lidars data.
 */
class SICK_API SickComponent
        : /*public QThread
        , */
        public QObject, public ComponentBase
{
    Q_OBJECT

public:
    /// Constructor
    SickComponent(QString name);

    /// Destructor
    ~SickComponent();

   // void run() {}

    virtual void stopActivity(); /*!< To stop the processing thread */
    virtual void startActivity(); /*!< To start the processing thread */

    /**
     * @brief Configure compenent.
     * @param config XML file passed in order to configure the Sick Component.
     *
     * This function instanciate every sensors configured through the XML file.
     * The XML file must be formated as expected by this function.
     * Depending on used sensors and how many, you should define these three property in a "Sick" node (X must start from '0') :
     * - sickldmrs_X
     * - sicklms151_X
     * - sicklms511_X
     *
     * For example, let's say we have two Sick LMS151, one LDMRS and one LMS511 :
     *
     * <tt> <Sick type="SickComponent" sickldmrs_0="192.168.0.1:2111" sicklms151_0="192.168.0.10:2111" sicklms151_1="192.168.0.11:2111" sicklms511_0="192.168.1.50:2111" /> </tt>
     *
     * Do not forget \c type="SickComponent".
     */
    virtual ComponentBase::COMPONENT_CONFIGURATION configureComponent(XmlComponentConfig config);

    SickComponent * myParent;

private:
    //! Vector of the different sensors used.
    std::vector<AbstractSickSensor*> *S_sensors;
};

} // namespace pacpus

#endif // SICKCOMPONENT_H
