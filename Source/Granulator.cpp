/*
  ==============================================================================

    Granulator.cpp
    Created: 7 Nov 2016 3:45:34pm
    Author:  Tristan Pantano

  ==============================================================================
*/

#include "Granulator.h"

const int Grain::MINGRAINSIZEINMS = 20;
const int Grain::MAXGRAINSIZEINMS = 400;

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

void Grain::synthesize(AudioSampleBuffer& outputBuffer, AudioSampleBuffer* sourceBuffer, int startSample, int numSamples)
{
    const float* inL = sourceBuffer->getReadPointer(0);
    const float* inR = sourceBuffer->getReadPointer(1);
    float* outL = outputBuffer.getWritePointer(0);
    float* outR = outputBuffer.getWritePointer(1);
    
    int readIndex;
    double envGain;
    for(int i = startSample; i < startSample+numSamples; i++)
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
Granulator::Granulator() : sampleRate(44100.0), basePitchInHz(440.0), semiTranspose(0.0), centTranspose(0.0), grainSizeInSec(0.040), useFixedRatio(false), fixedRatio(1.0), chaosPercent(0.0), numRepetitions(0), currentRepetition(0), gen(rd()), chaosDist(0, 201), samplesTilNextGrain(0), currentReadIndex(0), loopStartIndexPercent(0.0), source(nullptr) {}
void Granulator::prepareToPlay(double sr, int samplesPerBlock)
{
    sampleRate = sr;
    grainEnvelope.initialize(0.010, sr); //10 ms fade each end
    for(int i = 0; i < MAXGRAINPOLYPHONY; i++)
    {
        grainArray[i].setEnvelopePtr(&grainEnvelope);
    }
}

//Listener
void Granulator::parameterChanged(const String &parameterID, float newValue)
{
    if(parameterID == "gloopstart")
    {
        loopStartIndexPercent = newValue;
        restartReadIndex();
    }
    else if(parameterID == "gloopsize")
    {
        loopLengthPercent = newValue;
    }
    else if(parameterID == "gsize")
    {
        grainSizeInSec = newValue/1000.0;
    }
    else if(parameterID == "gsemitones")
    {
        semiTranspose = newValue;
    }
    else if(parameterID == "gcents")
    {
        centTranspose = newValue;
    }
    else if(parameterID == "gfixtoggle")
    {
        useFixedRatio = (bool)newValue;
    }
    else if(parameterID == "gfixed")
    {
        fixedRatio = newValue/100.0;
    }
    else if(parameterID == "gchaos")
    {
        chaosPercent = newValue/100.0;
    }
    else if(parameterID == "grepeat")
    {
        numRepetitions = newValue;
        currentRepetition = newValue;
    }
}

//SequenceStrategy
void Granulator::setBasePitch(double pitchInHz)
{
    basePitchInHz = pitchInHz;
}
double Granulator::getTargetPitch()
{
    double randomCents = chaosDist(gen)*chaosPercent - 100.0;
    double detuneCoefficient = pow(2, (randomCents + centTranspose + semiTranspose*100.0)/1200.0);
    return basePitchInHz * detuneCoefficient;
}
int Granulator::nextDurationInSamples()
{
    return grainSizeInSec*sampleRate; //fixed 40ms duration
}
int Granulator::nextInteronsetInSamples()
{
    return int((1.0/getTargetPitch())*sampleRate);
}
void Granulator::advanceReadIndex(int samplesElapsed)
{
    if(currentRepetition >= 0)
    {
        currentRepetition--;
        return;
    }
    else
    {
        currentRepetition = numRepetitions;
    }
    
    if(source != nullptr)
    {
        currentReadIndex += int(samplesElapsed*getResamplingRatio());
        if(currentReadIndex > (loopStartIndexPercent+loopLengthPercent)*source->getNumSamples())
        {
            restartReadIndex();
        }
    }
}
void Granulator::restartReadIndex()
{
    if(source != nullptr)
    {
        currentReadIndex = loopStartIndexPercent * source->getNumSamples();
        if(currentReadIndex > source->getNumSamples())
        {
            currentReadIndex = source->getNumSamples() - 1;
        }
    }
}
double Granulator::getResamplingRatio()
{
    if(useFixedRatio){return fixedRatio;}
    else
    {
        return (sampleRate/sourceSampleRate)*(getTargetPitch()/sourcePitchInHz);
    }
}

//Scheduler
void Granulator::retrigger()
{
    for(int i = 0; i < MAXGRAINPOLYPHONY; i++)
    {
        grainArray[i].setActive(false);
    }
    restartReadIndex();
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
                grainArray[i].synthesize(outputBuffer, source, startSample, numSamples);
            }
        }
        
        //Output new grains if needed
        while(samplesTilNextGrain < numSamples)
        {
            int newGrainIndex = initFreeGrain();
            if(newGrainIndex == -1)
            {
                return;
            }
            else
            {
                grainArray[newGrainIndex].synthesize(outputBuffer, source, samplesTilNextGrain+startSample, numSamples-samplesTilNextGrain);
                
                int interonset = nextInteronsetInSamples();
                advanceReadIndex(interonset);
                samplesTilNextGrain += interonset;
            }
        }
        samplesTilNextGrain -= numSamples;
    }
}
int Granulator::initFreeGrain()
{
    for(int i = 0; i < MAXGRAINPOLYPHONY; i++)
    {
        if(!grainArray[i].isActive())
        {
            int grainLength = nextDurationInSamples();
            jassert(grainLength < source->getNumSamples());
            int overshoot = (grainLength+currentReadIndex) - source->getNumSamples();
            if(overshoot > 0)
            {
                grainArray[i].initialize(currentReadIndex-overshoot, grainLength);
                restartReadIndex();
            }
            else
            {
                grainArray[i].initialize(currentReadIndex, grainLength);
            }
            
            return i;
        }
    }
    
    return -1; //no free grains
}

//Source
void Granulator::setSource(AudioSampleBuffer* s, double sr, double pitch)
{
    source = s;
    sourceSampleRate = sr;
    sourcePitchInHz = pitch;
}