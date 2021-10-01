Action Command_Case(int iClient, int iArgs)
{
	if(iClient > 0 && IsClientInGame(iClient))
	{
		ShowMainMenu(iClient);
	}
	
	return Plugin_Handled;
}

Action Command_AddBalance(int iClient, int iArgs)
{
	char sArguments[256];
	char sArg[65];
	
	int iLen;
	int iTargetList[MAXPLAYERS]; 
	int iTargetCount;
	int iAmount;
	
	bool bTnIsMl;
	
	if(iArgs != 2)
	{
		ReplyToCommand(iClient, "[SM] Usage: <command> <#userid|name> <amount>");
		return Plugin_Handled;
	}
	
	GetCmdArgString(sArguments, sizeof(sArguments));
	
	iLen = BreakString(sArguments, sArg, sizeof(sArg));
	
	if (iLen == -1)
	{
		/* Safely null terminate */
		iLen = 0;
		sArguments[0] = '\0';
	}

	char sTargetName[MAX_TARGET_LENGTH];
	
	if ((iTargetCount = ProcessTargetString(sArg, iClient, iTargetList, MAXPLAYERS, COMMAND_FILTER_CONNECTED, sTargetName, sizeof(sTargetName), bTnIsMl)) > 0)
	{
		iAmount = StringToInt(sArguments[iLen]);
		
		for (int iIter = 0; iIter < iTargetCount; iIter++)
		{
			if(IsClientInGame(iTargetList[iIter]))
			{
				CPrintToChat(iTargetList[iIter], "%T", "CreditsAdded", iTargetList[iIter], iAmount, iClient);
				CReplyToCommand(iClient, "%T", "CreditsAddedAdmin", iClient, iAmount, iTargetList[iIter]);
				
				LogMessage("\"%L\" gave \"%L\" <%i> credits.", iClient, iTargetList[iIter], iAmount);		
			
				SetBalance(iTargetList[iIter], GetBalance(iTargetList[iIter]) + iAmount);
			}
		}
	}
	else
	{
		ReplyToTargetError(iClient, iTargetCount);
	}
	
	return Plugin_Handled;
}