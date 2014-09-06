#ifndef _SERIALIZE_FORM_H_
#define _SERIALIZE_FORM_H_

#pragma once

#include "skse/GameData.h"
#include "skse/GameForms.h"

#pragma warning(push)
#pragma warning(disable: 4351)
/*

SerialFormData is intended to be an easy framework for saving and loading Form data
through the SKSE serialization process. Includes error handling during Deserialize.

Example:

	Serialization_Save --------------------------------------->
		if (intfc->OpenRecord('WXYZ', kSerializationDataVersion))
		{
			SerialFormData entry(someObject->formID);
			intfc->WriteRecordData(&entry, sizeof(entry));
		}

	Serialization_Load --------------------------------------->
		if (type == 'WXYZ')
		{
			SerialFormData entry;
			UInt32 sizeRead = intfc->ReadRecordData(&entry, sizeof(SerialFormData));
			if (sizeRead == sizeof(SerialFormData))
			{
				TESForm someObject = NULL;
				UInt32 result = entry.Deserialize(&someObject);
				if (result != SerialFormData::kResult_Succeeded)
				{
					SerialFormData::OutputError(result);
					return;
				}
				DoThingsWithSomeObject(someObject);
			}
		}

*/


class SerialFormData
{
public:
	enum
	{
		kResult_Succeeded,
		kResult_NullForm,
		kResult_InvalidForm,
		kResult_ModNotLoaded
	};

	char			modName[0x104];
	UInt32			formID;


private:
	static const char*	RUNTIME_FORM;

	void Serialize(UInt32 fullFormID)
	{
		formID = fullFormID & 0x00FFFFFF;

		if (formID)
		{
			UInt32 formIndex = (fullFormID & 0xFF000000) >> 24;

			if (formIndex == 0xFF)
				strcpy_s(modName, RUNTIME_FORM);
			else
			{
				DataHandler* pData = DataHandler::GetSingleton();
				ModInfo* mInfo = (pData) ? pData->modList.modInfoList.GetNthItem(formIndex) : NULL;
				strcpy_s(modName, (mInfo) ? mInfo->name : "");
			}
		}
	}


public:
	static void OutputError(UInt32 errorCode)
	{
		switch (errorCode)
		{
			case kResult_NullForm:
				_MESSAGE("Deserialization Error: Null Form");
				break;
			case kResult_InvalidForm:
				_MESSAGE("Deserialization Error: Invalid/Corrupt Form Data");
				break;
			case kResult_ModNotLoaded:
				_MESSAGE("Deserialization Error: Missing Source Plugin, Form Cannot Be Loaded.");
				break;
			default:
				break;
		}
	}

	UInt32 Deserialize(UInt32* const out_formID) //Deserialize data and retrieve formID
	{
		(*out_formID) = 0;

		if (!formID)
			return kResult_NullForm;

		DataHandler* pData = DataHandler::GetSingleton();
		UInt32 fullFormID = (pData) ? pData->GetModIndex(modName) : 0xFFFFFFFF;
		if (fullFormID >= 0xFF)
		{
			if (strcmp(modName, RUNTIME_FORM) == 0)
				fullFormID = 0xFF;
			else
				return kResult_ModNotLoaded;
		}

		fullFormID = (fullFormID << 24) | formID;

		(*out_formID) = fullFormID;
		return (LookupFormByID(fullFormID)) ? kResult_Succeeded : kResult_InvalidForm;
	}

	UInt32 Deserialize(TESForm** const out_form) //Deserialize data and retrieve form
	{
		UInt32 formID;
		UInt32 result = Deserialize(&formID);
		(*out_form) = LookupFormByID(formID);
		return result;
	}

	SerialFormData() : modName(), formID(0) {}
	SerialFormData(UInt32 formID) : modName() { Serialize(formID); } //Serialize from formID
	SerialFormData(TESForm* form) : modName() { Serialize(form->formID); } //Serialize from form
};

const char* SerialFormData::RUNTIME_FORM = "RUNTIME_FORM";


#pragma warning(pop)

#endif