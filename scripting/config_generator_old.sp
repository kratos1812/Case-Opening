#include <sourcemod>
#include <sdktools>
#include <eItems>
#include <Profiler>
#include <ripext>

#pragma semicolon 1
#pragma newdecls required

#define PLUGIN_VERSION "0.0.2"

public Plugin myinfo = 
{
	name = "Case Opening Config Generator",
	author = "kRatoss",
	description = "Creates a config for kRatoss' CaseOpening plugin with the latest skins and gloves",
	version = PLUGIN_VERSION,
	url = "http://steamcommunity.com/profiles/76561198090457091/"
};

stock int g_iWeaponDefIndex[] = {
/* 0*/ 9, /* 1*/ 7, /* 2*/ 16, /* 3*/ 60, /* 4*/ 1, /* 5*/ 61, /* 6*/ 32, /* 7*/ 4, /* 8*/ 2, 
/* 9*/ 36, /*10*/ 63, /*11*/ 3, /*12*/ 30, /*13*/ 64, /*14*/ 35, /*15*/ 25, /*16*/ 27, /*17*/ 29, 
/*18*/ 14, /*19*/ 28, /*20*/ 34, /*21*/ 17, /*22*/ 33, /*23*/ 24, /*24*/ 19, /*25*/ 26, /*26*/ 10, /*27*/ 13, 
/*28*/ 40, /*29*/ 8, /*30*/ 39, /*31*/ 38, /*32*/ 11, /*33*/ 507, /*34*/ 508, /*35*/ 500, 
/*36*/ 514, /*37*/ 515, /*38*/ 505, /*39*/ 516, /*40*/ 509, /*41*/ 512, /*42*/ 506,
/*43*/ 519, /*44*/ 520, /*45*/ 522, /*46*/ 523, /*47*/ 23, /*48*/ 503, /*49*/ 517,
/*50*/ 518, /*51*/ 521, /*52*/ 525
};

KeyValues 
	g_hSkinsKV;

StringMap 
	g_hWeaponIndex,
	g_hWeaponNames,
	g_hFetchedSkinNames,
	g_hSkinsPrices;

char 
	g_sConditions[][] = {" (Factory New)", " (Minimal Wear)", " (Field-Tested)", " (Well-Worn)", " (Battle-Scarred)"}, 
	//g_sFilters[][] = {"Bloodhound Gloves", "Broken Fang Gloves", "Driver Gloves", "Hand Wraps | ", "Hydra Gloves", "Moto Gloves", "Specialist Gloves", "Sport Gloves", "Sealed Graffiti", "Sticker", "Music Kit", " Case", "Case Key", "Pin", "Capsule", "Patch ", "Access Pass", "X-Ray P250 Package", "Swap Tool", "Operation ", "Package"},
	g_sConfigPath[PLATFORM_MAX_PATH];
	
int 
	g_iFailedSkins,
	g_iSuccesfullSkins;

public void OnPluginStart()
{
	delete g_hWeaponIndex; // delete old handles, might not be required?
	g_hWeaponIndex = new StringMap();
	
	delete g_hWeaponNames;
	g_hWeaponNames = new StringMap();
	
	delete g_hFetchedSkinNames;
	g_hFetchedSkinNames = new StringMap();
	
	delete g_hSkinsPrices;
	g_hSkinsPrices = new StringMap();
	
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
	Profiler hProf = new Profiler();
	hProf.Start();
	
	char m_sPaintDisplayName[64], m_sWeaponDisplayName[32], m_sBuffer[128], m_sKey[32], m_sFolderPath[PLATFORM_MAX_PATH];
	BuildPath(Path_SM, g_sConfigPath, sizeof(g_sConfigPath), "configs/generator/g_skins.cfg");
	
	if(!FileExists(g_sConfigPath))
	{
		BuildPath(Path_SM, m_sFolderPath, sizeof(m_sFolderPath), "configs/generator");
		if(CreateDirectory(m_sFolderPath, 511))
		{
			LogMessage("Created directory.");
		}
	}
	
	g_hSkinsKV = new KeyValues("Skins");
	
	int m_iPaintsCount = eItems_GetPaintsCount(), m_iSkinId, m_iWeaponCount = eItems_GetWeaponCount(), m_iDefIndex;
	for(int m_iPaint = 0; m_iPaint < m_iPaintsCount; m_iPaint++)
	{
		eItems_GetSkinDisplayNameBySkinNum(m_iPaint, m_sPaintDisplayName, sizeof(m_sPaintDisplayName));
		for (int m_iWeapon = 0; m_iWeapon < m_iWeaponCount; m_iWeapon++)
		{
			if (eItems_IsNativeSkin(m_iPaint, m_iWeapon, ITEMTYPE_WEAPON))
			{
				m_iDefIndex = eItems_GetWeaponDefIndexByWeaponNum(m_iWeapon);
				eItems_GetWeaponDisplayNameByWeaponNum(m_iWeapon, m_sWeaponDisplayName, sizeof(m_sWeaponDisplayName));
				
				FormatEx(m_sKey, sizeof(m_sKey), "%i", m_iSkinId++);
				if(g_hSkinsKV.JumpToKey(m_sKey, true))
				{
					FormatEx(m_sBuffer, sizeof(m_sBuffer), "%s | %s", m_sWeaponDisplayName, m_sPaintDisplayName);
					g_hSkinsKV.SetString("name", m_sBuffer);
					
					if(StrContains(m_sBuffer, "knife", false) != -1 || StrContains(m_sBuffer, "bayonet", false) != -1)
					{
						g_hSkinsKV.SetNum("rarity", 6);
					}
					else
					{
						g_hSkinsKV.SetNum("rarity", eItems_GetSkinRarity(eItems_GetSkinDefIndexBySkinNum(m_iPaint)));
					}
					
					g_hSkinsKV.SetNum("skin_index", eItems_GetSkinDefIndexBySkinNum(m_iPaint));
					g_hSkinsKV.SetNum("weapon_index", GetWeaponIndexByDefIndex(m_iDefIndex));
					
					ReplaceString(m_sBuffer, sizeof(m_sBuffer), " (Phase 1)", "", false);
					ReplaceString(m_sBuffer, sizeof(m_sBuffer), " (Phase 2)", "", false);
					ReplaceString(m_sBuffer, sizeof(m_sBuffer), " (Phase 3)", "", false);
					ReplaceString(m_sBuffer, sizeof(m_sBuffer), " (Phase 4)", "", false);
					ReplaceString(m_sBuffer, sizeof(m_sBuffer), " (Ruby)", "", false);
					ReplaceString(m_sBuffer, sizeof(m_sBuffer), " (Black Pearl)", "", false);
					ReplaceString(m_sBuffer, sizeof(m_sBuffer), " (Sapphire)", "", false);
					
					g_hWeaponNames.SetString(m_sBuffer, m_sKey);
				}
				g_hSkinsKV.GoBack();
			}
		}
	}
	
	HTTPRequest hMarketRequest = new HTTPRequest("https://csgobackpack.net/api/GetItemsList/v2/?prettyprint=yes");
	hMarketRequest.Get(HTTPRequest_OnPriceRetrieved, m_iSkinId);
	
	hProf.Stop();
	LogMessage("Loaded skins from the game in %f seconds", hProf.Time); // hProf.Timer = 0.082806
	delete hProf;
}

int GetWeaponIndexByDefIndex(int m_iDefIndex)
{
	int m_iId;
	char m_sKey[32];
	FormatEx(m_sKey, sizeof(m_sKey), "%i", m_iDefIndex);
	
	g_hWeaponIndex.GetValue(m_sKey, m_iId); 
	return m_iId; 
}

void HTTPRequest_OnPriceRetrieved(HTTPResponse m_hResponse, int m_iSkinId)
{
	if (m_hResponse.Status != HTTPStatus_OK) 
	{
		LogMessage("HTTPRequest_OnPriceRetrieved Failed. %i %i", m_hResponse.Status, m_iSkinId);
		return;
	}
	
	JSONObject m_hObject = view_as<JSONObject>(m_hResponse.Data), m_hItemsList = view_as<JSONObject>(m_hObject.Get("items_list")), m_hItemProfile, m_hItemPriceList; 	
	JSONObjectKeys m_hKeys = m_hItemsList.Keys();

	if(!m_hObject.GetBool("success"))
	{
		LogMessage("HTTPRequest_OnPriceRetrieved Failed #2. m_iSkinId: %i", m_iSkinId);
		return;
	}
	
	char m_sSectionName[128], m_sTempBuffer[128];
	// bool m_bContinue;
	
	g_iFailedSkins = 0;
	g_iSuccesfullSkins = 0;
	g_hFetchedSkinNames.Clear();
	while (m_hKeys.ReadKey(m_sSectionName, sizeof(m_sSectionName))) 
	{
		delete m_hItemProfile;
		m_hItemProfile = view_as<JSONObject>(m_hItemsList.Get(m_sSectionName));
		
		for(int iIndex = 0; iIndex < sizeof(g_sConditions); iIndex++)
		{
			ReplaceString(m_sSectionName, sizeof(m_sSectionName), g_sConditions[iIndex], "", false);
		}
		
		if(!m_hItemProfile.HasKey("price"))
		{
			continue;
		}
		
		m_hItemProfile.GetString("type", m_sTempBuffer, sizeof(m_sTempBuffer));
		if(strcmp(m_sTempBuffer, "Weapon") != 0)
		{
			continue;
		}
		
		if(m_hItemProfile.HasKey("knife_type"))
		{
			if(StrContains(m_sSectionName, " | ", false) == -1) // If vanilla knife
			{
				continue;
			}
		}
		
		ReplaceString(m_sSectionName, sizeof(m_sSectionName), "★ ", "", false);
		ReplaceString(m_sSectionName, sizeof(m_sSectionName), "StatTrak™ ", "", false);
		ReplaceString(m_sSectionName, sizeof(m_sSectionName), "Souvenir ", "", false);
		ReplaceString(m_sSectionName, sizeof(m_sSectionName), " (Phase 1)", "", false);
		ReplaceString(m_sSectionName, sizeof(m_sSectionName), " (Phase 2)", "", false);
		ReplaceString(m_sSectionName, sizeof(m_sSectionName), " (Phase 3)", "", false);
		ReplaceString(m_sSectionName, sizeof(m_sSectionName), " (Phase 4)", "", false);
		ReplaceString(m_sSectionName, sizeof(m_sSectionName), " (Ruby)", "", false);
		ReplaceString(m_sSectionName, sizeof(m_sSectionName), " (Black Pearl)", "", false);
		ReplaceString(m_sSectionName, sizeof(m_sSectionName), " (Sapphire)", "", false);
		ReplaceString(m_sSectionName, sizeof(m_sSectionName), "&#39", "'", false);
		ReplaceString(m_sSectionName, sizeof(m_sSectionName), "%27", "'", false);
		
		if(g_hFetchedSkinNames.GetString(m_sSectionName, m_sTempBuffer, sizeof(m_sTempBuffer))) // If the skin has already been fetched
		{
			continue;
		}
		
		if(m_hItemProfile.HasKey("price"))
		{
			delete m_hItemPriceList;
			m_hItemPriceList = view_as<JSONObject>(m_hItemProfile.Get("price"));
			
			if(m_hItemPriceList.HasKey("24_hours")) // "24_hours"
			{
				m_hItemPriceList = view_as<JSONObject>(m_hItemPriceList.Get("24_hours"));
				LoadPrice(m_hItemPriceList, m_sSectionName);
			}
			else
			{
				if(m_hItemPriceList.HasKey("all_time"))
				{
					m_hItemPriceList = view_as<JSONObject>(m_hItemPriceList.Get("all_time"));
					LoadPrice(m_hItemPriceList, m_sSectionName);
				}
				else
				{
					LogMessage("Failed to get \"24_hours\" or \"24_hours\" for \"%s\".", m_sSectionName);
				}
			}
		}
	}
	
	delete m_hObject;
	delete m_hItemsList;
	delete m_hItemProfile;
	delete m_hItemPriceList;
	delete m_hKeys; 
	
	g_hSkinsKV.Rewind();
	if(g_hSkinsKV.ExportToFile(g_sConfigPath))
	{
		PrintToServer("Exported to \"%s\".", g_sConfigPath);
	}
	
	g_hSkinsKV.Rewind();
	
	float m_fPrice;
	int m_iCount;
	
	g_hSkinsKV.GotoFirstSubKey();
	do
	{
		m_fPrice = g_hSkinsKV.GetFloat("price_market", -69.0);
		if(m_fPrice == -69.0)
		{
			m_iCount++;
		}
	}
	while (g_hSkinsKV.GotoNextKey());
	
	LogMessage("Failed to fetch price from %i skins.", m_iCount);
	LogMessage("Total %i Skins failed", g_iFailedSkins);
	LogMessage("Loaded price for %i skins", g_iSuccesfullSkins);
	
	MakeCases();
}

void LoadPrice(const JSONObject m_hObject, char[] m_sSkinName)
{
	char m_sKey[32];
	float m_fPrice;
	
	if(g_hWeaponNames.GetString(m_sSkinName, m_sKey, sizeof(m_sKey)))
	{
		if(g_hSkinsKV.JumpToKey(m_sKey))
		{
			m_fPrice = m_hObject.GetFloat("average");
			
			g_hSkinsKV.SetFloat("price_market", m_fPrice);
			g_hSkinsKV.GoBack();
			g_hFetchedSkinNames.SetString(m_sSkinName, m_sSkinName);
			g_hSkinsPrices.SetValue(m_sKey, m_fPrice);
			
			g_iSuccesfullSkins++;
		}
		else
		{
			g_iFailedSkins++;
			// LogMessage("Failed to find skin with id \"%s\" in config.", m_sKey);
		}
	}
	else
	{
		g_iFailedSkins++;
		// LogMessage("Failed to fetch skin with name \"%s\".", m_sSkinName);
	}
}

void MakeCases()
{
	char m_sConfigPath[PLATFORM_MAX_PATH], m_sCaseDisplayName[128], m_sSkinDisplayName[128], m_sWeaponDisplayName[128], m_hSkinFullName[128], m_sKey[64], m_sBuffer[64], m_sValue[64];
	BuildPath(Path_SM, m_sConfigPath, sizeof(m_sConfigPath), "configs/generator/g_cases.cfg");
	
	eItems_CrateItem m_eItem_CrateItem;
	KeyValues m_hCases = new KeyValues("Cases");
	
	int m_iCaseCount = eItems_GetCratesCount(), m_iItemsCount, m_iSkinNum, m_iWeaponNum, m_iHashSize, m_iCrateItem, m_iItemId;
	
	float m_fPrice, m_fPriceSum;
	
	StringMapSnapshot m_hSnap;
	
	for(int m_iCaseNum = 0; m_iCaseNum < m_iCaseCount; m_iCaseNum++)
	{
		m_iItemsCount = eItems_GetCrateItemsCountByCrateNum(m_iCaseNum);
		
		eItems_GetCrateDisplayNameByCrateNum(m_iCaseNum, m_sCaseDisplayName, sizeof(m_sCaseDisplayName)); // The Cobblestone Collection
		
		m_fPriceSum = 0.0;
		if(m_hCases.JumpToKey(m_sCaseDisplayName, true))
		{
			if(m_hCases.JumpToKey("Skins", true))
			{
				m_hSnap = g_hWeaponNames.Snapshot();
				m_iHashSize = m_hSnap.Length;
				
				m_iCrateItem = 0;
				 
				for(int iIndex = 0; iIndex < m_iHashSize; iIndex++)
				{
					m_hSnap.GetKey(iIndex, m_sKey, sizeof(m_sKey));
					
					if(StrContains(m_sKey, "knife", false) != -1 || StrContains(m_sKey, "bayonet", false) != -1)
					{
						g_hWeaponNames.GetString(m_sKey, m_sKey, sizeof(m_sKey));
						g_hSkinsPrices.GetValue(m_sKey, m_fPrice);
						
						if(m_fPrice == 0.0)
						{
							// LogMessage("Price = 0 for %s", m_hSkinFullName);
							// continue;
						}
						
						Format(m_sBuffer, sizeof(m_sBuffer), "%i", m_iCrateItem++);
						
						Format(m_sValue, sizeof(m_sValue), "%s-%i", m_sKey, RoundToNearest(1000 / m_fPrice));
						m_hCases.SetString(m_sBuffer, m_sValue);
					}
				}
				
				m_iItemId = 0;
				for(m_iCrateItem = 0; m_iCrateItem < m_iItemsCount; m_iCrateItem++)
				{
					eItems_GetCrateItemByCrateNum(m_iCaseNum, m_iCrateItem, m_eItem_CrateItem, sizeof(m_eItem_CrateItem));
					
					m_iSkinNum = eItems_GetSkinNumByDefIndex(m_eItem_CrateItem.SkinDefIndex);
					eItems_GetSkinDisplayNameBySkinNum(m_iSkinNum, m_sSkinDisplayName, sizeof(m_sSkinDisplayName));
					
					m_iWeaponNum = eItems_GetWeaponNumByDefIndex(m_eItem_CrateItem.WeaponDefIndex);
					eItems_GetWeaponDisplayNameByWeaponNum(m_iWeaponNum, m_sWeaponDisplayName, sizeof(m_sWeaponDisplayName));
					
					Format(m_hSkinFullName, sizeof(m_hSkinFullName), "%s | %s", m_sWeaponDisplayName, m_sSkinDisplayName);
					
					g_hWeaponNames.GetString(m_hSkinFullName, m_sKey, sizeof(m_sKey));
					g_hSkinsPrices.GetValue(m_sKey, m_fPrice);
					
					m_fPriceSum += m_fPrice;
					
					if(m_fPrice == 0.0)
					{
						LogMessage("Price = 0 for %s", m_hSkinFullName);
					}
					else
					{
						Format(m_sBuffer, sizeof(m_sBuffer), "%i", m_iItemId++);
						Format(m_sValue, sizeof(m_sValue), "%s-%i", m_sKey, RoundToNearest(1000 / m_fPrice));
						m_hCases.SetString(m_sBuffer, m_sValue);
					}
				}		

				m_hCases.GoBack();
				
				m_fPrice = (m_fPriceSum / m_iItemsCount);
				
				m_hCases.SetFloat("price", m_fPrice * 10);
				m_hCases.SetFloat("return", m_fPrice);
			}
			
			m_hCases.GoBack();
		}
	}
	m_hCases.Rewind();
	m_hCases.ExportToFile(m_sConfigPath);
	
	delete m_hCases;
}