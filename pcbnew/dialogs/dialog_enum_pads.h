/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2014 CERN
 * Copyright (C) 2021 KiCad Developers, see AUTHORS.txt for contributors.
 *
 * @author Maciej Suminski <maciej.suminski@cern.ch>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, you may find one here:
 * http://www.gnu.org/licenses/old-licenses/gpl-2.0.html
 * or you may search the http://www.gnu.org website for the version 2 license,
 * or you may write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 */

#ifndef __dialog_enum_pads__
#define __dialog_enum_pads__

/**
 * @file dialog_enum_pads.h
 */

#include "dialog_enum_pads_base.h"

class DIALOG_ENUM_PADS : public DIALOG_ENUM_PADS_BASE
{
public:
    DIALOG_ENUM_PADS( wxWindow* parent );

    ///< Return the starting number that is going to be used for the first enumerated pad.
    int GetStartNumber() const;

    ///< Return common prefix for all enumerated pads.
    wxString GetPrefix() const;
};

#endif // __dialog_enum_pads__
