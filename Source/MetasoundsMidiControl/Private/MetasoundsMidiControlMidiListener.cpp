// Fill out your copyright notice in the Description page of Project Settings.


#include "MetasoundsMidiControlMidiListener.h"

#include "MIDIDeviceInputController.h"
#include "Editor.h"
#include "Components/AudioComponent.h"
#include "MetasoundsMidiControlSettings.h"
#include "MetasoundsMidiControl.h"

void UMetasoundsMidiControlMidiListener::ListenToMIDIDevice(FMetasoundsMidiControlModule* Module, UMIDIDeviceInputController* MIDIDevice)
{
	check(Module);
	OwningModule = Module;

	check(MIDIDevice);
	InputController = MIDIDevice;

	InputController->OnMIDINoteOn.AddDynamic(this, &UMetasoundsMidiControlMidiListener::OnNoteDown);
	InputController->OnMIDINoteOff.AddDynamic(this, &UMetasoundsMidiControlMidiListener::OnNoteUp);
	InputController->OnMIDIControlChange.AddDynamic(this, &UMetasoundsMidiControlMidiListener::OnMidiControlChange);
}

void UMetasoundsMidiControlMidiListener::OnNoteDown(UMIDIDeviceInputController* MIDIDeviceController, int32 Timestamp, int32 Channel, int32 Note, int32 Velocity)
{
	OwningModule->OnNoteDown(MIDIDeviceController, Timestamp, Channel, Note, Velocity);
}

void UMetasoundsMidiControlMidiListener::OnNoteUp(UMIDIDeviceInputController* MIDIDeviceController, int32 Timestamp, int32 Channel, int32 Note, int32 Velocity)
{
	OwningModule->OnNoteUp(MIDIDeviceController, Timestamp, Channel, Note, Velocity);
}

void UMetasoundsMidiControlMidiListener::OnMidiControlChange(UMIDIDeviceInputController* MIDIDeviceController, int32 Timestamp, int32 Channel, int32 ControlID, int32 Velocity)
{
	OwningModule->OnMidiControlChange(MIDIDeviceController, Timestamp, Channel, ControlID, Velocity);
}
