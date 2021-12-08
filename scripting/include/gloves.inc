#if defined _gloves_included_
  #endinput
#endif
#define _gloves_included_

// returns if the client is using any gloves or not
native bool Gloves_IsClientUsingGloves(int client);

// registers arms model, so that it can be set when the player decides to NOT use any gloves (i.e. use default) using !gloves menu
native void Gloves_RegisterCustomArms(int client, const char[] armsModel);

// any plugin that changes player model OR player arm model, should call this native instead of setting it itself with SetEntPropString(client, Prop_Send, "m_szArmsModel", armsModel);
// even if the plugin is not setting an arms model, but only a player model, you have to call this function and set a default arms model or arms will be invisible when using default gloves
native void Gloves_SetArmsModel(int client, const char[] armsModel);

// returns the registered custom arms model. "" if not set
native void Gloves_GetArmsModel(int client, char[] armsModel, int size);

// Set clients a glove.
native void Gloves_SetClientGloves(int client, int team, int group, int glove);

// Set a float to a gloves
native void Gloves_SetClientFloat(int client, float wear);

native int Gloves_GetGloves(int client, int team);

native int Gloves_GetGlovesType(int client, int team);