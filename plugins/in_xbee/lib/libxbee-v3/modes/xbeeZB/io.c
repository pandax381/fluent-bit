/*
	libxbee - a C library to aid the use of Digi's XBee wireless modules
	          running in API mode.

	Copyright (C) 2009 onwards  Attie Grande (attie@attie.co.uk)

	libxbee is free software: you can redistribute it and/or modify it
	under the terms of the GNU Lesser General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	libxbee is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU Lesser General Public License for more details.

	You should have received a copy of the GNU Lesser General Public License
	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../../internal.h"
#include "../../xbee_int.h"
#include "../../mode.h"
#include "../../pkt.h"
#include "../../log.h"
#include "../common.h"
#include "io.h"

xbee_err xbee_sZB_io_parseInputs(struct xbee *xbee, struct xbee_pkt *pkt, unsigned char *data, int len) {
	int sampleCount;
	int sample, channel;
	int ioMask;

	if (len < 4) return XBEE_ELENGTH;
	
	sampleCount = data[0];
	data++; len--;

	/* mask is ordered:
	    MSB - Supply Voltage
	          * n/a *
	          * n/a *
	          * n/a *
	          AD3
	          AD2
	          AD1
	       -  AD0
	       -  * n/a *
	          * n/a *
	          * n/a *
	          CD/DIO12
	          PWM/DIO11
	          RSSI/DIO10
	          * n/a *
	       -  * n/a *
	       -  CTS/DIO7
	          RTS/DIO6
	          Assoc/DIO5
	          DIO4
	          AD3/DIO3
	          AD2/DIO2
	          AD1/DIO1
	    LSB - AD0/DIO0
	*/
	ioMask = ((data[2] << 16) & 0xFF0000) | ((data[0] << 8) & 0xFF00) | (data[1] & 0xFF);
	data += 3; len -= 3;
	
	/* poke out the n/a fields, just incase */
	ioMask &= 0x8F1CFF;
	
	for (sample = 0; sample < sampleCount; sample++) {
		int mask;

		
		if (ioMask & 0x001CFF) {
			int digitalValue;
			
			if (len < 2) return XBEE_ELENGTH;
			
			digitalValue = ((data[0] << 8) & 0xFF00) | (data[1] & 0xFF);
			/* poke out the n/a fields */
			digitalValue &= 0x1CFF;

			mask = 0x000001;
			for (channel = 0; channel <= 12; channel++, mask <<= 1) {
				if (ioMask & mask) {
					if (xbee_pktDigitalAdd(pkt, channel, digitalValue & mask)) {
						xbee_log(1,"Failed to add digital sample information to packet (channel D%d)", channel);
					}
				}
			}
			data += 2; len -= 2;
		}

		mask = 0x010000;
		for (channel = 0; channel <= 4; channel++, mask <<= 1) {
			if (channel == 4) {
				mask = 0x800000;
				channel = 7;
			}
			if (ioMask & mask) {
				
				if (len < 2) return XBEE_ELENGTH;
				
				if (xbee_pktAnalogAdd(pkt, channel, ((data[0] << 8) & 0x3F00) | (data[1] & 0xFF))) {
					xbee_log(1,"Failed to add analog sample information to packet (channel A%d)", channel);
				}
				data += 2; len -= 2;
			}
		}
	}

	return XBEE_ENONE;
}

/* ######################################################################### */

xbee_err xbee_sZB_io_rx_func(struct xbee *xbee, void *arg, unsigned char identifier, struct xbee_tbuf *buf, struct xbee_frameInfo *frameInfo, struct xbee_conAddress *address, struct xbee_pkt **pkt) {
	struct xbee_pkt *iPkt;
	xbee_err ret;
	
	if (!xbee || !frameInfo || !buf || !address || !pkt) return XBEE_EMISSINGPARAM;
	
	if (buf->len < 16) return XBEE_ELENGTH;
	
	if ((ret = xbee_pktAlloc(&iPkt, NULL, buf->len - 12)) != XBEE_ENONE) return ret;

	address->addr64_enabled = 1;
	memcpy(address->addr64, &(buf->data[1]), 8);
	address->addr16_enabled = 1;
	memcpy(address->addr16, &(buf->data[9]), 2);	
	
	iPkt->options = buf->data[11];
	
	iPkt->dataLen = buf->len - 12;
	if (iPkt->dataLen > 0) {
		memcpy(iPkt->data, &(buf->data[12]), iPkt->dataLen);
	}
	iPkt->data[iPkt->dataLen] = '\0';
	
	*pkt = iPkt;
	
	return XBEE_ENONE;
}

xbee_err xbee_sZB_io_rx_funcPost(struct xbee *xbee, struct xbee_con *con, struct xbee_pkt *pkt) {
	xbee_sZB_io_parseInputs(xbee, pkt, pkt->data, pkt->dataLen);
	
	return XBEE_ENONE;
}

/* ######################################################################### */

struct xbee_modeDataHandlerRx xbee_sZB_io_rx  = {
	.identifier = 0x92,
	.func = xbee_sZB_io_rx_func,
	.funcPost = xbee_sZB_io_rx_funcPost,
};
struct xbee_modeConType xbee_sZB_io = {
	.name = "I/O",
	.allowFrameId = 0,
	.useTimeout = 0,
	.addressRules = ADDR_16OR64_NOEP,
	.save_addr16 = 1,
	.save_addr64 = 1,
	.rxHandler = &xbee_sZB_io_rx,
	.txHandler = NULL,
};
