// Loads the skins items from a config files and store them in StringMap based on the weapon classname
void LoadItems()
{
	char szConfigPath[PLATFORM_MAX_PATH];
	BuildPath(Path_SM, szConfigPath, sizeof(szConfigPath), "configs/cases/skins.cfg"); // Build the config file path
	
	KeyValues hKeyValues = new KeyValues("Skins");
	if(hKeyValues.ImportFromFile(szConfigPath))
	{
		if(hKeyValues.GotoFirstSubKey()) // Go to the first case and begin reading it
		{
			char szSkinName[64], szSkinId[32];
			int iDefIndex, iSkinIndex, iRarity;
			float fPrice;
			enum_skin pWeaponSkin;
			
			g_hSkins = new ArrayList(sizeof(pWeaponSkin)); // Create the list of the skins and begin reading the config
			
			for(int iWeaponIndex = 0; iWeaponIndex < sizeof(g_iWeaponDefIndex); iWeaponIndex++)
			{
				delete g_hMarketItemMenu[iWeaponIndex];
				g_hMarketItemMenu[iWeaponIndex] = new Menu(MarketSkinItem_Handler, MENU_ACTIONS_ALL);
				g_hMarketItemMenu[iWeaponIndex].ExitBackButton = true;
			}
			
			do
			{
				hKeyValues.GetString("name", szSkinName, sizeof(szSkinName));
				
				iDefIndex = hKeyValues.GetNum("weapon_index");
				iSkinIndex = hKeyValues.GetNum("skin_index");
				iRarity = hKeyValues.GetNum("rarity");
				fPrice = hKeyValues.GetFloat("price_market") * g_fPriceMultiplier;
				
				pWeaponSkin.iWeaponIndex = iDefIndex;
				pWeaponSkin.iSkinIndex = iSkinIndex;
				pWeaponSkin.iRarity = iRarity;
				pWeaponSkin.fPrice = fPrice;
				strcopy(pWeaponSkin.szName, sizeof(enum_skin::szName), szSkinName);
				
				g_hSkins.PushArray(pWeaponSkin); // Add the skin data to the list
				
				hKeyValues.GetSectionName(szSkinId, sizeof(szSkinId));
				
				if(pWeaponSkin.fPrice > 0.0)
				{
					Format(szSkinName, sizeof(szSkinName), "%T", "BuySkinItem", LANG_SERVER, szSkinName, pWeaponSkin.fPrice);
				}
				else
				{
					Format(szSkinName, sizeof(szSkinName), "%T", "BuySkinItem_NoPrice", LANG_SERVER, szSkinName);
				}
				g_hMarketItemMenu[iDefIndex].AddItem(szSkinId, szSkinName);
			}
			while (hKeyValues.GotoNextKey());
		}
		else
		{
			SetFailState("Failed to load the skins from \"%s\".", szConfigPath);
		}
	}
	else
	{
		SetFailState("Failed to load the skins from \"%s\".", szConfigPath);
	}
	
	LogMessage("Loaded %i skins.", g_hSkins.Length);
	delete hKeyValues;
}

// Loads the cases from a config and store them in an ArrayList.
void LoadCases()
{
	char szConfigPath[PLATFORM_MAX_PATH];
	BuildPath(Path_SM, szConfigPath, sizeof(szConfigPath), "configs/cases/cases.cfg"); // Build the config file path

	KeyValues hKeyValues = new KeyValues("Cases");
	if(hKeyValues.ImportFromFile(szConfigPath))
	{
		if(hKeyValues.GotoFirstSubKey()) // Go to the first case and begin reading it
		{
			char szCaseName[32], szKey[12], szSkinData[64];
			int nItems;
			ArrayList hCaseSkins;
			
			g_hCasesNames = new ArrayList(ByteCountToCells(64)); // Create the list of the cases names
			g_hCasesPrices = new ArrayList(); // Create the list of the cases prices
			g_hCasesSkins = new ArrayList(); // Create the list that will hold hCaseSkins(all the skins);
			g_hCasesReturns = new ArrayList(); // This holds the return value for cases
			
			do{
				hKeyValues.GetSectionName(szCaseName, sizeof(szCaseName)); // Get the case name
				g_hCasesNames.PushString(szCaseName); // Save the case name
				
				g_hCasesPrices.Push(hKeyValues.GetFloat("price")); // Get the case price
				g_hCasesReturns.Push(hKeyValues.GetFloat("return")); // Get the case return value
				
				if(hKeyValues.JumpToKey("Skins")) // Go to the skins list and read them
				{
					hCaseSkins = new ArrayList(ByteCountToCells(64)); // Create a new list of skins data
					
					do{
						nItems = 0;
						
						do 
						{
							Format(szKey, sizeof(szKey), "%i", nItems++);
							hKeyValues.GetString(szKey, szSkinData, sizeof(szSkinData), "no-skin"); // Get the skin data
							
							if(StrEqual(szSkinData, "no-skin"))
							{
								break;
							}
							else
							{
								hCaseSkins.PushString(szSkinData);
							}
						}
						while (nItems > -1); // This would make an infinite loop, but we break it inside
					}
					while (hKeyValues.GotoNextKey());
					g_hCasesSkins.Push(hCaseSkins);					
				}
				else
				{
					SetFailState("Failed to read skins for \"%s\" case.", szCaseName);
				}
			}
			while (hKeyValues.GoBack() && hKeyValues.GotoNextKey());
		}
		else
		{
			SetFailState("Failed to load the cases from \"%s\".", szConfigPath);
		}
	}
	else
	{
		SetFailState("Failed to load the cases from \"%s\".", szConfigPath);
	}
	
	LogMessage("Loaded %i cases.", g_hCasesNames.Length);
	delete hKeyValues;
}

void CreateMenus()
{
	delete g_hMarketMenu;
	g_hMarketMenu = new Menu(Market_Handler, MENU_ACTIONS_ALL);
	g_hMarketMenu.ExitBackButton = true;
	
	char sItem[64];
	
	for(int iWeaponIndex = 0; iWeaponIndex < sizeof(g_sWeaponClasses); iWeaponIndex++)
	{
		Format(sItem, sizeof(sItem), "%T", g_sWeaponClasses[iWeaponIndex], LANG_SERVER);
		g_hMarketMenu.AddItem("", sItem);
	}
	
	delete g_hGlovesMarketMenu;
	g_hGlovesMarketMenu = new Menu(GlovesMarket_Handler);
	g_hGlovesMarketMenu.ExitBackButton = true;
	
	for(int iGloveType = 0; iGloveType < sizeof(g_sGlovesType); iGloveType++)
	{
		Format(sItem, sizeof(sItem), "%T", g_sGlovesType[iGloveType], LANG_SERVER);
		g_hGlovesMarketMenu.AddItem("", sItem);
	}
}

void LoadDrops()
{
	char szConfigPath[PLATFORM_MAX_PATH];
	BuildPath(Path_SM, szConfigPath, sizeof(szConfigPath), "configs/cases/drops.cfg"); // Build the config file path

	KeyValues hKeyValues = new KeyValues("Drops");
	if(hKeyValues.ImportFromFile(szConfigPath))
	{
		delete g_hDropsSkins;
		g_hDropsSkins = new ArrayList(ByteCountToCells(64)); // Create a new list of skins data
			
		char szKey[12], szSkinData[64];
		int nItems;
		
		do 
		{
			Format(szKey, sizeof(szKey), "%i", nItems++);
			hKeyValues.GetString(szKey, szSkinData, sizeof(szSkinData), "no-skin"); // Get the skin data
			
			if(StrEqual(szSkinData, "no-skin"))
			{
				break;
			}
			else
			{
				g_hDropsSkins.PushString(szSkinData);
			}
		}
		while (nItems > -1); // This would make an infinite loop, but we break it inside			
	}
	else
	{
		SetFailState("Failed to load the drops from \"%s\".", szConfigPath);
	}
	
	LogMessage("Loaded %i drop skins.", g_hDropsSkins.Length);
	delete hKeyValues;
}

void LoadGloves()
{
	char szConfigPath[PLATFORM_MAX_PATH];
	BuildPath(Path_SM, szConfigPath, sizeof(szConfigPath), "configs/cases/gloves.cfg"); // Build the config file path
	
	KeyValues hKeyValues = new KeyValues("Gloves");
	if(hKeyValues.ImportFromFile(szConfigPath))
	{
		if(hKeyValues.GotoFirstSubKey()) // Go to the gloves type
		{
			enum_glove hGloves;
			
			char sGloveName[64], sGloveType[64], sSkinId[128];
			
			float fPrice;
			
			int iIndex;
			int iDefIndex;
			
			g_hGloves = new ArrayList(sizeof(enum_glove));
			
			for(int iType = 0; iType < sizeof(g_sGlovesType); iType++)
			{
				delete g_hGloveMarketItemMenu[iType];
				g_hGloveMarketItemMenu[iType] = new Menu(MarketGlovesItem_Handler, MENU_ACTIONS_ALL);
				g_hGloveMarketItemMenu[iType].ExitBackButton = true;
			}
			
			do
			{
				hKeyValues.GetString("name", sGloveName, sizeof(sGloveName));
				hKeyValues.GetString("type", sGloveType, sizeof(sGloveType));
				
				fPrice = hKeyValues.GetFloat("price_market");
				
				iIndex = hKeyValues.GetNum("index");
				iDefIndex =  hKeyValues.GetNum("defindex");
				
				strcopy(hGloves.sName, sizeof(enum_glove::sName), sGloveName);
				strcopy(hGloves.sType, sizeof(enum_glove::sType), sGloveType);
				
				hGloves.fPrice = fPrice;
				
				hGloves.iIndex = iIndex;
				hGloves.iType = GetGloveTypeNum(sGloveType);
				hGloves.iDefIndex = iDefIndex;
				
				if(hGloves.iType == -1)
				{
					LogError("\"%s\" has a invalid type", sGloveName);
				}
				
				g_hGloves.PushArray(hGloves);
				
				if(hGloves.fPrice > 0.0)
				{
					Format(sGloveName, sizeof(sGloveName), "%T", "BuySkinItem", LANG_SERVER, sGloveName, hGloves.fPrice);
				}
				else
				{
					Format(sGloveName, sizeof(sGloveName), "%T", "BuySkinItem_NoPrice", LANG_SERVER, sGloveName);
				}
				
				hKeyValues.GetSectionName(sSkinId, sizeof(sSkinId));
				g_hGloveMarketItemMenu[hGloves.iType].AddItem(sSkinId, sGloveName);
			}
			while (hKeyValues.GotoNextKey());
		}
		
		for(int iGloveType = 0; iGloveType < sizeof(g_sGlovesType); iGloveType++)
		{
			if(g_hGloveMarketItemMenu[iGloveType].ItemCount == 0)
			{
				if(g_hGlovesMarketMenu.RemoveItem(iGloveType))
				{
					iGloveType--;
				}
			}
		}
	}
}