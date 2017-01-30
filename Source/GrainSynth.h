/*
  ==============================================================================

    GrainSynth.h
    Created: 6 Nov 2016 4:49:22pm
    Author:  Tristan Pantano

  ==============================================================================
*/

#ifndef GRAINSYNTH_H_INCLUDED
#define GRAINSYNTH_H_INCLUDED

#include "../JuceLibraryCode/JuceHeader.h"
#include "dRowAudio.h"
#include "Granulator.h"
#include "Envelope.h"

//==============================================================================
//GrainSynth: synthesiser object providing overhead for voices and sound creation
//==============================================================================
class GrainSynth : public Synthesiser, public AudioProcessorValueTreeState::Listener
{
public:
    GrainSynth();

    void prepareToPlay(double sampleRate, int samplesPerBlock);
    void createNewSoundFromFile(AudioFormatReader* source);
    
    void parameterChanged(const String &parameterID, float newValue) override;
    
private:
    drow::PitchDetector pitchDetector;
    ScopedPointer<AudioSampleBuffer> detectionBuffer;
};

//==============================================================================
//GrainSynthSound: a descriptor of a sound this synth will play; holds loaded file data
//==============================================================================
class GrainSynthSound : public SynthesiserSound
{
public:
    GrainSynthSound(AudioFormatReader* source, const BigInteger& midiNotes, double samplePitchInHz);
    
    bool appliesToNote(int midiNoteNumber) override;
    bool appliesToChannel(int midiChannel) override;
    
private:
    friend class GrainSynthVoice;
    
    BigInteger midiNotes;
    double defaultPitchInHz;
    double sourceSampleRate;
    
    ScopedPointer<AudioSampleBuffer> bufferedFile;
    ScopedPointer<AudioFormatReaderSource> fileSource;
};

//==============================================================================
//GrainSynthVoice: an actual voice / sound-generator implementation controlled by MIDI
//==============================================================================
class GrainSynthVoice : public SynthesiserVoice
{
public:
    GrainSynthVoice();
    
    Granulator* getGranulator() { return &granulator; }
    
    void parameterChanged(const String &parameterID, float newValue);
    
    bool canPlaySound (SynthesiserSound* s) override;
    
    void startNote (int midiNoteNumber, float velocity, SynthesiserSound* s, int pitchWheel) override;
    void stopNote (float velocity, bool allowTailOff) override;
    
    void pitchWheelMoved (int newValue) override;
    void controllerMoved (int controllerNumber, int newValue) override;
    
    void prepareToPlay(double sampleRate, int samplesPerBlock);
    void renderNextBlock (AudioSampleBuffer& outputBuffer, int startSample, int numSamples) override;
    
private:
    AudioSampleBuffer voiceBuffer;
    Granulator granulator;
    double timeSinceTrigger, grainAmpGain;
    bool isReleasing;
    Envelope ampEnv, hpEnv, lpEnv;
    
    double lpFreq, lpResonance, lpDepth, hpFreq, hpResonance, hpDepth;
    IIRFilter leftHighpass, rightHighpass, leftLowpass, rightLowpass;
};


#endif  // GRAINSYNTH_H_INCLUDED
