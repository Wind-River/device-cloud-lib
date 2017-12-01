/**
 * @file
 * @brief Contains implementations for handling base64 data
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
 */

#include "shared/iot_base64.h"

/**
 * @brief Place holder for the base64 invalid number value
 */
#define XX 100

/**
 * @brief Conversion table for decoding base64
 */
static const int base64_index[256] = {
	XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX,
	XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, 62, XX, XX, XX, 63,
	52, 53, 54, 55, 56, 57, 58, 59, 60, 61, XX, XX, XX, XX, XX, XX, XX, 0,  1,  2,  3,  4,  5,  6,
	7,  8,  9,  10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, XX, XX, XX, XX, XX,
	XX, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48,
	49, 50, 51, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX,
	XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX,
	XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX,
	XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX,
	XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX,
	XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX,
};

/** @var base64_list
 *   A 64 character alphabet.
 *
 *   A 64-character subset of International Alphabet IA5, enabling
 *   6 bits to be represented per printable character.  (The proposed
 *   subset of characters is represented identically in IA5 and ASCII.)
 *   The character "=" signifies a special processing function used for
 *   padding within the printable encoding procedure.
 *
 *   \verbatim
    Value Encoding  Value Encoding  Value Encoding  Value Encoding
       0 A            17 R            34 i            51 z
       1 B            18 S            35 j            52 0
       2 C            19 T            36 k            53 1
       3 D            20 U            37 l            54 2
       4 E            21 V            38 m            55 3
       5 F            22 W            39 n            56 4
       6 G            23 X            40 o            57 5
       7 H            24 Y            41 p            58 6
       8 I            25 Z            42 q            59 7
       9 J            26 a            43 r            60 8
      10 K            27 b            44 s            61 9
      11 L            28 c            45 t            62 +
      12 M            29 d            46 u            63 /
      13 N            30 e            47 v
      14 O            31 f            48 w         (pad) =
      15 P            32 g            49 x
      16 Q            33 h            50 y
    \endverbatim
 */
static const char base64_list[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

/**
 * @brief Decode a minimal memory block
 *
 * This function decodes a minimal memory area of four bytes into its decoded
 * equivalent.
 *
 * @param[out]     out                           pointer to destination
 * @param[in]      in                            pointer to source
 *
 * @returns -1 on error (illegal character) or the number of bytes decoded
 */
IOT_SECTION ssize_t iot_base64_decode_block( unsigned char out[3], const unsigned char in[4] );

/**
 * @brief Encodes one block of data in base64
 *
 * @param[out]     out                 output
 * @param[in]      in                  data to encode
 * @param[in]      len                 length of input data
 */
IOT_SECTION static void iot_base64_encode_block( uint8_t out[4], const uint8_t in[3], size_t len );

ssize_t iot_base64_decode( uint8_t *out, size_t out_len, const uint8_t *in, size_t in_len )
{
	ssize_t num_bytes = -1;
	if ( out && in )
	{
		size_t i = 0u;
		num_bytes = 0;

		while ( i < in_len && out_len > 0u && num_bytes >= 0 )
		{
			unsigned char out_temp[3];
			ssize_t out_chars = iot_base64_decode_block( out_temp, (const unsigned char *)in );
			if ( out_chars < 1 || out_chars > 3 )
				num_bytes = -1;
			else if ( out_chars > 0 && out_chars <= 3 )
			{
				ssize_t j;
				for ( j = 0; j < out_chars && out_len > 0u; ++j )
				{
					*out = (uint8_t)out_temp[j];
					--out_len;
					++out;
					++num_bytes;
				}
			}
			in += 4;
			i += 4u;
		}
	}
	return num_bytes;
}

ssize_t iot_base64_decode_block( unsigned char out[3], const unsigned char in[4] )
{
	ssize_t i, num_bytes = 3;
	char tmp[4];

	for ( i = 3; i >= 0 && num_bytes >= 0; i-- )
	{
		if ( in[i] == '=' )
		{
			tmp[i] = 0;
			--num_bytes;
		}
		else
		{
			tmp[i] = (char)base64_index[(unsigned char)in[i]];
		}

		if ( tmp[i] == XX )
			num_bytes = -1;
	}

	out[0] = (unsigned char)( tmp[0] << 2 | tmp[1] >> 4 );
	out[1] = (unsigned char)( tmp[1] << 4 | tmp[2] >> 2 );
	out[2] = (unsigned char)( ( ( tmp[2] << 6 ) & 0xc0 ) | tmp[3] );

	return num_bytes;
}

size_t iot_base64_decode_size( size_t in_bytes )
{
	size_t result = 0u;
	if ( in_bytes > 0u )
		result = 3u * ( 1u + ( ( in_bytes - 1u ) / 4u ) );
	return result;
}

size_t iot_base64_encode( uint8_t *out, size_t out_len, const uint8_t *in, size_t in_len )
{
	size_t result = 0u;
	if ( out && out_len > 0u )
	{
		if ( in && in_len > 0u )
		{
			while ( result + 4u <= out_len && in_len > 0u )
			{
				iot_base64_encode_block( out, in, in_len );

				out += 4;
				result += 4u;
				in += 3;
				if ( in_len >= 3u )
					in_len -= 3u;
				else
					in_len = 0u;
			}
		}
	}
	return result;
}

void iot_base64_encode_block( uint8_t out[4], const uint8_t in[3], size_t len )
{
	out[0] = (uint8_t)base64_list[in[0] >> 2];
	out[1] = (uint8_t)base64_list[( in[0] & 0x03 ) << 4];
	out[2] = '=';
	out[3] = '=';
	if ( len > 1u )
	{
		out[1] = (uint8_t)base64_list[( ( in[0] & 0x03 ) << 4 ) | ( ( in[1] & 0xf0 ) >> 4 )];
		out[2] = (uint8_t)base64_list[( in[1] & 0x0f ) << 2];
		if ( len > 2u )
		{
			out[2] = (uint8_t)base64_list[( ( in[1] & 0x0f ) << 2 ) | ( ( in[2] & 0xc0 ) >> 6 )];
			out[3] = (uint8_t)base64_list[in[2] & 0x3f];
		}
	}
}

size_t iot_base64_encode_size( size_t in_bytes )
{
	size_t result = 0u;
	if ( in_bytes > 0u )
		result = 4u * ( 1u + ( ( in_bytes - 1u ) / 3u ) );
	return result;
}
