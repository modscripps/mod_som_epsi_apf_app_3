/*
 * crc16bit.h
 *
 *  Created on: Dec 8, 2021
 *      Author: aleboyer
 */

#ifndef CRC16BIT_H
#define CRC16BIT_H (0x0100U)

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * $Id: crc16bit.c,v 1.1 2020/08/02 14:06:18 swift Exp $
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Copyright University of Washington.   Written by Dana Swift.
 *
 * This software was developed at the University of Washington using funds
 * generously provided by the US Office of Naval Research, the National
 * Science Foundation, and NOAA.
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2.1 of the License, or (at
 * your option) any later version.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser
 * General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, write to the Free Software Foundation,
 * Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */
/** RCS log of revisions to the C source code.
 *
 * \begin{verbatim}
 * $Log: crc16bit.c,v $
 * Revision 1.1  2020/08/02 14:06:18  swift
 * Apf11Sbe41cpStsPal firmware.
 *
 * \end{verbatim}
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
#define crc16bitChangeLog "$RCSfile: crc16bit.c,v $  $Revision: 1.1 $  $Date: 2020/08/02 14:06:18 $"

/* declare functions with external linkage */
unsigned int Crc16Bit(const unsigned char *msg,unsigned int len);
unsigned int Crc16BitInit(unsigned int *crc);
unsigned int Crc16BitIterate(unsigned int *crc, unsigned char byte);
unsigned int Crc16BitTerminate(unsigned int *crc);

#endif /* CRC16BIT_H */

