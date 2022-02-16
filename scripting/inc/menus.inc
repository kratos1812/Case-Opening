void ShowMainMenu(int iClient)
{
	char sBuffer[128];
	
	Menu hMenu = new Menu(MainMenu_Handler);
	hMenu.SetTitle("%T", "MainMenuTitle", iClient, PLUGIN_VERSION, GetBalance(iClient), GetTotalSkinsCount(iClient), GetTotalGlovesCount(iClient));
	
	FormatEx(sBuffer, sizeof(sBuffer), "%T", "OpenCases", iClient);
	hMenu.AddItem("open", sBuffer);
	
	FormatEx(sBuffer, sizeof(sBuffer), "%T", "Inventory", iClient);
	hMenu.AddItem("inventory", sBuffer);
	
	if(g_bEnableMarketSkins)
	{
		FormatEx(sBuffer, sizeof(sBuffer), "%T", "UnlockSkins", iClient);
		hMenu.AddItem("unlockskins", sBuffer);
	}

	if(g_bEnableMarketGloves)
	{
		FormatEx(sBuffer, sizeof(sBuffer), "%T", "UnlockGloves", iClient);
		hMenu.AddItem("unlockgloves", sBuffer);		
	}

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
			else if(StrEqual(sInfo, "unlockgloves"))
			{
				ShowGlovesMarket(iClient);
			}
		}
	}
}

void ShowCasesMenu(int iClient, int iPos = 0)
{
	char sBuffer[128];
	
	Menu hMenu = new Menu(CasesCategories_Handler, MENU_ACTIONS_ALL);
	hMenu.SetTitle("%T", "CasesTitle_Categories", iClient, PLUGIN_VERSION, GetBalance(iClient), GetTotalSkinsCount(iClient), GetTotalGlovesCount(iClient));
	
	StringMapSnapshot hSnap = g_hCasesCategories.Snapshot();
	for(int iIndex = 0; iIndex < hSnap.Length; iIndex++)
	{
		hSnap.GetKey(iIndex, sBuffer, sizeof(sBuffer));
		hMenu.AddItem(sBuffer, sBuffer);
	}
	
	delete hSnap;
	
	hMenu.ExitBackButton = true;
	hMenu.DisplayAt(iClient, iPos, MENU_TIME_FOREVER);
}

public int CasesCategories_Handler(Menu hMenu, MenuAction iAction, int iClient, int iParam2)
{
	switch(iAction)
	{
		case MenuAction_End:delete hMenu;
		case MenuAction_Select:
		{
			char sBuffer[32];
			hMenu.GetItem(iParam2, sBuffer, sizeof(sBuffer));
			
			ShowCasesMenuFinal(iClient, 0, sBuffer);
		}
		
		case MenuAction_Cancel:
		{	
			switch(iParam2)
			{
				case MenuCancel_ExitBack:ShowMainMenu(iClient);
			}
		}
	}
}

void ShowCasesMenuFinal(int iClient, int iPos = 0, const char[] sCategory = "Default Cases")
{
	float fPrice;
	
	char sBuffer[128];
	
	Menu hMenu = new Menu(Cases_Handler);
	hMenu.SetTitle("%T", "CasesTitle", iClient, PLUGIN_VERSION, GetBalance(iClient), GetTotalSkinsCount(iClient), GetTotalGlovesCount(iClient));
	
	FormatEx(sBuffer, sizeof(sBuffer), "%T", g_bPreviewMode[iClient] ? "PreviewModeON" : "PreviewModeOFF", iClient);
	hMenu.AddItem("preview", sBuffer);
	
	ArrayList hCases;
	g_hCasesCategories.GetValue(sCategory, hCases); 
	
	for(int iCaseId = 0; iCaseId < g_hCasesNames.Length; iCaseId++) // Loop through existing cases
	{
		g_hCasesNames.GetString(iCaseId, sBuffer, sizeof(sBuffer)); // Get the case name
		if(hCases.FindString(sBuffer) != -1) 
		{
			fPrice = g_hCasesPrices.Get(iCaseId);
			Format(sBuffer, sizeof(sBuffer), "%T", "CasesItem", iClient, sBuffer, fPrice); // Format the item(add price)
			hMenu.AddItem(sCategory, sBuffer, fPrice > GetBalance(iClient) ? ITEMDRAW_DISABLED:ITEMDRAW_DEFAULT);
		}
	}
	
	hMenu.ExitBackButton = true;
	hMenu.DisplayAt(iClient, iPos, MENU_TIME_FOREVER);
}

int Cases_Handler(Menu hMenu, MenuAction iAction, int iClient, int iParam2)
{
	switch(iAction)
	{
		case MenuAction_End:delete hMenu;
		case MenuAction_Select:
		{
			char sCategory[64];
			
			if(iParam2 == 0)
			{
				hMenu.GetItem(iParam2 + 1, sCategory, sizeof(sCategory));
				g_bPreviewMode[iClient] = !g_bPreviewMode[iClient]; // Switch the mode
				ShowCasesMenuFinal(iClient, hMenu.Selection, sCategory);
			}
			else
			{
				hMenu.GetItem(iParam2, sCategory, sizeof(sCategory));
				if(g_bPreviewMode[iClient])
				{
					ShowPreviewMenu(iClient, iParam2 - 1, sCategory);
				}
				else
				{
					if(g_bCanOpen[iClient])
					{
						g_bCanOpen[iClient] = false;
						CreateTimer(g_fOpenDelay, Timer_ResetDelay, GetClientUserId(iClient), TIMER_FLAG_NO_MAPCHANGE);
						
						UnlockSkin(iClient, iParam2 - 1, sCategory);
					}
					else
					{
						CPrintToChat(iClient, "%T", "NotYet", iClient, g_fOpenDelay);
					}
					
					ShowCasesMenuFinal(iClient, hMenu.Selection, sCategory);
				}
			}
		}
		
		case MenuAction_Cancel:
		{	
			switch(iParam2)
			{
				case MenuCancel_ExitBack:ShowCasesMenu(iClient);
			}
		}
	}
	
	return 0;
}

Action Timer_ResetDelay(Handle hTimer, int iUserId)
{
	int iClient = GetClientOfUserId(iUserId);
	if(iClient != 0)
	{
		g_bCanOpen[iClient] = true;
	}
}

void ShowPreviewMenu(int iClient, int iCaseId, const char[] sCategory = "Default Cases")
{
	char sSkinData[64], sBuffer[32], sCaseName[32];
	ArrayList hCases;
	
	g_hCasesCategories.GetValue(sCategory, hCases);
	hCases.GetString(iCaseId, sBuffer, sizeof(sBuffer)); // Here, iCaseId, becomes the idx of the case, FROM THE CATEGORY LIST
	
	for(iCaseId = 0; iCaseId < g_hCasesNames.Length; iCaseId++)
	{
		g_hCasesNames.GetString(iCaseId, sCaseName, sizeof(sCaseName));
		
		if(strcmp(sBuffer, sCaseName) == 0)
		{
			break;
		}
	}

	ArrayList hSkins = g_hCasesSkins.Get(iCaseId); // Get the skins array handle
	
	Menu hMenu = new Menu(PreviewCase_Handler);
	hMenu.SetTitle("%T", "PreviewCasesTitle", iClient, PLUGIN_VERSION, GetBalance(iClient), GetTotalSkinsCount(iClient), GetTotalGlovesCount(iClient)); 
	
	for(int iSkinId = 0; iSkinId < hSkins.Length; iSkinId++) // Loop through skins
	{
		hSkins.GetString(iSkinId, sSkinData, sizeof(sSkinData)); // Get skin data
		AddSkinToMenu(hMenu, sSkinData, sCategory);
	}
	
	hMenu.ExitButton = false;
	hMenu.ExitBackButton = true;
	hMenu.Display(iClient, MENU_TIME_FOREVER);
}

int PreviewCase_Handler(Menu hMenu, MenuAction iAction, int iClient, int iParam2)
{
	switch(iAction)
	{
		case MenuAction_End:delete hMenu;
		case MenuAction_Cancel:
		{
			if(iParam2 == MenuCancel_ExitBack)
			{
				char sCategory[64];
				hMenu.GetItem(0, sCategory, sizeof(sCategory));
				ShowCasesMenuFinal(iClient, .sCategory=sCategory);
			}
		}
	}
}

void ShowInventory(int iClient)
{
	int iCount;
	char sItem[32], sInfo[32];
	Menu hMenu = new Menu(Inventory_Handler);
	hMenu.SetTitle("%T", "InventoryMainMenu", iClient, PLUGIN_VERSION, GetBalance(iClient), GetTotalSkinsCount(iClient), GetTotalGlovesCount(iClient)); 
	
	Format(sItem, sizeof(sItem), "%T", "See gloves", iClient, GetTotalGlovesCount(iClient));	
	hMenu.AddItem("gloves", sItem);
	
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
		case MenuAction_End: delete hMenu;
		case MenuAction_Select:
		{
			char sKey[32];
			hMenu.GetItem(iParam2, sKey, sizeof(sKey));
			
			if(StrEqual(sKey, "gloves"))
			{
				ShowGlovesInventory(iClient);
			}
			else
			{
				ShowWeaponSkins(iClient, sKey, 0);			
			}
		}
		
		case MenuAction_Cancel:
		{
			if(iParam2 == MenuCancel_ExitBack)
			{
				ShowMainMenu(iClient);
			}
		}
	}
}

void ShowWeaponSkins(int iClient, const char[] sWeaponClassname, int iStart = 0)
{
	char sSkinName[128];
	
	ArrayList hClientSkins;
	g_hInventory[iClient].GetValue(sWeaponClassname, hClientSkins);
	
	Menu hSkinsMenu = new Menu(SkinsMenu_Handler);
	hSkinsMenu.SetTitle("%T", "SkinsMenu", iClient, PLUGIN_VERSION, GetBalance(iClient), GetTotalSkinsCount(iClient), sWeaponClassname, GetTotalGlovesCount(iClient)); 
	
	hSkinsMenu.AddItem(sWeaponClassname, "Default");
	
	for(int iSkinIndex = 0; iSkinIndex < hClientSkins.Length; iSkinIndex++)
	{
		GetSkinName(hClientSkins.Get(iSkinIndex), sSkinName, sizeof(sSkinName));
		hSkinsMenu.AddItem(sWeaponClassname, sSkinName);
	}
	
	hSkinsMenu.ExitBackButton = true;
	hSkinsMenu.DisplayAt(iClient, iStart, MENU_TIME_FOREVER);				
}

int SkinsMenu_Handler(Menu hMenu, MenuAction iAction, int iClient, int iParam2)
{
	switch(iAction)
	{
		case MenuAction_End: delete hMenu;
		case MenuAction_Select:
		{
			char sKey[128];
			hMenu.GetItem(iParam2, sKey, sizeof(sKey));
			
			if(iParam2 != 0) // Default
			{
				char sItem[128], sSkinName[64];
		
				ArrayList hClientSkins;
				g_hInventory[iClient].GetValue(sKey, hClientSkins);
				
				Menu hSkinsMenu = new Menu(SkinMenu_Handler, MENU_ACTIONS_ALL);
				hSkinsMenu.SetTitle("%T", "ConfigureSkin", iClient, PLUGIN_VERSION, GetBalance(iClient), GetTotalSkinsCount(iClient), sKey, GetTotalGlovesCount(iClient)); 
				hSkinsMenu.ExitBackButton = true;
				
				int iSkinId = hClientSkins.Get(iParam2 - 1);
				Format(sKey, sizeof(sKey), "%s;%i", sKey, iSkinId);
				float fBalance = GetBalance(iClient);
				
				GetSkinName(iSkinId, sSkinName, sizeof(sSkinName));
				Format(sItem, sizeof(sItem), "%T", "Equipt", iClient, sSkinName);
				hSkinsMenu.AddItem(sKey, sItem);
				
				Format(sItem, sizeof(sItem), "%T", "SetNameTag", iClient, g_fNameTagPrice);
				hSkinsMenu.AddItem(sKey, sItem, fBalance < g_fNameTagPrice ? ITEMDRAW_DISABLED:ITEMDRAW_DEFAULT);
				
				Format(sItem, sizeof(sItem), "%T", "SetFloat", iClient, g_fFloatPrice);
				hSkinsMenu.AddItem(sKey, sItem, fBalance < g_fFloatPrice ? ITEMDRAW_DISABLED:ITEMDRAW_DEFAULT);
				
				Format(sItem, sizeof(sItem), "%T", "RandomSeed", iClient, g_fSeedPrice);
				hSkinsMenu.AddItem(sKey, sItem, fBalance < g_fSeedPrice ? ITEMDRAW_DISABLED:ITEMDRAW_DEFAULT);
				
				Format(sItem, sizeof(sItem), "%T", "ToggleST", iClient, g_fStarTrackPrice);
				hSkinsMenu.AddItem(sKey, sItem, fBalance < g_fStarTrackPrice ? ITEMDRAW_DISABLED:ITEMDRAW_DEFAULT);
				
				enum_skin hSkin;
				g_hSkins.GetArray(iSkinId, hSkin);
				
				if(g_bEnableQuickSell)
				{
					if(hSkin.fPrice != 0.0)
					{
						Format(sItem, sizeof(sItem), "%T", "QuickSell", iClient, hSkin.fPrice);
						hSkinsMenu.AddItem(sKey, sItem);
					}
					else
					{
						Format(sItem, sizeof(sItem), "%T", "QuickSell_NoPrice", iClient);
						hSkinsMenu.AddItem(sKey, sItem, ITEMDRAW_DISABLED);
					}					
				}

				hSkinsMenu.Display(iClient, MENU_TIME_FOREVER);			
			}
			else
			{
				ShowWeaponSkins(iClient, sKey, hMenu.Selection);
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
		case MenuAction_Cancel:
		{
			if(iParam2 == MenuCancel_ExitBack)
			{
				ShowInventory(iClient);
			}
		}
	}
}

int SkinMenu_Handler(Menu hMenu, MenuAction iAction, int iParam1, int iParam2)
{
	switch(iAction)
	{
		case MenuAction_DrawItem:
		{
			float fBalance = GetBalance(iParam1);
			
			switch(iParam2)
			{
				case 1: // Set nametag
				{
					return fBalance < g_fNameTagPrice ? ITEMDRAW_DISABLED:ITEMDRAW_DEFAULT;
				}
				case 2: // Set float
				{
					return fBalance < g_fFloatPrice ? ITEMDRAW_DISABLED:ITEMDRAW_DEFAULT;
				}
				case 3: // Random seed
				{
					return fBalance < g_fSeedPrice ? ITEMDRAW_DISABLED:ITEMDRAW_DEFAULT;
				}
				case 4: // Toggle StarTrack
				{
					return fBalance < g_fStarTrackPrice ? ITEMDRAW_DISABLED:ITEMDRAW_DEFAULT;
				}
			}
			return ITEMDRAW_DEFAULT;
		}
		
		case MenuAction_End: 
		{
            if (iParam1 != MenuEnd_Selected) 
            {
                delete hMenu;
            }
        }

		case MenuAction_Select:
		{
			int iClient = iParam1;
			
			char sKey[128], sData[2][128];
			hMenu.GetItem(iParam2, sKey, sizeof(sKey));
			ExplodeString(sKey, ";", sData, sizeof(sData), sizeof(sData[]));
			
			switch(iParam2)
			{
				case 0: // Equipt
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
					SetBalance(iClient, GetBalance(iClient) - g_fNameTagPrice);
				}
				case 2: // Set float
				{
					g_bWaintingForFloat[iClient] = true;
					CPrintToChat(iClient, "%T", "FloatInChat", iClient);
					strcopy(g_sTempWeapon[iClient], sizeof(g_sTempWeapon[]), sData[0]);
					SetBalance(iClient, GetBalance(iClient) - g_fFloatPrice);
				}
				case 3: // Random seed
				{
					int iSeed = GetRandomInt(0, 8192);
					Weapons_SetClientSeed(iClient, sData[0], iSeed);
					CPrintToChat(iClient, "%T", "NewSeed", iClient, iSeed);
					SetBalance(iClient, GetBalance(iClient) - g_fSeedPrice);
				}
				case 4: // Toggle StarTrack
				{
					Weapons_ToggleClientStarTrack(iClient, sData[0]);
					SetBalance(iClient, GetBalance(iClient) - g_fStarTrackPrice);
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
						
						if(Weapons_GetClientSkin(iClient, sData[0]) == hWeaponSkin.iSkinIndex)
						{
							Weapons_SetClientSkin(iClient, sData[0], 0);
						}
						
						SetBalance(iClient, GetBalance(iClient) + hWeaponSkin.fPrice);
						
						iIndex = hClientSkins.FindValue(iItemId);
						if(iIndex != -1)
						{
							hClientSkins.Erase(iIndex);
						}
						CPrintToChat(iClient, "%T", "SkinSold", iClient, g_sItemRarity[hWeaponSkin.iRarity][1], hWeaponSkin.szName, hWeaponSkin.fPrice);
						DB_DeleteSkin(GetSteamAccountID(iClient), iItemId);
					}
					
					ShowWeaponSkins(iClient, sData[0], 0);
				}
			}
			
			if(iParam2 != 5)
			{
				hMenu.SetTitle("%T", "ConfigureSkin", iClient, PLUGIN_VERSION, GetBalance(iClient), GetTotalSkinsCount(iClient), sData[0], GetTotalGlovesCount(iClient)); 
				hMenu.Display(iParam1, MENU_TIME_FOREVER);
			}
			else
			{
				delete hMenu;
			}
		}
		
		case MenuAction_Cancel:
		{
			if(iParam2 == MenuCancel_ExitBack)
			{
				char sKey[128], sData[2][128];
				hMenu.GetItem(0, sKey, sizeof(sKey));
				ExplodeString(sKey, ";", sData, sizeof(sData), sizeof(sData[]));
				
				ShowWeaponSkins(iParam1, sData[0]);
			}
		}
	}
	
	return 0;
}

void ShowSkinsMarket(int iClient)
{
	g_hMarketMenu.SetTitle("%T", "SkinsMarketTitle", iClient, PLUGIN_VERSION, GetBalance(iClient), GetTotalSkinsCount(iClient), GetTotalGlovesCount(iClient));
	g_hMarketMenu.Display(iClient, MENU_TIME_FOREVER);
}

void ShowGlovesMarket(int iClient)
{
	g_hGlovesMarketMenu.SetTitle("%T", "SkinsMarketTitle", iClient, PLUGIN_VERSION, GetBalance(iClient), GetTotalSkinsCount(iClient), GetTotalGlovesCount(iClient));
	g_hGlovesMarketMenu.Display(iClient, MENU_TIME_FOREVER);
}

int Market_Handler(Menu hMenu, MenuAction iAction, int iClient, int iParam2)
{
	switch(iAction)
	{
		case MenuAction_Select:
		{
			g_hMarketItemMenu[iParam2].SetTitle("%T", "SkinsMarketTitle", iClient, PLUGIN_VERSION, iClient, PLUGIN_VERSION, GetBalance(iClient), GetTotalSkinsCount(iClient), GetTotalGlovesCount(iClient));
			g_hMarketItemMenu[iParam2].Display(iClient, MENU_TIME_FOREVER);
		}
		
		case MenuAction_Cancel:
		{
			if(iParam2 == MenuCancel_ExitBack)
			{
				ShowMainMenu(iClient);
			}
		}
	}
	
	return 0;
}

int GlovesMarket_Handler(Menu hMenu, MenuAction iAction, int iClient, int iParam2)
{
	switch(iAction)
	{
		case MenuAction_Select:
		{
			g_hGloveMarketItemMenu[iParam2].SetTitle("%T", "SkinsMarketTitle", iClient, PLUGIN_VERSION, GetBalance(iClient), GetTotalSkinsCount(iClient), GetTotalGlovesCount(iClient));
			g_hGloveMarketItemMenu[iParam2].Display(iClient, MENU_TIME_FOREVER);
		}
		
		case MenuAction_Cancel:
		{
			if(iParam2 == MenuCancel_ExitBack)
			{
				ShowMainMenu(iClient);
			}
		}
	}
	
	return 0;
}

int MarketSkinItem_Handler(Menu hMenu, MenuAction iAction, int iClient, int iParam2)
{
	switch(iAction)
	{
		case MenuAction_DrawItem:
		{
			enum_skin hWeaponSkin;
			
			char sInfo[32];
			hMenu.GetItem(iParam2, sInfo, sizeof(sInfo));
			
			int iSkinId = StringToInt(sInfo);
			
			if(g_hSkins.GetArray(iSkinId, hWeaponSkin))
			{
				if(hWeaponSkin.fPrice == 0.0 || GetBalance(iClient) < hWeaponSkin.fPrice || ClientHasSkin(iClient, iSkinId, hWeaponSkin.iWeaponIndex))
				{
					return ITEMDRAW_DISABLED;
				}
			}
			
			return ITEMDRAW_DEFAULT;
		}
		
		case MenuAction_Select:
		{
			char sKey[32];
			hMenu.GetItem(iParam2, sKey, sizeof(sKey));
			
			int iItemId = StringToInt(sKey);
			
			enum_skin hItem;
			g_hSkins.GetArray(iItemId, hItem);
			
			if(hItem.iRarity > 3)
			{
				CPrintToChatAll("%T", "SkinBoughtAll", iClient, iClient, g_sItemRarity[hItem.iRarity][1], hItem.szName, hItem.fPrice);
			}
			else
			{
				CPrintToChat(iClient, "%T", "SkinBought", iClient, g_sItemRarity[hItem.iRarity][1], hItem.szName, hItem.fPrice);
			}
			
			DB_InsertNewSkin(iClient, iItemId);
			AddSkinToInventory(iClient, iItemId, hItem.iWeaponIndex);
			ClientCommand(iClient, "play %s", g_sRaritySounds[hItem.iRarity]);
			
			SetBalance(iClient, GetBalance(iClient) - hItem.fPrice);
			
			hMenu.DisplayAt(iClient, hMenu.Selection, MENU_TIME_FOREVER);
		}
		
		case MenuAction_Cancel:
		{
			if(iParam2 == MenuCancel_ExitBack)
			{
				ShowSkinsMarket(iClient);
			}
		}
	}		
	
	return ITEMDRAW_DEFAULT;
}

int MarketGlovesItem_Handler(Menu hMenu, MenuAction iAction, int iClient, int iParam2)
{
	switch(iAction)
	{
		case MenuAction_DrawItem:
		{
			enum_glove hGloves;
			
			char sInfo[32];
			hMenu.GetItem(iParam2, sInfo, sizeof(sInfo));
			
			int iSkinId = StringToInt(sInfo);
			
			if(iSkinId == -1)
			{
				return ITEMDRAW_DISABLED;
			}
			
			int iLength = g_hGloves.Length;
			
			if(iSkinId < iLength && g_hGloves.GetArray(iSkinId, hGloves))
			{
				if(hGloves.fPrice == 0.0 || GetBalance(iClient) < hGloves.fPrice || ClientHasGlove(iClient, iSkinId, hGloves.iType))
				{
					return ITEMDRAW_DISABLED;
				}
			}
			
			return ITEMDRAW_DEFAULT;
		}
		
		case MenuAction_Select:
		{
			char sKey[32], sGloveFullName[64];
			hMenu.GetItem(iParam2, sKey, sizeof(sKey));
			
			int iItemId = StringToInt(sKey);
			
			enum_glove hGloves;
			g_hGloves.GetArray(iItemId, hGloves);
			
			Format(sGloveFullName, sizeof(sGloveFullName), "%s | %s", g_sGlovesType[hGloves.iType], hGloves.sName);
			CPrintToChatAll("%T", "SkinBoughtAll", iClient, iClient, "\x09", sGloveFullName, hGloves.fPrice);
			
			DB_InsertNewSkin(iClient, iItemId, Type_Gloves);
			AddGloveToInventory(iClient, iItemId, hGloves.iType);
			ClientCommand(iClient, "play %s", g_sRaritySounds[7]);
			
			SetBalance(iClient, GetBalance(iClient) - hGloves.fPrice);
			
			hMenu.DisplayAt(iClient, hMenu.Selection, MENU_TIME_FOREVER);
		}
		
		case MenuAction_Cancel:
		{
			if(iParam2 == MenuCancel_ExitBack)
			{
				ShowGlovesMarket(iClient);
			}
		}
	}
	
	return ITEMDRAW_DEFAULT;
}

void ShowGlovesInventory(int iClient)
{
	Menu hMenu = new Menu(GlovesTypesMenu_Handler);
	hMenu.SetTitle("%T", "InventoryMainMenu", iClient, PLUGIN_VERSION, GetBalance(iClient), GetTotalSkinsCount(iClient), GetTotalGlovesCount(iClient)); 
	
	char sItem[64];
	
	int iCount;
	
	Format(sItem, sizeof(sItem), "%T", "UnEquiptT", iClient);
	hMenu.AddItem("uneq_t", sItem);			
	
	Format(sItem, sizeof(sItem), "%T", "UnEquiptCT", iClient);
	hMenu.AddItem("uneq_ct", sItem);			

	for(int iGloveType = 0; iGloveType < sizeof(g_sGlovesType); iGloveType++)
	{
		iCount = GetGlovesCount(iClient, iGloveType);
		
		if(iCount > 0)
		{
			Format(sItem, sizeof(sItem), "%T (%i)", g_sGlovesType[iGloveType], iClient, iCount);
			hMenu.AddItem(g_sGlovesType[iGloveType], sItem);			
		}
	}
	
	if(hMenu.ItemCount == 2)
	{
		Format(sItem, sizeof(sItem), "%T", "NoGloves", iClient);
		hMenu.AddItem("", sItem, ITEMDRAW_DISABLED);			
	}
	
	hMenu.ExitBackButton = true;
	hMenu.Display(iClient, MENU_TIME_FOREVER);
}

int GlovesTypesMenu_Handler(Menu hMenu, MenuAction iAction, int iParam1, int iParam2)
{
	switch(iAction)
	{
		case MenuAction_End:
		{
			if(iParam1 != MenuEnd_Selected)
			{
				delete hMenu;
			}
		}
		case MenuAction_Select:
		{
			int iClient = iParam1;
			
			enum_glove hGlove;
			
			char sGloveType[64], sInfo[32];
			hMenu.GetItem(iParam2, sGloveType, sizeof(sGloveType));
			
			if(StrEqual(sGloveType, "uneq_t"))
			{
				Gloves_SetClientGloves(iClient, CS_TEAM_T, 0, 0);
				
				hMenu.SetTitle("%T", "InventoryMainMenu", iClient, PLUGIN_VERSION, GetBalance(iClient), GetTotalSkinsCount(iClient), GetTotalGlovesCount(iClient)); 
				hMenu.Display(iClient, MENU_TIME_FOREVER);
			}
			else if(StrEqual(sGloveType, "uneq_ct")) 
			{
				Gloves_SetClientGloves(iClient, CS_TEAM_CT, 0, 0);
				
				hMenu.SetTitle("%T", "InventoryMainMenu", iClient, PLUGIN_VERSION, GetBalance(iClient), GetTotalSkinsCount(iClient), GetTotalGlovesCount(iClient)); 
				hMenu.Display(iClient, MENU_TIME_FOREVER);
			}
			else
			{
				delete hMenu;
				
				Menu hGlovesMenu = new Menu(GlovesList_Handler);
				hGlovesMenu.ExitBackButton = true;
				hGlovesMenu.SetTitle("%T", "GlovesMenu", iClient, PLUGIN_VERSION, GetBalance(iClient), GetTotalSkinsCount(iClient), sGloveType, GetTotalGlovesCount(iClient)); 
				
				ArrayList hGlovesList;
				g_hInventory[iClient].GetValue(sGloveType, hGlovesList);
				
				int iLen = hGlovesList.Length, iGloveId;
				for(int iGlove = 0; iGlove < iLen; iGlove++)
				{
					iGloveId = hGlovesList.Get(iGlove);
					g_hGloves.GetArray(iGloveId, hGlove);
					
					Format(sInfo, sizeof(sInfo), "%i", iGloveId);
					hGlovesMenu.AddItem(sInfo, hGlove.sName);
				}
				
				hGlovesMenu.Display(iClient, MENU_TIME_FOREVER);				
			}
		}
		case MenuAction_Cancel:
		{
			if(iParam2 == MenuCancel_ExitBack)
			{
				ShowInventory(iParam1);
			}
		}
	}
}

void ShowGlovesByType(int iClient, const char[] sType, int iStart = 0)
{
	enum_glove hGlove;
	
	char sInfo[64];
	
	Menu hGlovesMenu = new Menu(GlovesList_Handler);
	hGlovesMenu.ExitBackButton = true;
	hGlovesMenu.SetTitle("%T", "GlovesMenu", iClient, PLUGIN_VERSION, GetBalance(iClient), GetTotalSkinsCount(iClient), sType, GetTotalGlovesCount(iClient)); 
	
	ArrayList hGlovesList;
	g_hInventory[iClient].GetValue(sType, hGlovesList);
	
	int iLen = hGlovesList.Length, iGloveId;
	for(int iGlove = 0; iGlove < iLen; iGlove++)
	{
		iGloveId = hGlovesList.Get(iGlove);
		g_hGloves.GetArray(iGloveId, hGlove);
		
		Format(sInfo, sizeof(sInfo), "%i", iGloveId);
		hGlovesMenu.AddItem(sInfo, hGlove.sName);
	}
	
	hGlovesMenu.DisplayAt(iClient, iStart, MENU_TIME_FOREVER);		
}

int GlovesList_Handler(Menu hMenu, MenuAction iAction, int iClient, int iParam2)
{
	switch(iAction)
	{
		case MenuAction_End:delete hMenu;
		case MenuAction_Select:
		{
			enum_glove hGlove;
			
			char sInfo[32], sItem[64];
			hMenu.GetItem(iParam2, sInfo, sizeof(sInfo));
			
			float fBalance = GetBalance(iClient);
			
			int iGlove = StringToInt(sInfo);
			g_hGloves.GetArray(iGlove, hGlove);
			
			Format(sInfo, sizeof(sInfo), "%i", iGlove);
			
			Menu hGloveMenu = new Menu(GloveMenu_Handler, MENU_ACTIONS_ALL);
			hGloveMenu.SetTitle("%T", "ConfigureGlove", iClient, PLUGIN_VERSION, GetBalance(iClient), GetTotalSkinsCount(iClient), GetTotalGlovesCount(iClient), hGlove.sType, hGlove.sType, hGlove.sName); 
			hGloveMenu.ExitBackButton = true;
			
			Format(sItem, sizeof(sItem), "%T", "EquiptT", iClient, hGlove.sName);
			hGloveMenu.AddItem(sInfo, sItem);
			
			Format(sItem, sizeof(sItem), "%T", "EquiptCT", iClient, hGlove.sName);
			hGloveMenu.AddItem(sInfo, sItem);
				
			Format(sItem, sizeof(sItem), "%T", "SetFloatGloves", iClient, g_fFloatPrice);
			hGloveMenu.AddItem(sInfo, sItem, fBalance < g_fFloatPrice ? ITEMDRAW_DISABLED:ITEMDRAW_DEFAULT);
			
			if(g_bQuickSellGloves)
			{
				if(hGlove.fPrice != 0.0)
				{
					Format(sItem, sizeof(sItem), "%T", "QuickSell", iClient, hGlove.fPrice);
					hGloveMenu.AddItem(sInfo, sItem);
				}
				else
				{
					Format(sItem, sizeof(sItem), "%T", "QuickSell_NoPrice", iClient);
					hGloveMenu.AddItem(sInfo, sItem, ITEMDRAW_DISABLED);
				}				
			}
			
			hGloveMenu.Display(iClient, MENU_TIME_FOREVER);
		}		
		case MenuAction_Cancel:
		{
			if(iParam2 == MenuCancel_ExitBack)
			{
				ShowGlovesInventory(iClient);
			}
		}
	}
}

int GloveMenu_Handler(Menu hMenu, MenuAction iAction, int iParam1, int iParam2)
{
	switch(iAction)
	{
		case MenuAction_End:
		{
			if(iParam1 != MenuEnd_Selected)
			{
				delete hMenu;
			}
		}
		case MenuAction_DrawItem:
		{
			float fBalance = GetBalance(iParam1);
			
			switch(iParam2)
			{
				case 2: // Set float
				{
					return fBalance < g_fFloatPrice ? ITEMDRAW_DISABLED:ITEMDRAW_DEFAULT;
				}
			}
			return ITEMDRAW_DEFAULT;
		}
		case MenuAction_Select:
		{
			enum_glove hGlove;
			
			char sInfo[32];
			hMenu.GetItem(iParam2, sInfo, sizeof(sInfo));
			
			int iGlove = StringToInt(sInfo);
			g_hGloves.GetArray(iGlove, hGlove);
			
			int iClient = iParam1;
			
			switch(iParam2)
			{
				case 0: // Equipt T
				{
					Gloves_SetClientGloves(iClient, CS_TEAM_T, hGlove.iDefIndex, hGlove.iIndex);
				}
				
				case 1: // Equipt CT
				{
					Gloves_SetClientGloves(iClient, CS_TEAM_CT, hGlove.iDefIndex, hGlove.iIndex);
				}
				
				case 2: // Set Float
				{
					g_bWaitingForFloatGloves[iClient] = true;
					CPrintToChat(iClient, "%T", "FloatInChat", iClient);
					strcopy(g_sTempWeapon[iClient], sizeof(g_sTempWeapon[]), sInfo);
					SetBalance(iClient, GetBalance(iClient) - g_fFloatPrice);
				}
				
				case 3: // Quick Sell
				{
					ArrayList hGlovesList;
					g_hInventory[iClient].GetValue(hGlove.sType, hGlovesList);
					
					int iIndex = hGlovesList.FindValue(iGlove);
					if(iIndex != -1)
					{
						hGlovesList.Erase(iIndex);
					}
					
					char sGloveFullName[64];
					Format(sGloveFullName, sizeof(sGloveFullName), "%s | %s", g_sGlovesType[hGlove.iType], hGlove.sName);
					
					CPrintToChat(iClient, "%T", "SkinSold", iClient, "\x09", sGloveFullName, hGlove.fPrice);
					DB_DeleteSkin(GetSteamAccountID(iClient), iGlove, Type_Gloves);
					
					if(Gloves_GetGloves(iClient, CS_TEAM_CT) == hGlove.iIndex && Gloves_GetGlovesType(iClient, CS_TEAM_CT) == hGlove.iDefIndex)
					{
						Gloves_SetClientGloves(iClient, CS_TEAM_CT, 0, 0);
					}
					
					if(Gloves_GetGloves(iClient, CS_TEAM_T) == hGlove.iIndex && Gloves_GetGlovesType(iClient, CS_TEAM_T) == hGlove.iDefIndex)
					{
						Gloves_SetClientGloves(iClient, CS_TEAM_T, 0, 0);
					}
					
					SetBalance(iClient, GetBalance(iClient) + hGlove.fPrice);
				}
			}
			
			if(iParam2 != 3)
			{
				hMenu.SetTitle("%T", "ConfigureGlove", iClient, PLUGIN_VERSION, GetBalance(iClient), GetTotalSkinsCount(iClient), GetTotalGlovesCount(iClient), hGlove.sType, hGlove.sType, hGlove.sName); 
				hMenu.Display(iClient, MENU_TIME_FOREVER);
			}
			else
			{
				ShowGlovesByType(iClient, hGlove.sType, hMenu.Selection);
				delete hMenu;
			}
		}
		
		case MenuAction_Cancel:
		{	
			if(iParam2 == MenuCancel_ExitBack)
			{
				enum_glove hGlove;
			
				char sInfo[32];
				hMenu.GetItem(0, sInfo, sizeof(sInfo));
				
				int iGlove = StringToInt(sInfo);
				g_hGloves.GetArray(iGlove, hGlove);
				
				ShowGlovesByType(iParam1, hGlove.sType);
			}			
		}
	}

	return 0;
}