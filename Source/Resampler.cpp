/*
  ==============================================================================

    Resampler.cpp
    Created: 28 Oct 2016 4:04:41pm
    Author:  Tristan Pantano

  ==============================================================================
*/

#include "Resampler.h"

//==============================================================================
//Resampler
//==============================================================================
const int Resampler::MAXSAMPLELENGTHINSECONDS = 8;

Resampler::Resampler(){}

void Resampler::setNewSoundFile(AudioFormatReader* source)
{
    if(source != nullptr)
    {
        this->clearSounds();
        BigInteger noteRange;
        noteRange.setRange(0, 127, true);
        this->addSound(new ResamplerSound("sample", *source, noteRange, 24, 0, 0.001, MAXSAMPLELENGTHINSECONDS));
    }
}

//==============================================================================
//ResamplerSound
//==============================================================================
ResamplerSound::ResamplerSound (const String& soundName,
                            AudioFormatReader& source,
                            const BigInteger& notes,
                            const int midiNoteForNormalPitch,
                            const double attackTimeSecs,
                            const double releaseTimeSecs,
                            const double maxSampleLengthSeconds)
: name (soundName),
midiNotes (notes),
midiRootNote (midiNoteForNormalPitch)
{
    sourceSampleRate = source.sampleRate;
    
    if (sourceSampleRate <= 0 || source.lengthInSamples <= 0)
    {
        length = 0;
        attackSamples = 0;
        releaseSamples = 0;
    }
    else
    {
        length = jmin ((int) source.lengthInSamples,
                       (int) (maxSampleLengthSeconds * sourceSampleRate));
        
        data = new AudioSampleBuffer (jmin (2, (int) source.numChannels), length + 4);
        
        source.read (data, 0, length + 4, 0, true, true);
        
        attackSamples = roundToInt (attackTimeSecs * sourceSampleRate);
        releaseSamples = roundToInt (releaseTimeSecs * sourceSampleRate);
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
ResamplerVoice::ResamplerVoice() : retriggerEnabled(true), startPos(0), pitchRatio(0.0), sourceSamplePosition(0.0){}

bool ResamplerVoice::canPlaySound(SynthesiserSound* sound)
{
    return dynamic_cast<const ResamplerSound*> (sound) != nullptr;
}

void ResamplerVoice::startNote(int midiNoteNumber, float velocity, SynthesiserSound* s, int pitchWheel)
{
    if (const ResamplerSound* const sound = dynamic_cast<const ResamplerSound*> (s))
    {
        if(retriggerEnabled)
        {
            sourceSamplePosition =  startPos;
        }
        
        pitchRatio = pow (2.0, (midiNoteNumber - sound->midiRootNote) / 12.0)* sound->sourceSampleRate / getSampleRate();
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

void ResamplerVoice::renderNextBlock(AudioSampleBuffer& outputBuffer, int startSample, int numSamples)
{
    if (const ResamplerSound* const playingSound = static_cast<ResamplerSound*> (getCurrentlyPlayingSound().get()))
    {
        const float* const inL = playingSound->data->getReadPointer (0);
        const float* const inR = playingSound->data->getNumChannels() > 1
        ? playingSound->data->getReadPointer(1) : playingSound->data->getReadPointer(0);
        //^use right channel if it exists (stereo input), otherwise use the left channel (mono input)
        
        float* outL = outputBuffer.getWritePointer (0, startSample);
        float* outR = outputBuffer.getWritePointer (1, startSample);
        
        for(int i = startSample; i < startSample+numSamples; i++)
        {
            const int pos = (int) sourceSamplePosition;
            const float alpha = (float) (sourceSamplePosition - pos);
            const float invAlpha = 1.0f - alpha;
            
            // just using a very simple linear interpolation here..
            float l = (inL [pos] * invAlpha + inL [pos + 1] * alpha);
            float r = (inR != nullptr) ? (inR [pos] * invAlpha + inR [pos + 1] * alpha) : l;
            
            *outL++ += l;
            *outR++ += r;
            
            sourceSamplePosition += pitchRatio;
            
            if (sourceSamplePosition > playingSound->length)
            {
                sourceSamplePosition = startPos;
            }
        }
    }
}