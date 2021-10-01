#if defined _kRatoss_included
 #endinput
#endif
#define _kRatoss_included

/**
 * Remove cstrike.inc dependencies for *some plugins.
 */
#if !defined    _cstrike_included
	#define CS_TEAM_NONE        0   /**< No team yet. */
	#define CS_TEAM_SPECTATOR   1   /**< Spectators. */
	#define CS_TEAM_T           2   /**< Terrorists. */
	#define CS_TEAM_CT          3   /**< Counter-Terrorists. */

	#define CS_SLOT_PRIMARY     0   /**< Primary weapon slot. */
	#define CS_SLOT_SECONDARY   1   /**< Secondary weapon slot. */
	#define CS_SLOT_KNIFE       2   /**< Knife slot. */
	#define CS_SLOT_GRENADE     3   /**< Grenade slot (will only return one grenade). */
	#define CS_SLOT_C4          4   /**< C4 slot. */
	#define CS_SLOT_BOOST       11  /**< Slot for healthshot and shield (will only return one weapon/item). */
	#define CS_SLOT_UTILITY     12  /**< Slot for tablet. */
#endif

/**
 * @param sCommands			The commands to be registred. Usage: "sm_test1;sm_test2;sm_command;sm_etc"
 * @param hCallback      	A function to use as a callback for when the commands are invoked.
 * @param sDescription   	Optional description to use for commands creation.
 * @param iFlags         	Optional flags to use for commands creation.
 * @error               	Commands names are the same as an existing convar.
 */
stock void RegConsoleCmds(const char[] sCommands, ConCmd hCallback, const char[] sDescription="", int iFlags=0)
{
    char m_sCommandsBuffer[32][32]; // A maximum of 32 commands, 32 chars each.

    int iCmdNum = ExplodeString(sCommands, ";", m_sCommandsBuffer, sizeof(m_sCommandsBuffer), sizeof(m_sCommandsBuffer[]));

    for (int iIter = 0; iIter < iCmdNum; iIter++) 
    {
        RegConsoleCmd(m_sCommandsBuffer[iIter], hCallback, sDescription, iFlags);
    }
}

/**
 * @param sCommands			The commands to be registred. Usage: "sm_test1;sm_test2;sm_command;sm_etc"
 * @param hCallback      	A function to use as a callback for when the commands are invoked.
 * @param iAdminFlags		Administrative flags (bitstring) to use for permissions.
 * @param sDescription   	Optional description to use for commands creation.
 * @param iFlags         	Optional flags to use for commands creation.
 * @param sGroup         String containing the command group to use.  If empty,
 *                      the plugin's filename will be used instead.
 * @error               	Commands names are the same as an existing convar.
 */
stock void RegAdminCmds(const char[] sCommands, ConCmd hCallback, int iAdminFlags, const char[] sDescription="", const char[] sGroup="", int iFlags=0)
{
    char m_sCommandsBuffer[32][32]; // A maximum of 32 commands, 32 chars each.

    int iCmdNum = ExplodeString(sCommands, ";", m_sCommandsBuffer, sizeof(m_sCommandsBuffer), sizeof(m_sCommandsBuffer[]));

    for (int iIter = 0; iIter < iCmdNum; iIter++) 
    {
        RegAdminCmd(m_sCommandsBuffer[iIter], hCallback, iAdminFlags, sDescription, sGroup, iFlags);
    }
}

/**
 * Adds \n every x characters in a string
 * @param sBuffer		Buffer string to copy to
 * @param iLen			The length of the buffer
 * @param iSpaceIndex	The character index where it should put the new-line symbol
 */
stock void MakeLinesFromString(char[] sBuffer, int iLen, int iSpaceIndex)
{
	char[] sLocalBuffer = new char[iLen];
	
	strcopy(sLocalBuffer, iLen, sBuffer);

	iSpaceIndex += 1;

	for(int iPos = iSpaceIndex - 1; iPos < strlen(sLocalBuffer); iPos += iSpaceIndex)
	{
		for(int iPos2 = iPos; iPos2 < strlen(sLocalBuffer); iPos2 += iSpaceIndex)
		{
			sLocalBuffer[iPos2 + 1] = sLocalBuffer[iPos2];
		}
		
		sLocalBuffer[iPos] = '\n';
	}
	
	strcopy(sBuffer, iLen, sLocalBuffer);
}

/**
 * @param m_iClient     Client index to check.
 * @return              True if the client is valid, false otherwise.
 */
stock bool IsValidClient(int m_iClient)
{
	return view_as<bool>(m_iClient >= 1 && m_iClient <= MaxClients && IsClientInGame(m_iClient));
}

/**
 * Returns if the game is in warmup period.
 * @return     True if the warmup is active, false otherwise.
 */
stock bool IsWarmup()
{
	return view_as<bool>(GameRules_GetProp("m_bWarmupPeriod") == 1);
}

stock bool SafeRemoveWeapon(int iClient, int iWeapon)
{
	int m_iOwnerEntity;
	int m_iWorldentity;
	
	if (!IsValidEntity(iWeapon) || !IsValidEdict(iWeapon))
	{
		return false;
	}
		
	if (!HasEntProp(iWeapon, Prop_Send, "m_hOwnerEntity"))
	{
		return false;
	}
	
	m_iOwnerEntity = GetEntPropEnt(iWeapon, Prop_Send, "m_hOwnerEntity");
	
	if (m_iOwnerEntity != iClient)
	{
		SetEntPropEnt(iWeapon, Prop_Send, "m_hOwnerEntity", iClient);
	}
	
	CS_DropWeapon(iClient, iWeapon, false);
	
	if (HasEntProp(iWeapon, Prop_Send, "m_hWeaponWorldModel"))
	{
		m_iWorldentity = GetEntPropEnt(iWeapon, Prop_Send, "m_hWeaponWorldModel");
		
		if (IsValidEdict(m_iWorldentity) && IsValidEntity(m_iWorldentity) && !AcceptEntityInput(m_iWorldentity, "Kill"))
		{
			return false;
		}
	}
	
	if (!AcceptEntityInput(iWeapon, "Kill"))
	{
		return false;
	}
	
	return true;
}

/**
 * Gets the total amount of players(not bots) on the server
 *
 * @param bAliveOnly     	If true we won't count death players.
 * @param bNoBots     		If true we won't count fake players(Bots, GOTV, SourceTV).
 * @param bInTeamsOnly     	If true we won't count spectators.
 * @param iTeam         	If specified, we will count players only from this team.
 * @return          		Count of valid(in-game & connected) players.
 */
stock int GetPlayersCount(bool bAliveOnly = false, bool bNoBots = false, bool bInTeamsOnly = false, int iTeam = CS_TEAM_NONE)
{
	int m_iCount = 0;
	
	for (int m_iClient = 1; m_iClient <= MaxClients; m_iClient++)
	{
		if(!IsClientInGame(m_iClient))
		{
			continue;
		}

		if(bAliveOnly && !IsPlayerAlive(m_iClient))
		{
			continue;
		}

		if(bNoBots && (IsFakeClient(m_iClient) || IsClientSourceTV(m_iClient) || IsClientReplay(m_iClient)))
		{
			continue;
		}

		if(bInTeamsOnly && GetClientTeam(m_iClient) == CS_TEAM_SPECTATOR || GetClientTeam(m_iClient) == CS_TEAM_NONE)
		{
			continue;
		}

		if(iTeam != CS_TEAM_NONE && GetClientTeam(m_iClient) != iTeam)
		{
			continue;
		}

		m_iCount++;	
	}	
	
	return m_iCount;
}