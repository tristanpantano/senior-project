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
GrainSynthVoice::GrainSynthVoice() : voiceBuffer(2, 2048)
{
    timeSinceTrigger = 0.0;
    isReleasing = false;
}

void GrainSynthVoice::parameterChanged(const String &parameterID, float newValue)
{
    if(parameterID == "ampenvatk")
    {
        ampEnv.setAttack(newValue*0.001);
    }
    else if(parameterID == "ampenvhold")
    {
        ampEnv.setHold(newValue*0.001);
    }
    else if(parameterID == "ampenvdec")
    {
        ampEnv.setDecay(newValue*0.001);
    }
    else if(parameterID == "ampenvsus")
    {
        ampEnv.setSustain(newValue);
    }
    else if(parameterID == "ampenvrel")
    {
        ampEnv.setRelease(newValue*0.001);
    }
}

bool GrainSynthVoice::canPlaySound(SynthesiserSound* s)
{
    //returns true if the SynthesiserSound can be cast as a GrainSynthSound
    return dynamic_cast<const GrainSynthSound*>(s) != nullptr;
}

void GrainSynthVoice::startNote(int midiNoteNumber, float velocity, SynthesiserSound* s, int pitchWheel)
{
    if(const GrainSynthSound* const sound = dynamic_cast<const GrainSynthSound*> (s))
    {
        isReleasing = false;
        timeSinceTrigger = 0.0;
        
        granulator.retrigger();
        granulator.setSource(sound->bufferedFile, sound->sourceSampleRate, sound->defaultPitchInHz);
        granulator.setTargetPitch(MidiMessage::getMidiNoteInHertz(midiNoteNumber));
    }
    else
    {
        jassertfalse; //sound attached is not a GrainSynthSound
    }
}
void GrainSynthVoice::stopNote(float velocity, bool allowTailOff)
{
    if(allowTailOff)
    {
        if(!isReleasing) //potential for multiple stop note messages
        {
            isReleasing = true;
            timeSinceTrigger = 0.0;
        }
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
        voiceBuffer.clear();
        
        //Render granulator output
        granulator.renderNextBlock(voiceBuffer, startSample, numSamples);
        
        //Envelope handling
        for(int currentSample = startSample; currentSample < startSample+numSamples; currentSample++)
        {
            timeSinceTrigger += 1.0 / getSampleRate();
            
            //Process volume envelope
            double volEnvValue;
            if(isReleasing)
            {
                volEnvValue = ampEnv.getReleaseValue(timeSinceTrigger);
                if(volEnvValue <= 0.0)
                {
                    voiceBuffer.applyGain(currentSample, voiceBuffer.getNumSamples() - currentSample, 0.0);
                    clearCurrentNote();
                    break;
                }
            }
            else
            {
                volEnvValue = ampEnv.getEnvelopeValue(timeSinceTrigger);
            }
            float* writeChannel;
            for(int channel = 0; channel < voiceBuffer.getNumChannels(); channel++)
            {
                writeChannel = voiceBuffer.getWritePointer(channel);
                writeChannel[currentSample] *= volEnvValue;
            }
        }
        
        //Add internal voice buffer to output buffer
        for(int channel = 0; channel < outputBuffer.getNumChannels(); channel++)
        {
            outputBuffer.addFrom(channel, startSample, voiceBuffer, channel, startSample, numSamples);
        }
    }
    
    
}