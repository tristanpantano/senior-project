/*
  ==============================================================================

    Envelope.cpp
    Created: 20 Jan 2017 11:18:07am
    Author:  Tristan Pantano

  ==============================================================================
*/

#include "Envelope.h"

Envelope::Envelope()
{
    atk = 0.0;
    hld = 0.0;
    dec = 0.0;
    susLvl = 1.0;
    rel = 0.0;
    lastLvl = 0.0;
}
Envelope::Envelope(float attack, float hold, float decay, float sustainLevel, float release)
{
    atk = attack;
    hld = hold;
    dec = decay;
    susLvl = sustainLevel;
    rel = release;
    lastLvl = 0.0;
}
Envelope::~Envelope(){}

float Envelope::getEnvelopeValue(float t)
{
    float envLevel;
    if(t <= atk)
    {
        envLevel = t / atk;
    }
    else if(t <= atk+hld)
    {
        envLevel = 1.0;
    }
    else if(t <= atk+hld+dec)
    {
        float tSegment = t - (atk+hld);
        envLevel = 1.0 - (1.0-susLvl)*tSegment/dec;
    }
    else
    {
        envLevel = susLvl;
    }
    lastLvl = envLevel;
    return envLevel;
}
float Envelope::getReleaseValue(float t)
{
    float envLevel;
    if(rel != 0.0)
    {
        envLevel = lastLvl - lastLvl*t/rel;
    }
    else
    {
        envLevel = 0.0;
    }
    
    if(envLevel < 0.0)
    {
        envLevel = 0.0;
    }
    return envLevel;
}