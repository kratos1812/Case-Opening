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
	
	for (int iClient = 1; iClient <= MaxClients; iClient++)
	{
		if(IsClientInGame(iClient))
		{
			OnClientPostAdminCheck(iClient);
			OnClientPutInServer(iClient);
		}
	}
}

void DB_LoadFromDB(int iClient)
{
	char sBuffer[256];
	char sQuery[] = "SELECT `skinId` FROM `kratoss_case_opening` WHERE `accountId` = '%i';";
	
	Format(sBuffer, sizeof(sBuffer), sQuery, GetSteamAccountID(iClient));
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
		int iSkinId;
		
		ArrayList hSkins = new ArrayList();
		ArrayList hSkinsForIndex[sizeof(g_iWeaponDefIndex)];
		
		enum_skin hWeaponSkin;
		
		while(hResults.FetchRow())
		{
			iSkinId = hResults.FetchInt(0);
			hSkins.Push(iSkinId);
		}
		
		for(int iIter = 0; iIter < sizeof(g_iWeaponDefIndex); iIter++)
		{
			hSkinsForIndex[iIter] = new ArrayList();
		}
		
		for(int iItemId = 0; iItemId < hSkins.Length; iItemId++)
		{
			g_hSkins.GetArray(hSkins.Get(iItemId), hWeaponSkin);
			
			if(hWeaponSkin.iWeaponIndex != -1)
			{
				hSkinsForIndex[hWeaponSkin.iWeaponIndex].Push(hSkins.Get(iItemId));
			}
		}
		
		for(int iIndex = 0; iIndex < sizeof(g_sWeaponClasses); iIndex++)
		{
			g_hInventory[iClient].SetValue(g_sWeaponClasses[iIndex], hSkinsForIndex[iIndex]);
		}
		
		delete hSkins;
	}
}

void DB_InsertNewSkin(int iClient, int iItemUID)
{
	char sBuffer[256];
	char sQuery[] = "INSERT INTO `kratoss_case_opening`(`skinId`, `accountId`) VALUES('%i', '%i');"; 
	
	Format(sBuffer, sizeof(sBuffer), sQuery, iItemUID, GetSteamAccountID(iClient));
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

void DB_DeleteSkin(int iAccountId, int iSkinId)
{
	char sBuffer[256];
	char sQuery[] = "DELETE FROM `kratoss_case_opening` WHERE `skinId` = '%i' AND `accountId` = '%i'"; 
	
	Format(sBuffer, sizeof(sBuffer), sQuery, iSkinId, iAccountId);
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