stock void DebugCases()
{
	char szName[64], szSkinsData[64];
	ArrayList hSkins;
	
	for(int iIter = 0; iIter < g_hCasesNames.Length; iIter++)
	{
		PrintToServer("------------------------------------------");
		
		g_hCasesNames.GetString(iIter, szName, sizeof(szName));
		PrintToServer("%i. Searching for %s", iIter, szName);
		
		PrintToServer("%i. Price: %i", iIter, g_hCasesPrices.Get(iIter));
		
		hSkins = g_hCasesSkins.Get(iIter);
		PrintToServer("%i. Skins Count: %i", iIter, hSkins.Length);
		
		for(int iIndex = 0; iIndex < hSkins.Length; iIndex++)
		{
			hSkins.GetString(iIndex, szSkinsData, sizeof(szSkinsData));
			PrintToServer("%i(%i). Skins Data: %s", iIter, iIndex, szSkinsData);
		}
	}
}

stock void AddSkinToMenu(Menu hMenu, const char[] sSkinData)
{
	char sData[Data_Max][64], sItem[64];
	enum_skin hWeaponSkin;
	
	ExplodeString(sSkinData, "-", sData, sizeof(sData), sizeof(sData[]));
	if(g_hSkins.GetArray(StringToInt(sData[Data_Id]), hWeaponSkin))
	{
		Format(sItem, sizeof(sItem), "%s | Price: [$%.2f]", hWeaponSkin.szName, hWeaponSkin.fPrice);
		hMenu.AddItem("", sItem, ITEMDRAW_DISABLED);
	}
}

stock float GetBalance(int iClient)
{
	if(AreClientCookiesCached(iClient))
	{
		char sBalance[32];
		g_hBalance.Get(iClient, sBalance, sizeof(sBalance));
		
		if(!sBalance[0])
		{
			return 0.0;
		}
		return StringToFloat(sBalance);
	}
	else
	{
		LogMessage("Couldn't get balance for %L", iClient);
		return -1.0;
	}
}

stock void SetBalance(int iClient, float fAmount) // It can be negative as well, doesnt really matter
{
	char sBalance[32];
	Format(sBalance, sizeof(sBalance), "%f", fAmount);// PrintToChatAll("sBalance = %f, fAmount = %f", sBalance, fAmount);
	g_hBalance.Set(iClient, sBalance);		
}

stock int IndexByDefIndex(int iDefIndex)
{
	for(int iIter = 0; iIter < sizeof(g_iWeaponDefIndex); iIter++)
	{
		if(g_iWeaponDefIndex[iIter] == iDefIndex)
		{
			return iIter;
		}
	}
	
	return -1;
}

stock int GetSkinsCount(int iClient, int iIndex)
{
	ArrayList hSkins;
	g_hInventory[iClient].GetValue(g_sWeaponClasses[iIndex], hSkins);
	return hSkins.Length;
}

stock int GetTotalSkinsCount(int iClient)
{
	int iTotal;
	ArrayList hSkins;
	
	for(int iIndex = 0; iIndex < sizeof(g_iWeaponDefIndex); iIndex++)
	{
		if(g_hInventory[iClient].GetValue(g_sWeaponClasses[iIndex], hSkins))
		{
			iTotal += hSkins.Length;
		}
	}

	return iTotal;
}

stock int GetSkinName(int iSkinId, char[] sName, int iMaxLen)
{
	enum_skin hSkin;
	g_hSkins.GetArray(iSkinId, hSkin);
	return Format(sName, iMaxLen, hSkin.szName);
}

bool ClientHasSkin(int iClient, int iItemId, int iIndex)
{
	ArrayList hArray;
	g_hInventory[iClient].GetValue(g_sWeaponClasses[iIndex], hArray);
	if(hArray.FindValue(iItemId) != -1)
	{
		return true;
	}
	return false;
}

stock int RarityIntToString(int iRarity, int iMaxLen, char[] sOutput)
{
	return Format(sOutput, iMaxLen, g_sItemRarity[iRarity][1]);
}

stock bool IsKnife(const char[] sClassname)
{
	return StrContains(sClassname, "knife", false) != -1 || StrContains(sClassname, "bayonet", false) != -1;
}

stock bool GetRandomDrop(enum_skin hSkin, int iSkinId)
{
	int iChances, iItemId;
	char sSkinData[64], sData[Data_Max][10];
	
	enum_skin hDummySkin;
	
	ArrayList hSkinsChances = new ArrayList();
	
	for(int iCaseSkinId = 0; iCaseSkinId < g_hDropsSkins.Length; iCaseSkinId++) // Loop through existing skins
	{
		g_hDropsSkins.GetString(iCaseSkinId, sSkinData, sizeof(sSkinData)); // Get skin data string
		ExplodeString(sSkinData, "-", sData, sizeof(sData), sizeof(sData[])); // Explode the data from the data string
		
		iItemId = StringToInt(sData[Data_Id]); // We want to use the index from skins.cfg of the item
		if(g_hSkins.GetArray(iItemId, hDummySkin)) // Check if we have a skin for this ID
		{
			iChances = StringToInt(sData[Data_Chance]); // Get total number of entries of the skin
			
			while (--iChances > -1)
			{
				hSkinsChances.Push(iItemId); // Add a entry for this skin id from 
			}
		}
		else
		{
			LogError("Error while oppenig the skin. Please contact the author. Error data: %i;%i;%i", iItemId, iCaseSkinId, g_hSkins.Length);
		}
	}
	
	hSkinsChances.Sort(Sort_Random, Sort_Integer); // Sort skins chances;
	iItemId = hSkinsChances.Get(GetRandomInt(0, hSkinsChances.Length -1)); // Extract the id from skins.cfg of the winning item.
	g_hSkins.GetArray(iItemId, hSkin); // Get the skin
	
	iSkinId = iItemId;
}