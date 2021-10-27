void Event_PlayerDeath(Event hEvent, const char[] sName, bool bDontBroadcast)
{
	int iVictim = GetClientOfUserId(hEvent.GetInt("userid"));
	int iClient = GetClientOfUserId(hEvent.GetInt("attacker"));
	
	if(iVictim != iClient)
	{
		if(iClient > 0 && IsClientInGame(iClient) && !IsFakeClient(iClient))
		{
			SetBalance(iClient, GetBalance(iClient) + g_fKillReward);
		}
	}
}

void Event_WinPanel(Event hEvent, const char[] sName, bool bDontBroadcast)
{
	if(g_bDropsEnabled)
	{
		Protobuf hMessage;
		Protobuf hEntityUpdates;
		
		enum_skin hRandomSkin;
		
		int iItemId[2] = { 444, 444 };
		int iSkinId;
		int iTries;
		
		for(int iClient = 1; iClient <= MaxClients; iClient++)
		{
			if(IsClientInGame(iClient) && !IsFakeClient(iClient))
			{
				if(GetRandomFloat(0.0, 100.0) <= g_fDropsChance)
				{
					GetRandomDrop(hRandomSkin, iSkinId);
					
					if(ClientHasSkin(iClient, iSkinId, hRandomSkin.iWeaponIndex)) // If client already has this skin
					{
						if(iTries++ < 3)
						{
							iClient--;
							continue;
						}
					}
					
					hMessage = view_as<Protobuf>(StartMessageAll("SendPlayerItemDrops", USERMSG_RELIABLE));
					hEntityUpdates = hMessage.AddMessage("entity_updates");
					
					hEntityUpdates.SetInt("accountid", GetSteamAccountID(iClient));
					hEntityUpdates.SetInt64("itemid", iItemId);
					hEntityUpdates.SetInt("defindex", g_iWeaponDefIndex[hRandomSkin.iWeaponIndex]); 
					hEntityUpdates.SetInt("paintindex", hRandomSkin.iSkinIndex);
					hEntityUpdates.SetInt("rarity", hRandomSkin.iRarity);
					
					EndMessage();
					
					DB_InsertNewSkin(iClient, iSkinId);
					AddSkinToInventory(iClient, iSkinId, hRandomSkin.iWeaponIndex);
				}
			}
		}
	}
}