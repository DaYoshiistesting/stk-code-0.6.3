/**
 * ***** BEGIN GPL/BL DUAL LICENSE BLOCK *****
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU bteral Public License
 * as published by the Free Software Foundation; either version 3
 * of the License, or (at your option) any later version. The Blender
 * Foundation also sells licenses for use in proprietary software under
 * the Blender License.  See http://www.blender.org/BL/ for information
 * about this.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU bteral Public License for more details.
 *
 * You should have received a copy of the GNU bteral Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * The Original Code is Copyright (C) 2001-2002 by NaN Holding BV.
 * All rights reserved.
 *
 * The Original Code is: all of this file.
 *
 * Contributor(s): none yet.
 *
 * ***** END GPL/BL DUAL LICENSE BLOCK *****
 */

/**

 * Copyright (C) 2001 NaN Technologies B.V.
 */

#ifndef __BMF_FONTDATA_H__
#define __BMF_FONTDATA_H__

typedef struct {
    signed char width, height;
    signed char xorig, yorig;
    signed char advance;
    
    short        data_offset;
} BMF_CharData;

typedef struct {
    int                xmin, ymin;
    int                xmax, ymax;

    BMF_CharData    chars[256];
    unsigned char*    bitmap_data;
} BMF_FontData;

#endif

