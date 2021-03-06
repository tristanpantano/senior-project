/*
  ==============================================================================

    Granulator.h
    Created: 7 Nov 2016 3:45:34pm
    Author:  Tristan Pantano

  ==============================================================================
*/

#ifndef GRANULATOR_H_INCLUDED
#define GRANULATOR_H_INCLUDED

#include "../JuceLibraryCode/JuceHeader.h"
#include <algorithm>
#include <random>

const int MAXGRAINPOLYPHONY = 128;

class GrainEnvelope
{
public:
    GrainEnvelope();
    
    void initialize(double fadeInSec, double sampleRate);
    double synthesize(int indexInGrain, int grainSampleLength);
    
private:
    bool initialized;
    int numFadeSamples;
    ScopedPointer<double> fadeArray;
};

//==============================================================================
//Grain
//==============================================================================
class Grain
{
public:
    Grain();
    
    void initialize(int sourceReadIndex, int lengthInSamples);
    bool isActive() { return active; }
    
    void synthesize(AudioSampleBuffer& outputBuffer, AudioSampleBuffer* sourceBuffer, int startSample, int numSamples);
    
    void setEnvelopePtr(GrainEnvelope* env){ envelope = env; }
    void setActive(bool isActive){ active = isActive; }
    
    static const int MINGRAINSIZEINMS;
    static const int MAXGRAINSIZEINMS;
    
private:
    bool active;
    int startIndexInSource, sampleLength, posInGrain;
    double grainGain;
    GrainEnvelope* envelope;
};

//==============================================================================
//Granulator
//==============================================================================
class Granulator
{
public:
    Granulator();
    void prepareToPlay(double sr, int samplesPerBlock);
    
    //Listener
    void parameterChanged(const String &parameterID, float newValue);
    
    //SequenceStrategy
    void setBasePitch(double pitchInHz);
    double getTargetPitch();
    int nextDurationInSamples();
    int nextInteronsetInSamples();
    void advanceReadIndex(int samplesElapsed);
    void restartReadIndex();
    double getResamplingRatio();
    
    //Scheduler
    void retrigger();
    void renderNextBlock(AudioSampleBuffer& outputBuffer, int startSample, int numSamples);
    int initFreeGrain(); //returns -1 if no free grains
    
    //Source
    void setSource(AudioSampleBuffer* s, double sr, double pitch);
    
private:
    double sampleRate; //for output buffer
    
    //SequenceStrategy
    double basePitchInHz;
    double semiTranspose;
    double centTranspose;
    double grainSizeInSec;
    bool useFixedRatio;
    double fixedRatio;
    double chaosPercent;
    int numRepetitions, currentRepetition;
    
    //Chaos
    std::random_device rd;
    std::mt19937 gen;
    std::uniform_int_distribution<int> chaosDist;
    
    //Scheduler
    int samplesTilNextGrain;
    int currentReadIndex;
    float loopStartIndexPercent, loopLengthPercent;
    
    //Grain
    Grain grainArray[MAXGRAINPOLYPHONY];
    
    //Envelope
    GrainEnvelope grainEnvelope;
    
    //Source
    AudioSampleBuffer* source;
    double sourceSampleRate;
    double sourcePitchInHz;
};



#endif  // GRANULATOR_H_INCLUDED
