/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#ifndef PLUGINEDITOR_H_INCLUDED
#define PLUGINEDITOR_H_INCLUDED

#include "../JuceLibraryCode/JuceHeader.h"
#include "PluginProcessor.h"


//==============================================================================
/**
*/
class TextureSynthAudioProcessorEditor  : public AudioProcessorEditor, public Button::Listener, public ChangeListener
{
public:
    TextureSynthAudioProcessorEditor (TextureSynthAudioProcessor&);
    ~TextureSynthAudioProcessorEditor();

    //==============================================================================
    void paint (Graphics&) override;
    void resized() override;
    
    //==============================================================================
    //Button Listener
    void buttonClicked(Button* button) override;
    void buttonStateChanged(Button* button) override;
    
    //Change Listener
    void changeListenerCallback(ChangeBroadcaster* source) override;
    
    //Bound rectangles
    static const Rectangle<int> thumbnailBounds;

    //Constants
    static const int numSliders;
    
private:
    //Buttons
    TextButton fileLoadButton;
    void loadButtonClicked();
    
    //Sliders
    int sliderAttachIndex;
    ScopedPointer<AudioProcessorValueTreeState::SliderAttachment> *sliderAttachments;
    void initSlider(Slider& slider, const String parameterID);
    Slider sliderGainEnvAtk, sliderGainEnvHold, sliderGainEnvDec, sliderGainEnvSus, sliderGainEnvRel;
    Slider sliderHpfEnvAtk, sliderHpfEnvHold, sliderHpfEnvDec, sliderHpfEnvSus, sliderHpfEnvRel, sliderHpfDepth;
    Slider sliderLpfEnvAtk, sliderLpfEnvHold, sliderLpfEnvDec, sliderLpfEnvSus, sliderLpfEnvRel, sliderLpfDepth;
    
    //Knobs
    void initKnob(Slider& knob, const String parameterID);
    Slider knobGain, knobHpCutoff, knobHpReso, knobLpCutoff, knobLpReso;
    
    //Thumbnail
    void paintThumbnail(Graphics& g, const Rectangle<int> thumbnailBounds);
    
    
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    TextureSynthAudioProcessor& processor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TextureSynthAudioProcessorEditor)
};


#endif  // PLUGINEDITOR_H_INCLUDED
