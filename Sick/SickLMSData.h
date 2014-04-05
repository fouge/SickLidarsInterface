/*********************************************************************
//  created:    2014/02/11 - 12:08
//  filename:   SickLMSData.cpp
//
//  author:     Cyril Fougeray
//              Copyright Heudiasyc UMR UTC/CNRS 6599
//
//  version:    $Id: $
//
//  purpose:    Structures to store Sick LMS data
//
*********************************************************************/

#ifndef __SICKLMS_DATA_H__
#define __SICKLMS_DATA_H__

#include "Pacpus/kernel/cstdint.h"
#include "Pacpus/kernel/road_time.h"

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


namespace pacpus{

/*!
* \brief Structure containing scan configuration.
* \author Based on Konrad Banachowicz work.
*/
typedef struct _scanCfg {
    /*!
     * \brief Scanning frequency.
     * 1/100 Hz
     */
    int scaningFrequency;

    /*!
     * \brief Scanning resolution.
     * 1/10000 degree
     */
    int angleResolution;

    /*!
     * \brief Start angle.
     * 1/10000 degree
     */
    int startAngle;

    /*!
     * \brief Stop angle.
     * 1/10000 degree
     */
    int stopAngle;
} scanCfg;


/*!
* \brief Structure containing single scan message.
* \author Based on Konrad Banachowicz work.
*/
typedef struct _scanData {

    /*!
     * \brief Scanning frequency.
     * 1/100 Hz
     */
    u_int32_t scanFrequency;

    /*!
     * \brief Scanning resolution.
     * 1/10000 degree
     */
    u_int32_t angleResolution;

    /*!
     * \brief Start angle.
     * 1/10000 degree
     */
    int32_t startAngle;

    /////////////////////////////////////////////////////
    ////////////////// LMS1xx & LMS5xx //////////////////

    /*!
     * \brief Number of samples in dist1.
     *
     */
    int dist_len1;

    /*!
     * \brief Radial distance for the first reflected pulse
     *
     */
    uint16_t* dist1;

    /*!
     * \brief Number of samples in dist2.
     *
     */
    int dist_len2;

    /*!
     * \brief Radial distance for the second reflected pulse
     *
     */
    uint16_t* dist2;


    /////////////////////////////////////////////////////
    //////////////////   LMS5xx only   //////////////////

    /*!
     * \brief Number of samples in dist3.
     *
     */
    int dist_len3;

    /*!
     * \brief Radial distance for the first reflected pulse
     *
     */
    uint16_t* dist3;

    /*!
     * \brief Number of samples in dist4.
     *
     */
    int dist_len4;

    /*!
     * \brief Radial distance for the second reflected pulse
     *
     */
    uint16_t* dist4;

    /*!
     * \brief Number of samples in dist5.
     *
     */
    int dist_len5;

    /*!
     * \brief Radial distance for the second reflected pulse
     *
     */
    uint16_t* dist5;


    /////////////////////////////////////////////////////
    //////////////////   LMS1xx only   //////////////////

    /*!
     * \brief Number of samples in rssi1.
     *
     */
    int rssi_len1;

    /*!
     * \brief Energy values for the first reflected pulse
     *
     */
    uint16_t* rssi1;

    /*!
     * \brief Number of samples in rssi2.
     *
     */
    int rssi_len2;

    /*!
     * \brief Energy values for the second reflected pulse
     *
     */
    uint16_t* rssi2;
} scanData;


/*!
  * \brief Status of the scanner
  */
typedef enum {
    undefined = 0,
    initialisation = 1,
    configuration = 2,
    idle = 3,
    rotated = 4,
    in_preparation = 5,
    ready = 6,
    ready_for_measurement = 7
} status_t;


typedef struct{
    u_int16_t scanNumber;           //!< number of the scan
    u_int16_t scannerStatus;
        //!< - 00 00 OK
        //!< - 00 01 Error
        //!< - 00 02 Pollution Warning
        //!< - 00 04 Pollution Error

    road_time_t time;               //!< DBT timestamp
    road_timerange_t timerange;     //!< DBT timerange
    u_int32_t scanFrequency;        //!< Frequency of the scan [1/100 Hz]
    u_int32_t angleResolution;      //!< Angle resolution (default is 5000 <=> 0.5 degree) [1/10000 degree]
    int32_t startAngle;             //!< Angle of the first scanned point.
    int dist_len1;                  //!< Number of points (1st echo).
    uint32_t dataPos_dist1;         //!< Distance between the sensor and the remote point (1st echo).
    int dist_len2;                  //!< Number of points (2nd echo).
    uint32_t dataPos_dist2;         //!< Distance between the sensor and the remote point (2nd echo).
    int dist_len3;                  //!< Number of points (3rd echo).
    uint32_t dataPos_dist3;         //!< Distance between the sensor and the remote point (3rd echo).  @b LMS5xx @b only.
    int dist_len4;                  //!< Number of points (4th echo).
    uint32_t dataPos_dist4;         //!< Distance between the sensor and the remote point (4th echo).  @b LMS5xx @b only.
    int dist_len5;                  //!< Number of points (5th echo).
    uint32_t dataPos_dist5;         //!< Distance between the sensor and the remote point (5th echo).  @b LMS5xx @b only.
    int rssi_len1;                  //!< Number of energy values (1st echo).
    uint32_t dataPos_rssi1;         //!< Energy of the returned pulse (1st echo). @b LMS1xx @b only.
    int rssi_len2;                  //!< Number of energy values (2nd echo).
    uint32_t dataPos_rssi2;         //!< Energy of the returned pulse (2nd echo).  @b LMS1xx @b only.
}SickLMS_dbt;

}
#endif
