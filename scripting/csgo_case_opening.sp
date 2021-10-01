#include <sourcemod>
#include <sdktools>
#include <kRatossCSGO>
#include <clientprefs>
#include <multicolors>
#include <weapons>

#pragma semicolon 1
#pragma newdecls required

#define PLUGIN_VERSION "BETA"

// Custom files.
#include "inc/globals.inc"
#include "inc/config.inc"
#include "inc/helpers.inc"
#include "inc/menus.inc"
#include "inc/commands.inc"
#include "inc/open.inc"
#include "inc/db.inc"
#include "inc/events.inc"

public Plugin myinfo = 
{
	name = "Case Opening",
	author = "kRatoss",
	description = "Adds the ability to open skins and equipt them.",
	version = PLUGIN_VERSION,
	url = "https://steamcommunity.com/id/kr4toss/"
};

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
	AddCommandListener(CommandListener_BlockWSCommand, "sm_knife");
	AddCommandListener(CommandListener_BlockWSCommand, "sm_knife");
	AddCommandListener(CommandListener_BlockWSCommand, "sm_knife");
	
	CreateConVar("sm_cases_version", PLUGIN_VERSION, "Case Opening by kRatoss Version", FCVAR_REPLICATED|FCVAR_NOTIFY|FCVAR_DONTRECORD);
	
	g_hFloatPrice = CreateConVar("sm_cases_float_price", "200", "Price for setting a float for a weapon.", FCVAR_NOTIFY, true, 0.0);
	g_hFloatPrice.AddChangeHook(OnConvarsChanged);
	
	g_hSeedPrice = CreateConVar("sm_cases_seed_price", "200", "Price for rolling a random seed(pattern) for a weapon.", FCVAR_NOTIFY, true, 0.0);
	g_hSeedPrice.AddChangeHook(OnConvarsChanged);
	
	g_hStatTrackPrice = CreateConVar("sm_cases_statrack_price", "500", "Price for toggling StatTrack for a weapon.", FCVAR_NOTIFY, true, 0.0);
	g_hStatTrackPrice.AddChangeHook(OnConvarsChanged);
	
	g_hNameTagPrice = CreateConVar("sm_cases_nametag_price", "500", "Price for setting a nametag on a weapon.", FCVAR_NOTIFY, true, 0.0);
	g_hNameTagPrice.AddChangeHook(OnConvarsChanged);
	
	g_hKillReward = CreateConVar("sm_cases_kill_reward", "10", "How many credits does a player gets for a kill.", FCVAR_NOTIFY, true, 0.0);
	g_hKillReward.AddChangeHook(OnConvarsChanged);
	
	AutoExecConfig(true, "case_opening_kratoss");
	
	g_hBalance = new Cookie("Case Opening Balance", "Stores the credits of the players", CookieAccess_Private);
	
	LoadTranslations("case_opening.phrases");
	
	Database.Connect(DB_OnConnect, "case_opening");
	
	HookEvent("player_death", Event_PlayerDeath);
}

public void OnMapStart()
{
	LoadItems();
	LoadCases();
	CreateMenus();
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
}

public void OnClientPutInServer(int iClient)
{
	g_hInventory[iClient] = new StringMap();
	g_bPreviewMode[iClient] = false; // This should not be saved, it's useless to
	g_bWaintingForFloat[iClient] = false;
	g_bWaitingForName[iClient] = false;
	strcopy(g_sTempWeapon[iClient], sizeof(g_sTempWeapon[]), "");
}

public void OnClientDisconnect(int iClient)
{
	ArrayList hTempArray;
	for(int iIndex = 0; iIndex < sizeof(g_sWeaponClasses); iIndex++)
	{
		g_hInventory[iClient].GetValue(g_sWeaponClasses[iIndex], hTempArray);
		delete hTempArray;
	}
	delete g_hInventory[iClient];
	g_bPreviewMode[iClient] = false; 
	g_bWaintingForFloat[iClient] = false;
	g_bWaitingForName[iClient] = false;
	strcopy(g_sTempWeapon[iClient], sizeof(g_sTempWeapon[]), "");
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
	}
}

void OnConvarsChanged(ConVar hConVar, const char[] sOldValue, const char[] sNewValue)
{
	if(hConVar == g_hFloatPrice)
	{
		g_iFloatPrice = StringToInt(sNewValue);
	}
	else if(hConVar == g_hSeedPrice)
	{
		g_iSeedPrice = StringToInt(sNewValue);
	}
	else if(hConVar == g_hStatTrackPrice)
	{
		g_iStarTrackPrice = StringToInt(sNewValue);
	}
	else if(hConVar == g_hNameTagPrice)
	{
		g_iNameTagPrice = StringToInt(sNewValue);
	}
	else if(hConVar == g_hKillReward)
	{
		g_iKillReward = StringToInt(sNewValue);
	}
}

public void OnConfigsExecuted()
{
	g_iFloatPrice = g_hFloatPrice.IntValue;
	g_iSeedPrice = g_hSeedPrice.IntValue;
	g_iStarTrackPrice = g_hStatTrackPrice.IntValue;
	g_iKillReward = g_hKillReward.IntValue;
}

Action CommandListener_BlockWSCommand(int iClient, const char[] sCommand, int iArgc)
{
	return Plugin_Stop;
}