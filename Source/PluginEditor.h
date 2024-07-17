/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "GUI/kLookAndFeel.h"
#include "Utility/KiTiK_utilityViz.h"

//==============================================================================
/**
*/
class DisburserAudioProcessorEditor : public juce::AudioProcessorEditor, public juce::Timer
{
public:
    DisburserAudioProcessorEditor (DisburserAudioProcessor&);
    ~DisburserAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;
    void timerCallback() override;

private:

    DisburserAudioProcessor& audioProcessor;

    Laf lnf;

    juce::URL url{ "https://kwhaley5.gumroad.com/" };
    juce::HyperlinkButton gumroad{ "More Plugins", url };

    FFTComp fftComp;

    juce::Slider scatter, cutoff, smash;
    juce::AudioProcessorValueTreeState::SliderAttachment scatterAT, cutoffAT, smashAT;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DisburserAudioProcessorEditor)
};
