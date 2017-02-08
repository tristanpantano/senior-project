/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

const int TextureSynthAudioProcessor::MAXPOLYPHONY = 4;

String TextureSynthAudioProcessor::grainParamArray[] = {"gloopstart", "gloopsize", "gsize", "gsemitones", "gcents", "gfixtoggle", "gfixed", "gchaos", "grepeat"}; // "gphasedecorrelate", "gstereowidth"};
const int TextureSynthAudioProcessor::NUMGRANULATORPARAMS = 9;

String TextureSynthAudioProcessor::synthParamArray[] = {"ampenvatk", "ampenvhold", "ampenvdec", "ampenvsus", "ampenvrel", "ampgain", "hpcutoff", "hpreso", "hpenvatk", "hpenvhold", "hpenvdec", "hpenvsus", "hpenvrel", "hpdepth", "lpcutoff", "lpreso", "lpenvatk", "lpenvhold", "lpenvdec", "lpenvsus", "lpenvrel", "lpdepth"};
const int TextureSynthAudioProcessor::NUMSYNTHPARAMS = 22;

String TextureSynthAudioProcessor::verbParamArray[] = {"rdrywet", "rsize", "rdamping", "rwidth"};
const int TextureSynthAudioProcessor::NUMVERBPARAMS = 4;

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
    NormalisableRange<float> dBRange(Decibels::gainToDecibels(0.0), Decibels::gainToDecibels(2.0), 0.0, getSkewFromMidpoint(Decibels::gainToDecibels(0.0), Decibels::gainToDecibels(2.0), 0.0));
    double depthSkew = getSkewFromMidpoint(20.0, 20000.0, 2000.0);
    NormalisableRange<float> hzRange(20.0, 20000.0, 0.0, getSkewFromMidpoint(20.0, 20000.0, 632.46));
    NormalisableRange<float> qRange(0.10, 18.0, 0.0, getSkewFromMidpoint(0.10, 18.0, 1.34));
    NormalisableRange<float> envTimeRange(0.0, 15000.0, 0.0, getSkewFromMidpoint(0.0, 15000.0, 1000.0));
    NormalisableRange<float> relTimeRange(6.0, 15000.0, 0.0, getSkewFromMidpoint(6.0, 15000.0, 1000.0));
    
    std::function<String (float)> dbValueToTextFunction = [](float value){ return Decibels::toString(value) + " dB"; };
    std::function<String (float)> hzValueToTextFunction = [](float value){ return String(value) + " hz";};
    std::function<String (float)> msValueToTextFunction = [](float value){ return String(value) + " ms";};
    std::function<String (float)> percentValueToTextFunction = [](float value){ return String(value) + "%";};
    
    //Granulator Parameters
    mState->createAndAddParameter(grainParamArray[0], "Loop Start", "", NormalisableRange<float>(0.0, 1.0, 0.0), 0.0, nullptr, nullptr);
    mState->createAndAddParameter(grainParamArray[1], "Loop Size", "", NormalisableRange<float>(0.0, 1.0, 0.0, getSkewFromMidpoint(0.0, 1.0, 0.25)), 1.0, nullptr, nullptr);
    mState->createAndAddParameter(grainParamArray[2], "Grain Size", " ms", NormalisableRange<float>(Grain::MINGRAINSIZEINMS, Grain::MAXGRAINSIZEINMS, 1.0), 40.0, nullptr, nullptr);
    mState->createAndAddParameter(grainParamArray[3], "Coarse Tune", " st", NormalisableRange<float>(-12.0, 12.0, 1.0), 0.0, nullptr, nullptr);
    mState->createAndAddParameter(grainParamArray[4], "Fine Tune", " cents", NormalisableRange<float>(-100.0, 100.0, 1.0), 0.0, nullptr, nullptr);
    mState->createAndAddParameter(grainParamArray[5], "Fixed Toggle", "", NormalisableRange<float>(0.0, 1.0, 1.0), 0.0, nullptr, nullptr);
    mState->createAndAddParameter(grainParamArray[6], "Read Rate", " %", NormalisableRange<float>(0.0, 200.0, 1.0, getSkewFromMidpoint(0.0, 200.0, 50.0)), 100.0, percentValueToTextFunction, nullptr);
    mState->createAndAddParameter(grainParamArray[7], "Chaos", " %", NormalisableRange<float>(0.0, 100.0, 1.0), 0.0, percentValueToTextFunction, nullptr);
    mState->createAndAddParameter(grainParamArray[8], "Grain Repeat", "", NormalisableRange<float>(0.0, 32.0, 1.0), 0.0, nullptr, nullptr);
    
    //Synth Parameters
    mState->createAndAddParameter(synthParamArray[0], "Amp. Atk.", " ms", envTimeRange, 0.0, msValueToTextFunction, nullptr);
    mState->createAndAddParameter(synthParamArray[1], "Amp. Hold", " ms", envTimeRange, 0.0, msValueToTextFunction, nullptr);
    mState->createAndAddParameter(synthParamArray[2], "Amp. Dec.", " ms", envTimeRange, 0.0, msValueToTextFunction, nullptr);
    mState->createAndAddParameter(synthParamArray[3], "Amp. Sus.", " %", NormalisableRange<float>(0.0, 100.0, 1.0), 100.0, percentValueToTextFunction, nullptr);
    mState->createAndAddParameter(synthParamArray[4], "Amp. Rel.", " ms", relTimeRange, 6.0, msValueToTextFunction, nullptr);
    mState->createAndAddParameter(synthParamArray[5], "Amp. Gain", " dB", dBRange, 0.0, dbValueToTextFunction, nullptr);
    
    mState->createAndAddParameter(synthParamArray[6], "HPF Cutoff", " hz", hzRange, 20.0, hzValueToTextFunction, nullptr);
    mState->createAndAddParameter(synthParamArray[7], "HPF Res.", "", qRange, 0.71, nullptr, nullptr);
    mState->createAndAddParameter(synthParamArray[8], "HPF Atk.", " ms", envTimeRange, 0.0, msValueToTextFunction, nullptr);
    mState->createAndAddParameter(synthParamArray[9], "HPF Hold", " ms", envTimeRange, 0.0, msValueToTextFunction, nullptr);
    mState->createAndAddParameter(synthParamArray[10], "HPF Dec.", " ms", envTimeRange, 0.0, msValueToTextFunction, nullptr);
    mState->createAndAddParameter(synthParamArray[11], "HPF Sus.", " %", NormalisableRange<float>(0.0, 100.0, 1.0), 100.0, percentValueToTextFunction, nullptr);
    mState->createAndAddParameter(synthParamArray[12], "HPF Rel.", " ms", relTimeRange, 6.0, msValueToTextFunction, nullptr);
    mState->createAndAddParameter(synthParamArray[13], "HPF Depth", " hz", NormalisableRange<float>(-20000.0, 20000.0, 1.0, depthSkew, true), 0.0, hzValueToTextFunction, nullptr);
    
    mState->createAndAddParameter(synthParamArray[14], "LPF Cutoff", " hz", hzRange, 20000.0, hzValueToTextFunction, nullptr);
    mState->createAndAddParameter(synthParamArray[15], "LPF Res.", "", qRange, 0.71, nullptr, nullptr);
    mState->createAndAddParameter(synthParamArray[16], "LPF Atk.", " ms", envTimeRange, 0.0, msValueToTextFunction, nullptr);
    mState->createAndAddParameter(synthParamArray[17], "LPF Hold", " ms", envTimeRange, 0.0, msValueToTextFunction, nullptr);
    mState->createAndAddParameter(synthParamArray[18], "LPF Dec.", " ms", envTimeRange, 0.0, msValueToTextFunction, nullptr);
    mState->createAndAddParameter(synthParamArray[19], "LPF Sus.", " %", NormalisableRange<float>(0.0, 100.0, 1.0), 100.0, percentValueToTextFunction, nullptr);
    mState->createAndAddParameter(synthParamArray[20], "LPF Rel.", " ms", relTimeRange, 6.0, msValueToTextFunction, nullptr);
    mState->createAndAddParameter(synthParamArray[21], "LPF Depth", " hz", NormalisableRange<float>(-20000.0, 20000.0, 1.0, depthSkew, true), 0.0, hzValueToTextFunction, nullptr);
    
    //ShimmerVerb Parameters
    mState->createAndAddParameter(verbParamArray[0], "Rvb Dry/Wet", " %", NormalisableRange<float>(0, 100.0, 1.0), 10.0, percentValueToTextFunction, nullptr);
    mState->createAndAddParameter(verbParamArray[1], "Rvb Size", " %", NormalisableRange<float>(0, 100.0, 1.0), 50.0, percentValueToTextFunction, nullptr);
    mState->createAndAddParameter(verbParamArray[2], "Rvb Damping", " %", NormalisableRange<float>(0, 100.0, 1.0), 33.0, percentValueToTextFunction, nullptr);
    mState->createAndAddParameter(verbParamArray[3], "Rvb Width", " %", NormalisableRange<float>(0, 100.0, 1.0), 50.0, percentValueToTextFunction, nullptr);
    
    //Attach synth params
    for(int i = 0; i < NUMSYNTHPARAMS; i++)
    {
        mState->addParameterListener(synthParamArray[i], &synth);
        AudioProcessorParameter* tempForInit = mState->getParameter(synthParamArray[i]);
        tempForInit->setValueNotifyingHost(tempForInit->getValue());
    }
    //Attach grain params
    for(int i = 0; i < NUMGRANULATORPARAMS; i++)
    {
        mState->addParameterListener(grainParamArray[i], &synth);
        AudioProcessorParameter* tempForInit = mState->getParameter(grainParamArray[i]);
        tempForInit->setValueNotifyingHost(tempForInit->getValue());
    }
    //Attach shimmerverb params
    for(int i = 0; i < NUMVERBPARAMS; i++)
    {
        mState->addParameterListener(verbParamArray[i], &shimmerVerb);
        AudioProcessorParameter* tempForInit = mState->getParameter(verbParamArray[i]);
        tempForInit->setValueNotifyingHost(tempForInit->getValue());
    }
    
}
double TextureSynthAudioProcessor::getSkewFromMidpoint(double min, double max, double midpoint)
{
    return log (0.5) / log ((midpoint - min) / (max - min));
}
void TextureSynthAudioProcessor::parameterChanged(const String &parameterID, float newValue)
{
}
TextureSynthAudioProcessor::~TextureSynthAudioProcessor()
{
    //Detach processor state listeners
    for(int i = 0; i < NUMSYNTHPARAMS; i++)
    {
        mState->removeParameterListener(synthParamArray[i], &synth);
    }
    for(int i = 0; i < NUMGRANULATORPARAMS; i++)
    {
        mState->removeParameterListener(grainParamArray[i], &synth);
    }
    for(int i = 0; i < NUMVERBPARAMS; i++)
    {
        mState->removeParameterListener(verbParamArray[i], &shimmerVerb);
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
    shimmerVerb.prepareToPlay(sampleRate, samplesPerBlock);
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
    
    //Render active granulator voices
    synth.renderNextBlock(buffer, midiMessages, 0, numSamples);
    
    //Apply ShimmerVerb
    shimmerVerb.renderNextBlock(buffer, 0, numSamples);
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
