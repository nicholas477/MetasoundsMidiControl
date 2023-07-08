// Copyright Epic Games, Inc. All Rights Reserved.

#include "MetasoundsMidiControl.h"
#include "Editor.h"
#include "Subsystems/AssetEditorSubsystem.h"
#include "MetasoundSource.h"
#include "MIDIDeviceManager.h"
#include "MIDIDeviceInputController.h"
#include "MetasoundsMidiControlMidiListener.h"
#include "ISettingsModule.h"
#include "MetasoundsMidiControlSettings.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "AudioDevice.h"

DEFINE_LOG_CATEGORY(LogMetasoundsMidiControl);

#define LOCTEXT_NAMESPACE "FMetasoundsMidiControlModule"

void FMetasoundsMidiControlModule::StartupModule()
{
	UAssetEditorSubsystem* AssetEditorSubsystem = GEditor->GetEditorSubsystem<UAssetEditorSubsystem>();
	check(AssetEditorSubsystem);

	AssetEditorOpenHandle = AssetEditorSubsystem->OnAssetEditorOpened().AddRaw(this, &FMetasoundsMidiControlModule::OnAssetEditorOpened);
	FCoreUObjectDelegates::OnObjectPreSave.AddRaw(this, &FMetasoundsMidiControlModule::OnObjectPreSave);

	ISettingsModule& SettingsModule = FModuleManager::LoadModuleChecked<ISettingsModule>("Settings");
	SettingsModule.RegisterSettings(
		"Editor",
		"Plugins",
		"Metasounds Fuckery",
		INVTEXT("Metasounds Fuckery"),
		INVTEXT("Settings related to the Metasounds Fuckery plugin."),
		GetMutableDefault<UMetasoundsMidiControlSettings>());

	TickDelegate = FTickerDelegate::CreateRaw(this, &FMetasoundsMidiControlModule::Tick);
	TickDelegateHandle = FTSTicker::GetCoreTicker().AddTicker(TickDelegate);
}

void FMetasoundsMidiControlModule::ShutdownModule()
{
	if (AssetEditorOpenHandle.IsValid())
	{
		AssetEditorOpenHandle.Reset();
	}

	FTSTicker::GetCoreTicker().RemoveTicker(TickDelegateHandle);
}

static TArray<FMetasoundsMidiControlVoice> GetUnusedVoices(const TArray<FMetasoundsMidiControlVoice>& Voices)
{
	return Voices.FilterByPredicate([](const FMetasoundsMidiControlVoice& Voice) -> bool
	{
		return !Voice.bIsInUse;
	});
}

// The front of the array will have the oldest voice
static void SortByAgeDescending(TArray<FMetasoundsMidiControlVoice>& Voices)
{
	Voices.Sort([](const FMetasoundsMidiControlVoice& LHS, const FMetasoundsMidiControlVoice& RHS) -> bool
	{
		return LHS.LastNoteUpTime < RHS.LastNoteUpTime;
	});
}

void FMetasoundsMidiControlModule::OnNoteDown(UMIDIDeviceInputController* MIDIDeviceController, int32 Timestamp, int32 Channel, int32 Note, int32 Velocity)
{
	if (!GetMutableDefault<UMetasoundsMidiControlSettings>()->bIsEnabled)
	{
		return;
	}

	const EMetasoundsVoiceMode VoiceMode = GetMutableDefault<UMetasoundsMidiControlSettings>()->VoiceMode;
	if (VoiceMode == EMetasoundsVoiceMode::EMultiVoiceMode)
	{
		FMetasoundsMidiControlVoice* VoiceToUse = nullptr;
		// if we are in multivoice mode then look for an unused voice
		TArray<FMetasoundsMidiControlVoice> UnusedVoices = GetUnusedVoices(Voices);

		// If there are some unused voices
		if (UnusedVoices.Num() > 0)
		{
			SortByAgeDescending(UnusedVoices);
			for (FMetasoundsMidiControlVoice& Voice : Voices)
			{
				if (Voice.VoiceNumber == UnusedVoices[0].VoiceNumber)
				{
					VoiceToUse = &Voice;
					break;
				}
			}
		}
		else
		{
			// Otherwise just look for the oldest note
			SortByAgeDescending(Voices);
			VoiceToUse = &Voices[0];
		}

		VoiceToUse->bIsInUse = true;
		VoiceToUse->LastNoteDownTime = FPlatformTime::Seconds();
		VoiceToUse->CurrentNote = Note;
		VoiceToUse->AudioComponent->SetTriggerParameter(GetMutableDefault<UMetasoundsMidiControlSettings>()->NoteDownTriggerParameterName);
		VoiceToUse->AudioComponent->SetFloatParameter(GetMutableDefault<UMetasoundsMidiControlSettings>()->NoteVelocityParameterName, Velocity);
		VoiceToUse->AudioComponent->SetFloatParameter(GetMutableDefault<UMetasoundsMidiControlSettings>()->NotePitchParameterName, Note);
	}
	else if (VoiceMode == EMetasoundsVoiceMode::ESingleVoiceMode)
	{
		UAudioComponent* Voice = GEditor->GetPreviewAudioComponent();
		if (Voice)
		{
			Voice->SetTriggerParameter(GetMutableDefault<UMetasoundsMidiControlSettings>()->NoteDownTriggerParameterName);
			Voice->SetFloatParameter(GetMutableDefault<UMetasoundsMidiControlSettings>()->NoteVelocityParameterName, Velocity);
			Voice->SetFloatParameter(GetMutableDefault<UMetasoundsMidiControlSettings>()->NotePitchParameterName, Note);
		}
	}
}

void FMetasoundsMidiControlModule::OnNoteUp(UMIDIDeviceInputController* MIDIDeviceController, int32 Timestamp, int32 Channel, int32 Note, int32 Velocity)
{
	if (!GetMutableDefault<UMetasoundsMidiControlSettings>()->bIsEnabled)
	{
		return;
	}

	const EMetasoundsVoiceMode VoiceMode = GetMutableDefault<UMetasoundsMidiControlSettings>()->VoiceMode;
	if (VoiceMode == EMetasoundsVoiceMode::EMultiVoiceMode)
	{
		for (FMetasoundsMidiControlVoice& Voice : Voices)
		{
			if (Voice.bIsInUse && Voice.CurrentNote == Note)
			{
				Voice.bIsInUse = false;
				Voice.LastNoteUpTime = FPlatformTime::Seconds();
				Voice.AudioComponent->SetTriggerParameter(GetMutableDefault<UMetasoundsMidiControlSettings>()->NoteUpTriggerParameterName);
				//return;
			}
		}
	}
	else if (VoiceMode == EMetasoundsVoiceMode::ESingleVoiceMode)
	{
		UAudioComponent* Voice = GEditor->GetPreviewAudioComponent();
		if (Voice)
		{
			Voice->SetTriggerParameter(GetMutableDefault<UMetasoundsMidiControlSettings>()->NoteUpTriggerParameterName);
		}
	}
}

void FMetasoundsMidiControlModule::OnMidiControlChange(UMIDIDeviceInputController* MIDIDeviceController, int32 Timestamp, int32 Channel, int32 Note, int32 Velocity)
{
	if (auto Settings = GetMutableDefault<UMetasoundsMidiControlSettings>())
	{
		const EMetasoundsVoiceMode VoiceMode = Settings->VoiceMode;
		if (auto MidiParams = Settings->MidiControlParameterNames.Find(Note))
		{
			if (VoiceMode == EMetasoundsVoiceMode::EMultiVoiceMode)
			{
				for (FMetasoundsMidiControlVoice& Voice : Voices)
				{
					if (MidiParams->NoteParameterName != NAME_None)
					{
						Voice.AudioComponent->SetFloatParameter(MidiParams->NoteParameterName, Note);
					}

					if (MidiParams->VelocityParameterName != NAME_None)
					{
						Voice.AudioComponent->SetFloatParameter(MidiParams->VelocityParameterName, Velocity);
					}
				}
			}
			else if (VoiceMode == EMetasoundsVoiceMode::ESingleVoiceMode)
			{
				UAudioComponent* Voice = GEditor->GetPreviewAudioComponent();
				if (Voice)
				{
					if (MidiParams->NoteParameterName != NAME_None)
					{
						Voice->SetFloatParameter(MidiParams->NoteParameterName, Note);
					}

					if (MidiParams->VelocityParameterName != NAME_None)
					{
						Voice->SetFloatParameter(MidiParams->VelocityParameterName, Velocity);
					}
				}
			}
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Unable to find MIDI control change mapping for note: %d"), Note);
		}
	}
}

void FMetasoundsMidiControlModule::OnAssetEditorOpened(UObject* Asset)
{
	if (!GetMutableDefault<UMetasoundsMidiControlSettings>()->bIsEnabled)
	{
		return;
	}

	if (UMetaSoundSource* NewMetasoundAsset = Cast<UMetaSoundSource>(Asset))
	{
		MetasoundAsset = NewMetasoundAsset;
		ReloadMidiDevices();
		CreateVoices(MetasoundAsset);
	}
}

void FMetasoundsMidiControlModule::ReloadMidiDevices()
{
	TArray<FFoundMIDIDevice> MIDIDevices;
	UMIDIDeviceManager::FindMIDIDevices(MIDIDevices);

	for (FFoundMIDIDevice& MIDIDevice : MIDIDevices)
	{
		if (MIDIDevice.bCanReceiveFrom)
		{
			if (!MIDIInputControllers.Contains(MIDIDevice.DeviceID))
			{
				const int32 BufferSize = GetMutableDefault<UMetasoundsMidiControlSettings>()->MidiBufferSize;
				UMIDIDeviceInputController* NewInputController = UMIDIDeviceManager::CreateMIDIDeviceInputController(MIDIDevice.DeviceID, BufferSize);
				if (NewInputController)
				{
					NewInputController->AddToRoot();
					MIDIInputControllers.Add(MIDIDevice.DeviceID, NewInputController);

					UMetasoundsMidiControlMidiListener* NewMidiListener = NewObject<UMetasoundsMidiControlMidiListener>();
					NewMidiListener->AddToRoot();
					MidiListeners.Add(NewMidiListener);
					NewMidiListener->ListenToMIDIDevice(this, NewInputController);

					UE_LOG(LogMetasoundsMidiControl, Log, TEXT("Added new midi device! ID: %d, Name: %s"), MIDIDevice.DeviceID, *MIDIDevice.DeviceName);
				}
				else
				{
					UE_LOG(LogMetasoundsMidiControl, Warning, TEXT("Unable to add midi device ID: %d, Name: %s"), MIDIDevice.DeviceID, *MIDIDevice.DeviceName);
				}
			}
		}
	}
}

void FMetasoundsMidiControlModule::SetEnabled(bool bEnabled)
{
	if (bEnabled)
	{
		RecreateVoices();
	}
	else
	{
		DestroyVoices();
	}
}

void FMetasoundsMidiControlModule::RecreateVoices()
{
	if (IsValid(MetasoundAsset))
	{
		UE_LOG(LogMetasoundsMidiControl, Log, TEXT("Recreating metasounds voices..."));
		CreateVoices(MetasoundAsset);
	}
}

void FMetasoundsMidiControlModule::CreateVoices(UMetaSoundSource* MetasoundSource)
{
	DestroyVoices();

	if (!GetMutableDefault<UMetasoundsMidiControlSettings>()->bIsEnabled)
	{
		return;
	}

	for (int i = 0; i < GetMutableDefault<UMetasoundsMidiControlSettings>()->NumVoices; ++i)
	{
		USoundCue* NewSoundCue = NewObject<USoundCue>();
		check(NewSoundCue);
		NewSoundCue->AddToRoot();

		// Set world to NULL as it will most likely become invalid in the next PIE/Simulate session and the
		// component will be left with invalid pointer.
		UAudioComponent* NewVoice = FAudioDevice::CreateComponent(NewSoundCue);
		check(NewVoice);
		NewVoice->bAutoDestroy = false;
		NewVoice->bIsUISound = true;
		NewVoice->bAllowSpatialization = false;
		NewVoice->bReverb = false;
		NewVoice->bCenterChannelOnly = false;
		NewVoice->bIsPreviewSound = true;
		NewVoice->bPreviewComponent = true;
		NewVoice->Sound = MetasoundSource;

		NewVoice->Play();

		NewVoice->AddToRoot();
		Voices.Add({ i, NewVoice, NewSoundCue, false, 0.f, 0.f, 0 });
	}
}

void FMetasoundsMidiControlModule::DestroyVoices()
{
	for (FMetasoundsMidiControlVoice& Voice : Voices)
	{
		Voice.AudioComponent->Stop();
		Voice.AudioComponent->RemoveFromRoot();

		Voice.SoundCue->RemoveFromRoot();
	}
	Voices.Empty();
}

void FMetasoundsMidiControlModule::OnObjectPreSave(UObject* Object, FObjectPreSaveContext SaveContext)
{
	if (!GetMutableDefault<UMetasoundsMidiControlSettings>()->bIsEnabled)
	{
		return;
	}

	if (Object == MetasoundAsset && ((SaveContext.GetSaveFlags() & ESaveFlags::SAVE_FromAutosave) == 0))
	{
		bUpdateMetasound = true;
	}
}

bool FMetasoundsMidiControlModule::Tick(float DeltaTime)
{
	if (!GetMutableDefault<UMetasoundsMidiControlSettings>()->bIsEnabled)
	{
		return true;
	}

	if (bUpdateMetasound)
	{
		UE_LOG(LogMetasoundsMidiControl, Log, TEXT("Reloading Metasound asset..."));
		ReloadMidiDevices();
		CreateVoices(MetasoundAsset);
		bUpdateMetasound = false;
	}
	return true;
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FMetasoundsMidiControlModule, MetasoundsMidiControl)