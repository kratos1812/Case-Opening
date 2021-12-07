#if defined _cases_included_
  #endinput
#endif
#define _cases_included_

/**
 * Retrieves the balance of a client.
 *
 * @param iClient       Client index.
 * 
 * @return              Client balance if succesfull.
 * 
 * @error               Invalid client index, client a bot or client not in game.
 */
native float Cases_GetClientBalance(int iClient);

/**
 * Retrieves the balance of a client.
 *
 * @param iClient       Client index. 
 * @param fAmount       Amount to set. Can be > 0.0 or < 0.0. Must be a float.
 * 
 * @error               Invalid client index, client a bot or client not in game.
 */
native void Cases_SetClientBalance(int iClient, float fAmount);