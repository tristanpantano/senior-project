/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#ifndef PLUGINPROCESSOR_H_INCLUDED
#define PLUGINPROCESSOR_H_INCLUDED

#include "../JuceLibraryCode/JuceHeader.h"
#include "GrainSynth.h"

//==============================================================================
/**
*/
class TextureSynthAudioProcessor  : public AudioProcessor
{
public:
    //==============================================================================
    TextureSynthAudioProcessor();
    ~TextureSynthAudioProcessor();

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool setPreferredBusArrangement (bool isInput, int bus, const AudioChannelSet& preferredSet) override;
   #endif

    void processBlock (AudioSampleBuffer&, MidiBuffer&) override;

    //==============================================================================
    AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const String getProgramName (int index) override;
    void changeProgramName (int index, const String& newName) override;

    //==============================================================================
    void getStateInformation (MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;
    AudioProcessorValueTreeState& getValueTreeState();
    
    //==============================================================================
    static const int MAXPOLYPHONY;
    
    static String grainParamArray[];
    static const int NUMGRANULATORPARAMS;
    
    static String synthParamArray[];
    static const int NUMSYNTHPARAMS;
    
    //==============================================================================
    //File reading methods
    void setFileReader(File& file);
    void loadFileFromAddress(const String &absPath);
    
    AudioThumbnail* getThumbnail(){ return & thumbnail; }
    
private:
    //==============================================================================
    //File Reading
    AudioFormatManager mFormatManager;
    ScopedPointer<AudioFormatReader> mFileReader;
    String fileAddress;
    
    //Waveform thumbnail
    AudioThumbnailCache thumbnailCache;
    AudioThumbnail thumbnail;
    
    //State and params
    ScopedPointer<AudioProcessorValueTreeState> mState;
    ScopedPointer<UndoManager> mUndoManager;
    void initParams();
    
    //State save/load
    XmlElement createXmlState();
    void loadXmlState(XmlElement* xmlState);
    
    //Synthesiser (MIDI handling) component
    GrainSynth synth;
    void initSynth();

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TextureSynthAudioProcessor)
};


#endif  // PLUGINPROCESSOR_H_INCLUDED
