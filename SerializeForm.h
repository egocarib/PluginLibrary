#ifndef _SERIALIZE_FORM_H_
#define _SERIALIZE_FORM_H_

#include "skse/GameData.h"

/*

SerialFormData is intended to be an easy framework for saving and loading Form data
through the SKSE serialization process. If unexpected problems occur during the load
process, an appropriate error code will be returned from the Deserialize method,
indicating if the form originates from a mod that is no longer loaded, if the Form
is a NULL form, or if the form is invalid (possibly due to corrupted cosave data)

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
				if (result == SerialFormData::kResult_Succeeded)
					DoThingsWithSomeObject(someObject);
			}
		}

*/


class SerialFormData
{
public:
	enum
	{
		kResult_Succeeded;
		kResult_NullForm;
		kResult_InvalidForm;
		kResult_ModNotLoaded;
	};

	char			modName[0x104];
	UInt32			formID;

	SerialFormData();
	SerialFormData(UInt32 formID); //Serialize from formID
	SerialFormData(TESForm* form); //Serialize from form
	UInt32 Deserialize(UInt32* out_formID); //Deserialize data and retrieve formID
	UInt32 Deserialize(TESForm** out_form); //Deserialize data and retrieve form


private:
	static const char*	RUNTIME_FORM = "RUNTIME_FORM";

	UInt32 Deserialize(UInt32* out_formID)
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

	UInt32 Deserialize(TESForm** out_form)
	{
		UInt32 formID;
		UInt32 result = Deserialize(&formID);
		(*out_form) = LookupFormByID(formID);
		return result;
	}

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

	SerialFormData() : modName(), formID(0) {}
	SerialFormData(UInt32 formID) : modName() { Serialize(formID); }
	SerialFormData(TESForm* form) : modName() { Serialize(form->formID); }
};



#endif