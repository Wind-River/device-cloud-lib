/**
 * @file
 * @brief internal definitions inside the IoT library
 *
 * @copyright Copyright (C) 2017 Wind River Systems, Inc. All Rights Reserved.
 *
 * @license The right to copy, distribute or otherwise make use of this software
 * may be licensed only pursuant to the terms of an applicable Wind River
 * license agreement.  No license to Wind River intellectual property rights is
 * granted herein.  All rights not licensed by Wind River are reserved by Wind
 * River.
 */
#ifndef IOT_DEFS_H
#define IOT_DEFS_H

#include "iot.h"

/**
 * @def UNUSED
 * @brief Macro used to define an unused attribute to the compiler
 */
#ifdef __GNUC__
#	define UNUSED(x) PARAM_ ## x __attribute((__unused__))
#else
#	define  __attribute__(x) /* gcc specific */
#	define UNUSED(x) x
#endif /* ifdef __GNUC__ */

/**
 * @brief Maximum length of an ID for Internet of Things devices
 * @note A minimum length of 36 should be used to support UUIDs
 */
#define IOT_ID_MAX_LEN                 36u
/** @brief Maximum name length (128 = Matches AWS name) */
#define IOT_NAME_MAX_LEN               128u
/** @brief Maximum length of a host name on POSIX systems */
#ifndef _POSIX_HOST_NAME_MAX
#	ifdef __clang__
#	pragma clang diagnostic push
#	pragma clang diagnostic ignored "-Wreserved-id-macro"
#		define _POSIX_HOST_NAME_MAX    64
#	endif /* ifdef __clang__ */
#endif
/** @brief Maximum host name length */
#define IOT_HOST_MAX_LEN _POSIX_HOST_NAME_MAX
/** @brief Maximum username length */
#define IOT_USERNAME_MAX_LEN           32u
/** @brief Maximum password length */
#define IOT_PASSWORD_MAX_LEN           32u
/** @brief Maximum topic length */
#define IOT_TOPIC_MAX_LEN              255u
/** @brief Maximum length for proxy type */
#define IOT_PROXY_TYPE_MAX_LEN         16u

/** @brief Maximum length of cloud certification validating or not */
/*#define IOT_VALIDATE_CERT_MAX_LEN      6u*/
/** @brief Maximum length of cloud connection token  */
/*#define IOT_TOKEN_MAX_LEN              255u*/

/** @brief Number of microseconds in millisecond */
#define IOT_MICROSECONDS_IN_MILLISECOND 1000u
/** @brief Number of milliseconds in a second */
#define IOT_MILLISECONDS_IN_SECOND     1000u
/** @brief Number of nanoseconds in a millisecond */
#define IOT_NANOSECONDS_IN_MILLISECOND 1000000u
/** @brief Number of nanoseconds in a second */
#define IOT_NANOSECONDS_IN_SECOND      1000000000uL
/** @brief Number of seconds in a minute */
#define IOT_SECONDS_IN_MINUTE          60u
/** @brief Number of minutes in an hour */
#define IOT_MINUTES_IN_HOUR            60u
/** @brief Number of hours in a day */
#define IOT_HOURS_IN_DAY               24u

/** @brief Character used to split a name into levels (i.e parent & children) */
#define IOT_SPLIT_PARENT               "/"
/** @brief Character used to split a name into parts (i.e. name & version) */
#define IOT_SPLIT_VERSION              ";"

/** @brief Flag indicating whether 'accuracy' field is set */
#define IOT_FLAG_LOCATION_ACCURACY               (0x01)
/** @brief Flag indicating whether 'altitude' field is set */
#define IOT_FLAG_LOCATION_ALTITUDE               (0x02)
/** @brief Flag indicating whether 'altitude_accuracy' field is set */
#define IOT_FLAG_LOCATION_ALTITUDE_ACCURACY      (0x04)
/** @brief Flag indicating whether 'heading' field is set */
#define IOT_FLAG_LOCATION_HEADING                (0x08)
/** @brief Flag indicating whether 'source_type' field is set */
#define IOT_FLAG_LOCATION_SOURCE                 (0x10)
/** @brief Flag indicating whether 'speed' field is set */
#define IOT_FLAG_LOCATION_SPEED                  (0x20)
/** @brief Flag indicating whether 'tag' field is set */
#define IOT_FLAG_LOCATION_TAG                    (0x40)

/** @brief Maximum number of retries for file transfer.  For unlimited retries use -1 */
#define IOT_TRANSFER_MAX_RETRIES      -1
/** @brief Curl low speed limit in bytes/sec */
#define IOT_TRANSFER_LOW_SPEED_LIMIT   50L
/** @brief Curl low speed timeout in seconds */
#define IOT_TRANSFER_LOW_SPEED_TIMEOUT 30L

/** @brief Current item ( action or telemetry ) state */
enum iot_item_state
{
	/** @brief item is deregistered */
	IOT_ITEM_DEREGISTERED = 0,
	/** @brief item is pending to be deregistered */
	IOT_ITEM_DEREGISTER_PENDING,
	/** @brief item is registered */
	IOT_ITEM_REGISTERED,
	/** @brief item is pending to be registered */
	IOT_ITEM_REGISTER_PENDING,
};

#endif /* ifndef IOT_DEFS_H */

