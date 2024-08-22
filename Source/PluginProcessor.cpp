/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
DisburserAudioProcessor::DisburserAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
    scatter = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("scatter"));
    cutoff = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("cutoff"));
    smash = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("smash"));
}

DisburserAudioProcessor::~DisburserAudioProcessor()
{
}

//==============================================================================
const juce::String DisburserAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool DisburserAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool DisburserAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool DisburserAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double DisburserAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int DisburserAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int DisburserAudioProcessor::getCurrentProgram()
{
    return 0;
}

void DisburserAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String DisburserAudioProcessor::getProgramName (int index)
{
    return {};
}

void DisburserAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void DisburserAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    juce::dsp::ProcessSpec spec;
    spec.maximumBlockSize = samplesPerBlock;
    spec.sampleRate = sampleRate;
    spec.numChannels = getTotalNumInputChannels();

    for (auto& all : allpasses)
    {
        all.prepare(spec);
    }

    fftData.prepare(sampleRate);
}

void DisburserAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool DisburserAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void DisburserAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    auto block = juce::dsp::AudioBlock<float>(buffer);
    auto* dataLeft = block.getChannelPointer(0);
    auto* dataRight = block.getChannelPointer(1);

    auto coef = juce::dsp::IIR::Coefficients<float>::makeAllPass(getSampleRate(), cutoff->get(), smash->get());
    auto scatterValue = scatter->get();

    //Prevent lots of popping when moving the amount button
    scatterValues.push_back(scatterValue);
    if (scatterValues.size() > 60)
    {
        scatterValues.erase(scatterValues.begin());
    }

    auto scatterSize = scatterValues.size();

    for (int i = 0; i < scatterSize; i++)
    {
        avgValue += scatterValues[i];
    }

    if((avgValue / scatterSize) == scatterValue)
    {
        for (int s = 0; s < block.getNumSamples(); s++)
        {
            auto left = dataLeft[s];
            auto right = dataRight[s];
            auto allpass = allpasses.data();

            for (int filter = 0; filter < scatterValue; filter += 2)
            {
                if (s % 100 == 0)
                {
                    allpass[filter].coefficients = coef;
                    allpass[filter + 1].coefficients = coef;
                }
                left = allpass[filter].processSample(left);
                right = allpass[filter + 1].processSample(right);
            }

            dataLeft[s] = left;
            dataRight[s] = right;

        }
    }

    fftData.pushNextSampleIntoFifo(buffer);
    avgValue = 0;
}

//==============================================================================
bool DisburserAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* DisburserAudioProcessor::createEditor()
{
    return new DisburserAudioProcessorEditor (*this);
}

//==============================================================================
void DisburserAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    juce::MemoryOutputStream mos(destData, true);
    apvts.state.writeToStream(mos);
}

void DisburserAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    auto tree = juce::ValueTree::readFromData(data, sizeInBytes);
    if (tree.isValid()) 
        apvts.replaceState(tree);
}

juce::AudioProcessorValueTreeState::ParameterLayout DisburserAudioProcessor::createParameterLayout()
{
    using namespace juce;

    AudioProcessorValueTreeState::ParameterLayout layout;

    auto scatterRange = NormalisableRange<float>(0, 64, 2, 1);
    auto cutoffRange = makeLogarithmicRange(20.f, 20000.f);
    auto smashRange = NormalisableRange<float>(.71, 10, .1, 1);

    layout.add(std::make_unique<AudioParameterFloat>(juce::ParameterID{"scatter",1}, "Scatter", scatterRange, 0));
    layout.add(std::make_unique<AudioParameterFloat>(juce::ParameterID{"cutoff",1}, "Cutoff", cutoffRange, 200));
    layout.add(std::make_unique<AudioParameterFloat>(juce::ParameterID{"smash",1}, "Smash", smashRange, .71));

    return layout;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new DisburserAudioProcessor();
}
