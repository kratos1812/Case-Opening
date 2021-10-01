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
			char szSkinName[64];
			int iDefIndex, iSkinIndex, iRarity, iPrice;
			enum_skin pWeaponSkin;
			
			g_hSkins = new ArrayList(sizeof(pWeaponSkin)); // Create the list of the skins and begin reading the config
			
			do
			{
				hKeyValues.GetString("name", szSkinName, sizeof(szSkinName));
				
				iDefIndex = hKeyValues.GetNum("weapon_index");
				iSkinIndex = hKeyValues.GetNum("skin_index");
				iRarity = hKeyValues.GetNum("rarity");
				iPrice = hKeyValues.GetNum("price");
				
				pWeaponSkin.iWeaponIndex = iDefIndex;
				pWeaponSkin.iSkinIndex = iSkinIndex;
				pWeaponSkin.iRarity = iRarity;
				pWeaponSkin.iPrice = iPrice;
				strcopy(pWeaponSkin.szName, sizeof(enum_skin::szName), szSkinName);
				
				g_hSkins.PushArray(pWeaponSkin); // Add the skin data to the list
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
				
				g_hCasesPrices.Push(hKeyValues.GetNum("price")); // Get the case price
				g_hCasesReturns.Push(hKeyValues.GetNum("return")); // Get the case return value
				
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
	// DebugCases();
}

void CreateMenus()
{
	delete g_hMarketMenu;
	g_hMarketMenu = new Menu(Market_Handler, MENU_ACTIONS_ALL);
	g_hMarketMenu.ExitBackButton = true;
	
	enum_skin hItem;
	char sKey[10], sItem[64];
	
	for(int iItemId = 0; iItemId < g_hSkins.Length; iItemId++)
	{
		g_hSkins.GetArray(iItemId, hItem);
		
		FormatEx(sKey, sizeof(sKey), "%i", iItemId);
		FormatEx(sItem, sizeof(sItem), "%T", "SkinsMarketItem", LANG_SERVER, hItem.szName, hItem.iPrice);
		
		g_hMarketMenu.AddItem(sKey, sItem);
	}
}