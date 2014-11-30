/*
 * Implementation of T=1
 *
 * Copyright (C) 2003, Olaf Kirch <okir@suse.de>
 *
 * improvements by:
 * Copyright (C) 2004 Ludovic Rousseau <ludovic.rousseau@free.fr>
 */

/* $Id: proto-t1.h 3292 2009-01-26 13:02:58Z rousseau $ */

#ifndef __PROTO_T1_H__
#define __PROTO_T1_H__



#include <stdint.h>

#include <unistd.h>

#include "buffer.h"
#ifdef __cplusplus
extern "C" {
#endif
    

/* T=1 protocol constants */
#define T1_I_BLOCK		0x00
#define T1_R_BLOCK		0x80
#define T1_S_BLOCK		0xC0
#define T1_MORE_BLOCKS		0x20

enum {
	IFD_PROTOCOL_RECV_TIMEOUT = 0x0000,
	IFD_PROTOCOL_T1_BLOCKSIZE,
	IFD_PROTOCOL_T1_CHECKSUM_CRC,
	IFD_PROTOCOL_T1_CHECKSUM_LRC,
	IFD_PROTOCOL_T1_IFSC,
	IFD_PROTOCOL_T1_IFSD,
	IFD_PROTOCOL_T1_STATE,
	IFD_PROTOCOL_T1_MORE
};

#define T1_BUFFER_SIZE		(3 + 254 + 2)

/* see /usr/include/PCSC/ifdhandler.h for other values
 * this one is for internal use only */
#define IFD_PARITY_ERROR 699

typedef struct {
	int		lun;
	int		state;

	unsigned char	ns;	/* reader side */
	unsigned char	nr;	/* card side */
	unsigned int	ifsc;
	unsigned int	ifsd;

	unsigned char	wtx;
	unsigned int	retries;
	unsigned int	rc_bytes;

    unsigned int	(*checksum)(const uint8_t *, size_t, unsigned char *);

	char			more;	/* more data bit */
	unsigned char	previous_block[4];	/* to store the last R-block */
} t1_state_t;

int t1_transceive(t1_state_t *t1, unsigned int dad,
		const void *snd_buf, size_t snd_len,
		void *rcv_buf, size_t rcv_len);
int t1_init(t1_state_t *t1, int lun);
void t1_release(t1_state_t *t1);
int t1_set_param(t1_state_t *t1, int type, long value);
int t1_negotiate_ifsd(t1_state_t *t1, unsigned int dad, int ifsd);
unsigned int t1_build(t1_state_t *, unsigned char *,
	unsigned char, unsigned char, ct_buf_t *, size_t *);

    
#ifdef __cplusplus
}
#endif
        
#endif

