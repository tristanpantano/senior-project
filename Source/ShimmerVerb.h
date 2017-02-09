//
//  ShimmerVerb.h
//  TextureSynth
//
//  Created by Tristan Pantano on 2/7/17.
//
//

#ifndef ShimmerVerb_h
#define ShimmerVerb_h

#include "../JuceLibraryCode/JuceHeader.h"

class ShimmerVerb : public AudioProcessorValueTreeState::Listener
{
public:
    ShimmerVerb();
    
    void parameterChanged(const String &parameterID, float newValue) override;
    
    void prepareToPlay(double sampleRate, int samplesPerBlock);
    void renderNextBlock (AudioSampleBuffer& outputBuffer, int startSample, int numSamples);
    
    
private:
    Reverb reverb;
    Reverb::Parameters params;
};


#endif /* ShimmerVerb_h */
