#include <sourcemod>
#include <sdktools>
#include <eItems>
#include <profiler>
#include <ripext>

#pragma semicolon 1
#pragma newdecls required

#define PLUGIN_VERSION "0.0.2"

public Plugin myinfo = 
{
	name = "[OPTIONAL] Case Opening Config Generator",
	author = "kRatoss",
	description = "Creates a config for kRatoss' CaseOpening plugin with the latest skins and gloves",
	version = PLUGIN_VERSION,
	url = "http://steamcommunity.com/profiles/76561198090457091/"
};

char 
	g_sFilters[][] = {" (Factory New)", " (Minimal Wear)", " (Field-Tested)", " (Well-Worn)", " (Battle-Scarred)", "StatTrak™ ", "★ ", "Souvenir "};

int g_iWeaponDefIndex[] = {
/* 0*/ 9, /* 1*/ 7, /* 2*/ 16, /* 3*/ 60, /* 4*/ 1, /* 5*/ 61, /* 6*/ 32, /* 7*/ 4, /* 8*/ 2, 
/* 9*/ 36, /*10*/ 63, /*11*/ 3, /*12*/ 30, /*13*/ 64, /*14*/ 35, /*15*/ 25, /*16*/ 27, /*17*/ 29, 
/*18*/ 14, /*19*/ 28, /*20*/ 34, /*21*/ 17, /*22*/ 33, /*23*/ 24, /*24*/ 19, /*25*/ 26, /*26*/ 10, /*27*/ 13, 
/*28*/ 40, /*29*/ 8, /*30*/ 39, /*31*/ 38, /*32*/ 11, /*33*/ 507, /*34*/ 508, /*35*/ 500, 
/*36*/ 514, /*37*/ 515, /*38*/ 505, /*39*/ 516, /*40*/ 509, /*41*/ 512, /*42*/ 506,
/*43*/ 519, /*44*/ 520, /*45*/ 522, /*46*/ 523, /*47*/ 23, /*48*/ 503, /*49*/ 517,
/*50*/ 518, /*51*/ 521, /*52*/ 525
};

StringMap 
	g_hWeaponIndex,
	g_hWeaponNames;
	
KeyValues 
	g_hSkinsKv,
	g_hGlovesKv;

public void OnPluginStart()
{
	delete g_hWeaponIndex; // delete old handles, might not be required?
	g_hWeaponIndex = new StringMap();
	
	delete g_hWeaponNames;
	g_hWeaponNames = new StringMap();
	
	char m_sKey[32];
	for(int m_iIter = 0; m_iIter < sizeof(g_iWeaponDefIndex); m_iIter++)
	{
		FormatEx(m_sKey, sizeof(m_sKey), "%d", g_iWeaponDefIndex[m_iIter]);
		g_hWeaponIndex.SetValue(m_sKey, m_iIter);
	}
}

public void OnMapStart()
{
	if(eItems_AreItemsSynced())
	{
		eItems_OnItemsSynced();
	}
}

public void eItems_OnItemsSynced()
{
	Profiler m_hProfiler = new Profiler();
	m_hProfiler.Start();
	
	LoadSkins();
	LoadGloves();
	
	m_hProfiler.Stop();
	LogMessage("Loaded skins and gloves from the game. Duration: %f", m_hProfiler.Time);
	delete m_hProfiler;
	
	HTTPRequest hMarketRequest = new HTTPRequest("https://csgobackpack.net/api/GetItemsList/v2/");
	hMarketRequest.Get(HTTPRequest_OnPriceRetrieved, 0);
}

void HTTPRequest_OnPriceRetrieved(HTTPResponse m_hResponse, int m_iNullData)
{
	Profiler m_hProfiler = new Profiler();
	m_hProfiler.Start();
	
	if (m_hResponse.Status != HTTPStatus_OK) 
	{
		LogMessage("HTTPRequest_OnPriceRetrieved Failed #1. Status: %i ", m_hResponse.Status);
		return;
	}
	
	JSONObject m_hObject = view_as<JSONObject>(m_hResponse.Data), m_hItemsList = view_as<JSONObject>(m_hObject.Get("items_list")), m_hItemProfile, m_hItemPriceList; 	
	JSONObjectKeys m_hKeys = m_hItemsList.Keys();

	if(!m_hObject.GetBool("success"))
	{
		LogMessage("HTTPRequest_OnPriceRetrieved Failed #2.");
		return;
	}
	
	StringMap m_hScannedItems = new StringMap();
	
	int m_iDummy, m_iType;
	
	float m_fPrice;
	
	char m_sSectionName[256], m_sTempBuffer[32];
	while (m_hKeys.ReadKey(m_sSectionName, sizeof(m_sSectionName))) 
	{
		delete m_hItemProfile;
		m_hItemProfile = view_as<JSONObject>(m_hItemsList.Get(m_sSectionName));
		
		ReplaceString(m_sSectionName, sizeof(m_sSectionName), "&#39", "'", false);
		ReplaceString(m_sSectionName, sizeof(m_sSectionName), "%27", "'", false);
		for(int m_iFilter = 0; m_iFilter < sizeof(g_sFilters); m_iFilter++)
		{
			ReplaceString(m_sSectionName, sizeof(m_sSectionName), g_sFilters[m_iFilter], "", false);
		}
		
		if (m_hScannedItems.GetValue(m_sSectionName, m_iDummy) && m_iDummy == 1)
		{
			m_iDummy = 0;
			continue;
		}
		m_hScannedItems.SetValue(m_sSectionName, true);
		
		if(!m_hItemProfile.HasKey("type"))
		{
			LogMessage("Item \"%s\" has no \"type\" key. Skipping..", m_sSectionName);
			continue;
		}
		
		m_iType = 0;
		if(!m_hItemProfile.GetString("type", m_sTempBuffer, sizeof(m_sTempBuffer)))
		{
			continue;
		}
		
		if(strcmp(m_sTempBuffer, "Weapon") == 0)
		{
			m_iType = 1;
		}
		
		else if(strcmp(m_sTempBuffer, "Gloves") == 0)
		{
			m_iType = 2;
		}
		
		if(m_iType == 0)
		{
			continue;
		}
		
		if(!m_hItemProfile.HasKey("price"))
		{
			LogMessage("Item \"%s\" has no \"price\" key!", m_sSectionName);
			continue;
		}
		
		delete m_hItemPriceList;
		m_hItemPriceList = view_as<JSONObject>(m_hItemProfile.Get("price"));
		
		if(m_hItemPriceList.HasKey("24_hours"))
		{
			m_hItemPriceList = view_as<JSONObject>(m_hItemPriceList.Get("24_hours"));
		}
		else if(m_hItemPriceList.HasKey("all_time"))
		{
			m_hItemPriceList = view_as<JSONObject>(m_hItemPriceList.Get("all_time"));
		}
		else
		{
			LogMessage("Failed to get \"24_hours\" or \"all_time\" for \"%s\".", m_sSectionName);
			continue;
		}
		
		// because ripext doesnt not have GetDataType
		m_fPrice = m_hItemPriceList.GetFloat("average");
		if(m_fPrice == 0.0)
		{
			m_iDummy = m_hItemPriceList.GetInt("average"); 
			m_fPrice = float(m_iDummy); 
			if(m_fPrice == 0.0)
			{
				// this is so dumb but really rare skins such as dragon lore and howl needs this
				m_hItemPriceList = view_as<JSONObject>(m_hItemProfile.Get("price"));
				if(m_hItemPriceList.HasKey("all_time"))
				{
					m_hItemPriceList = view_as<JSONObject>(m_hItemPriceList.Get("all_time"));
					m_fPrice = m_hItemPriceList.GetFloat("average");
					if(m_fPrice == 0.0)
					{
						m_iDummy = m_hItemPriceList.GetInt("average"); 
						m_fPrice = float(m_iDummy);
					}
				}
			}
		}
		
		if(m_fPrice == 0.0)
		{
			LogMessage("Failed to fetch price for \"%s\".", m_sSectionName);
			continue;
		}
		
		LoadPrice(m_fPrice, m_sSectionName, m_iType);
	}
	
	delete m_hScannedItems;
	
	FinishConfigs();
	
	m_hProfiler.Stop();
	LogMessage("Cached skins and gloves prices from CSGOBackPack. Duration: %f", m_hProfiler.Time);
	delete m_hProfiler;
}

void LoadGloves()
{
	KeyValues m_sKeyValues = new KeyValues("Gloves");
	
	char m_sKvKey[18], m_sConfigPath[PLATFORM_MAX_PATH], m_sGloveType[64], m_sGloveSkinName[32], m_sBuffer[64];
	
	int m_iGlovesCount = eItems_GetGlovesCount(), m_iPaintsCount = eItems_GetPaintsCount(), m_iGloveNum, m_iPaintNum, m_iKvSkinId;
	for(m_iPaintNum = 0; m_iPaintNum < m_iPaintsCount; m_iPaintNum++)
	{
		for(m_iGloveNum = 0; m_iGloveNum < m_iGlovesCount; m_iGloveNum++)
		{
			if (eItems_IsNativeSkin(m_iPaintNum, m_iGloveNum, ITEMTYPE_GLOVES))
			{
				FormatEx(m_sKvKey, sizeof(m_sKvKey), "%i", m_iKvSkinId++);
				
				if(m_sKeyValues.JumpToKey(m_sKvKey, true))
				{
					eItems_GetGlovesDisplayNameByGlovesNum(m_iGloveNum, m_sGloveType, sizeof(m_sGloveType));
					eItems_GetSkinDisplayNameBySkinNum(m_iPaintNum, m_sGloveSkinName, sizeof(m_sGloveSkinName));
					
					m_sKeyValues.SetString("name", m_sGloveSkinName);
					m_sKeyValues.SetString("type", m_sGloveType);
					
					m_sKeyValues.SetNum("index", eItems_GetGlovesDefIndexByGlovesNum(m_iGloveNum));
					m_sKeyValues.SetNum("defindex", eItems_GetSkinDefIndexBySkinNum(m_iPaintNum));
					m_sKeyValues.SetFloat("price_market", 0.0);
					
					m_sKeyValues.GoBack();
					
					FormatEx(m_sBuffer, sizeof(m_sBuffer), "%s | %s", m_sGloveType, m_sGloveSkinName);
					g_hWeaponNames.SetString(m_sBuffer, m_sKvKey);
				}
			}
		}		
	}
	
	BuildPath(Path_SM, m_sConfigPath, sizeof(m_sConfigPath), "configs/cases/auto_generated/g_gloves.cfg");
	if(!FileExists(m_sConfigPath))
	{
		char m_sFolderPath[PLATFORM_MAX_PATH];
		BuildPath(Path_SM, m_sFolderPath, sizeof(m_sFolderPath), "configs/cases/auto_generated");
		
		if(CreateDirectory(m_sFolderPath, 511))
		{
			LogMessage("Created \"%s\" directory.", m_sFolderPath);
		}
	}
	
	g_hGlovesKv = new KeyValues("Gloves");
	g_hGlovesKv.Import(m_sKeyValues);
	
	m_sKeyValues.ExportToFile(m_sConfigPath);
	delete m_sKeyValues;
}

void LoadSkins()
{
	KeyValues m_sKeyValues = new KeyValues("Skins");
	
	char m_sKvKey[18], m_sPaintDisplayName[32], m_sWeaponDisplayName[32], m_sBuffer[64], m_sConfigPath[PLATFORM_MAX_PATH], m_sDummy[64];
	
	int m_iPaintsCount = eItems_GetPaintsCount(), m_iWeaponsCount = eItems_GetWeaponCount(), m_iWeaponNum, m_iPaintNum, m_iKvSkinId;
	for(m_iPaintNum = 0; m_iPaintNum < m_iPaintsCount; m_iPaintNum++)
	{
		for (m_iWeaponNum = 0; m_iWeaponNum < m_iWeaponsCount; m_iWeaponNum++)
		{
			if (eItems_IsNativeSkin(m_iPaintNum, m_iWeaponNum, ITEMTYPE_WEAPON))
			{
				FormatEx(m_sKvKey, sizeof(m_sKvKey), "%i", m_iKvSkinId++);
				
				if(m_sKeyValues.JumpToKey(m_sKvKey, true))
				{
					eItems_GetSkinDisplayNameBySkinNum(m_iPaintNum, m_sPaintDisplayName, sizeof(m_sPaintDisplayName));
					eItems_GetWeaponDisplayNameByWeaponNum(m_iWeaponNum, m_sWeaponDisplayName, sizeof(m_sWeaponDisplayName));
					
					FormatEx(m_sBuffer, sizeof(m_sBuffer), "%s | %s", m_sWeaponDisplayName, m_sPaintDisplayName);
					
					// Comment here all ReplaceString()s to enable skin phases in config, but it will fail to load the price.
					ReplaceString(m_sBuffer, sizeof(m_sBuffer), " (Phase 1)", "", false);
					ReplaceString(m_sBuffer, sizeof(m_sBuffer), " (Phase 2)", "", false);
					ReplaceString(m_sBuffer, sizeof(m_sBuffer), " (Phase 3)", "", false);
					ReplaceString(m_sBuffer, sizeof(m_sBuffer), " (Phase 4)", "", false);
					ReplaceString(m_sBuffer, sizeof(m_sBuffer), " (Ruby)", "", false);
					ReplaceString(m_sBuffer, sizeof(m_sBuffer), " (Black Pearl)", "", false);
					ReplaceString(m_sBuffer, sizeof(m_sBuffer), " (Sapphire)", "", false);
					ReplaceString(m_sBuffer, sizeof(m_sBuffer), " (Emerald)", "", false);
					
					if(g_hWeaponNames.GetString(m_sBuffer, m_sDummy, sizeof(m_sDummy))) // this is because atm glock 18 gamma doppler has 5 different skins with the same name for whatever reason
					{
						m_iKvSkinId--;
						m_sKeyValues.GoBack();
						continue;
					}
					
					m_sKeyValues.SetString("name", m_sBuffer); 
					
					m_sKeyValues.SetNum("skin_index", eItems_GetSkinDefIndexBySkinNum(m_iPaintNum));
					m_sKeyValues.SetNum("weapon_index", GetWeaponIndexByDefIndex(eItems_GetWeaponDefIndexByWeaponNum(m_iWeaponNum)));
					m_sKeyValues.SetNum("rarity", (StrContains(m_sBuffer, "knife", false) != -1 || StrContains(m_sBuffer, "bayonet", false) != -1) ? 6 : eItems_GetSkinRarity(eItems_GetSkinDefIndexBySkinNum(m_iPaintNum)));
					m_sKeyValues.SetFloat("price_market", 0.0);
					
					m_sKeyValues.GoBack();
					
					/*
					-- Un-comment here to enable skin phases in config, but it will fail to load the price.
					
					ReplaceString(m_sBuffer, sizeof(m_sBuffer), " (Phase 1)", "", false);
					ReplaceString(m_sBuffer, sizeof(m_sBuffer), " (Phase 2)", "", false);
					ReplaceString(m_sBuffer, sizeof(m_sBuffer), " (Phase 3)", "", false);
					ReplaceString(m_sBuffer, sizeof(m_sBuffer), " (Phase 4)", "", false);
					ReplaceString(m_sBuffer, sizeof(m_sBuffer), " (Ruby)", "", false);
					ReplaceString(m_sBuffer, sizeof(m_sBuffer), " (Black Pearl)", "", false);
					ReplaceString(m_sBuffer, sizeof(m_sBuffer), " (Sapphire)", "", false);
					*/
					g_hWeaponNames.SetString(m_sBuffer, m_sKvKey);
				}
			}			
		}
	}

	BuildPath(Path_SM, m_sConfigPath, sizeof(m_sConfigPath), "configs/cases/auto_generated/g_skins.cfg");
	if(!FileExists(m_sConfigPath))
	{
		char m_sFolderPath[PLATFORM_MAX_PATH];
		BuildPath(Path_SM, m_sFolderPath, sizeof(m_sFolderPath), "configs/cases/auto_generated");
		
		if(CreateDirectory(m_sFolderPath, 511))
		{
			LogMessage("Created \"%s\" directory.", m_sFolderPath);
		}
	}
	
	m_sKeyValues.ExportToFile(m_sConfigPath);
	
	g_hSkinsKv = new KeyValues("Skins");
	g_hSkinsKv.Import(m_sKeyValues);
	
	delete m_sKeyValues;
}

stock int GetWeaponIndexByDefIndex(int m_iDefIndex)
{
	int m_iId;
	char m_sKey[32];
	FormatEx(m_sKey, sizeof(m_sKey), "%i", m_iDefIndex);
	
	g_hWeaponIndex.GetValue(m_sKey, m_iId); 
	return m_iId; 
}

bool LoadPrice(const float m_fPrice, const char[] m_sName, const int m_iType)
{
	char m_sKey[64];
	
	switch(m_iType)
	{
		case 1:
		{
			if(!g_hWeaponNames.GetString(m_sName, m_sKey, sizeof(m_sKey)))
			{
				// LogMessage("Failed to set price for \"%s\".", m_sName);
				return false;
			}
			
			if(m_fPrice != 0.0)
			{
				if(g_hSkinsKv.JumpToKey(m_sKey))
				{
					g_hSkinsKv.SetFloat("price_market", m_fPrice);
					g_hSkinsKv.GoBack();
					
					return true;
				}
				
				LogMessage("Failed to set price for \"%s\". Failed to jump to key \"%s\".", m_sName, m_sKey);
				return false;
			}
			
			LogMessage("Failed to set price for \"%s\". Price = 0.0", m_sName);
			return false;
		}
		case 2: // g_hGlovesKv
		{
			if(!g_hWeaponNames.GetString(m_sName, m_sKey, sizeof(m_sKey)))
			{
				LogMessage("Failed to set price for \"%s\"(glove).", m_sName);
				return false;
			}
			
			if(m_fPrice != 0.0)
			{
				if(g_hGlovesKv.JumpToKey(m_sKey))
				{
					g_hGlovesKv.SetFloat("price_market", m_fPrice);
					g_hGlovesKv.GoBack();
					
					return true;
				}
				
				LogMessage("Failed to set price for \"%s\". Failed to jump to key \"%s\".", m_sName, m_sKey);
				return false;
			}
			
			LogMessage("Failed to set price for \"%s\". Price = 0.0", m_sName);
			return false;
		}
	}
	
	LogMessage("Failed to set price for \"%s\". %f %i", m_sName, m_fPrice, m_iType);
	return false;
}

void FinishConfigs()
{
	char m_sConfigPath[PLATFORM_MAX_PATH];
	BuildPath(Path_SM, m_sConfigPath, sizeof(m_sConfigPath), "configs/cases/auto_generated/g_skins.cfg");
	if(!FileExists(m_sConfigPath))
	{
		char m_sFolderPath[PLATFORM_MAX_PATH];
		BuildPath(Path_SM, m_sFolderPath, sizeof(m_sFolderPath), "configs/cases/auto_generated");
		
		if(CreateDirectory(m_sFolderPath, 511))
		{
			LogMessage("Created \"%s\" directory.", m_sFolderPath);
		}
	}
	
	g_hSkinsKv.ExportToFile(m_sConfigPath);
	
	BuildPath(Path_SM, m_sConfigPath, sizeof(m_sConfigPath), "configs/cases/auto_generated/g_gloves.cfg");
	g_hGlovesKv.ExportToFile(m_sConfigPath);
}