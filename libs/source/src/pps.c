/*
 pps.c
 Protocol Parameters Selection
 
 This file is part of the Unix driver for Towitoko smartcard readers
 Copyright (C) 2000 2001 Carlos Prados <cprados@yahoo.com>
 
 This library is free software; you can redistribute it and/or
 modify it under the terms of the GNU Lesser General Public
 License as published by the Free Software Foundation; either
 version 2 of the License, or (at your option) any later version.
 
 This library is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 Lesser General Public License for more details.
 
	You should have received a copy of the GNU Lesser General Public License
	along with this library; if not, write to the Free Software Foundation,
	Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */



#include "ft_ccid.h"


/*
 * Not exported funtions declaration
 */

static bool PPS_Match (BYTE * request, unsigned len_request, BYTE * reply, unsigned len_reply);

static unsigned PPS_GetLength (BYTE * block);

static BYTE PPS_GetPCK (BYTE * block, unsigned length);

int
PPS_Exchange (int lun, BYTE * params, unsigned *length, unsigned char *pps1)
{
	BYTE confirm[PPS_MAX_LENGTH];
	unsigned len_request, len_confirm;
	int ret;
	
	len_request = PPS_GetLength (params);
	params[len_request - 1] = PPS_GetPCK(params, len_request - 1);

	/* Send PPS request */
	if (CCID_Transmit (lun, len_request, params, isCharLevel(lun) ? 4 : 0, 0)
		!= IFD_SUCCESS)
		return PPS_ICC_ERROR;
	
	/* Get PPS confirm */
	len_confirm = sizeof(confirm);
	if (CCID_Receive (lun, &len_confirm, confirm, NULL) != IFD_SUCCESS)
		return PPS_ICC_ERROR;
	

	
	if (!PPS_Match (params, len_request, confirm, len_confirm))
		ret = PPS_HANDSAKE_ERROR;
	else
		ret = PPS_OK;
	
	*pps1 = 0x11;	/* default TA1 */
	
	/* if PPS1 is echoed */
	if (PPS_HAS_PPS1 (params) && PPS_HAS_PPS1 (confirm))
		*pps1 = confirm[2];
	
	/* Copy PPS handsake */
	memcpy (params, confirm, len_confirm);
	(*length) = len_confirm;
	
	return ret;
}

static bool
PPS_Match (BYTE * request, unsigned len_request, BYTE * confirm, unsigned len_confirm)
{
	/* See if the reply differs from request */
	if ((len_request == len_confirm) &&	/* same length */
		memcmp (request, confirm, len_request))	/* different contents */
		return false;
	
	if (len_request < len_confirm)	/* confirm longer than request */
		return false;
	
	/* See if the card specifies other than default FI and D */
	if ((PPS_HAS_PPS1 (confirm)) && (confirm[2] != request[2]))
		return false;
	
	return true;
}

static unsigned
PPS_GetLength (BYTE * block)
{
	unsigned length = 3;
	
	if (PPS_HAS_PPS1 (block))
		length++;
	
	if (PPS_HAS_PPS2 (block))
		length++;
	
	if (PPS_HAS_PPS3 (block))
		length++;
	
	return length;
}

static BYTE
PPS_GetPCK (BYTE * block, unsigned length)
{
	BYTE pck;
	unsigned i;
	
	pck = block[0];
	for (i = 1; i < length; i++)
		pck ^= block[i];
	
	return pck;
}

///////////////////////////


void extra_egt(ATR_t *atr, _ccid_descriptor *ccid_desc, DWORD Protocol)
{
	/* This function use an EGT value for cards who comply with followings
	 * criterias:
	 * - TA1 > 11
	 * - current EGT = 0x00 or 0xFF
	 * - T=0 or (T=1 and CWI >= 2)
	 *
	 * Without this larger EGT some non ISO 7816-3 smart cards may not
	 * communicate with the reader.
	 *
	 * This modification is harmless, the reader will just be less restrictive
	 */
    
	unsigned int card_baudrate;
	unsigned int default_baudrate;
	double f, d;
	int i;
    
	/* if TA1 not present */
	if (! atr->ib[0][ATR_INTERFACE_BYTE_TA].present)
		return;
    
	(void)ATR_GetParameter(atr, ATR_PARAMETER_D, &d);
	(void)ATR_GetParameter(atr, ATR_PARAMETER_F, &f);
    
	/* may happen with non ISO cards */
	if ((0 == f) || (0 == d))
		return;
    
	/* Baudrate = f x D/F */
	card_baudrate = (unsigned int) (1000 * ccid_desc->dwDefaultClock * d / f);
    
	default_baudrate = (unsigned int) (1000 * ccid_desc->dwDefaultClock
                                       * ATR_DEFAULT_D / ATR_DEFAULT_F);
    
	/* TA1 > 11? */
	if (card_baudrate <= default_baudrate)
		return;
    
	/* Current EGT = 0 or FF? */
	if (atr->ib[0][ATR_INTERFACE_BYTE_TC].present &&
		((0x00 == atr->ib[0][ATR_INTERFACE_BYTE_TC].value) ||
         (0xFF == atr->ib[0][ATR_INTERFACE_BYTE_TC].value)))
	{
		if (SCARD_PROTOCOL_T0 == Protocol)
		{
			/* Init TC1 */
			atr->ib[0][ATR_INTERFACE_BYTE_TC].present = true;
			atr->ib[0][ATR_INTERFACE_BYTE_TC].value = 2;
		
		}
        
		if (SCARD_PROTOCOL_T1 == Protocol)
		{
			/* TBi (i>2) present? BWI/CWI */
			for (i=2; i<ATR_MAX_PROTOCOLS; i++)
			{
				/* CWI >= 2 ? */
				if (atr->ib[i][ATR_INTERFACE_BYTE_TB].present &&
					((atr->ib[i][ATR_INTERFACE_BYTE_TB].value & 0x0F) >= 2))
				{
					/* Init TC1 */
					atr->ib[0][ATR_INTERFACE_BYTE_TC].present = true;
					atr->ib[0][ATR_INTERFACE_BYTE_TC].value = 2;
					
					/* only the first TBi (i>2) must be used */
					break;
				}
			}
		}
	}
} /* extra_egt */



/////////////
RESPONSECODE  
Scrd_Negotiate(unsigned int reader_index)
{
	ATR_t            atr;
	BYTE             pps[PPS_MAX_LENGTH];
	int              protocol;
	int              convention;
    CcidDesc         *ccid_slot;
    RESPONSECODE ret;
    _ccid_descriptor *ccid_descriptor = get_ccid_descriptor(reader_index);
    

	memset(&atr, 0, sizeof(atr));
	memset(pps, 0, sizeof(pps));
	
    ccid_slot = get_ccid_slot(reader_index);
#if 0
    if( FT_READER_UA != gDevType)
    {
        if (ccid_slot->nATRLength == 18 )
        {
            if (memcmp(ccid_slot->pcATRBuffer+3, "\x53\x6c\x65\x34\x34\x33\x32\x2d\x34\x32\x3d", 11) == 0)
            {
                    ccid_descriptor->cardProtocol = T_RAW;
            }
        }
    }
#endif
    
	/* Parse ATR */
	ATR_InitFromArray(&atr, ccid_slot->pcATRBuffer, ccid_slot->nATRLength);

	if (ATR_MALFORMED == ATR_GetDefaultProtocol(&atr, &protocol))
		return IFD_PROTOCOL_NOT_SUPPORTED;
    
    
	/* TA2 present -> specific mode */
	if (atr.ib[1][ATR_INTERFACE_BYTE_TA].present)
	{
		if (pps[1] != (atr.ib[1][ATR_INTERFACE_BYTE_TA].value & 0x0F))
		{
			/* wrong protocol */
			return IFD_PROTOCOL_NOT_SUPPORTED;
		}
	}
    ccid_descriptor->cardProtocol = protocol;
    
//    if( FT_READER_UA != gDevType){
        return IFD_SUCCESS;
//    }
    
	/* TCi (i>2) indicates CRC instead of LRC */
	if (T_1 == protocol)
	{
		t1_state_t *t1 = &(ccid_slot -> t1);
		int i;
        
		/* TCi (i>2) present? */
		for (i=2; i<ATR_MAX_PROTOCOLS; i++)
			if (atr.ib[i][ATR_INTERFACE_BYTE_TC].present)
			{
				if (0 == atr.ib[i][ATR_INTERFACE_BYTE_TC].value)
				{
						(void)t1_set_param(t1, IFD_PROTOCOL_T1_CHECKSUM_LRC, 0);
				}
				else
					if (1 == atr.ib[i][ATR_INTERFACE_BYTE_TC].value)
					{
                    
						(void)t1_set_param(t1, IFD_PROTOCOL_T1_CHECKSUM_CRC, 0);
					}
					else
						;//DEBUG_COMM2("Wrong value for TCi: %d",
                         //           atr.ib[i][ATR_INTERFACE_BYTE_TC].value);
                
				/* only the first TCi (i>2) must be used */
				break;
			}
	}

	/* TA2 absent -> negotiate mode */
    //haozi iR301 is not support so set 0
	if (!atr.ib[1][ATR_INTERFACE_BYTE_TA].present &&  0)
	{
		BYTE FI, DI;

		// assigned default value
		FI = DI = 1;

		if(atr.ib[0][ATR_INTERFACE_BYTE_TA].present)
		{
			ATR_GetIntegerValue (&atr, ATR_INTEGER_VALUE_DI, &DI);
			ATR_GetIntegerValue (&atr, ATR_INTEGER_VALUE_FI, &FI);
		}

		if(DI > 1
			&& DI < 10)
		{
			// do PPS
			unsigned int   len;

			/* Generate PPS */
			pps[0] = 0xFF;
			pps[1] |= 0x10; /* PTS1 presence */
			pps[1] |= protocol;
			pps[2] = atr.ib[0][ATR_INTERFACE_BYTE_TA].value;

			if (PPS_Exchange(reader_index,  pps, &len, &pps[2]) != PPS_OK) {
                return IFD_ERROR_PTS_FAILURE;
            } 

		}
		else
		{
		;//	DbgPrint("######### No PPS #########\n");
		}
	}
	else
	{
		/* TA2 present -> specific mode */

		//DbgPrint("######### Specific mode #########\n");

		pps[1] |= 0x10; /* PTS1 presence */
		pps[2] = atr.ib[0][ATR_INTERFACE_BYTE_TA].value;
	}

	// set parameters for T=0 or T=1
	ATR_GetConvention(&atr, &convention);

	if(protocol == T_1)
	{
		int i;
		BYTE param[] = {
			0x11,	/* Fi/Di		*/
			0x10,	/* TCCKS		*/
			0x00,	/* GuardTime	*/
			0x4D,	/* BWI/CWI		*/
			0x00,	/* ClockStop	*/
			0x20,	/* IFSC			*/
			0x00	/* NADValue		*/
		};

		/* TA1 is not default */
		if (PPS_HAS_PPS1(pps))
			param[0] = pps[2];

		/* TCi (i>2) indicates CRC instead of LRC */
		for (i=2; i<ATR_MAX_PROTOCOLS; i++)
		{
			if (atr.ib[i][ATR_INTERFACE_BYTE_TC].present)
			{
				if (1 == atr.ib[i][ATR_INTERFACE_BYTE_TC].value)
				{
					// Use CRC
					param[1] |= 0x01;
				}
				else
					//Wrong value for TCi
					return IFD_ERROR_PTS_FAILURE;//haozi 

				/* only the first TCi (i>2) must be used */
				break;
			}
		}
		
		/* the CCID should ignore this bit */
		if (ATR_CONVENTION_INVERSE == convention)
			param[1] |= 0x02;

		/* get TC1 Extra guard time */
		if (atr.ib[0][ATR_INTERFACE_BYTE_TC].present)
			param[2] = atr.ib[0][ATR_INTERFACE_BYTE_TC].value;

		/* TBi (i>2) present? BWI/CWI */
		for (i=2; i<ATR_MAX_PROTOCOLS; i++)
		{
			if (atr.ib[i][ATR_INTERFACE_BYTE_TB].present)
			{
				param[3] = atr.ib[i][ATR_INTERFACE_BYTE_TB].value;

				/* only the first TBi (i>2) must be used */
				break;
			}
		}

		/* TAi (i>2) present? IFSC */
		for (i=2; i<ATR_MAX_PROTOCOLS; i++)
		{
			if (atr.ib[i][ATR_INTERFACE_BYTE_TA].present)
			{

				param[5] = atr.ib[i][ATR_INTERFACE_BYTE_TA].value;

				/* only the first TAi (i>2) must be used */
				break;
			}
		}
        ret = SetParameters(reader_index, protocol, sizeof(param), param);

		if(ret != IFD_SUCCESS)
		{
			//DbgPrint("######### Set T=1 parameters failed #########\n");
			return IFD_ERROR_PTS_FAILURE;//haozi
		}

		//DbgPrint("######### Set T=1 parameters Success #########\n");

		
	}
	// t=0
	else
	{
		BYTE param[] = {
			0x11,	/* Fi/Di			*/
			0x00,	/* TCCKS			*/
			0x00,	/* GuardTime		*/
			0x0A,	/* WaitingInteger	*/
			0x00	/* ClockStop		*/
		};

		/* TA1 is not default */
		if (PPS_HAS_PPS1(pps))
			param[0] = pps[2];

		/* the CCID should ignore this bit */
		if (ATR_CONVENTION_INVERSE == convention)
			param[1] |= 0x02;

		/* get TC1 Extra guard time */
		if (atr.ib[0][ATR_INTERFACE_BYTE_TC].present)
			param[2] = atr.ib[0][ATR_INTERFACE_BYTE_TC].value;

		/* TC2 WWT */
		if (atr.ib[1][ATR_INTERFACE_BYTE_TC].present)
			param[3] = atr.ib[1][ATR_INTERFACE_BYTE_TC].value;
		
		ret = SetParameters(reader_index, protocol, sizeof(param), param);
        
		if(ret != IFD_SUCCESS)
		{
			//DbgPrint("######### Set T=1 parameters failed #########\n");
			return IFD_ERROR_PTS_FAILURE;//haozi
		}
	}
    


	return IFD_SUCCESS;
}


