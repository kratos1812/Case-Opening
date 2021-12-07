void UnlockSkin(int iClient, int iCaseId)
{
	int iChances, iItemId;
	
	char sSkinData[64], sData[Data_Max][10];
	
	ArrayList hSkins = g_hCasesSkins.Get(iCaseId); // Get the skins from the case.
	ArrayList hSkinsChances = new ArrayList();
	
	enum_skin hWeaponSkin;
	
	float fPrice = g_hCasesPrices.Get(iCaseId);
	SetBalance(iClient, GetBalance(iClient) - fPrice); // Remove credits
	
	for(int iCaseSkinId = 0; iCaseSkinId < hSkins.Length; iCaseSkinId++) // Loop through existing skins
	{
		hSkins.GetString(iCaseSkinId, sSkinData, sizeof(sSkinData)); // Get skin data string
		ExplodeString(sSkinData, "-", sData, sizeof(sData), sizeof(sData[])); // Explode the data from the data string
		
		iItemId = StringToInt(sData[Data_Id]); // We want to use the index from skins.cfg of the item
		if(g_hSkins.GetArray(iItemId, hWeaponSkin)) // Check if we have a skin for this ID
		{
			iChances = StringToInt(sData[Data_Chance]); // Get total number of entries of the skin
			
			while (--iChances > -1)
			{
				hSkinsChances.Push(iItemId); // Add a entry for this skin id from 
			}
		}
		else
		{
			PrintToChat(iClient, "Error while oppenig the skin. Please contact the author. Error data: %i;%i;%i", iItemId, iCaseSkinId, hSkins.Length);
		}
	}
	
	hSkinsChances.Sort(Sort_Random, Sort_Integer); // Sort skins chances;
	iItemId = hSkinsChances.Get(GetRandomInt(0, hSkinsChances.Length -1)); // Extract the id from skins.cfg of the winning item.
	g_hSkins.GetArray(iItemId, hWeaponSkin); // Get the skin
	
	ClientCommand(iClient, "play %s", g_sRaritySounds[hWeaponSkin.iRarity]);
	if(ClientHasSkin(iClient, iItemId, hWeaponSkin.iWeaponIndex)) // If client already has this skin
	{
		float fReturn = g_hCasesReturns.Get(iCaseId);
		SetBalance(iClient, GetBalance(iClient) + fReturn);
		
		CPrintToChat(iClient, "%T", "SkinAlreadyOwned", iClient, g_sItemRarity[hWeaponSkin.iRarity][1], hWeaponSkin.szName, fReturn);
	}
	else
	{
		if(hWeaponSkin.iRarity > 3)
		{
			CPrintToChatAll("%T", "SkinOpenedAll", iClient, iClient, g_sItemRarity[hWeaponSkin.iRarity][1], hWeaponSkin.szName, hWeaponSkin.fPrice);
		}
		else
		{
			CPrintToChat(iClient, "%T", "SkinOpened", iClient, g_sItemRarity[hWeaponSkin.iRarity][1], hWeaponSkin.szName, hWeaponSkin.fPrice);
		}
		
		DB_InsertNewSkin(iClient, iItemId);
		AddSkinToInventory(iClient, iItemId, hWeaponSkin.iWeaponIndex);
	}

	delete hSkinsChances;
}

void AddSkinToInventory(int iClient, int iItemId, int iIndex)
{
	ArrayList hArray;
	g_hInventory[iClient].GetValue(g_sWeaponClasses[iIndex], hArray);
	hArray.Push(iItemId);
	hArray = null;
}

void AddGloveToInventory(int iClient, int iItemId, int iType)
{
	ArrayList hArray;
	g_hInventory[iClient].GetValue(g_sGlovesType[iType], hArray);
	hArray.Push(iItemId);
	hArray = null;
}