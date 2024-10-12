/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "BinaryData.h"
#include "PluginProcessor.h"
#include "GUI/kLookAndFeel.h"
#include "Utility/KiTiK_utilityViz.h"
#include "GUI/rotarySliderWithLabels.h"

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

    void updateRSWL();

    DisburserAudioProcessor& audioProcessor;

    Laf lnf;

    juce::URL url{ "https://kwhaley5.gumroad.com/" };
    juce::HyperlinkButton gumroad{ "More Plugins", url };

    FFTComp fftComp;

    juce::Slider cutoff;
    std::unique_ptr<RotarySliderWithLabels> scatter, smash;

    juce::AudioProcessorValueTreeState::SliderAttachment cutoffAT;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> scatterAT, smashAT;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DisburserAudioProcessorEditor)
};
