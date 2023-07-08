// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "MetasoundsMidiControlSettings.generated.h"

UENUM(BlueprintType)
enum class EMetasoundsVoiceMode : uint8
{
	ESingleVoiceMode UMETA(DisplayName="Single Voice Mode"),
	EMultiVoiceMode UMETA(DisplayName = "Multi Voice Mode"),
	EMultiVoiceCustomMode UMETA(DisplayName = "Multi Voice Mode, Custom Setup", ToolTip="In this mode, the voices are fed to the metasound as separate triggers. For example, if there are 5 voices, then the triggers are OnNoteDown0 - OnNoteDown4. NOTE: THIS IS CURRENTLY UNIMPLEMENTED")
};

USTRUCT(BlueprintType)
struct FMidiControlParameterNames
{
	GENERATED_BODY()
		
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Midi Control Parameter Names")
		FName NoteParameterName;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Midi Control Parameter Names")
		FName VelocityParameterName;
};

/**
 * 
 */
UCLASS(config = EditorPerProjectUserSettings)
class METASOUNDSMIDICONTROL_API UMetasoundsMidiControlSettings : public UObject
{
	GENERATED_BODY()
	
public:
	// This plugin automatically creates and plays whatever metasound you have open, so if you have 32 voices it will play your metasound 32 times simultaneously. Therefore, you should probably leave this disabled
	UPROPERTY(EditAnywhere, Config, Category = "Config")
		bool bIsEnabled = false;

	UPROPERTY(EditAnywhere, Config, Category = "Config")
		FName NoteDownTriggerParameterName = "OnNoteDown";

	UPROPERTY(EditAnywhere, Config, Category = "Config")
		FName NoteUpTriggerParameterName = "OnNoteUp";

	UPROPERTY(EditAnywhere, Config, Category = "Config")
		FName NoteVelocityParameterName = "Velocity";

	UPROPERTY(EditAnywhere, Config, Category = "Config")
		FName NotePitchParameterName = "Note";

	UPROPERTY(EditAnywhere, Config, Category = "Config")
		TMap<int32, FMidiControlParameterNames> MidiControlParameterNames;

	UPROPERTY(EditAnywhere, Config, Category = "Config", meta=(ClampMin=1, ClampMax=32, EditCondition="VoiceMode != EMetasoundsVoiceMode::ESingleVoiceMode"))
		int32 NumVoices = 10;

	// The size of the midi receive buffer (in bytes)
	UPROPERTY(EditAnywhere, Config, Category = "Config", meta = (ClampMin = 1, ConfigRestartRequired = true))
		int32 MidiBufferSize = 1024;

	UPROPERTY(EditAnywhere, Config, Category = "Config")
		EMetasoundsVoiceMode VoiceMode = EMetasoundsVoiceMode::EMultiVoiceMode;

	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
};
