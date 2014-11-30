/*
 * Checksum handling
 *
 * Copyright Matthias Bruestle 1999-2002
 * For licensing, see the file LICENCE
 */

#ifndef __CHECKSUM_H__
#define __CHECKSUM_H__


#include <stdint.h>
#include <unistd.h>

extern unsigned int	csum_lrc_compute(const uint8_t *, size_t, unsigned char *);
extern unsigned int	csum_crc_compute(const uint8_t *, size_t, unsigned char *);

#endif

