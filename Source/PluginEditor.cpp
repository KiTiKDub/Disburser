/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
DisburserAudioProcessorEditor::DisburserAudioProcessorEditor (DisburserAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p),
    cutoffAT(p.apvts, "cutoff", cutoff), fftComp(p.fftData)
{
    
    setLookAndFeel(&lnf);
    updateRSWL();

    cutoff.setSliderStyle(juce::Slider::SliderStyle::LinearBar);

    cutoff.setTextBoxStyle(juce::Slider::NoTextBox, false, 100, 20);

    addAndMakeVisible(fftComp);
    addAndMakeVisible(*scatter);
    addAndMakeVisible(*smash);
    addAndMakeVisible(cutoff);
    addAndMakeVisible(gumroad);
    
    setSize (600, 150);
    startTimerHz(24);
}

DisburserAudioProcessorEditor::~DisburserAudioProcessorEditor()
{
    setLookAndFeel(nullptr);
}

//==============================================================================
void DisburserAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colours::black);

    g.setColour (juce::Colours::white);

    auto bounds = getLocalBounds();
    auto top = bounds.removeFromTop(bounds.getHeight() * .25);
    auto logoArea = top;
    logoArea.removeFromRight(logoArea.getWidth() * .9);
    logoArea.expand(logoArea.getWidth() * .2, logoArea.getHeight() * .2);

    auto logo = juce::ImageCache::getFromMemory(BinaryData::KITIK_LOGO_NO_BKGD_png, BinaryData::KITIK_LOGO_NO_BKGD_pngSize);
    g.drawImage(logo, logoArea.toFloat(), juce::RectanglePlacement::centred);

    auto newFont = juce::Font(juce::Typeface::createSystemTypefaceFor(BinaryData::offshore_ttf, BinaryData::offshore_ttfSize));
    g.setFont(newFont);
    g.setFont (top.getHeight() * .95);

    g.drawFittedText("Disburser", top.toNearestInt(), juce::Justification::Justification::centred, 1);
    g.drawHorizontalLine(top.getBottom() + 5, bounds.getX(), bounds.getWidth());
}

void DisburserAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds();
    auto top = bounds.removeFromTop(bounds.getHeight() * .25);

    auto leftKnob = bounds.removeFromLeft(bounds.getWidth() * .15);
    auto rLeftKnob = leftKnob.reduced(leftKnob.getWidth() * .05, 0);
    auto rightKnob = bounds.removeFromRight(bounds.getWidth() * .175);
    auto rRightKnob = rightKnob.reduced(rightKnob.getWidth() * .05, 0);
    auto middle = bounds.reduced(bounds.getWidth() * .025, 0);
    middle.removeFromTop(middle.getHeight() * .25);
    middle.removeFromBottom(middle.getHeight() * .33);

    fftComp.setBounds(middle);
    scatter->setBounds(rLeftKnob);
    cutoff.setBounds(middle);
    smash->setBounds(rRightKnob);

    auto linkSpace = top.removeFromRight(top.getWidth() * .15);
    auto font = juce::Font();
    gumroad.setFont(font, false, juce::Justification::centred);
    gumroad.setColour(0x1001f00, juce::Colours::white);
    gumroad.setBounds(linkSpace);
}

void DisburserAudioProcessorEditor::timerCallback()
{
    fftComp.repaint();
}

void DisburserAudioProcessorEditor::updateRSWL()
{
    auto& scatterParam = getParam(audioProcessor.apvts, "scatter");
    auto& smashParam = getParam(audioProcessor.apvts, "smash");

    scatter = std::make_unique<RotarySliderWithLabels>(&scatterParam, "", "Scatter");
    smash = std::make_unique<RotarySliderWithLabels>(&smashParam, "", "Smash");

    makeAttachment(scatterAT, audioProcessor.apvts, "scatter", *scatter);
    makeAttachment(smashAT, audioProcessor.apvts, "smash", *smash);

    addLabelPairs(scatter->labels, 1, 3, scatterParam, "");
    addLabelPairs(smash->labels, 1, 3, smashParam, "");

    scatter.get()->onValueChange = [this, &scatterParam]()
        {
            addLabelPairs(scatter->labels, 1, 3, scatterParam, "");
        };

    smash.get()->onValueChange = [this, &smashParam]()
        {
            addLabelPairs(smash->labels, 1, 3, smashParam, "");
        };
}
