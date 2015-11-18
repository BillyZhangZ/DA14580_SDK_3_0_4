/**
 ****************************************************************************************
 *
 * @file image.h
 *
 * @brief Definition of image header structure.
 *
 * Copyright (C) 2014. Dialog Semiconductor Ltd, unpublished work. This computer 
 * program includes Confidential, Proprietary Information and is a Trade Secret of 
 * Dialog Semiconductor Ltd.  All use, disclosure, and/or reproduction is prohibited 
 * unless authorized in writing. All Rights Reserved.
 *
 * <bluetooth.support@diasemi.com> and contributors.
 *
 ****************************************************************************************
 */

#ifndef __IMAGE_H
#define __IMAGE_H

#include <stdint.h>

struct image_header {
	uint8_t signature[2];
	uint8_t reserved_prog[2];
	uint8_t code_size[4];
	uint8_t CRC[4] ;
	uint8_t version[16];
	uint8_t timestamp[4];
	uint8_t reserved[32];
};

#endif  /* __IMAGE_H */
