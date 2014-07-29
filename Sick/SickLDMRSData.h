/*********************************************************************
//  created:    2014/02/11 - 12:08
//  filename:   SickLDMRSData.h
//
//  author:     Cyril Fougeray
//              Copyright Heudiasyc UMR UTC/CNRS 6599
//
//  version:    $Id: $
//
//  purpose:    Structures to store Sick LDMRS data
//
*********************************************************************/

#ifndef __SICKLDMRS_DATA_H__
#define __SICKLDMRS_DATA_H__

#include "Pacpus/kernel/cstdint.h"
#include "Pacpus/kernel/road_time.h"

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

namespace pacpus{

/*!
 * \brief The DataHeader struct
 *
 * The DataHeader struct describes general information about the message used with.
 * On Sick LDMRS, DataHeader corresponds exactly to the very first data carried into the whole message.
 * See [Ethernet data protocol LD-MRS, page 4](docs/BAMessdatenProtokoll_LDMRSen_8014492_20110601.pdf).
 * > Warning : the data from the sensor is coded in Big Endian format.
 */
struct DataHeader {
    u_int32_t magicWord;            //!< 0xAFFEC0C2 for the Sick LDMRS sensor (this value must be found in order to decode the message).
    u_int32_t sizePreviousMessage;  //!< Size in bytes of the previous message.
    u_int32_t sizeCurrentMessage;   //!< Size of the message content without the header (DataHeader).
    // u_int8_t reserved
    u_int8_t deviceId;              //!< Unused in data received directly from LD-MRS sensors.
    u_int16_t dataType;             //!< Type of information carried into the message.
                                    ///< Types used are :
                                    ///<     - Points : 0x2202
                                    ///<     - Objects : 0x2221
    u_int64_t ntpTime;              //!< Time of the sensor when the message is created
};


/*!
 * \brief The ScanHeader struct
 *
 * General information about points measured.
 * Data type is 0x2202
 * @see DataHeader
 *
 * see Ethernet data protocol LD-MRS page 5
 */
struct SICKLDMRS_API ScanHeader {
    u_int16_t scanNumber;       //!< Number of the scan since the sensor started measuring.
    u_int16_t scannerStatus;    //!< Status of the scanner
                                /**<
                                 * - 0x0007: reserved,
                                 * - 0x0008: set frequency reached,
                                 * - 0x0010: external sync signal detected,
                                 * - 0x0020: sync ok,
                                 * - 0x0040: sync master (instead of slave),
                                 * - 0xFF80: reserved
                                 */

    u_int16_t phaseOffset;  ///<
    u_int64_t startNtpTime; //!< NTP time first measurement
    u_int64_t endNtpTime;   //!< NTP time last measurement
    u_int16_t ticksPerRot;  //!< Angle ticks per rotation (used to compute the real angle of a point)
    int16_t startAngle;     //!< Angle of the first measured value
    int16_t endAngle;       //!< Angle of the last measured value
    u_int16_t numPoints;    //!< Number of scanned points during this scan @see ScanPoint

    // mounting position; reference ?
//    int16_t mountingYawAngle;
//    int16_t mountingPitchAngle;
//    int16_t mountingRollAngle;
//    int16_t mountingX;
//    int16_t mountingY;
//    int16_t mountingZ;

    // u_int16_t reserved;

};


/*!
 * \brief The ScanPoint struct
 *
 * Used to describe a point.
 * Data type 0x2202 @see DataHeader
 */
struct SICKLDMRS_API ScanPoint{
    u_char layerEcho;           //!< 4 LSB : Layer (scan layer of the point)
                                //!< 4 MSB : Echo
    u_char flags;
    u_int16_t angle;            //!< Angle in number of ticks. You can easily compute the real angle :
                                //!< \f$ angle (degree) = \frac{angle (ticks)}{ScanHeader.ticksPerRot}\f$ @see ScanHeader

    u_int16_t distance;         //!< Distance of the point from the sensor in centimeters.
    u_int16_t echoPulseWidth;   //!< Width of echo pulse (cm)
    // u_int16_t reserved;
};

/*!
 * \brief The ScanObject struct (not used)
 *
 * Used to describe an object.
 * Data type 0x2221 @see DataHeader
 */
struct SICKLDMRS_API ScanObject{
    // TODO
};



/*! \brief The SickLDMRS_dbt struct
 *
 * Data recorded in the DBITE file (.dbt).
 */
typedef struct
{
    u_int64_t timeStartFromSensor;  //!< NTP time (creation of the message on sensor).
    ScanHeader hScan;               //!< General information about points recorded. @see ScanHeader
    road_time_t time;               //!< DBT timestamp.
    road_timerange_t timerange;     //!< DBT timerange.
    int32_t dataPos;                //!< The position of the data in the binary file associated to the dbt file (utc file).
} SickLDMRS_dbt;



#ifdef SICKLDMRS_SH_MEM
/// Structure to write in shared memory, followed by the points.
typedef struct{
    ScanHeader scanInfo;            //!< General information about points recorded. @see ScanHeader
    road_time_t time;               //!< DBT timestamp
    road_timerange_t timerange;     //!< DBT timerange

        /// In shared memory, followed by ScanPoint[scanInfo.numPoints] @see ScanPoint

} SickLDMRS_shMem;
#endif

}

#endif
