/*
  ==============================================================================

    Resampler.h
    Created: 28 Oct 2016 4:04:41pm
    Author:  Tristan Pantano

  ==============================================================================
*/

#ifndef RESAMPLER_H_INCLUDED
#define RESAMPLER_H_INCLUDED

#include "../JuceLibraryCode/JuceHeader.h"

//==============================================================================
//Resampler: a synthesiser that can play back a single audio file across its voices
//==============================================================================
class Resampler : public Synthesiser
{
public:
    Resampler();
    
    static const int MAXSAMPLELENGTHINSECONDS;
    
    void setNewSoundFile(AudioFormatReader* source);
};

//==============================================================================
//ResamplerVoice: playback logic for an audio file loaded into a ResamplerSound
//==============================================================================
class ResamplerVoice : public SynthesiserVoice
{
public:
    //==============================================================================
    ResamplerVoice();
    
    //==============================================================================
    bool canPlaySound (SynthesiserSound*) override;
    
    void startNote (int midiNoteNumber, float velocity, SynthesiserSound* s, int pitchWheel) override;
    void stopNote (float velocity, bool allowTailOff) override;
    
    void pitchWheelMoved (int newValue) override;
    void controllerMoved (int controllerNumber, int newValue) override;
    
    void renderNextBlock (AudioSampleBuffer& outputBuffer, int startSample, int numSamples) override;
    
    
private:
    //==============================================================================
    bool retriggerEnabled;
    
    int startPos;
    
    double pitchRatio;
    double sourceSamplePosition;
};

//==============================================================================
//ResamplerSound:
//==============================================================================
class ResamplerSound : public SynthesiserSound
{
public:
    ResamplerSound (const String& name,
                  AudioFormatReader& source,
                  const BigInteger& midiNotes,
                  int midiNoteForNormalPitch,
                  double attackTimeSecs,
                  double releaseTimeSecs,
                  double maxSampleLengthSeconds);

    /** Destructor. */
    ~ResamplerSound();

    //==============================================================================
    /** Returns the sample's name */
    const String& getName() const noexcept                  { return name; }

    /** Returns the audio sample data.
     This could return nullptr if there was a problem loading the data.
     */
    AudioSampleBuffer* getAudioData() const noexcept        { return data; }


    //==============================================================================
    bool appliesToNote (int midiNoteNumber) override;
    bool appliesToChannel (int midiChannel) override;


private:
    //==============================================================================
    friend class ResamplerVoice;

    String name;
    ScopedPointer<AudioSampleBuffer> data;
    double sourceSampleRate;
    BigInteger midiNotes;
    int length, attackSamples, releaseSamples;
    int midiRootNote;

JUCE_LEAK_DETECTOR (ResamplerSound)
};


#endif  // RESAMPLER_H_INCLUDED
