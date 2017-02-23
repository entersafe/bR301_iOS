/*
 atr.c
 ISO 7816 ICC's answer to reset abstract data type implementation
 
 This file is part of the Unix driver for Towitoko smartcard readers
 Copyright (C) 2000 Carlos Prados <cprados@yahoo.com>
 
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

static unsigned
atr_num_ib_table[16] =
{
  0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4
};

/*
 * Exported variables definition
 */

static unsigned
atr_f_table[16] =
{
  372, 372, 558, 744, 1116, 1488, 1860, 0, 0, 512, 768, 1024, 1536, 2048, 0, 0
};

static unsigned
atr_d_table[16] =
{
  0, 1, 2, 4, 8, 16, 32, 64, 12, 20, 0, 0, 0, 0, 0, 0
};

static unsigned
atr_i_table[4] =
{
  25, 50, 100, 0
};

/*
 * Exported functions definition
 */

// modified by Zhou Yu 2009.4.28, formalize none 7816 compliant Fi/Di
int
ATR_AdjustFiDi(BYTE atr_buffer[ATR_MAX_SIZE], unsigned length)
{
	ATR_t atr;
	BYTE  TDi;
	BYTE  Fi, Di;
	unsigned   i;
	BYTE  table[16][16] =
	{0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11,
	0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11,
	0x12, 0x12, 0x22, 0x32, 0x11, 0x11, 0x11, 0x11, 0x11, 0x92, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11,
	0x13, 0x13, 0x23, 0x33, 0x42, 0x53, 0x11, 0x11, 0x11, 0x93, 0xa3, 0xb3, 0x11, 0x11, 0x11, 0x11,
	0x14, 0x14, 0x24, 0x34, 0x44, 0x54, 0x64, 0x11, 0x11, 0x94, 0xa4, 0xb4, 0xc4, 0xd4, 0x11, 0x11,
	0x15, 0x15, 0x25, 0x35, 0x45, 0x55, 0x65, 0x11, 0x11, 0x95, 0xa5, 0xb5, 0xc5, 0xd5, 0x11, 0x11,
	0x16, 0x16, 0x26, 0x36, 0x46, 0x56, 0x66, 0x11, 0x11, 0x96, 0xa6, 0xb6, 0xc6, 0xd6, 0x11, 0x11,
	0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11,
	0x18, 0x18, 0x28, 0x38, 0x48, 0x58, 0x68, 0x11, 0x11, 0x98, 0xa8, 0xb8, 0xc8, 0xc8, 0x11, 0x11,
	0x19, 0x19, 0x29, 0x39, 0x49, 0x59, 0x69, 0x11, 0x11, 0x99, 0xa9, 0xb9, 0xc9, 0xc9, 0x11, 0x11,
	0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11,
	0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11,
	0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11,
	0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11,
	0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11,
	0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11
	};

	ATR_InitFromArray(&atr, atr_buffer, length);

	if(ATR_NOT_FOUND 
		== ATR_GetIntegerValue(&atr, ATR_INTEGER_VALUE_FI, &Fi))
	{
		Fi = 1;
	}

	if(ATR_NOT_FOUND 
		== ATR_GetIntegerValue(&atr, ATR_INTEGER_VALUE_DI, &Di))
	{
		Di = 1;
	}

	/* Check size of buffer */
	if (length < 2)
		return (ATR_MALFORMED);

	TDi = atr_buffer[1];

	/* Check TAi is present */
	if ((TDi | 0xEF) == 0xFF)
	{
		atr_buffer[2] = table[Di][Fi];
	}
	else
	{
		return (ATR_NOT_FOUND);
	}

	/* recalculate TCK */
	if ((atr.TCK).present)
	{
		atr_buffer[length - 1] = 0;

		for(i = 1; i < length - 1; i++)
		{
			atr_buffer[length - 1] ^=
				atr_buffer[i];

		}

	}

	return (ATR_OK);
}

int
ATR_InitFromArray (ATR_t * atr, const BYTE atr_buffer[ATR_MAX_SIZE], unsigned length)
{
  BYTE TDi;
  unsigned pointer = 0, pn = 0;

  /* Check size of buffer */
  if (length < 2)
    return (ATR_MALFORMED);

  /* Store T0 and TS */
  atr->TS = atr_buffer[0];

  atr->T0 = TDi = atr_buffer[1];
  pointer = 1;

  /* Store number of historical bytes */
  atr->hbn = TDi & 0x0F;

  /* TCK is not present by default */
  (atr->TCK).present = false;

  /* Extract interface bytes */
  while (pointer < length)
  {
      /* Check buffer is long enought */
      if (pointer + atr_num_ib_table[(0xF0 & TDi) >> 4] >= length)
      {
          return (ATR_MALFORMED);
      }
      /* Check TAi is present */
      if ((TDi | 0xEF) == 0xFF)
      {
          pointer++;
          atr->ib[pn][ATR_INTERFACE_BYTE_TA].value = atr_buffer[pointer];
          atr->ib[pn][ATR_INTERFACE_BYTE_TA].present = true;
      }
      else
          atr->ib[pn][ATR_INTERFACE_BYTE_TA].present = false;
      /* Check TBi is present */
      if ((TDi | 0xDF) == 0xFF)
      {
          pointer++;
          atr->ib[pn][ATR_INTERFACE_BYTE_TB].value = atr_buffer[pointer];
          atr->ib[pn][ATR_INTERFACE_BYTE_TB].present = true;
      }
      else
          atr->ib[pn][ATR_INTERFACE_BYTE_TB].present = false;

      /* Check TCi is present */
      if ((TDi | 0xBF) == 0xFF)
      {
	  pointer++;
	  atr->ib[pn][ATR_INTERFACE_BYTE_TC].value = atr_buffer[pointer];
	  atr->ib[pn][ATR_INTERFACE_BYTE_TC].present = true;
      }
      else
          atr->ib[pn][ATR_INTERFACE_BYTE_TC].present = false;

      /* Read TDi if present */
      if ((TDi | 0x7F) == 0xFF)
      {
          pointer++;
          TDi = atr->ib[pn][ATR_INTERFACE_BYTE_TD].value = atr_buffer[pointer];
          atr->ib[pn][ATR_INTERFACE_BYTE_TD].present = true;
          (atr->TCK).present = ((TDi & 0x0F) != ATR_PROTOCOL_TYPE_T0);
          if (pn >= ATR_MAX_PROTOCOLS)
              return (ATR_MALFORMED);
          pn++;
      }
      else
      {
          atr->ib[pn][ATR_INTERFACE_BYTE_TD].present = false;
          break;
      }
  }

  /* Store number of protocols */
  atr->pn = pn + 1;

  /* Store historical bytes */
  if (pointer + atr->hbn >= length)
    return (ATR_MALFORMED);

  memcpy (atr->hb, atr_buffer + pointer + 1, atr->hbn);
  pointer += (atr->hbn);

  /* Store TCK  */
  if ((atr->TCK).present)
  {

      if (pointer + 1 >= length)
          return (ATR_MALFORMED);

      pointer++;

      (atr->TCK).value = atr_buffer[pointer];
  }

  atr->length = pointer + 1;
  return (ATR_OK);
}

int
ATR_GetConvention (ATR_t * atr, int *convention)
{
  if (atr->TS == 0x3B)
    (*convention) = ATR_CONVENTION_DIRECT;
  else if (atr->TS == 0x3F)
    (*convention) = ATR_CONVENTION_INVERSE;
  else
    return (ATR_MALFORMED);
  return (ATR_OK);
}

int
ATR_GetIntegerValue (ATR_t * atr, int name, BYTE * value)
{
  int ret;

  if (name == ATR_INTEGER_VALUE_FI)
  {
      if (atr->ib[0][ATR_INTERFACE_BYTE_TA].present)
      {
          (*value) = (atr->ib[0][ATR_INTERFACE_BYTE_TA].value & 0xF0) >> 4;
          ret = ATR_OK;
      }
      else
          ret = ATR_NOT_FOUND;
  }
  else if (name == ATR_INTEGER_VALUE_DI)
  {
      if (atr->ib[0][ATR_INTERFACE_BYTE_TA].present)
      {
          (*value) = (atr->ib[0][ATR_INTERFACE_BYTE_TA].value & 0x0F);
          ret = ATR_OK;
      }
      else
          ret = ATR_NOT_FOUND;
  }
  else if (name == ATR_INTEGER_VALUE_II)
  {
      if (atr->ib[0][ATR_INTERFACE_BYTE_TB].present)
      {
          (*value) = (atr->ib[0][ATR_INTERFACE_BYTE_TB].value & 0x60) >> 5;
          ret = ATR_OK;
      }
      else
          ret = ATR_NOT_FOUND;
  }
  else if (name == ATR_INTEGER_VALUE_PI1)
  {
      if (atr->ib[0][ATR_INTERFACE_BYTE_TB].present)
      {
          (*value) = (atr->ib[0][ATR_INTERFACE_BYTE_TB].value & 0x1F);
          ret = ATR_OK;
      }
      else
          ret = ATR_NOT_FOUND;
  }
  else if (name == ATR_INTEGER_VALUE_PI2)
  {
      if (atr->ib[1][ATR_INTERFACE_BYTE_TB].present)
      {
          (*value) = atr->ib[1][ATR_INTERFACE_BYTE_TB].value;
          ret = ATR_OK;
      }
      else
          ret = ATR_NOT_FOUND;
  }
  else if (name == ATR_INTEGER_VALUE_N)
  {
      if (atr->ib[0][ATR_INTERFACE_BYTE_TC].present)
      {
          (*value) = atr->ib[0][ATR_INTERFACE_BYTE_TC].value;
          ret = ATR_OK;
      }
      else
          ret = ATR_NOT_FOUND;
  }
  else
      ret = ATR_NOT_FOUND;

  return ret;
}

int
ATR_GetParameter (ATR_t * atr, int name, double *parameter)
{
  BYTE FI, DI, II, PI1, PI2, N;

  if (name == ATR_PARAMETER_F)
  {
      if (ATR_GetIntegerValue (atr, ATR_INTEGER_VALUE_FI, &FI) == ATR_OK)
          (*parameter) = (double) (atr_f_table[FI]);
      else
          (*parameter) = (double) ATR_DEFAULT_F;
      
      return (ATR_OK);
  }
  else if (name == ATR_PARAMETER_D)
  {
      if (ATR_GetIntegerValue (atr, ATR_INTEGER_VALUE_DI, &DI) == ATR_OK)
          (*parameter) = (double) (atr_d_table[DI]);
      else
          (*parameter) = (double) ATR_DEFAULT_D;
      
      return (ATR_OK);
  }

  else if (name == ATR_PARAMETER_I)
  {
      if (ATR_GetIntegerValue (atr, ATR_INTEGER_VALUE_II, &II) == ATR_OK)
          (*parameter) = (double) (atr_i_table[II]);
      else
          (*parameter) = ATR_DEFAULT_I;
      
      return (ATR_OK);
  }
  else if (name == ATR_PARAMETER_P)
  {
      if (ATR_GetIntegerValue (atr, ATR_INTEGER_VALUE_PI2, &PI2) == ATR_OK)
          (*parameter) = (double) PI2;
      else if (ATR_GetIntegerValue (atr, ATR_INTEGER_VALUE_PI1, &PI1) == ATR_OK)
          (*parameter) = (double) PI1;
      else
          (*parameter) = (double) ATR_DEFAULT_P;
      
      return (ATR_OK);
  }

  else if (name == ATR_PARAMETER_N)
  {
      if (ATR_GetIntegerValue (atr, ATR_INTEGER_VALUE_N, &N) == ATR_OK)
          (*parameter) = (double) N;
      else
          (*parameter) = (double) ATR_DEFAULT_N;
      
      return (ATR_OK);
  }

  return (ATR_NOT_FOUND);
}

/*
 * This function was greatly inspired by ATRDecodeAtr() and
 * PHGetDefaultProtocol() from pcsc-lite
 *
 * It was rewritten by Ludovic Rousseau, 2004
 */
#define PROTOCOL_UNSET -1
int ATR_GetDefaultProtocol(ATR_t * atr, int *protocol)
{
	int i;
    
	/* default value */
	*protocol = PROTOCOL_UNSET;

	for (i=0; i<ATR_MAX_PROTOCOLS; i++)
		if (atr->ib[i][ATR_INTERFACE_BYTE_TD].present && (PROTOCOL_UNSET == *protocol))
		{
			/* set to the first protocol byte found */
			*protocol = atr->ib[i][ATR_INTERFACE_BYTE_TD].value & 0x0F;
		}

	/* specific mode if TA2 present */
	if (atr->ib[1][ATR_INTERFACE_BYTE_TA].present)
	{
		*protocol = atr->ib[1][ATR_INTERFACE_BYTE_TA].value & 0x0F;
	}

	if (PROTOCOL_UNSET == *protocol)
	{
		// no default protocol found in ATR. Using T=0
		*protocol = ATR_PROTOCOL_TYPE_T0;
	}

	return ATR_OK;
}

