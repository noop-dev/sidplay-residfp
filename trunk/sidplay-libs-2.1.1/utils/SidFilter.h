/***************************************************************************
                          SidFilter.cpp  -  filter type decoding support
                             -------------------
    begin                : Sun Mar 11 2001
    copyright            : (C) 2001 by Simon White
    email                : s_a_white@email.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "sidplay/sidtypes.h"

class SID_EXTERN SidFilter
{
protected:
    bool         m_status;
    const char  *m_errorString;
    sid_filter_t m_filter;

public:
    SidFilter ();
    ~SidFilter () {};

    void                read      (const char *filename);
    const char*         error     (void) { return m_errorString; }
    const sid_filter_t* provide   () const;

    operator bool () { return m_status; }
    const SidFilter&    operator= (const SidFilter    &filter);
    const sid_filter_t &operator= (const sid_filter_t &filter);
    const sid_filter_t *operator= (const sid_filter_t *filter);
};
