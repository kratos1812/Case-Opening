public void DB_OnConnect(Database hDatabase, const char[] sError, any pData)
{
	if(hDatabase == null)
	{
		SetFailState("DB_OnConnect: \"%s\".", sError);
	}
	
	g_hDatabase = hDatabase;
	DB_CreateTables();
}

void DB_CreateTables()
{
	for (int iIter = 0; iIter < sizeof(g_sMySQLTables); iIter++) // Might add more tables later, idk
	{
		g_hDatabase.Query(CreateTables_SQLCallback, g_sMySQLTables[iIter], iIter, DBPrio_High);
	}
}

public void CreateTables_SQLCallback(Database hDatabase, DBResultSet hResults, const char[] sError, int iId)
{
	if(hDatabase == null || hResults == null)
	{
		SetFailState("Failed to create MySQL tables. Error: \"%s\".", sError);
	}
	
	if(iId == sizeof(g_sMySQLTables) - 1)
	{
		for (int iClient = 1; iClient <= MaxClients; iClient++)
		{
			if(IsClientInGame(iClient))
			{
				OnClientPostAdminCheck(iClient);
				OnClientPutInServer(iClient);
			}
		}		
	}
}

void DB_LoadFromDB(int iClient)
{
	char sBuffer[256];
	char sQuery[] = "SELECT `skinId`, `type` FROM `kratoss_case_opening` WHERE `accountId` = '%i';";
	
	g_hDatabase.Format(sBuffer, sizeof(sBuffer), sQuery, GetSteamAccountID(iClient));
	g_hDatabase.Query(SelectData_SQLCallback, sBuffer, GetClientUserId(iClient));
}

public void SelectData_SQLCallback(Database hDatabase, DBResultSet hResults, const char[] sError, int iUserId)
{
	if(hDatabase == null || hResults == null)
	{
		LogError("MySQL error: \"%s\".", sError);
		return;
	}
	
	int iClient = GetClientOfUserId(iUserId);
	
	if(iClient >= 1 && IsClientInGame(iClient))
	{
		int iItemId;
		int iId;
		int iSkinsCount = g_hSkins.Length;
		int iType;
		int iGlovesCount = g_hGloves.Length;
		
		ArrayList hSkins = new ArrayList();
		ArrayList hSkinsForIndex[sizeof(g_iWeaponDefIndex)];
		ArrayList hGloves = new ArrayList();
		ArrayList hGlovesForType[Gloves_Max];
		
		enum_skin hWeaponSkin;
		enum_glove hGlove;
		
		for(int iIter = 0; iIter < sizeof(g_iWeaponDefIndex); iIter++)
		{
			hSkinsForIndex[iIter] = new ArrayList();
		}
		
		for(int iIter = 0; iIter < sizeof(g_sGlovesType); iIter++)
		{
			hGlovesForType[iIter] = new ArrayList();
		}
		
		while(hResults.FetchRow())
		{
			iItemId = hResults.FetchInt(0);
			iType = hResults.FetchInt(1);
			
			switch(iType)
			{
				case Type_WeaponSkin:
				{
					if(iItemId >= iSkinsCount)
					{
						continue;
					}
					
					hSkins.Push(iItemId);
				}
				case Type_Gloves:
				{
					if(iItemId >= iGlovesCount)
					{
						continue;
					}
					
					hGloves.Push(iItemId);
				}
			}
		}
		
		for(iItemId = 0; iItemId < hSkins.Length; iItemId++)
		{
			iId = hSkins.Get(iItemId);
			
			if(g_hSkins.GetArray(iId, hWeaponSkin))
			{
				hSkinsForIndex[hWeaponSkin.iWeaponIndex].Push(iId);
			}
		}
		
		for(int iIndex = 0; iIndex < sizeof(g_sWeaponClasses); iIndex++)
		{
			g_hInventory[iClient].SetValue(g_sWeaponClasses[iIndex], hSkinsForIndex[iIndex]);
		}
		
		for(iItemId = 0; iItemId < hGloves.Length; iItemId++)
		{
			iId = hGloves.Get(iItemId);
			
			g_hGloves.GetArray(iId, hGlove);
			hGlovesForType[hGlove.iType].Push(iId);
		}
		
		for(iType = 0; iType < sizeof(g_sGlovesType); iType++)
		{
			g_hInventory[iClient].SetValue(g_sGlovesType[iType], hGlovesForType[iType]); 
		}
		
		delete hGloves;
		delete hSkins;
	}
}

void DB_InsertNewSkin(int iClient, int iItemUID, enItemType enType = Type_WeaponSkin)
{
	char sBuffer[256];
	char sQuery[] = "INSERT INTO `kratoss_case_opening`(`skinId`, `accountId`, `type`) VALUES('%i', '%i', '%i');"; 
	
	g_hDatabase.Format(sBuffer, sizeof(sBuffer), sQuery, iItemUID, GetSteamAccountID(iClient), enType);
	g_hDatabase.Query(InsertSkin_SQLCallback, sBuffer);
}

public void InsertSkin_SQLCallback(Database hDatabase, DBResultSet hResults, const char[] sError, any pData)
{
	if(hDatabase == null || hResults == null)
	{
		LogError("MySQL error: \"%s\".", sError);
		return;
	}
}

void DB_DeleteSkin(int iAccountId, int iSkinId, int iType = Type_WeaponSkin)
{
	char sBuffer[256];
	char sQuery[] = "DELETE FROM `kratoss_case_opening` WHERE `skinId` = '%i' AND `accountId` = '%i' AND `type` = '%i';"; 
	
	g_hDatabase.Format(sBuffer, sizeof(sBuffer), sQuery, iSkinId, iAccountId, iType);
	g_hDatabase.Query(DeleteSkin_SQLCallback, sBuffer);
}

public void DeleteSkin_SQLCallback(Database hDatabase, DBResultSet hResults, const char[] sError, any pData)
{
	if(hDatabase == null || hResults == null)
	{
		LogError("MySQL error: \"%s\".", sError);
		return;
	}
}