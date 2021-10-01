void Event_PlayerDeath(Event hEvent, const char[] sName, bool bDontBroadcast)
{
	int iVictim = GetClientOfUserId(hEvent.GetInt("userid"));
	int iClient = GetClientOfUserId(hEvent.GetInt("attacker"));
	
	if(iVictim != iClient)
	{
		if(iClient > 0 && IsClientInGame(iClient) && !IsFakeClient(iClient))
		{
			SetBalance(iClient, GetBalance(iClient) + g_iKillReward);
		}
	}
}