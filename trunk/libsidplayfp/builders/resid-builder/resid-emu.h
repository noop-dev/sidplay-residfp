/***************************************************************************
                          resid-emu.h  -  ReSid Emulation
                             -------------------
    begin                : Fri Apr 4 2001
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

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include "resid/sid.h"
#include "sidplayfp/sidemu.h"
#include "sidplayfp/EventScheduler.h"

//FIXME
#define RESID_NAMESPACE reSID

#ifdef RESID_NAMESPACE
#   define RESID_NS ::RESID_NAMESPACE
#else
#   define RESID_NS
#endif

enum {
    OUTPUTBUFFERSIZE = 32768
};

class ReSID: public sidemu
{
private:
    EventContext *m_context;
    event_phase_t m_phase;
    RESID_NS::SID &m_sid;
    event_clock_t m_accessClk;
    const  char  *m_error;
    bool          m_status;
    bool          m_locked;
    uint8_t       m_voiceMask;
    static char   m_credit[180];

public:
    ReSID  (sidbuilder *builder);
    ~ReSID (void);

    // Standard component functions
    const char   *credits (void) {return m_credit;}
    void          reset   () { sidemu::reset (); }
    void          reset   (uint8_t volume);
    uint8_t       read    (uint_least8_t addr);
    void          write   (uint_least8_t addr, uint8_t data);
    const char   *error   (void) const {return m_error;}

    // Standard SID functions
    void          clock   ();
    void          filter  (bool enable);
    void          voice   (uint_least8_t num, bool mute);

    bool          getStatus() const { return m_status; }

    // Specific to resid
    void sampling (float systemclock, float freq,
    const sampling_method_t method, const bool fast);

    void bias     (const double dac_bias);
    void model    (sid2_model_t model);
    // Must lock the SID before using the standard functions.
    bool lock     (EventContext *env);
};
