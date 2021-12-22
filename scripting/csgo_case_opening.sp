#include <sourcemod>
#include <sdktools>
#include <kRatossCSGO>
#include <clientprefs>
#include <multicolors>
#include <weapons>
#include <gloves>

#undef REQUIRE_PLUGIN
#tryinclude <shop>
#tryinclude <store>
#define REQUIRE_PLUGIN

#pragma semicolon 1
#pragma newdecls required

#define PLUGIN_VERSION "1.0.9b"

// Custom files.
#include "inc/globals.inc"
#include "inc/config.inc"
#include "inc/helpers.inc"
#include "inc/menus.inc"
#include "inc/commands.inc"
#include "inc/open.inc"
#include "inc/db.inc"
#include "inc/events.inc"
#include "inc/natives.inc"

public Plugin myinfo = 
{
	name = "Case Opening",
	author = "kRatoss",
	description = "Adds the ability to open skins and equipt them.",
	version = PLUGIN_VERSION,
	url = "https://steamcommunity.com/id/kr4toss/"
};

public APLRes AskPluginLoad2(Handle hMyself, bool bLate, char[] sError, int iErrMax)
{
	MarkNativeAsOptional("Shop_GetClientCredits");
	MarkNativeAsOptional("Shop_GiveClientCredits");
	
	MarkNativeAsOptional("Store_GetClientCredits");
	MarkNativeAsOptional("Store_SetClientCredits");
	
	RegPluginLibrary("case_opening");

	CreateNative("Cases_GetClientBalance", Native_GetClientBalance);
	CreateNative("Cases_SetClientBalance", Native_SetClientBalance);
	
	return APLRes_Success;
}

public void OnPluginStart()
{ 	
	RegAdminCmds("sm_cases_addcash;sm_cases_givecredits", Command_AddBalance, ADMFLAG_ROOT, "Gives or takes credits from players");
	
	RegConsoleCmds("sm_case;sm_cases;sm_cs", Command_Case, "Opens the main case opening menu");
	
	// Remove https://github.com/kgns/weapons/blob/master/addons/sourcemod/scripting/weapons.sp#L93-L99 commands
	AddCommandListener(CommandListener_BlockWSCommand, "buyammo1");
	AddCommandListener(CommandListener_BlockWSCommand, "sm_ws");
	AddCommandListener(CommandListener_BlockWSCommand, "buyammo2");
	AddCommandListener(CommandListener_BlockWSCommand, "sm_nametag");
	AddCommandListener(CommandListener_BlockWSCommand, "sm_wslang");
	AddCommandListener(CommandListener_BlockWSCommand, "sm_seed");
	AddCommandListener(CommandListener_BlockWSCommand, "sm_knife");
	AddCommandListener(CommandListener_BlockWSCommand, "sm_glove");
	AddCommandListener(CommandListener_BlockWSCommand, "sm_gloves");
	
	CreateConVar("sm_cases_version", PLUGIN_VERSION, "Case Opening by kRatoss Version", FCVAR_REPLICATED|FCVAR_NOTIFY|FCVAR_DONTRECORD);
	
	g_hFloatPrice = CreateConVar("sm_cases_float_price", "200.0", "Price for setting a float for a weapon.", FCVAR_NOTIFY, true, 0.0);
	g_hFloatPrice.AddChangeHook(OnConvarsChanged);
	
	g_hSeedPrice = CreateConVar("sm_cases_seed_price", "200.0", "Price for rolling a random seed(pattern) for a weapon.", FCVAR_NOTIFY, true, 0.0);
	g_hSeedPrice.AddChangeHook(OnConvarsChanged);
	
	g_hStatTrackPrice = CreateConVar("sm_cases_statrack_price", "500.0", "Price for toggling StatTrack for a weapon.", FCVAR_NOTIFY, true, 0.0);
	g_hStatTrackPrice.AddChangeHook(OnConvarsChanged);
	
	g_hNameTagPrice = CreateConVar("sm_cases_nametag_price", "500.0", "Price for setting a nametag on a weapon.", FCVAR_NOTIFY, true, 0.0);
	g_hNameTagPrice.AddChangeHook(OnConvarsChanged);
	
	g_hKillReward = CreateConVar("sm_cases_kill_reward", "10.0", "How many credits does a player gets for a kill.", FCVAR_NOTIFY, true, 0.0);
	g_hKillReward.AddChangeHook(OnConvarsChanged);

	g_hDrops = CreateConVar("sm_cases_drops_enabled", "1.0", "Enable match end drops?", FCVAR_NOTIFY, true, 0.0, true, 1.0);
	g_hDrops.AddChangeHook(OnConvarsChanged);
	
	g_hDropsChance = CreateConVar("sm_cases_drops_chance", "25.0", "Drop percentage for each player. 100 = Everyone gets a drop. ", FCVAR_NOTIFY, true, 1.0, true, 100.0);
	g_hDropsChance.AddChangeHook(OnConvarsChanged);
	
	g_hCommandsOverride = CreateConVar("sm_cases_cmds_override", "1", "Open !cases menu when players use !ws, !knife, !glove, commands?", FCVAR_NOTIFY, true, 0.0, true, 1.0);
	g_hCommandsOverride.AddChangeHook(OnConvarsChanged);
	
	g_hEnableMarketSkins = CreateConVar("sm_cases_allow_buy_skins", "1", "Allow players to buy skins?", FCVAR_NOTIFY, true, 0.0, true, 1.0);
	g_hEnableMarketSkins.AddChangeHook(OnConvarsChanged);
	
	g_hEnableMarketGloves = CreateConVar("sm_cases_allow_buy_gloves", "1", "Allow players to buy gloves?", FCVAR_NOTIFY, true, 0.0, true, 1.0);
	g_hEnableMarketGloves.AddChangeHook(OnConvarsChanged);
	
	g_hEnableQuickSell  = CreateConVar("sm_cases_allow_quick_sell", "1", "Allow players to quick sell skins and gloves?", FCVAR_NOTIFY, true, 0.0, true, 1.0);
	g_hEnableQuickSell.AddChangeHook(OnConvarsChanged);
	
	g_hBalanceMode = CreateConVar("sm_cases_balance_mode", "0", "Balance mode. 0 = Plugin's custom balance. 1 = Shop Core balance. 2 = Store by Zephyrus balance.", FCVAR_NOTIFY, true, 0.0, true, 2.0);
	g_hBalanceMode.AddChangeHook(OnConvarsChanged);
	
	AutoExecConfig(true, "case_opening_kratoss");
	
	g_hBalance = new Cookie("Case Opening Balance", "Stores the credits of the players", CookieAccess_Private);
	
	LoadTranslations("case_opening.phrases");
	
	Database.Connect(DB_OnConnect, "case_opening");
	
	HookEvent("player_death", Event_PlayerDeath);
	HookEvent("cs_win_panel_match", Event_WinPanel);
}

public void OnMapStart()
{
	LoadItems();
	LoadCases();
	CreateMenus();
	LoadDrops();
	LoadGloves();
}

public void OnMapEnd()
{
	delete g_hCasesNames;
	delete g_hCasesPrices;
	ArrayList hArray;
	for(int iIter = 0; iIter < g_hCasesSkins.Length; iIter++)
	{
		hArray = g_hCasesSkins.Get(iIter);
		delete hArray;
	}
	delete g_hCasesSkins;
	delete g_hCasesReturns;
	delete g_hSkins;
	delete g_hGloves;
	delete g_hDropsSkins;
}

public void OnClientPutInServer(int iClient)
{
	g_hInventory[iClient] = new StringMap();
	g_bPreviewMode[iClient] = false; // This should not be saved, it's useless to
	g_bWaintingForFloat[iClient] = false;
	g_bWaitingForName[iClient] = false;
	g_bWaitingForFloatGloves[iClient] = false;
	strcopy(g_sTempWeapon[iClient], sizeof(g_sTempWeapon[]), "");
}

public void OnClientDisconnect(int iClient)
{
	g_bPreviewMode[iClient] = false; 
	g_bWaintingForFloat[iClient] = false;
	g_bWaitingForName[iClient] = false;
	g_bWaitingForFloatGloves[iClient] = false;
	strcopy(g_sTempWeapon[iClient], sizeof(g_sTempWeapon[]), "");
	
	if(g_hInventory[iClient])
	{
		ArrayList hTempArray;
		for(int iIndex = 0; iIndex < sizeof(g_sWeaponClasses); iIndex++)
		{
			g_hInventory[iClient].GetValue(g_sWeaponClasses[iIndex], hTempArray);
			delete hTempArray;
		}
		for(int iType = 0; iType < sizeof(g_sGlovesType); iType++)
		{
			g_hInventory[iClient].GetValue(g_sGlovesType[iType], hTempArray); 
			delete hTempArray;
		}
		delete g_hInventory[iClient];
	}
}

public void OnClientPostAdminCheck(int iClient)
{
	if(GetSteamAccountID(iClient) != 0)
	{
		DB_LoadFromDB(iClient);
	}
	else
	{
		LogMessage("Couldn't get ID from %L", iClient);
	}
}

public void OnClientSayCommand_Post(int iClient, const char[] sCommand, const char[] sArgs)
{
	if(iClient != 0 && IsClientInGame(iClient))
	{
		if(g_bWaitingForName[iClient])
		{
			g_bWaitingForName[iClient] = false;
			Weapons_SetClientNameTag(iClient, g_sTempWeapon[iClient], sArgs);
			strcopy(g_sTempWeapon[iClient], sizeof(g_sTempWeapon[]), "");
		}
		else if(g_bWaintingForFloat[iClient])
		{
			float fWear;
			if(!sArgs[0] || StringToFloatEx(sArgs, fWear) != strlen(sArgs))
			{
				CPrintToChat(iClient, "%T", "InvalidAmount", iClient);
			}
			else
			{
				g_bWaintingForFloat[iClient] = false;
				fWear = StringToFloat(sArgs);
				Weapons_SetClientWear(iClient, g_sTempWeapon[iClient], fWear);
				strcopy(g_sTempWeapon[iClient], sizeof(g_sTempWeapon[]), "");
			}
		}
		else if(g_bWaitingForFloatGloves[iClient])
		{
			float fWear;
			if(!sArgs[0] || StringToFloatEx(sArgs, fWear) != strlen(sArgs))
			{
				CPrintToChat(iClient, "%T", "InvalidAmount", iClient);
			}
			else
			{
				enum_glove hGloves;
				g_bWaitingForFloatGloves[iClient] = false;
				fWear = StringToFloat(sArgs);
				int iGlove = StringToInt(g_sTempWeapon[iClient]);
				g_hGloves.GetArray(iGlove, hGloves);
				strcopy(g_sTempWeapon[iClient], sizeof(g_sTempWeapon[]), "");
				Gloves_SetClientFloat(iClient, fWear);
			}
		}
	}
}

void OnConvarsChanged(ConVar hConVar, const char[] sOldValue, const char[] sNewValue)
{
	if(hConVar == g_hFloatPrice)
	{
		g_fFloatPrice = StringToFloat(sNewValue);
	}
	else if(hConVar == g_hSeedPrice)
	{
		g_fSeedPrice = StringToFloat(sNewValue);
	}
	else if(hConVar == g_hStatTrackPrice)
	{
		g_fStarTrackPrice = StringToFloat(sNewValue);
	}
	else if(hConVar == g_hNameTagPrice)
	{
		g_fNameTagPrice = StringToFloat(sNewValue);
	}
	else if(hConVar == g_hKillReward)
	{
		g_fKillReward = StringToFloat(sNewValue);
	}
	else if(hConVar == g_hDrops)
	{
		g_bDropsEnabled = StringToInt(sNewValue) == 1;
	}
	else if(hConVar == g_hDropsChance)
	{
		g_fDropsChance = StringToFloat(sNewValue);
	}
	else if(hConVar == g_hCommandsOverride)
	{
		g_bOverrideCommands = StringToInt(sNewValue) == 1;
	}
	else if(hConVar == g_hEnableMarketSkins)
	{
		g_bEnableMarketSkins = StringToInt(sNewValue) == 1;
	}
	else if(hConVar == g_hEnableMarketGloves)
	{
		g_bEnableMarketGloves = StringToInt(sNewValue) == 1;
	}
	else if(hConVar == g_hEnableQuickSell)
	{
		g_bEnableQuickSell = StringToInt(sNewValue) == 1;
	}
	else if(hConVar == g_hBalanceMode)
	{
		g_iBalanceMode = StringToInt(sNewValue);
	}
	
	CheckBalances();
}

public void OnConfigsExecuted()
{
	g_fFloatPrice = g_hFloatPrice.FloatValue;
	g_fSeedPrice = g_hSeedPrice.FloatValue;
	g_fStarTrackPrice = g_hStatTrackPrice.FloatValue;
	g_fNameTagPrice = g_hNameTagPrice.FloatValue;
	g_fKillReward = g_hKillReward.FloatValue;
	g_fDropsChance = g_hDropsChance.FloatValue;
	g_bDropsEnabled = g_hDrops.BoolValue;
	g_bEnableMarketSkins = g_hEnableMarketSkins.BoolValue;
	g_bEnableMarketGloves = g_hEnableMarketGloves.BoolValue;
	g_bEnableQuickSell = g_hEnableQuickSell.BoolValue;
	g_iBalanceMode = g_hBalanceMode.IntValue;
	
	CheckBalances();
}

Action CommandListener_BlockWSCommand(int iClient, const char[] sCommand, int iArgc)
{
	if(g_bOverrideCommands)
	{
		ShowMainMenu(iClient);
	}
	
	return Plugin_Stop;
}

public void OnAllPluginsLoaded()
{
	g_bStoreExists = LibraryExists("store_zephyrus");
	g_bShopExists = LibraryExists("shop");
}

public void OnLibraryAdded(const char[] sLibrary)
{
	if (StrEqual(sLibrary, "store_zephyrus"))
	{
		g_bStoreExists = true;
	}
	
	else if(StrEqual(sLibrary, "shop"))
	{
		g_bShopExists = true;
	}
}

public void OnLibraryRemoved(const char[] sLibrary)
{
	if (StrEqual(sLibrary, "store_zephyrus"))
	{
		g_bStoreExists = false;
	}
	
	else if(StrEqual(sLibrary, "shop"))
	{
		g_bShopExists = false;
	}
}