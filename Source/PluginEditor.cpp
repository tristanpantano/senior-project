#include "PluginProcessor.h"
#include "PluginEditor.h"

const Rectangle<int> TextureSynthAudioProcessorEditor::thumbnailBounds(10, 50, 300, 150);

const int TextureSynthAudioProcessorEditor::numSliders = 34; //increment by 1 for each knob/slider added

//==============================================================================
TextureSynthAudioProcessorEditor::TextureSynthAudioProcessorEditor (TextureSynthAudioProcessor& p)
    : AudioProcessorEditor (&p), processor (p)
{
    //load images
    cachedImage_tempBG_png = ImageFileFormat::loadFrom(TexImgBin::tempBG_png, TexImgBin::tempBG_pngSize);
    ImageCache::addImageToCache(cachedImage_tempBG_png, 0);
    
    //constants
    const double hzSkew = TextureSynthAudioProcessor::getSkewFromMidpoint(20.0, 20000.0, 2000.0);
    
    //array of attachments (to decrease code size)
    sliderAttachments = new ScopedPointer<AudioProcessorValueTreeState::SliderAttachment>[numSliders];
    sliderAttachIndex = 0;
    
    //Load button
    fileLoadButton.setButtonText("Load");
    fileLoadButton.addListener(this);
    addAndMakeVisible(fileLoadButton);
    
    //Loop controls
    initKnob(knobLoopStart, p.grainParamArray[0]);
    initKnob(knobLoopSize, p.grainParamArray[1]);
    initKnob(knobGrainSize, p.grainParamArray[2]);
    knobLoopSize.setSkewFactorFromMidPoint(0.25);
    knobLoopStart.setPopupDisplayEnabled(false, this);
    knobLoopSize.setPopupDisplayEnabled(false, this);
    knobLoopStart.addListener(this);
    knobLoopSize.addListener(this);
    
    //Tuning
    initKnob(knobCoarseTune, p.grainParamArray[3]);
    initKnob(knobFineTune, p.grainParamArray[4]);
    initKnob(knobChaos, p.grainParamArray[7]);
    initKnob(knobRepetitions, p.grainParamArray[8]);
    
    //Read Rate
    fixedRatioToggle.setButtonText("Fixed");
    fixedRatioToggle.setClickingTogglesState(true);
    fixedAttach = new AudioProcessorValueTreeState::ButtonAttachment(p.getValueTreeState(), p.grainParamArray[5], fixedRatioToggle);
    addAndMakeVisible(fixedRatioToggle);
    p.getValueTreeState().addParameterListener(p.grainParamArray[5], this);
    initKnob(knobReadRatio, p.grainParamArray[6]);
    ratioKnobEnable();
    
    //Amp Env
    initSlider(sliderGainEnvAtk, p.synthParamArray[0]);
    initSlider(sliderGainEnvHold, p.synthParamArray[1]);
    initSlider(sliderGainEnvDec, p.synthParamArray[2]);
    initSlider(sliderGainEnvSus, p.synthParamArray[3]);
    initSlider(sliderGainEnvRel, p.synthParamArray[4]);
    initKnob(knobGain, p.synthParamArray[5]);
    sliderGainEnvSus.setSkewFactorFromMidPoint(50.0);
    knobGain.setSkewFactorFromMidPoint(0.0);

    //HPF Env
    initKnob(knobHpCutoff, p.synthParamArray[6]);
    initKnob(knobHpReso, p.synthParamArray[7]);
    initSlider(sliderHpfEnvAtk, p.synthParamArray[8]);
    initSlider(sliderHpfEnvHold, p.synthParamArray[9]);
    initSlider(sliderHpfEnvDec, p.synthParamArray[10]);
    initSlider(sliderHpfEnvSus, p.synthParamArray[11]);
    initSlider(sliderHpfEnvRel, p.synthParamArray[12]);
    initSlider(sliderHpfDepth, p.synthParamArray[13]);
    knobHpCutoff.setSkewFactorFromMidPoint(632.46);
    knobHpReso.setSkewFactorFromMidPoint(1.34);
    sliderHpfEnvSus.setSkewFactorFromMidPoint(50.0);
    sliderHpfDepth.setSkewFactor(hzSkew, true);
    
    //LPF Env
    initKnob(knobLpCutoff, p.synthParamArray[14]);
    initKnob(knobLpReso, p.synthParamArray[15]);
    initSlider(sliderLpfEnvAtk, p.synthParamArray[16]);
    initSlider(sliderLpfEnvHold, p.synthParamArray[17]);
    initSlider(sliderLpfEnvDec, p.synthParamArray[18]);
    initSlider(sliderLpfEnvSus, p.synthParamArray[19]);
    initSlider(sliderLpfEnvRel, p.synthParamArray[20]);
    initSlider(sliderLpfDepth, p.synthParamArray[21]);
    knobLpCutoff.setSkewFactorFromMidPoint(632.46);
    knobLpReso.setSkewFactorFromMidPoint(1.34);
    sliderLpfEnvSus.setSkewFactorFromMidPoint(50.0);
    sliderLpfDepth.setSkewFactor(hzSkew, true);
    
    //ShimmerVerb
    initKnob(knobVerbDryWet, p.verbParamArray[0]);
    initKnob(knobVerbSize, p.verbParamArray[1]);
    initKnob(knobVerbDamp, p.verbParamArray[2]);
    initKnob(knobVerbWidth, p.verbParamArray[3]);
    
    //waveform thumbnail
    processor.getThumbnail()->addChangeListener(this);
    
    //set size at the end
    setSize(700, 300);
}

TextureSynthAudioProcessorEditor::~TextureSynthAudioProcessorEditor()
{
    for(int i = 0; i < numSliders; i++)
    {
        sliderAttachments[i] = nullptr;
    }
    processor.getValueTreeState().removeParameterListener(processor.grainParamArray[5], this);
    fixedAttach = nullptr;
}

//==============================================================================
void TextureSynthAudioProcessorEditor::paint (Graphics& g)
{
    Image bgImg = ImageCache::getFromHashCode(0);
    g.drawImage(bgImg, 0, 0, getWidth(), getHeight(), 0, 0, bgImg.getWidth(), bgImg.getHeight());
    
    //Thumbnail painting
    paintThumbnail(g, thumbnailBounds);
}

void TextureSynthAudioProcessorEditor::resized()
{
    //File load button
    fileLoadButton.setBounds(10, 205, 40, 20);
    
    //Loop controls
    knobLoopStart.setBounds(75, 205, 50, 50);
    knobLoopSize.setBounds(125, 205, 50, 50);
    knobGrainSize.setBounds(200, 205, 50, 50);
    
    //Tuning
    knobRepetitions.setBounds(330, 25, 50, 50);
    knobReadRatio.setBounds(330, 90, 50, 50);
    fixedRatioToggle.setBounds(330, 150, 50, 20);
    knobChaos.setBounds(330, 180, 50, 50);
    knobCoarseTune.setBounds(320, 245, 33, 33);
    knobFineTune.setBounds(355, 245, 33, 33);
    
    //Amp Env
    sliderGainEnvAtk.setBounds(425, 20, 25, 60);
    sliderGainEnvHold.setBounds(450, 20, 25, 60);
    sliderGainEnvDec.setBounds(475, 20, 25, 60);
    sliderGainEnvSus.setBounds(500, 20, 25, 60);
    sliderGainEnvRel.setBounds(525, 20, 25, 60);
    knobGain.setBounds(550, 20, 50, 60);
    
    //HPF Env
    sliderHpfEnvAtk.setBounds(425, 115, 25, 60);
    sliderHpfEnvHold.setBounds(450, 115, 25, 60);
    sliderHpfEnvDec.setBounds(475, 115, 25, 60);
    sliderHpfEnvSus.setBounds(500, 115, 25, 60);
    sliderHpfEnvRel.setBounds(525, 115, 25, 60);
    sliderHpfDepth.setBounds(550, 115, 25, 60);
    knobHpCutoff.setBounds(575, 120, 25, 25);
    knobHpReso.setBounds(575, 145, 25, 25);
    
    //LPF Env
    sliderLpfEnvAtk.setBounds(425, 210, 25, 60);
    sliderLpfEnvHold.setBounds(450, 210, 25, 60);
    sliderLpfEnvDec.setBounds(475, 210, 25, 60);
    sliderLpfEnvSus.setBounds(500, 210, 25, 60);
    sliderLpfEnvRel.setBounds(525, 210, 25, 60);
    sliderLpfDepth.setBounds(550, 210, 25, 60);
    knobLpCutoff.setBounds(575, 215, 25, 25);
    knobLpReso.setBounds(575, 240, 25, 25);
    
    
    //ShimmerVerb
    knobVerbSize.setBounds(625, 25, 50, 50);
    knobVerbDamp.setBounds(625, 90, 50, 50);
    knobVerbWidth.setBounds(625, 155, 50, 50);
    knobVerbDryWet.setBounds(625, 220, 50, 50);
}

//==============================================================================
void TextureSynthAudioProcessorEditor::changeListenerCallback(ChangeBroadcaster* source)
{
    if(source == processor.getThumbnail()) repaint();
}
void TextureSynthAudioProcessorEditor::sliderValueChanged (Slider *slider)
{
    if(slider == &knobLoopSize || slider == &knobLoopStart)
    {
        repaint();
    }
}

//==============================================================================
void TextureSynthAudioProcessorEditor::parameterChanged(const String &parameterID, float newValue)
{
    if(parameterID == "gfixtoggle")
    {
        ratioKnobEnable();
    }
}
void TextureSynthAudioProcessorEditor::ratioKnobEnable()
{
    Colour newColour;
    if(processor.getValueTreeState().getParameter("gfixtoggle")->getValue())
    {
        newColour = Colours::black;
    }
    else
    {
        newColour = Colours::lightgrey;
    }
    knobReadRatio.setColour(Slider::rotarySliderOutlineColourId, newColour);
    knobReadRatio.setColour(Slider::rotarySliderFillColourId, newColour);
}
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
void TextureSynthAudioProcessorEditor::initSlider(Slider &slider, const String parameterID)
{
    slider.setSliderStyle(Slider::SliderStyle::LinearVertical);
    slider.setSliderSnapsToMousePosition(false);
    slider.setMouseDragSensitivity(100);
    slider.setTextBoxStyle(Slider::NoTextBox, false, 90, 0);
    slider.setTextValueSuffix(processor.getValueTreeState().getParameter(parameterID)->getLabel());
    slider.setPopupDisplayEnabled(true, this);
    
    sliderAttachments[sliderAttachIndex] = new AudioProcessorValueTreeState::SliderAttachment(processor.getValueTreeState(), parameterID, slider);
    sliderAttachIndex++;
    
    slider.setSkewFactorFromMidPoint(1000.0);
    
    addAndMakeVisible(slider);
}

void TextureSynthAudioProcessorEditor::initKnob(Slider& knob, const String parameterID)
{
    knob.setSliderStyle(Slider::RotaryVerticalDrag);
    knob.setSliderSnapsToMousePosition(false);
    knob.setMouseDragSensitivity(100);
    knob.setRotaryParameters(float_Pi*5.0/4.0, float_Pi*11.0/4.0, true);
    knob.setColour(Slider::rotarySliderOutlineColourId, Colour(0,0,0));
    knob.setColour(Slider::rotarySliderFillColourId, Colour(0,0,0));
    knob.setTextBoxStyle(Slider::NoTextBox, false, 90, 0);
    knob.setTextValueSuffix(processor.getValueTreeState().getParameter(parameterID)->getLabel());
    knob.setPopupDisplayEnabled(true, this);
    
    sliderAttachments[sliderAttachIndex] = new AudioProcessorValueTreeState::SliderAttachment(processor.getValueTreeState(), parameterID, knob);
    sliderAttachIndex++;
    
    addAndMakeVisible(knob);
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
        
        //loop overlay
        uint8 full = 255;
        float alpha = 0.33;
        g.setColour(Colour(full, full, full, alpha));
        int overlayX = thumbnailBounds.getX() + thumbnailBounds.getWidth()*processor.getValueTreeState().getParameter(processor.grainParamArray[0])->getValue();
        int overlayWidth = thumbnailBounds.getWidth()*processor.getValueTreeState().getParameter(processor.grainParamArray[1])->getValue();
        if(overlayX + overlayWidth >= thumbnailBounds.getRight())
        {
            overlayWidth = thumbnailBounds.getRight() - overlayX;
            if(overlayWidth <= 0)
            {
                overlayWidth = 2;
                overlayX -= 2;
            }
        }
        if(overlayWidth <= 0)
        {
            overlayWidth = 2;
        }
        g.fillRect(overlayX, thumbnailBounds.getY(), overlayWidth, thumbnailBounds.getHeight());
    }
}