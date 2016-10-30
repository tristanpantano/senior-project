/*
  ==============================================================================

    Resampler.cpp
    Created: 28 Oct 2016 4:04:41pm
    Author:  Tristan Pantano

  ==============================================================================
*/

#include "Resampler.h"

const int ResamplerVoice::NUMSAMPLESFORSOUNDTOUCH = 2048;

//==============================================================================
//Resampler
//==============================================================================

Resampler::Resampler(){}

void Resampler::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    this->setCurrentPlaybackSampleRate(sampleRate);
    
    mPitchDetector.setMinMaxFrequency(20, 2000);
    mPitchDetector.setSampleRate(sampleRate);
    
    for(int i=0; i < this->getNumVoices(); i++)
    {
        ResamplerVoice* tempVoicePtr = (ResamplerVoice*)this->getVoice(i);
        tempVoicePtr->prepareToPlay(sampleRate, samplesPerBlock);
    }
}

void Resampler::setNewSoundFile(AudioFormatReader* source)
{
    if(source != nullptr)
    {
        //remove prior sound
        for(int i=0; i < this->getNumVoices(); i++)
        {
            ResamplerVoice* tempVoicePtr = (ResamplerVoice*)this->getVoice(i);
            tempVoicePtr->clearSoundtouchSource();
        }
        this->clearSounds();
        
        //calculate pitch of sample
        int length = source->lengthInSamples;
        detectionBuffer = new AudioSampleBuffer(1, length+4);
        source->read(detectionBuffer, 0, length + 4, 0, true, true);
        double detectedPitch = mPitchDetector.detectPitch(detectionBuffer->getWritePointer(0), source->lengthInSamples);
        int MIDINoteNumber = int(log(detectedPitch/440.0)/log(2) * 12) + 69;
        
        //create range of MIDI notes sound will trigger on
        BigInteger noteRange;
        noteRange.setRange(0, 127, true);
        
        //create new sound
        ResamplerSound* tempSoundPtr = new ResamplerSound("sample", source, noteRange, MIDINoteNumber);
        this->addSound(tempSoundPtr);
        for(int i=0; i < this->getNumVoices(); i++)
        {
            ResamplerVoice* tempVoicePtr = (ResamplerVoice*)this->getVoice(i);
            tempVoicePtr->createSoundtouchSource(tempSoundPtr->getFileData());
        }
    }
}

//==============================================================================
//ResamplerSound
//==============================================================================
ResamplerSound::ResamplerSound (const String& soundName,
                            AudioFormatReader* source,
                            const BigInteger& notes,
                            const int midiNoteForNormalPitch)
: name (soundName),
midiNotes (notes),
midiRootNote (midiNoteForNormalPitch)
{
    sourceSampleRate = source->sampleRate;
    
    if (sourceSampleRate <= 0 || source->lengthInSamples <= 0)
    {
        length = 0;
        source = nullptr;
        jassert("Loaded file must have samples/sample rate!");
    }
    else
    {
        length = (int)source->lengthInSamples;
        data = new AudioFormatReaderSource(source, false);
    }
}

ResamplerSound::~ResamplerSound(){}

bool ResamplerSound::appliesToNote (int midiNoteNumber)
{
    return midiNotes [midiNoteNumber];
}

bool ResamplerSound::appliesToChannel (int /*midiChannel*/)
{
    return true;
}

//==============================================================================
//ResamplerVoice
//==============================================================================
ResamplerVoice::ResamplerVoice() : soundtouchBuffer(2, NUMSAMPLESFORSOUNDTOUCH), soundtouchAudioInfo(soundtouchBuffer), soundtouchSource(nullptr), indexInSTBuffer(0), retriggerEnabled(true), loopingEnabled(true), startPos(0), pitchRatio(0.0), srRatio(0.0), sourceSamplePosition(0.0){}

bool ResamplerVoice::canPlaySound(SynthesiserSound* sound)
{
    return dynamic_cast<const ResamplerSound*> (sound) != nullptr;
}

void ResamplerVoice::startNote(int midiNoteNumber, float velocity, SynthesiserSound* s, int pitchWheel)
{
    if (const ResamplerSound* const sound = dynamic_cast<const ResamplerSound*> (s))
    {
        if(soundtouchSource!= nullptr)
        {
            if(retriggerEnabled)
            {
                soundtouchSource->setNextReadPosition(0);
            }
            
            srRatio = sound->sourceSampleRate/getSampleRate();
            pitchRatio = pow(2.0, (midiNoteNumber - sound->midiRootNote) / 12.0);
            
            //set soundtouch playback settings
            drow::SoundTouchProcessor::PlaybackSettings settings = drow::SoundTouchProcessor::PlaybackSettings(srRatio, 1.0, pitchRatio); //sr ratio, tempo ratio, pitch ratio
            soundtouchSource->setPlaybackSettings(settings);
            
            soundtouchSource->getNextAudioBlock(soundtouchAudioInfo); //grab first block
        }
    }
    else
    {
        jassertfalse; //sound attached is not a ResamplerSound
    }
}

void ResamplerVoice::stopNote(float /*velocity*/, bool allowTailOff)
{
    if (allowTailOff)
    {
        //trigger releasing
        clearCurrentNote();
    }
    else
    {
        clearCurrentNote();
    }
}

void ResamplerVoice::pitchWheelMoved (int newValue){}
void ResamplerVoice::controllerMoved (int controllerNumber, int newValue) {}

void ResamplerVoice::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    initSoundtouchSource();
}
void ResamplerVoice::renderNextBlock(AudioSampleBuffer& outputBuffer, int startSample, int numSamples)
{
    if (const ResamplerSound* const playingSound = static_cast<ResamplerSound*> (getCurrentlyPlayingSound().get()))
    {
        //copy soundtouch buffer to output buffer
        const float* stLeft = soundtouchBuffer.getReadPointer(0);
        const float* stRight = soundtouchBuffer.getReadPointer(1);
        float* outLeft = outputBuffer.getWritePointer(0);
        float* outRight = outputBuffer.getWritePointer(1);
        for(int i = 0; i < numSamples; i++)
        {
            outLeft[i+startSample] += stLeft[indexInSTBuffer];
            outRight[i+startSample] += stRight[indexInSTBuffer];
            
            indexInSTBuffer++;
            if(indexInSTBuffer >= soundtouchBuffer.getNumSamples())
            {
                indexInSTBuffer = 0;
                soundtouchSource->getNextAudioBlock(soundtouchAudioInfo);
            }
        }
    }
}

void ResamplerVoice::createSoundtouchSource(AudioFormatReaderSource* fileData)
{
    soundtouchSource = new drow::SoundTouchAudioSource((PositionableAudioSource*)fileData);
    initSoundtouchSource();
}
void ResamplerVoice::clearSoundtouchSource()
{
    soundtouchSource = nullptr;
}
void ResamplerVoice::initSoundtouchSource()
{
    if(soundtouchSource!=nullptr)
    {
        soundtouchSource->prepareToPlay(0, getSampleRate());
    }
}