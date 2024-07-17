/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "Utility/KiTiK_utilityViz.h"

//==============================================================================
/**
*/
class DisburserAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    DisburserAudioProcessor();
    ~DisburserAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    juce::AudioProcessorValueTreeState apvts{ *this, nullptr, "parameters", createParameterLayout() };

    FFTData fftData;

private:

    int avgValue{ 0 };
    std::vector<int> scatterValues;

    std::array<juce::dsp::IIR::Filter<float>, 64> allpasses;

    juce::AudioParameterFloat* scatter{ nullptr };
    juce::AudioParameterFloat* cutoff{ nullptr };
    juce::AudioParameterFloat* smash{ nullptr };
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DisburserAudioProcessor)
};

template<typename FloatType>
static inline juce::NormalisableRange<FloatType> makeLogarithmicRange(FloatType min, FloatType max)
{
    return juce::NormalisableRange<FloatType>
        (
            min, max,
            [](FloatType start, FloatType end, FloatType normalised)
            {
                return start + (std::pow(FloatType(2), normalised * FloatType(10)) - FloatType(1)) * (end - start) / FloatType(1023);
            },
            [](FloatType start, FloatType end, FloatType value)
            {
                return (std::log(((value - start) * FloatType(1023) / (end - start)) + FloatType(1)) / std::log(FloatType(2))) / FloatType(10);
            },
            [](FloatType start, FloatType end, FloatType value)
            {
                // optimised for frequencies: >3 kHz: 2 decimals
                if (value > FloatType(3000))
                    return juce::jlimit(start, end, FloatType(100) * juce::roundToInt(value / FloatType(100)));

                // optimised for frequencies: 1-3 kHz: 1 decimal
                if (value > FloatType(1000))
                    return juce::jlimit(start, end, FloatType(10) * juce::roundToInt(value / FloatType(10)));

                return juce::jlimit(start, end, FloatType(juce::roundToInt(value)));
            });
}
