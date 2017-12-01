/**
 * @file
 * @brief Header file defining md5 calculation operations
 *
 * @copyright Copyright (C) 2016-2017 Wind River Systems, Inc. All Rights Reserved.
 *
 * @license Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software distributed
 * under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES
 * OR CONDITIONS OF ANY KIND, either express or implied."
 *
 * The code in this file is based on an implementation provided by Alexander Peslyak.
 * The original code contained the following copyright information:
 *
 * This is an OpenSSL-compatible implementation of the RSA Data Security, Inc.
 * MD5 Message-Digest Algorithm (RFC 1321).
 *
 * Homepage:
 * http://openwall.info/wiki/people/solar/software/public-domain-source-code/md5
 *
 * Author:
 * Alexander Peslyak, better known as Solar Designer <solar at openwall.com>
 *
 * This software was written by Alexander Peslyak in 2001.  No copyright is
 * claimed, and the software is hereby placed in the public domain.
 * In case this attempt to disclaim copyright and place the software in the
 * public domain is deemed null and void, then the software is
 * Copyright (c) 2001 Alexander Peslyak and it is hereby released to the
 * general public under the following terms:
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted.
 *
 * There's ABSOLUTELY NO WARRANTY, express or implied.
 */
#ifndef DEVICE_MANAGER_MD5_H
#define DEVICE_MANAGER_MD5_H

/**
 * @brief Length of a MD5 hash
 */
#define DEVICE_MANAGER_MD5_DIGEST_LENGTH                     16u

/**
 * @brief Length of a MD5 hash when in hexidecimal format
 */
#define DEVICE_MANAGER_MD5_DIGEST_HEX_LENGTH                 (DEVICE_MANAGER_MD5_DIGEST_LENGTH * 2u)

/** @brief Any 32-bit or wider unsigned integer data type will do */
typedef unsigned int MD5_u32plus;

/** @brief MD5 context, used to calculate the sum between iterations */
typedef struct device_manager_md5_ctx {
	/** @brief low value */
	MD5_u32plus lo;
	/** @brief high value */
	MD5_u32plus hi;
	/** @{ */
	/** @brief 4 offsets used in md5 calculation */
	MD5_u32plus a, b, c, d;
	/** @} */
	/** @brief Buffer used during calculation */
	unsigned char buffer[DEVICE_MANAGER_MD5_DIGEST_LENGTH * 4u];
	/** @brief MD5 block */
	MD5_u32plus block[DEVICE_MANAGER_MD5_DIGEST_LENGTH];
} device_manager_md5_ctx_t;

/**
 * @brief Converts a calculated md5 result to a null-terminated hexidecimal string
 *
 * @param[out]     hex                           hexidecimal string representation
 * @param[in]      md5                           previously calculated md5
 *
 * @note the resulting hex-decimal characters will be in @b lower-case
 * @note @c hex must be of MD5_DIGEST_HEX_LENGTH + 1u length in size
 * @note @c md5sum must be of MD5_DIGEST_LENGTH in size
 */
void device_manager_md5_convert_to_hex( char* hex, const unsigned char* md5 );

/**
 * @brief Returns the final calculated result
 *
 * @param[in,out]  result                        location to place result
 * @param[in,out]  ctx                           md5 context structure, zeros the structure
 */
void device_manager_md5_final( unsigned char *result, struct device_manager_md5_ctx *ctx );

/**
 * @brief Initializes the MD5 Context structure
 *
 * @param[in,out]  ctx                           structure to initialize
 */
void device_manager_md5_initialize( struct device_manager_md5_ctx *ctx );

/**
 * @brief Called repeatably with chunks of data to calculate MD5 hash of data
 *
 * @param[in,out]  ctx                           md5 context structure
 * @param[in]      data                          data to use in calculating hash
 * @param[in]      size                          length of data
 */
void device_manager_md5_update( struct device_manager_md5_ctx *ctx, const void *data, unsigned long size );

#endif /* ifndef DEVICE_MANAGER_MD5_H */
