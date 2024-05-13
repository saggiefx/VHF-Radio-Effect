/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
VHFRadioAudioProcessorEditor::VHFRadioAudioProcessorEditor (VHFRadioAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (400, 274);
}

VHFRadioAudioProcessorEditor::~VHFRadioAudioProcessorEditor()
{
}

//==============================================================================
void VHFRadioAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.drawImage(ImageCache::getFromMemory(BinaryData::vhfradio_jpg, BinaryData::vhfradio_jpgSize),getLocalBounds().toFloat());

}

void VHFRadioAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
}
