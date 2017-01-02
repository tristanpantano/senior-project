/*
  ==============================================================================

    GrainSynth.cpp
    Created: 6 Nov 2016 4:49:22pm
    Author:  Tristan Pantano

  ==============================================================================
*/

#include "GrainSynth.h"

//==============================================================================
//GrainSynth: synthesiser object providing overhead for voices and sound creation
//==============================================================================
GrainSynth::GrainSynth(){}
void GrainSynth::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    this->setCurrentPlaybackSampleRate(sampleRate);
    
    pitchDetector.setMinMaxFrequency(20, 2000);
    pitchDetector.setSampleRate(sampleRate);
    
    for(int i=0; i < this->getNumVoices(); i++)
    {
        GrainSynthVoice* tempVoicePtr = (GrainSynthVoice*)this->getVoice(i);
        tempVoicePtr->prepareToPlay(sampleRate, samplesPerBlock);
    }
}

void GrainSynth::createNewSoundFromFile(AudioFormatReader* source)
{
    if(source != nullptr)
    {
        //remove prior sound
        this->clearSounds();
        
        //calculate pitch of sample
        int length = source->lengthInSamples;
        detectionBuffer = new AudioSampleBuffer(1, length+4);
        source->read(detectionBuffer, 0, length + 4, 0, true, true);
        double detectedPitch = pitchDetector.detectPitch(detectionBuffer->getWritePointer(0), source->lengthInSamples);
        
        //create range of MIDI notes sound will trigger on
        BigInteger noteRange;
        noteRange.setRange(0, 127, true);
        
        //create new sound
        GrainSynthSound* tempSoundPtr = new GrainSynthSound(source, noteRange, detectedPitch);
        this->addSound(tempSoundPtr);
    }
}

//==============================================================================
//GrainSynthSound: a descriptor of a sound this synth will play; holds loaded file data
//==============================================================================
GrainSynthSound::GrainSynthSound(AudioFormatReader* source, const BigInteger& notes, double samplePitchInHz) : midiNotes(notes), defaultPitchInHz(samplePitchInHz)
{
    if(source->sampleRate <= 0 || source->lengthInSamples <= 0)
    {
        jassert("Source must have samples!");
    }
    else
    {
        fileSource = new AudioFormatReaderSource(source, false);
        sourceSampleRate = source->sampleRate;
        bufferedFile = new AudioSampleBuffer(2, source->lengthInSamples);
        source->read(bufferedFile, 0, source->lengthInSamples, 0, true, true);
    }
}

bool GrainSynthSound::appliesToNote (int midiNoteNumber)
{
    return midiNotes[midiNoteNumber];
}
bool GrainSynthSound::appliesToChannel (int /*midiChannel*/)
{
    return true;
}

//==============================================================================
//GrainSynthVoice: an actual voice / sound-generator implementation controlled by MIDI
//==============================================================================
GrainSynthVoice::GrainSynthVoice(){}

bool GrainSynthVoice::canPlaySound(SynthesiserSound* s)
{
    //returns true if the SynthesiserSound can be cast as a GrainSynthSound
    return dynamic_cast<const GrainSynthSound*>(s) != nullptr;
}

void GrainSynthVoice::startNote(int midiNoteNumber, float velocity, SynthesiserSound* s, int pitchWheel)
{
    if(const GrainSynthSound* const sound = dynamic_cast<const GrainSynthSound*> (s))
    {
        granulator.retrigger();
        granulator.setSource(sound->bufferedFile, sound->sourceSampleRate, sound->defaultPitchInHz);
        granulator.setTargetPitch(MidiMessage::getMidiNoteInHertz(midiNoteNumber));
    }
    else
    {
        jassertfalse; //sound attached is not a GrainSynthSound
    }
}
void GrainSynthVoice::stopNote (float velocity, bool allowTailOff)
{
    if(allowTailOff)
    {
        //trigger releasing
        clearCurrentNote();
    }
    else
    {
        clearCurrentNote();
    }
}

void GrainSynthVoice::pitchWheelMoved (int newValue){}
void GrainSynthVoice::controllerMoved (int controllerNumber, int newValue){}

void GrainSynthVoice::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    granulator.prepareToPlay(sampleRate, samplesPerBlock);
}
void GrainSynthVoice::renderNextBlock (AudioSampleBuffer& outputBuffer, int startSample, int numSamples) 
{
    if (const GrainSynthSound* const playingSound = static_cast<GrainSynthSound*> (getCurrentlyPlayingSound().get()))
    {
        granulator.renderNextBlock(outputBuffer, startSample, numSamples);
    }
}