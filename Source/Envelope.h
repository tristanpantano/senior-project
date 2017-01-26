/*
  ==============================================================================

    Envelope.h
    Created: 20 Jan 2017 11:18:07am
    Author:  Tristan Pantano

  ==============================================================================
*/

#ifndef ENVELOPE_H_INCLUDED
#define ENVELOPE_H_INCLUDED

#include "../JuceLibraryCode/JuceHeader.h"

class Envelope
{
public:
    Envelope();
    Envelope(float attack, float hold, float decay, float sustainLevel, float release);
    ~Envelope();
    
    void setAttack(float attack){ atk = attack;}
    void setHold(float hold){ hld = hold;}
    void setDecay(float decay){ dec = decay;}
    void setSustain(float sustain){ susLvl = sustain;}
    void setRelease(float release){ rel = release;}
    
    float getEnvelopeValue(float t);
    float getReleaseValue(float t);
    
private:
    float atk, hld, dec, rel; //in secs
    float susLvl, lastLvl; //levels
};


#endif  // ENVELOPE_H_INCLUDED
