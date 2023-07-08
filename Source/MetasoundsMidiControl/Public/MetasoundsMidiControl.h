// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"
#include "Containers/Ticker.h"

class UMIDIDeviceInputController;
class UMetasoundsMidiControlMidiListener;
class UAudioComponent;
class UMetaSoundSource;
class USoundCue; 

struct FMetasoundsMidiControlVoice
{
	int32 VoiceNumber;
	UAudioComponent* AudioComponent;
	USoundCue* SoundCue;
	bool bIsInUse;
	float LastNoteDownTime;
	float LastNoteUpTime;
	int32 CurrentNote;
};

class FMetasoundsMidiControlModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	void OnNoteDown(UMIDIDeviceInputController* MIDIDeviceController, int32 Timestamp, int32 Channel, int32 Note, int32 Velocity);
	void OnNoteUp(UMIDIDeviceInputController* MIDIDeviceController, int32 Timestamp, int32 Channel, int32 Note, int32 Velocity);
	void OnMidiControlChange(UMIDIDeviceInputController* MIDIDeviceController, int32 Timestamp, int32 Channel, int32 Note, int32 Velocity);

	void RecreateVoices();
	void ReloadMidiDevices();
	void SetEnabled(bool bEnabled);

protected:
	UMetaSoundSource* MetasoundAsset;

	FDelegateHandle AssetEditorOpenHandle;
	void OnAssetEditorOpened(UObject* AssetEditor);

	void CreateVoices(UMetaSoundSource* MetasoundSource);
	void DestroyVoices();

	TMap<int32, UMIDIDeviceInputController*> MIDIInputControllers;
	TArray<UMetasoundsMidiControlMidiListener*> MidiListeners;

	TArray<FMetasoundsMidiControlVoice> Voices;

	void OnObjectPreSave(UObject* Object, FObjectPreSaveContext SaveContext);

	bool bUpdateMetasound = false;;
	FTickerDelegate TickDelegate;
	FTSTicker::FDelegateHandle TickDelegateHandle;
	bool Tick(float DeltaTime);
};

DECLARE_LOG_CATEGORY_EXTERN(LogMetasoundsMidiControl, Log, All);
