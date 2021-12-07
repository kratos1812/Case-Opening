public any Native_GetClientBalance(Handle hPlugins, int iNumParams)
{
	int iClient = GetNativeCell(1);
	
	if (iClient > 0 && iClient <= MaxClients)
	{
		if(IsClientInGame(iClient))
		{
			if(IsFakeClient(iClient))
			{
				return ThrowNativeError(SP_ERROR_NATIVE, "Client (%d) is a BOT.", iClient);
			}
			else
			{
				return GetBalance(iClient);
			}
		}
		else
		{
			return ThrowNativeError(SP_ERROR_NATIVE, "Client (%d) is not in game.", iClient);
		}
	}
	else
	{
		return ThrowNativeError(SP_ERROR_NATIVE, "Invalid client index (%d).", iClient);
	}
}

public any Native_SetClientBalance(Handle hPlugins, int iNumParams)
{
	int iClient = GetNativeCell(1);
	
	if (iClient > 0 && iClient <= MaxClients)
	{
		if(IsClientInGame(iClient))
		{
			if(IsFakeClient(iClient))
			{
				return ThrowNativeError(SP_ERROR_NATIVE, "Client (%d) is a BOT.", iClient);
			}
			else
			{
				float iAmount = GetNativeCell(2);
				
				SetBalance(iClient, GetBalance(iClient) + iAmount);
				return 0.0;
			}
		}
		else
		{
			return ThrowNativeError(SP_ERROR_NATIVE, "Client (%d) is not in game.", iClient);
		}
	}
	else
	{
		return ThrowNativeError(SP_ERROR_NATIVE, "Invalid client index (%d).", iClient);
	}
}