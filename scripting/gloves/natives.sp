/*  CS:GO Gloves SourceMod Plugin
 *
 *  Copyright (C) 2017 Kağan 'kgns' Üstüngel
 * 
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option) 
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT 
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS 
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with 
 * this program. If not, see http://www.gnu.org/licenses/.
 */

public APLRes AskPluginLoad2(Handle myself, bool late, char[] error, int err_max)
{
	CreateNative("Gloves_IsClientUsingGloves", Native_IsClientUsingGloves);
	CreateNative("Gloves_RegisterCustomArms", Native_RegisterCustomArms);
	CreateNative("Gloves_SetArmsModel", Native_SetArmsModel);
	CreateNative("Gloves_GetArmsModel", Native_GetArmsModel);
	CreateNative("Gloves_SetClientGloves", Native_SetGloves);
	CreateNative("Gloves_SetClientFloat", Native_SetFloat);
	CreateNative("Gloves_GetGloves", Native_GetGloves);
	CreateNative("Gloves_GetGlovesType", Native_GetGlovesType);
	return APLRes_Success;
}

public int Native_IsClientUsingGloves(Handle plugin, int numParams)
{
	int clientIndex = GetNativeCell(1);
	int playerTeam = GetClientTeam(clientIndex);
	return g_iGloves[clientIndex][playerTeam] != 0;
}

public int Native_RegisterCustomArms(Handle plugin, int numParams)
{
	int clientIndex = GetNativeCell(1);
	int playerTeam = GetClientTeam(clientIndex);
	GetNativeString(2, g_CustomArms[clientIndex][playerTeam], 256);
}

public int Native_SetArmsModel(Handle plugin, int numParams)
{
	int clientIndex = GetNativeCell(1);
	int playerTeam = GetClientTeam(clientIndex);
	GetNativeString(2, g_CustomArms[clientIndex][playerTeam], 256);
	if(g_iGloves[clientIndex][playerTeam] == 0)
	{
		SetEntPropString(clientIndex, Prop_Send, "m_szArmsModel", g_CustomArms[clientIndex][playerTeam]);
	}
}

public int Native_GetArmsModel(Handle plugin, int numParams)
{
	int clientIndex = GetNativeCell(1);
	int playerTeam = GetClientTeam(clientIndex);
	int size = GetNativeCell(3);
	SetNativeString(2, g_CustomArms[clientIndex][playerTeam], size);
}

public int Native_SetGloves(Handle plugin, int numParams)
{
	int clientIndex = GetNativeCell(1);
	int team = GetNativeCell(2);
	if (clientIndex < 1 || clientIndex > MaxClients)
	{
		return ThrowNativeError(SP_ERROR_NATIVE, "Invalid client index (%d).", clientIndex);
	}
	if(!IsClientInGame(clientIndex))
	{
		return ThrowNativeError(SP_ERROR_NATIVE, "Client (%d) is not in game.", clientIndex);
	}
	
	int groupId = GetNativeCell(3);
	int gloveId = GetNativeCell(4);
	
	g_iGroup[clientIndex][team] = groupId;
	g_iGloves[clientIndex][team] = gloveId;
	
	char updateFields[128];
	char teamName[4];
	if(team == CS_TEAM_T)
	{
		teamName = "t";
	}
	else if(team == CS_TEAM_CT)
	{
		teamName = "ct";
	}
	Format(updateFields, sizeof(updateFields), "%s_group = %d, %s_glove = %d", teamName, groupId, teamName, gloveId);
	UpdatePlayerData(clientIndex, updateFields);
	
	if(team == GetClientTeam(clientIndex))
	{
		if(g_iGroup[clientIndex][team] == 0)
		{
			int ent = GetEntPropEnt(clientIndex, Prop_Send, "m_hMyWearables");
			if(ent != -1)
			{
				AcceptEntityInput(ent, "KillHierarchy");
			}
			SetEntPropString(clientIndex, Prop_Send, "m_szArmsModel", g_CustomArms[clientIndex][team]);
		}
		
		int activeWeapon = GetEntPropEnt(clientIndex, Prop_Send, "m_hActiveWeapon");
		if(activeWeapon != -1)
		{
			SetEntPropEnt(clientIndex, Prop_Send, "m_hActiveWeapon", -1);
		}
		GivePlayerGloves(clientIndex);
		if(activeWeapon != -1)
		{
			DataPack dpack;
			CreateDataTimer(0.1, ResetGlovesTimer, dpack);
			dpack.WriteCell(clientIndex);
			dpack.WriteCell(activeWeapon);
		}
	}
	
	return 0;
}

public int Native_SetFloat(Handle plugin, int numParams)
{
	int clientIndex = GetNativeCell(1);
	if (clientIndex < 1 || clientIndex > MaxClients)
	{
		return ThrowNativeError(SP_ERROR_NATIVE, "Invalid client index (%d).", clientIndex);
	}
	if(!IsClientInGame(clientIndex))
	{
		return ThrowNativeError(SP_ERROR_NATIVE, "Client (%d) is not in game.", clientIndex);
	}
	
	int team = GetClientTeam(clientIndex);
	float wear = GetNativeCell(2);
	
	g_fFloatValue[clientIndex][team] = wear;
	
	char updateFields[30];
	Format(updateFields, sizeof(updateFields), "ct_float = %.2f, t_float = %.2f", g_fFloatValue[clientIndex][team], g_fFloatValue[clientIndex][team]);
	UpdatePlayerData(clientIndex, updateFields);
	
	if(team == GetClientTeam(clientIndex))
	{
		int activeWeapon = GetEntPropEnt(clientIndex, Prop_Send, "m_hActiveWeapon");
		if(activeWeapon != -1)
		{
			SetEntPropEnt(clientIndex, Prop_Send, "m_hActiveWeapon", -1);
		}
		GivePlayerGloves(clientIndex);
		if(activeWeapon != -1)
		{
			DataPack dpack;
			CreateDataTimer(0.1, ResetGlovesTimer, dpack);
			dpack.WriteCell(clientIndex);
			dpack.WriteCell(activeWeapon);
		}
	}
	
	return 0;
}

public int Native_GetGloves(Handle plugin, int numParams)
{
	int clientIndex = GetNativeCell(1);
	int team = GetNativeCell(2);
	if (clientIndex < 1 || clientIndex > MaxClients)
	{
		return ThrowNativeError(SP_ERROR_NATIVE, "Invalid client index (%d).", clientIndex);
	}
	if(!IsClientInGame(clientIndex))
	{
		return ThrowNativeError(SP_ERROR_NATIVE, "Client (%d) is not in game.", clientIndex);
	}

	return g_iGloves[clientIndex][team];
}

public int Native_GetGlovesType(Handle plugin, int numParams)
{
	int clientIndex = GetNativeCell(1);
	int team = GetNativeCell(2);
	if (clientIndex < 1 || clientIndex > MaxClients)
	{
		return ThrowNativeError(SP_ERROR_NATIVE, "Invalid client index (%d).", clientIndex);
	}
	if(!IsClientInGame(clientIndex))
	{
		return ThrowNativeError(SP_ERROR_NATIVE, "Client (%d) is not in game.", clientIndex);
	}
	
	return g_iGroup[clientIndex][team];
}