#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
TextureSynthAudioProcessorEditor::TextureSynthAudioProcessorEditor (TextureSynthAudioProcessor& p)
    : AudioProcessorEditor (&p), processor (p)
{
    //Load button
    fileLoadButton.setButtonText("Load");
    fileLoadButton.addListener(this);
    addAndMakeVisible(fileLoadButton);
    
    //waveform thumbnail
    processor.getThumbnail()->addChangeListener(this);
    
    //set size at the end
    setSize(800, 300);
}

TextureSynthAudioProcessorEditor::~TextureSynthAudioProcessorEditor()
{
}

//==============================================================================
void TextureSynthAudioProcessorEditor::paint (Graphics& g)
{
    g.fillAll (Colours::white);
    
    //Thumbnail painting
    const Rectangle<int> thumbnailBounds (10, 50, 300, 150);
    paintThumbnail(g, thumbnailBounds);
}

void TextureSynthAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    fileLoadButton.setBounds(10, 205, 40, 20);
    
}

//==============================================================================
void TextureSynthAudioProcessorEditor::changeListenerCallback(ChangeBroadcaster* source)
{
    if(source == processor.getThumbnail()) repaint();
}

//==============================================================================
void TextureSynthAudioProcessorEditor::buttonClicked(Button* button)
{
    if(button == &fileLoadButton)
    {
        this->loadButtonClicked();
    }
}
void TextureSynthAudioProcessorEditor::buttonStateChanged(Button* button){}

void TextureSynthAudioProcessorEditor::loadButtonClicked()
{
    FileChooser chooser("Select a wav file to play...", File::nonexistent, "*.wav");
    
    if(chooser.browseForFileToOpen())
    {
        File file(chooser.getResult());
        processor.setFileReader(file);
    }
}

//==============================================================================
void TextureSynthAudioProcessorEditor::initKnob(Slider& knob)
{
    knob.setSliderStyle(Slider::RotaryVerticalDrag);
    knob.setSliderSnapsToMousePosition(false);
    knob.setMouseDragSensitivity(100);
    knob.setRotaryParameters(float_Pi*5.0/4.0, float_Pi*11.0/4.0, true);
    knob.setColour(Slider::rotarySliderOutlineColourId, Colour(0,0,0));
    knob.setColour(Slider::rotarySliderFillColourId, Colour(0,0,0));
    knob.setTextBoxStyle(Slider::NoTextBox, false, 90, 0);
    knob.setPopupDisplayEnabled(true, this);
}

//==============================================================================
void TextureSynthAudioProcessorEditor::paintThumbnail(Graphics& g, const Rectangle<int> thumbnailBounds)
{
    AudioThumbnail* thumbnail = processor.getThumbnail();
    if(thumbnail->getNumChannels() == 0) //no file loaded
    {
        g.setColour (Colours::darkgrey);
        g.fillRect (thumbnailBounds);
        g.setColour (Colours::white);
        g.drawFittedText ("No File Loaded", thumbnailBounds, Justification::centred, 1.0f);
    }
    else
    {
        //Background
        g.setColour (Colours::black);
        g.fillRect (thumbnailBounds);
        //Waveform
        g.setColour (Colours::blueviolet);
        thumbnail->drawChannels (g, thumbnailBounds, 0.0, thumbnail->getTotalLength(), 1.0f);
    }
}