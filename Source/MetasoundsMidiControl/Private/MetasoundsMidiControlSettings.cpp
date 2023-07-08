// Fill out your copyright notice in the Description page of Project Settings.


#include "MetasoundsMidiControlSettings.h"

#include "Modules/ModuleManager.h"
#include "MetasoundsMidiControl.h"

void UMetasoundsMidiControlSettings::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	const FName PropertyName = (PropertyChangedEvent.Property != nullptr) ? PropertyChangedEvent.Property->GetFName() : NAME_None;

	if (PropertyName == GET_MEMBER_NAME_CHECKED(UMetasoundsMidiControlSettings, bIsEnabled))
	{
		FModuleManager::GetModuleChecked<FMetasoundsMidiControlModule>(TEXT("MetasoundsMidiControl")).SetEnabled(bIsEnabled);
	}

	if (PropertyName == GET_MEMBER_NAME_CHECKED(UMetasoundsMidiControlSettings, MidiBufferSize))
	{
		FModuleManager::GetModuleChecked<FMetasoundsMidiControlModule>(TEXT("MetasoundsMidiControl")).ReloadMidiDevices();
	}

	if (PropertyName == GET_MEMBER_NAME_CHECKED(UMetasoundsMidiControlSettings, NumVoices))
	{
		FModuleManager::GetModuleChecked<FMetasoundsMidiControlModule>(TEXT("MetasoundsMidiControl")).RecreateVoices();
	}
}
