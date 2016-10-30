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
#include "dRowAudio.h"

//==============================================================================
//Resampler: a synthesiser that can play back a single audio file across its voices
//==============================================================================
class Resampler : public Synthesiser
{
public:
    Resampler();
    
    void prepareToPlay(double sampleRate, int samplesPerBlock);
    void setNewSoundFile(AudioFormatReader* source);
    
private:
    drow::PitchDetector mPitchDetector;
    ScopedPointer<AudioSampleBuffer> detectionBuffer;
};

//==============================================================================
//ResamplerSound:
//==============================================================================
class ResamplerSound : public SynthesiserSound
{
public:
    ResamplerSound (const String& name,
                  AudioFormatReader* source,
                  const BigInteger& midiNotes,
                  int midiNoteForNormalPitch);

    /** Destructor. */
    ~ResamplerSound();

    //==============================================================================
    /** Returns the sample's name */
    const String& getName() const noexcept                  { return name; }
    
    AudioFormatReaderSource* getFileData() { return data; }

    //==============================================================================
    bool appliesToNote (int midiNoteNumber) override;
    bool appliesToChannel (int midiChannel) override;


private:
    //==============================================================================
    friend class ResamplerVoice;

    String name;
    ScopedPointer<AudioFormatReaderSource> data;
    double sourceSampleRate;
    BigInteger midiNotes;
    int length;
    int midiRootNote;

JUCE_LEAK_DETECTOR (ResamplerSound)
};

//==============================================================================
//ResamplerVoice: playback logic for an audio file loaded into a ResamplerSound
//==============================================================================
class ResamplerVoice : public SynthesiserVoice
{
public:
    //==============================================================================
    ResamplerVoice();
    //~ResamplerVoice();
    
    //==============================================================================
    bool canPlaySound (SynthesiserSound*) override;
    
    void startNote (int midiNoteNumber, float velocity, SynthesiserSound* s, int pitchWheel) override;
    void stopNote (float velocity, bool allowTailOff) override;
    
    void pitchWheelMoved (int newValue) override;
    void controllerMoved (int controllerNumber, int newValue) override;
    
    void prepareToPlay(double sampleRate, int samplesPerBlock);
    void renderNextBlock (AudioSampleBuffer& outputBuffer, int startSample, int numSamples) override;
    
    void createSoundtouchSource(AudioFormatReaderSource* fileData);
    void clearSoundtouchSource();
    void initSoundtouchSource();
    
    static const int NUMSAMPLESFORSOUNDTOUCH;
    
private:
    //==============================================================================
    AudioSampleBuffer soundtouchBuffer;
    AudioSourceChannelInfo soundtouchAudioInfo;
    ScopedPointer<drow::SoundTouchAudioSource> soundtouchSource;
    int indexInSTBuffer;
    
    bool retriggerEnabled, loopingEnabled;
    
    int startPos;
    
    double pitchRatio, srRatio;
    double sourceSamplePosition;
};


#endif  // RESAMPLER_H_INCLUDED
