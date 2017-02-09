//
//  ShimmerVerb.cpp
//  TextureSynth
//
//  Created by Tristan Pantano on 2/7/17.
//
//

#include "ShimmerVerb.h"

ShimmerVerb::ShimmerVerb(){}
void ShimmerVerb::parameterChanged(const String &parameterID, float newValue)
{
    if(parameterID == "rdrywet")
    {
        params.wetLevel = newValue/100.0;
        params.dryLevel = 1.0 - newValue/100.0;
    }
    else if(parameterID == "rsize")
    {
        params.roomSize = newValue/100.0;
    }
    else if(parameterID == "rdamping")
    {
        params.damping = newValue/100.0;
    }
    else if(parameterID == "rwidth")
    {
        params.width = newValue/100.0;
    }
    
    reverb.setParameters(params);
}

void ShimmerVerb::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    reverb.reset();
    reverb.setSampleRate(sampleRate);
    reverb.setParameters(params);
}
void ShimmerVerb::renderNextBlock (AudioSampleBuffer& outputBuffer, int startSample, int numSamples)
{
    float* left = outputBuffer.getWritePointer(0);
    float* right = outputBuffer.getWritePointer(1);
    reverb.processStereo(left, right, numSamples);
}