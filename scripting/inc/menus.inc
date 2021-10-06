void ShowMainMenu(int iClient)
{
	char sBuffer[128];
	
	Menu hMenu = new Menu(MainMenu_Handler);
	hMenu.SetTitle("%T", "MainMenuTitle", iClient, PLUGIN_VERSION, GetBalance(iClient), GetTotalSkinsCount(iClient));
	
	FormatEx(sBuffer, sizeof(sBuffer), "%T", "OpenCases", iClient);
	hMenu.AddItem("open", sBuffer);
	
	FormatEx(sBuffer, sizeof(sBuffer), "%T", "Inventory", iClient);
	hMenu.AddItem("inventory", sBuffer);
	
	FormatEx(sBuffer, sizeof(sBuffer), "%T", "UnlockSkins", iClient);
	hMenu.AddItem("unlockskins", sBuffer);
	
	hMenu.Display(iClient, MENU_TIME_FOREVER);
}

int MainMenu_Handler(Menu hMenu, MenuAction iAction, int iClient, int iParam2)
{
	switch(iAction)
	{
		case MenuAction_End:delete hMenu;
		case MenuAction_Select:
		{
			char sInfo[128];
			hMenu.GetItem(iParam2, sInfo, sizeof(sInfo));
			
			if(StrEqual(sInfo, "open"))
			{
				ShowCasesMenu(iClient);
			}
			else if(StrEqual(sInfo, "inventory"))
			{
				ShowInventory(iClient);
			}
			else if(StrEqual(sInfo, "unlockskins"))
			{
				ShowSkinsMarket(iClient);
			}
		}
	}
}

void ShowCasesMenu(int iClient)
{
	char sBuffer[128];
	
	Menu hMenu = new Menu(Cases_Handler, MENU_ACTIONS_ALL);
	hMenu.SetTitle("%T", "CasesTitle", iClient, PLUGIN_VERSION, GetBalance(iClient), GetTotalSkinsCount(iClient));
	
	FormatEx(sBuffer, sizeof(sBuffer), "%T", g_bPreviewMode[iClient] ? "PreviewModeON" : "PreviewModeOFF", iClient);
	hMenu.AddItem("preview", sBuffer);
	
	for(int iCaseId = 0; iCaseId < g_hCasesNames.Length; iCaseId++) // Loop through existing cases
	{
		g_hCasesNames.GetString(iCaseId, sBuffer, sizeof(sBuffer)); // Get the case name
		
		Format(sBuffer, sizeof(sBuffer), "%T", "CasesItem", iClient, sBuffer, g_hCasesPrices.Get(iCaseId)); // Format the item(add price)
		hMenu.AddItem("", sBuffer);
	}
	
	hMenu.ExitBackButton = true;
	hMenu.Display(iClient, MENU_TIME_FOREVER);
}

int Cases_Handler(Menu hMenu, MenuAction iAction, int iClient, int iParam2)
{
	switch(iAction)
	{
		case MenuAction_End:delete hMenu;
		case MenuAction_DrawItem:
		{
			if(iParam2 != 0)
			{
				if(GetBalance(iClient) < g_hCasesPrices.Get(iParam2 - 1)) 
				{
					return ITEMDRAW_DISABLED;
				}			
			}
			return ITEMDRAW_DEFAULT;
		}
		case MenuAction_Select:
		{
			if(iParam2 == 0)
			{
				g_bPreviewMode[iClient] = !g_bPreviewMode[iClient]; // Switch the mode
				ShowCasesMenu(iClient);
			}
			else
			{
				if(g_bPreviewMode[iClient])
				{
					ShowPreviewMenu(iClient, iParam2 - 1);
				}
				else
				{
					UnlockSkin(iClient, iParam2 - 1);
				}
			}
		}
	}
	
	if(iParam2 == MenuCancel_ExitBack)
	{
		ShowMainMenu(iClient);
	}
	
	return 0;
}

void ShowPreviewMenu(int iClient, int iCaseId)
{
	char sSkinData[64];
	ArrayList hSkins = g_hCasesSkins.Get(iCaseId); // Get the skins array handle
	
	Menu hMenu = new Menu(PreviewCase_Handler);
	hMenu.SetTitle("%T", "PreviewCasesTitle", iClient, PLUGIN_VERSION, GetBalance(iClient), GetTotalSkinsCount(iClient)); 
	
	for(int iSkinId = 0; iSkinId < hSkins.Length; iSkinId++) // Loop through skins
	{
		hSkins.GetString(iSkinId, sSkinData, sizeof(sSkinData)); // Get skin data
		AddSkinToMenu(hMenu, sSkinData);
	}
	
	hMenu.ExitButton = false;
	hMenu.ExitBackButton = true;
	hMenu.Display(iClient, MENU_TIME_FOREVER);
}

int PreviewCase_Handler(Menu hMenu, MenuAction iAction, int iClient, int iParam2)
{
	if(iAction == MenuAction_End)
	{
		delete hMenu;
	}
	else if(iParam2 == MenuCancel_ExitBack)
	{
		ShowCasesMenu(iClient);
	}
}

void ShowInventory(int iClient)
{
	int iCount;
	char sItem[32], sInfo[32];
	Menu hMenu = new Menu(Inventory_Handler);
	hMenu.SetTitle("%T", "InventoryMainMenu", iClient, PLUGIN_VERSION, GetBalance(iClient), GetTotalSkinsCount(iClient)); 
	
	for(int iIndex = 0; iIndex < sizeof(g_iWeaponDefIndex); iIndex++)
	{
		iCount = GetSkinsCount(iClient, iIndex);
		if(iCount != 0)
		{
			Format(sItem, sizeof(sItem), "%T (%i)", g_sWeaponClasses[iIndex], iClient, iCount);
			Format(sInfo, sizeof(sInfo), "%s", g_sWeaponClasses[iIndex]);
			
			hMenu.AddItem(sInfo, sItem);
		}
	}
	
	if(hMenu.ItemCount == 0)
	{
		Format(sItem, sizeof(sItem), "%T", "NoSkins", iClient);
			
		hMenu.AddItem("", sItem, ITEMDRAW_DISABLED);
	}
	
	hMenu.ExitBackButton = true;
	hMenu.Display(iClient, MENU_TIME_FOREVER);
}

int Inventory_Handler(Menu hMenu, MenuAction iAction, int iClient, int iParam2)
{
	switch(iAction)
	{
		case MenuAction_End:
		{
			delete hMenu;
		}
		case MenuAction_Select:
		{
			char sKey[32], sSkinName[64];
			hMenu.GetItem(iParam2, sKey, sizeof(sKey));
			
			ArrayList hClientSkins;
			g_hInventory[iClient].GetValue(sKey, hClientSkins);
			
			Menu hSkinsMenu = new Menu(SkinsMenu_Handler);
			hSkinsMenu.SetTitle("%T", "SkinsMenu", iClient, PLUGIN_VERSION, GetBalance(iClient), GetTotalSkinsCount(iClient), sKey); 
			
			hSkinsMenu.AddItem(sKey, "Default");
			
			for(int iSkinIndex = 0; iSkinIndex < hClientSkins.Length; iSkinIndex++)
			{
				GetSkinName(hClientSkins.Get(iSkinIndex), sSkinName, sizeof(sSkinName));
				hSkinsMenu.AddItem(sKey, sSkinName);
			}
			
			hSkinsMenu.ExitBackButton = true;
			hSkinsMenu.Display(iClient, MENU_TIME_FOREVER);
		}
	}
	
	if(iParam2 == MenuCancel_ExitBack)
	{
		ShowMainMenu(iClient);
	}
}

int SkinsMenu_Handler(Menu hMenu, MenuAction iAction, int iClient, int iParam2)
{
	switch(iAction)
	{
		case MenuAction_End:
		{
			delete hMenu;
		}
		case MenuAction_Select:
		{
			char sKey[32];
			hMenu.GetItem(iParam2, sKey, sizeof(sKey));
			
			if(iParam2 != 0) // Default
			{
				char sItem[128], sSkinName[64];
		
				ArrayList hClientSkins;
				g_hInventory[iClient].GetValue(sKey, hClientSkins);
				
				Menu hSkinsMenu = new Menu(SkinMenu_Handler);
				hSkinsMenu.SetTitle("%T", "ConfigureSkin", iClient, PLUGIN_VERSION, GetBalance(iClient), GetTotalSkinsCount(iClient), sKey); 
				hSkinsMenu.ExitBackButton = true;
				
				int iSkinId = hClientSkins.Get(iParam2 - 1);
				Format(sKey, sizeof(sKey), "%s;%i", sKey, iSkinId);
				
				int iBalance = GetBalance(iClient);
				
				GetSkinName(iSkinId, sSkinName, sizeof(sSkinName));
				Format(sItem, sizeof(sItem), "%T", "Equipt", iClient, sSkinName);
				hSkinsMenu.AddItem(sKey, sItem);
				
				Format(sItem, sizeof(sItem), "%T", "SetNameTag", iClient, g_iStarTrackPrice);
				hSkinsMenu.AddItem(sKey, sItem, iBalance < g_iStarTrackPrice ? ITEMDRAW_DISABLED:ITEMDRAW_DEFAULT);
				
				Format(sItem, sizeof(sItem), "%T", "SetFloat", iClient, g_iFloatPrice);
				hSkinsMenu.AddItem(sKey, sItem, iBalance < g_iFloatPrice ? ITEMDRAW_DISABLED:ITEMDRAW_DEFAULT);
				
				Format(sItem, sizeof(sItem), "%T", "RandomSeed", iClient, g_iSeedPrice);
				hSkinsMenu.AddItem(sKey, sItem, iBalance < g_iSeedPrice ? ITEMDRAW_DISABLED:ITEMDRAW_DEFAULT);
				
				Format(sItem, sizeof(sItem), "%T", "ToggleST", iClient, g_iStarTrackPrice);
				hSkinsMenu.AddItem(sKey, sItem, iBalance < g_iStarTrackPrice ? ITEMDRAW_DISABLED:ITEMDRAW_DEFAULT);
				
				enum_skin hSkin;
				g_hSkins.GetArray(iSkinId, hSkin);
				Format(sItem, sizeof(sItem), "%T", "QuickSell", iClient, hSkin.iPrice);
				hSkinsMenu.AddItem(sKey, sItem);
				
				hSkinsMenu.Display(iClient, MENU_TIME_FOREVER);				
			}
			else
			{
				if(IsKnife(sKey))
				{
					Weapons_SetClientKnife(iClient, "weapon_knife", true);
				}
				else
				{
					Weapons_SetClientSkin(iClient, sKey, 0);
				}
			}
		}
	}
	
	if(iParam2 == MenuCancel_ExitBack)
	{
		ShowInventory(iClient);
	}
}

int SkinMenu_Handler(Menu hMenu, MenuAction iAction, int iClient, int iParam2)
{
	switch(iAction)
	{
		case MenuAction_End:
		{
			if(iParam2 != MenuEnd_Selected)
			{
				delete hMenu;
			}
		}
		case MenuAction_Select:
		{
			char sKey[64], sData[2][32];
			hMenu.GetItem(iParam2, sKey, sizeof(sKey));
			ExplodeString(sKey, ";",  sData, sizeof(sData), sizeof(sData[]));
			
			switch(iParam2)
			{
				case 0: 
				{
					enum_skin hWeaponSkin;
					if(g_hSkins.GetArray(StringToInt(sData[1]), hWeaponSkin))
					{
						if(IsKnife(sData[0]))
						{
							Weapons_SetClientKnife(iClient, sData[0], true);
						}
						
						Weapons_SetClientSkin(iClient, sData[0], hWeaponSkin.iSkinIndex);
					}
				}
				case 1: // Set nametag
				{
					g_bWaitingForName[iClient] = true;
					CPrintToChat(iClient, "%T", "NameInChat", iClient);
					strcopy(g_sTempWeapon[iClient], sizeof(g_sTempWeapon[]), sData[0]);
					SetBalance(iClient, GetBalance(iClient) - g_iNameTagPrice);
				}
				case 2: // Set float
				{
					g_bWaintingForFloat[iClient] = true;
					CPrintToChat(iClient, "%T", "FloatInChat", iClient);
					strcopy(g_sTempWeapon[iClient], sizeof(g_sTempWeapon[]), sData[0]);
					SetBalance(iClient, GetBalance(iClient) - g_iFloatPrice);
				}
				case 3: // Random seed
				{
					int iSeed = GetRandomInt(0, 8192);
					Weapons_SetClientSeed(iClient, sData[0], iSeed);
					CPrintToChat(iClient, "%T", "NewSeed", iClient, iSeed);
					SetBalance(iClient, GetBalance(iClient) - g_iSeedPrice);
				}
				case 4: // Toggle StarTrack
				{
					Weapons_ToggleClientStarTrack(iClient, sData[0]);
					SetBalance(iClient, GetBalance(iClient) - g_iStarTrackPrice);
				}
				case 5: // Quick sell
				{
					ArrayList hClientSkins;
					g_hInventory[iClient].GetValue(sData[0], hClientSkins);
					
					enum_skin hWeaponSkin;
					
					int iIndex, iItemId = StringToInt(sData[1]);
					
					if(g_hSkins.GetArray(iItemId, hWeaponSkin))
					{
						if(IsKnife(sData[0]))
						{
							Weapons_SetClientKnife(iClient, "weapon_knife", true);
						}
						
						Weapons_SetClientSkin(iClient, sData[0], 0);
						
						SetBalance(iClient, GetBalance(iClient) + hWeaponSkin.iPrice);
						
						iIndex = hClientSkins.FindValue(iItemId);
						if(iIndex != -1)
						{
							hClientSkins.Erase(iIndex);
						}
						CPrintToChat(iClient, "%T", "SkinSold", iClient, g_sItemRarity[hWeaponSkin.iRarity][1], hWeaponSkin.szName, hWeaponSkin.iPrice);
						DB_DeleteSkin(GetSteamAccountID(iClient), iItemId);
					}
					
					ShowInventory(iClient);
				}
			}
			
			if(iParam2 != 5)
			{
				hMenu.Display(iClient, MENU_TIME_FOREVER);
			}
		}
	}
	
	if(iParam2 == MenuCancel_ExitBack)
	{
		ShowInventory(iClient);
	}
}

void ShowSkinsMarket(int iClient)
{
	g_hMarketMenu.SetTitle("%T", "SkinsMarketTitle", iClient, PLUGIN_VERSION, GetBalance(iClient), GetTotalSkinsCount(iClient));
	g_hMarketMenu.Display(iClient, MENU_TIME_FOREVER);
}

int Market_Handler(Menu hMenu, MenuAction iAction, int iClient, int iParam2)
{
	switch(iAction)
	{
		case MenuAction_DrawItem:
		{
			char sKey[10];
			hMenu.GetItem(iParam2, sKey, sizeof(sKey));
			
			int iItemId = StringToInt(sKey);
			
			enum_skin hItem;
			g_hSkins.GetArray(iItemId, hItem);
			
			int iBalance = GetBalance(iClient);
			if(iBalance < hItem.iPrice || ClientHasSkin(iClient, iItemId, hItem.iWeaponIndex))
			{
				return ITEMDRAW_DISABLED;
			}
			return ITEMDRAW_DEFAULT;
		}
		case MenuAction_Select:
		{
			char sKey[10];
			hMenu.GetItem(iParam2, sKey, sizeof(sKey));
			
			int iItemId = StringToInt(sKey);
			
			enum_skin hItem;
			g_hSkins.GetArray(iItemId, hItem);
			
			if(hItem.iRarity > 3)
			{
				CPrintToChat(iClient, "%T", "SkinBoughtAll", iClient, iClient, g_sItemRarity[hItem.iRarity][1], hItem.szName, hItem.iPrice);
			}
			else
			{
				CPrintToChat(iClient, "%T", "SkinBought", iClient, g_sItemRarity[hItem.iRarity][1], hItem.szName, hItem.iPrice);
			}
			
			DB_InsertNewSkin(iClient, iItemId);
			AddSkinToInventory(iClient, iItemId, hItem.iWeaponIndex);
			ClientCommand(iClient, "play %s", g_sRaritySounds[hItem.iRarity]);
			
			SetBalance(iClient, GetBalance(iClient) - hItem.iPrice);
			
			ShowSkinsMarket(iClient);
		}
	}
	
	if(iParam2 == MenuCancel_ExitBack)
	{
		ShowMainMenu(iClient);
	}
	
	return 0;
}