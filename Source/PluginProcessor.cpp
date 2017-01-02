/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

const int TextureSynthAudioProcessor::MAXPOLYPHONY = 4;

String TextureSynthAudioProcessor::grainParamArray[] = {"gsize", "gtimescale"};//, "gphasedecorrelate", "gdetune", "gcoarse", "gfine", "gstereowidth"};
const int TextureSynthAudioProcessor::NUMGRANULATORPARAMS = 2;

//==============================================================================
TextureSynthAudioProcessor::TextureSynthAudioProcessor() : mFileReader(nullptr), fileAddress(""), thumbnailCache(5), thumbnail(512, mFormatManager, thumbnailCache)
{
    mFormatManager.registerBasicFormats();
    
    mUndoManager = new UndoManager();
    mState = new AudioProcessorValueTreeState (*this, mUndoManager);
    
    initSynth();
    initParams();

    mState->state = ValueTree("TextureSynthParameters");
}
void TextureSynthAudioProcessor::initSynth()
{
    for(int i = 0; i < MAXPOLYPHONY; i++)
    {
        GrainSynthVoice* newVoice = new GrainSynthVoice();
        synth.addVoice((SynthesiserVoice*)newVoice);
    }
}
void TextureSynthAudioProcessor::initParams()
{
    //Granulator Parameters
    mState->createAndAddParameter(grainParamArray[0], "Grain Size", " ms", NormalisableRange<float>(10, 80, 1), 40, nullptr, nullptr);
    mState->createAndAddParameter(grainParamArray[1], "Time Scale", " %", NormalisableRange<float>(1, 50, 0.5), 12.5, nullptr, nullptr);
    for(int j = 0; j < synth.getNumVoices(); j++) //Make each synth's voice's granulator a listener
    {
        GrainSynthVoice* tempVoice = (GrainSynthVoice*)synth.getVoice(j);
        Granulator* granulator = tempVoice->getGranulator();
        for(int i = 0; i < NUMGRANULATORPARAMS; i++)
        {
            mState->addParameterListener(grainParamArray[i], granulator);
            AudioProcessorParameter* tempForInit = mState->getParameter(grainParamArray[i]);
            tempForInit->setValueNotifyingHost(tempForInit->getValue());
        }
    }
}
TextureSynthAudioProcessor::~TextureSynthAudioProcessor()
{
    //Detach synth's voice's granulators from being processor state listeners
    for(int j = 0; j < synth.getNumVoices(); j++)
    {
        GrainSynthVoice* tempVoice = (GrainSynthVoice*) synth.getVoice(j);
        Granulator* granulator = tempVoice->getGranulator();
        for(int i = 0; i < NUMGRANULATORPARAMS; i++)
        {
            mState->removeParameterListener(grainParamArray[i], granulator);
        }
    }
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

void TextureSynthAudioProcessor::processBlock(AudioSampleBuffer& buffer, MidiBuffer& midiMessages)
{
    const int numSamples = buffer.getNumSamples();
    
    buffer.clear();
    
    synth.renderNextBlock(buffer, midiMessages, 0, numSamples);
}

//==============================================================================
//==============================================================================
void TextureSynthAudioProcessor::setFileReader(File& file)
{
    mFileReader = mFormatManager.createReaderFor(file);
    fileAddress = file.getFullPathName();
    synth.createNewSoundFromFile(mFileReader);
    thumbnail.setSource(new FileInputSource(file));
}
void TextureSynthAudioProcessor::loadFileFromAddress(const String &absPath)
{
    File file(absPath);
    if(file.existsAsFile() && file.hasFileExtension("wav"))
    {
        setFileReader(file);
    }
    else if(!file.existsAsFile())
    {
        //couldn't load file from path
    }
    else
    {
        //file not a .wav
    }
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
    copyXmlToBinary(createXmlState(), destData);
}

void TextureSynthAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    ScopedPointer<XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    loadXmlState(xmlState);
}
XmlElement TextureSynthAudioProcessor::createXmlState()
{
    XmlElement xmlState("TextureSynthXML");
    
    xmlState.addChildElement(mState->state.createXml());
    
    XmlElement* xmlFileName = new XmlElement("TextureSynthLoadedFile");
    xmlFileName->setAttribute("path", fileAddress);
    xmlState.addChildElement(xmlFileName);
    
    return xmlState;
}
void TextureSynthAudioProcessor::loadXmlState(XmlElement* xmlState)
{
     if(xmlState != nullptr && xmlState->getTagName() == "TextureSynthXML")
     {
         forEachXmlChildElement (*xmlState, e)
         {
             if(e->hasTagName(mState->state.getType()))
             {
                 const XmlElement xmlParamState(*e);
                 mState->state = ValueTree::fromXml(xmlParamState);
             }
             else if(e->hasTagName("TextureSynthLoadedFile"))
             {
                 loadFileFromAddress(e->getStringAttribute("path"));
             }
         }
     }
    else
    {
        //improper xml state
    }
}
AudioProcessorValueTreeState& TextureSynthAudioProcessor::getValueTreeState()
{
    return *mState;
}

//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new TextureSynthAudioProcessor();
}
