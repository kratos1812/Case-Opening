ArrayList g_hCasesNames;
ArrayList g_hCasesPrices;
ArrayList g_hCasesReturns;
ArrayList g_hCasesSkins;
ArrayList g_hSkins;

bool g_bPreviewMode[MAXPLAYERS + 1];
bool g_bWaintingForFloat[MAXPLAYERS + 1];
bool g_bWaitingForName[MAXPLAYERS + 1];

char g_sTempWeapon[MAXPLAYERS + 1][32];

StringMap g_hInventory[MAXPLAYERS + 1]; // Player's inventory. Where the key is the classname of the weapon and the value is the ArrayList containing the skins.

Cookie g_hBalance; // No need to use a different MySQL table just to store the balance

Database g_hDatabase;

Menu g_hMarketMenu;

ConVar g_hFloatPrice;
ConVar g_hSeedPrice;
ConVar g_hStatTrackPrice;
ConVar g_hNameTagPrice;
ConVar g_hKillReward;

float g_fFloatPrice;
float g_fSeedPrice;
float g_fStarTrackPrice;
float g_fNameTagPrice;
float g_fKillReward;

enum struct enum_skin
{
	char szName[64];
	
	int iWeaponIndex; // THIS IS NOT THE DEF INDEX
	int iSkinIndex;
	int iRarity;
	float fPrice;
}

enum // Skin data enum, for a easier read of the code. 
{
	Data_Id = 0,
	Data_Chance,
	
	Data_Max
}

char g_sMySQLTables[][] = // Might add more
{
	"\
	CREATE TABLE IF NOT EXISTS `kratoss_case_opening`(\
	    `uId` INT UNSIGNED PRIMARY KEY NOT NULL AUTO_INCREMENT,\
	    `skinId` INT UNSIGNED NOT NULL,\
	    `accountId` INT UNSIGNED NOT NULL\
	)"
};

int g_iWeaponDefIndex[] = {
/* 0*/ 9, /* 1*/ 7, /* 2*/ 16, /* 3*/ 60, /* 4*/ 1, /* 5*/ 61, /* 6*/ 32, /* 7*/ 4, /* 8*/ 2, 
/* 9*/ 36, /*10*/ 63, /*11*/ 3, /*12*/ 30, /*13*/ 64, /*14*/ 35, /*15*/ 25, /*16*/ 27, /*17*/ 29, 
/*18*/ 14, /*19*/ 28, /*20*/ 34, /*21*/ 17, /*22*/ 33, /*23*/ 24, /*24*/ 19, /*25*/ 26, /*26*/ 10, /*27*/ 13, 
/*28*/ 40, /*29*/ 8, /*30*/ 39, /*31*/ 38, /*32*/ 11, /*33*/ 507, /*34*/ 508, /*35*/ 500, 
/*36*/ 514, /*37*/ 515, /*38*/ 505, /*39*/ 516, /*40*/ 509, /*41*/ 512, /*42*/ 506,
/*43*/ 519, /*44*/ 520, /*45*/ 522, /*46*/ 523, /*47*/ 23, /*48*/ 503, /*49*/ 517,
/*50*/ 518, /*51*/ 521, /*52*/ 525
};

Menu g_hMarketItemMenu[sizeof(g_iWeaponDefIndex)];

char g_sWeaponClasses[][] = {
/* 0*/ "weapon_awp", /* 1*/ "weapon_ak47", /* 2*/ "weapon_m4a1", /* 3*/ "weapon_m4a1_silencer", /* 4*/ "weapon_deagle", /* 5*/ "weapon_usp_silencer", /* 6*/ "weapon_hkp2000", /* 7*/ "weapon_glock", /* 8*/ "weapon_elite", 
/* 9*/ "weapon_p250", /*10*/ "weapon_cz75a", /*11*/ "weapon_fiveseven", /*12*/ "weapon_tec9", /*13*/ "weapon_revolver", /*14*/ "weapon_nova", /*15*/ "weapon_xm1014", /*16*/ "weapon_mag7", /*17*/ "weapon_sawedoff", 
/*18*/ "weapon_m249", /*19*/ "weapon_negev", /*20*/ "weapon_mp9", /*21*/ "weapon_mac10", /*22*/ "weapon_mp7", /*23*/ "weapon_ump45", /*24*/ "weapon_p90", /*25*/ "weapon_bizon", /*26*/ "weapon_famas", /*27*/ "weapon_galilar", 
/*28*/ "weapon_ssg08", /*29*/ "weapon_aug", /*30*/ "weapon_sg556", /*31*/ "weapon_scar20", /*32*/ "weapon_g3sg1", /*33*/ "weapon_knife_karambit", /*34*/ "weapon_knife_m9_bayonet", /*35*/ "weapon_bayonet", 
/*36*/ "weapon_knife_survival_bowie", /*37*/ "weapon_knife_butterfly", /*38*/ "weapon_knife_flip", /*39*/ "weapon_knife_push", /*40*/ "weapon_knife_tactical", /*41*/ "weapon_knife_falchion", /*42*/ "weapon_knife_gut",
/*43*/ "weapon_knife_ursus", /*44*/ "weapon_knife_gypsy_jackknife", /*45*/ "weapon_knife_stiletto", /*46*/ "weapon_knife_widowmaker", /*47*/ "weapon_mp5sd", /*48*/ "weapon_knife_css", /*49*/ "weapon_knife_cord", 
/*50*/ "weapon_knife_canis", /*51*/ "weapon_knife_outdoor", /*52*/ "weapon_knife_skeleton"
};

/* Skins rarity sounds at opening */
char g_sRaritySounds[][] = 
{
	"ui/item_drop2_uncommon.wav", 
	"ui/item_drop2_uncommon.wav", 
	"ui/item_drop2_uncommon.wav", 
	"ui/item_drop3_rare.wav", 
	"ui/item_drop4_mythical.wav", 
	"ui/item_drop5_legendary.wav", 
	"ui/item_drop6_ancient.wav", 
	"ui/item_drop6_ancient.wav"
};

/* Skins rarity */
char g_sItemRarity[][][] = 
{
	{ "Consumer grade", "\x01" },
	{ "Industrial grade", "\x0B" },
	{ "Mil-spec", "\x0C" },
	{ "Restricted", "\x03" },
	{ "Classified", "\x0E" },
	{ "Covert", "\x07" },
	{ "Exceedingly Rare", "\x09" },
	{ "Contraband", "\x10" }
};