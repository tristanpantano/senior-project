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

const int MAXGRAINPOLYPHONY = 16;

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
    
    void synthesize(AudioSampleBuffer& outputBuffer, AudioSampleBuffer* sourceBuffer, int startSample);
    
    void setEnvelopePtr(GrainEnvelope* env){ envelope = env; }
    
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
    
    //SequenceStrategy
    void setTargetPitch(double pitchInHz){ targetPitchInHz = pitchInHz; }
    int nextDurationInSamples();
    int nextInteronsetInSamples();
    void advanceReadIndex(int samplesElapsed);
    void restartReadIndex();
    
    //Scheduler
    void retrigger(int startPos);
    void renderNextBlock(AudioSampleBuffer& outputBuffer, int startSample, int numSamples);
    int initFreeGrain(AudioSampleBuffer& outputBuffer); //returns -1 if no free grains
    
    //Envelope
    
    //Source
    void setSource(AudioSampleBuffer* s){ source = s; }
    
private:
    double sampleRate;
    
    //SequenceStrategy
    double timeSkew;
    double targetPitchInHz;
    
    //Scheduler
    int samplesTilNextGrain;
    int currentReadIndex;
    
    //Grain
    Grain grainArray[MAXGRAINPOLYPHONY];
    
    //Envelope
    GrainEnvelope grainEnvelope;
    
    //Source
    AudioSampleBuffer* source;
};



#endif  // GRANULATOR_H_INCLUDED
