/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"


//==============================================================================
TextureSynthAudioProcessorEditor::TextureSynthAudioProcessorEditor (TextureSynthAudioProcessor& p)
    : AudioProcessorEditor (&p), processor (p)
{
    fileLoadButton.setButtonText("Load");
    fileLoadButton.addListener(this);
    addAndMakeVisible(fileLoadButton);
    
    
    setSize (400, 300);
}

TextureSynthAudioProcessorEditor::~TextureSynthAudioProcessorEditor()
{
}

//==============================================================================
void TextureSynthAudioProcessorEditor::paint (Graphics& g)
{
    g.fillAll (Colours::white);

    g.setColour (Colours::black);
    g.setFont (15.0f);
    g.drawFittedText ("Hello World!", getLocalBounds(), Justification::centred, 1);
}

void TextureSynthAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    fileLoadButton.setBounds(10, 10, 40, 20);
}

//==============================================================================
void TextureSynthAudioProcessorEditor::buttonClicked(Button* button)
{
    if(button == &fileLoadButton)
    {
        this->loadButtonClicked();
    }
}
void TextureSynthAudioProcessorEditor::buttonStateChanged(Button* button)
{
    
}

//==============================================================================
void TextureSynthAudioProcessorEditor::loadButtonClicked()
{
    FileChooser chooser("Select a wav file to play...", File::nonexistent, "*.wav");
    
    if(chooser.browseForFileToOpen())
    {
        File file (chooser.getResult());
        processor.setFileReader(processor.getFormatManager()->createReaderFor(file));
        processor.getResampler()->setNewSoundFile(processor.getFileReader());
    }
}
