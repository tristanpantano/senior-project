/*
  ==============================================================================

    Granulator.cpp
    Created: 7 Nov 2016 3:45:34pm
    Author:  Tristan Pantano

  ==============================================================================
*/

#include "Granulator.h"

GrainEnvelope::GrainEnvelope() : initialized(false){}

void GrainEnvelope::initialize(double fadeInSec, double sampleRate)
{
    numFadeSamples = int(fadeInSec * sampleRate);
    
    fadeArray = new double[numFadeSamples];
    for(int i = 0; i < numFadeSamples; i++)
    {
        fadeArray[i] = (1.0 + cos(float_Pi + float_Pi*i/numFadeSamples))/2.0;
    }
    
    initialized = true;
}

double GrainEnvelope::synthesize(int indexInGrain, int grainSampleLength)
{
    if(!initialized){return 1.0;}
    
    if(indexInGrain < numFadeSamples)
    {
        return fadeArray[indexInGrain];
    }
    else if(grainSampleLength - indexInGrain <= numFadeSamples)
    {
        return fadeArray[grainSampleLength - indexInGrain - 1];
    }
    else
    {
        return 1.0;
    }
}

//==============================================================================
//Grain
//==============================================================================
Grain::Grain() : active(false), startIndexInSource(0), sampleLength(0), posInGrain(0), grainGain(0.125) {}

void Grain::initialize(int sourceReadIndex, int lengthInSamples)
{
    active = true;
    startIndexInSource = sourceReadIndex;
    sampleLength = lengthInSamples;
    posInGrain = 0;
}

void Grain::synthesize(AudioSampleBuffer& outputBuffer, AudioSampleBuffer* sourceBuffer, int startSample)
{
    const float* inL = sourceBuffer->getReadPointer(0);
    const float* inR = sourceBuffer->getReadPointer(1);
    float* outL = outputBuffer.getWritePointer(0);
    float* outR = outputBuffer.getWritePointer(1);
    
    int readIndex;
    double envGain;
    for(int i = startSample; i < outputBuffer.getNumSamples(); i++)
    {
        if(posInGrain >= sampleLength) break;
        
        readIndex = startIndexInSource + posInGrain;
        envGain = envelope->synthesize(posInGrain,sampleLength);
        outL[i] += inL[readIndex] * grainGain * envGain;
        outR[i] += inR[readIndex] * grainGain * envGain;
        
        posInGrain++;
    }
    
    if(posInGrain >= sampleLength)
    {
        active = false;
    }
}

//==============================================================================
//Granulator
//==============================================================================
Granulator::Granulator() : sampleRate(44100.0), timeSkew(0.125), targetPitchInHz(440.0), samplesTilNextGrain(0), currentReadIndex(0), source(nullptr) {}
void Granulator::prepareToPlay(double sr, int samplesPerBlock)
{
    sampleRate = sr;
    grainEnvelope.initialize(0.005, sr);
    for(int i = 0; i < MAXGRAINPOLYPHONY; i++)
    {
        grainArray[i].setEnvelopePtr(&grainEnvelope);
    }
}

//SequenceStrategy
int Granulator::nextDurationInSamples()
{
    return int((40.0/1000.0)*sampleRate); //40 ms @ SR
}
int Granulator::nextInteronsetInSamples()
{
    return int((1.0/targetPitchInHz)*sampleRate);
}
void Granulator::advanceReadIndex(int samplesElapsed)
{
    currentReadIndex += int(samplesElapsed*timeSkew);
}
void Granulator::restartReadIndex()
{
    currentReadIndex = 0;
    //should handle looping and an inlaid startpos...
}

//Scheduler
void Granulator::retrigger(int startPos)
{
    currentReadIndex = startPos;
}
void Granulator::renderNextBlock(AudioSampleBuffer& outputBuffer, int startSample, int numSamples)
{
    if(source != nullptr)
    {
        //Output active grains
        for(int i = 0; i < MAXGRAINPOLYPHONY; i++)
        {
            if(grainArray[i].isActive())
            {
                grainArray[i].synthesize(outputBuffer, source, 0);
            }
        }
        
        //Output new grains if needed
        while(samplesTilNextGrain < outputBuffer.getNumSamples())
        {
            if(initFreeGrain(outputBuffer) == -1) return;
        }
        samplesTilNextGrain -= outputBuffer.getNumSamples();
    }
}
int Granulator::initFreeGrain(AudioSampleBuffer& outputBuffer)
{
    int i;
    for(i = 0; i < MAXGRAINPOLYPHONY; i++)
    {
        if(!grainArray[i].isActive())
        {
            int length = nextDurationInSamples();
            jassert(length < source->getNumSamples());
            int overshoot = (length+currentReadIndex) - source->getNumSamples();
            if(overshoot > 0)
            {
                grainArray[i].initialize(currentReadIndex-overshoot, length);
                restartReadIndex();
            }
            else
            {
                grainArray[i].initialize(currentReadIndex, length);
            }
            grainArray[i].synthesize(outputBuffer, source, samplesTilNextGrain);
            
            int interonset = nextInteronsetInSamples();
            advanceReadIndex(interonset);
            samplesTilNextGrain += interonset;
            
            return i;
        }
    }
    
    return -1; //no free grains
}

//Grain

//Envelope

//Source