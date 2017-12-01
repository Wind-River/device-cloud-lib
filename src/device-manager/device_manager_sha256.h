/**
 * @file
 * @brief Source file defining sha256 calculation operations
 *
 * @copyright Copyright (C) 2017 Wind River Systems, Inc. All Rights Reserved.
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
 * Disclaimer: This code is presented "as is" without any guarantees.
 * Details:    Implementation of the SHA-256 hashing algorithm.
 *             SHA-256 is one of the three algorithms in the SHA2
 *             specification. The others, SHA-384 and SHA-512, are not
 *             offered in this implementation.
 *             Algorithm specification can be found here:
 *             http://csrc.nist.gov/publications/fips/fips180-2/fips180-2withchangenotice.pdf
 */

#ifndef DEVICE_MANAGER_MDSHA256H
#define DEVICE_MANAGER_MDSHA256H

/**
 * @brief Length of an SHA256 hash
 */
#define DEVICE_MANAGER_SHA256_DIGEST_SIZE 32u
/**
 * @brief Length of an SHA256 hash when in hexidecimal format
 */
#define DEVICE_MANAGER_SHA256_DIGEST_HEX_LENGTH (DEVICE_MANAGER_SHA256_DIGEST_SIZE * 2u)

/** @brief SHA256 context, used to calculate the sum between iterations */
typedef struct {
	/** @brief data pointer to 64-bytes of data for calculation */
	unsigned char data[64];
	/** @brief length of data */
	unsigned int datalen;
	/** @brief bit length array */
	unsigned int bitlen[2];
	/** @brief state array */
	unsigned int state[8];
} device_manager_sha256_ctx;

/**
 * @brief Converts a calculated sha256 result to a null-terminated hexidecimal string
 *
 * @param[out]     hex                           hexidecimal string representation
 * @param[in]      sha256                        previously calculated sha256
 *
 * @note the resulting hex-decimal characters will be in @b lower-case
 * @note @c hex must be of SHA256_DIGEST_HEX_LENGTH + 1u length in size
 * @note @c md5sum must be of SHA256_DIGEST_LENGTH in size */
void device_manager_sha256_convert_to_hex( char* hex, const unsigned char* sha256 );
/**
 * @brief Initializes the SHA256 Context structure
 *
 * @param[in,out]  ctx                           structure to initialize
 */
void device_manager_sha256_init(device_manager_sha256_ctx *ctx);
/**
 * @brief Called repeatably with chunks of data to calculate SHA256 hash of data
 *
 * @param[in,out]  ctx                           sha256 context structure
 * @param[in]      data                          data to use in calculating hash
 * @param[in]      len                           length of data
 */
void device_manager_sha256_update(device_manager_sha256_ctx *ctx, unsigned char *data, unsigned int len);
/**
 * @brief Returns the final calculated result
 *
 * @param[in,out]  ctx                           sha256 context structure, zeros the structure
 * @param[in,out]  hash                          location to place result
 */
 void device_manager_sha256_final(device_manager_sha256_ctx *ctx, unsigned char *hash);

#endif /* DEVICE_MANAGER_MDSHA256H */
