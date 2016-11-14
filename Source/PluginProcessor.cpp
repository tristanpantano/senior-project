/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

const int TextureSynthAudioProcessor::MAXPOLYPHONY = 4;

//==============================================================================
TextureSynthAudioProcessor::TextureSynthAudioProcessor() : mFileReader(nullptr), fileAddress("")
{
    mFormatManager.registerBasicFormats();
    
    initSynth(); //do after reading in file stored in state
}

void TextureSynthAudioProcessor::initSynth()
{
    for(int i = 0; i < MAXPOLYPHONY; i++)
    {
        GrainSynthVoice* newVoice = new GrainSynthVoice();
        synth.addVoice((SynthesiserVoice*)newVoice);
    }
    synth.createNewSoundFromFile(mFileReader);
}

TextureSynthAudioProcessor::~TextureSynthAudioProcessor()
{
}

//==============================================================================
const String TextureSynthAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool TextureSynthAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool TextureSynthAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

double TextureSynthAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int TextureSynthAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int TextureSynthAudioProcessor::getCurrentProgram()
{
    return 0;
}

void TextureSynthAudioProcessor::setCurrentProgram (int index)
{
}

const String TextureSynthAudioProcessor::getProgramName (int index)
{
    return String();
}

void TextureSynthAudioProcessor::changeProgramName (int index, const String& newName)
{
}

//==============================================================================
void TextureSynthAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    synth.prepareToPlay(sampleRate, samplesPerBlock);
}

void TextureSynthAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool TextureSynthAudioProcessor::setPreferredBusArrangement (bool isInput, int bus, const AudioChannelSet& preferredSet)
{
    // Reject any bus arrangements that are not compatible with your plugin

    const int numChannels = preferredSet.size();

   #if JucePlugin_IsMidiEffect
    if (numChannels != 0)
        return false;
   #elif JucePlugin_IsSynth
    if (isInput || (numChannels != 1 && numChannels != 2))
        return false;
   #else
    if (numChannels != 1 && numChannels != 2)
        return false;

    if (! AudioProcessor::setPreferredBusArrangement (! isInput, bus, preferredSet))
        return false;
   #endif

    return AudioProcessor::setPreferredBusArrangement (isInput, bus, preferredSet);
}
#endif

void TextureSynthAudioProcessor::processBlock (AudioSampleBuffer& buffer, MidiBuffer& midiMessages)
{
    const int numSamples = buffer.getNumSamples();
    
    buffer.clear();
    
    synth.renderNextBlock(buffer, midiMessages, 0, numSamples);
}

//==============================================================================
//==============================================================================
void TextureSynthAudioProcessor::setFileReader(File* file)
{
    mFileReader = mFormatManager.createReaderFor(*file);
    fileAddress = file->getFullPathName();
    
    synth.createNewSoundFromFile(mFileReader);
}

//==============================================================================
//==============================================================================
bool TextureSynthAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

AudioProcessorEditor* TextureSynthAudioProcessor::createEditor()
{
    return new TextureSynthAudioProcessorEditor (*this);
}

//==============================================================================
void TextureSynthAudioProcessor::getStateInformation (MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void TextureSynthAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new TextureSynthAudioProcessor();
}
