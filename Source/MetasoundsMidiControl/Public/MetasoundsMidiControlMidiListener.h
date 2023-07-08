// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "MetasoundsMidiControlMidiListener.generated.h"

class UMIDIDeviceInputController;
class FMetasoundsMidiControlModule;

/**
 * 
 */
UCLASS()
class METASOUNDSMIDICONTROL_API UMetasoundsMidiControlMidiListener : public UObject
{
	GENERATED_BODY()

public:
	void ListenToMIDIDevice(FMetasoundsMidiControlModule* Module, UMIDIDeviceInputController* MIDIDevice);

	UMIDIDeviceInputController* GetInputController() const { return InputController; }

protected:
	UMIDIDeviceInputController* InputController;
	FMetasoundsMidiControlModule* OwningModule;

	UFUNCTION()
		void OnNoteDown(UMIDIDeviceInputController* MIDIDeviceController, int32 Timestamp, int32 Channel, int32 Note, int32 Velocity);

	UFUNCTION()
		void OnNoteUp(UMIDIDeviceInputController* MIDIDeviceController, int32 Timestamp, int32 Channel, int32 Note, int32 Velocity);

	UFUNCTION()
		void OnMidiControlChange(class UMIDIDeviceInputController* MIDIDeviceController, int32 Timestamp, int32 Channel, int32 ControlID, int32 Velocity);
};
