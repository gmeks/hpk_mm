/* ======== Basic Admin tool ========
* Copyright (C) 2004-2005 Erling K. Sæterdal
* No warranties of any kind
*
* License: zlib/libpng
*
* Author(s): Erling K. Sæterdal ( EKS )
* Credits:
*	Menu code based on code from CSDM ( http://www.tcwonline.org/~dvander/cssdm ) Created by BAILOPAN
*	Adminloading function from Xaphan ( http://www.sourcemod.net/forums/viewtopic.php?p=25807 ) Created by devicenull
* Helping on misc errors/functions: BAILOPAN,sslice,devicenull,PMOnoTo,cybermind ( most who idle in #sourcemod on GameSurge realy )
* ============================ */

#ifndef _INCLUDE_CONST_H
#define _INCLUDE_CONST_H

#include "edict.h"
#include "hl2sdk/strtools.h"

#define HPK_VERSION	"1.1.0"
#define HPK_DEBUG 0
#define BAT_ORANGEBOX 1

#define RECONNECT_REMEMEBERCOUNT 5
#define TEAM_NOT_CONNECTED -1

#define MAXPLAYERS ABSOLUTE_PLAYER_LIMIT +1

#if BAT_ORANGEBOX == 0
#define FnChangeCallback_t FnChangeCallback
#endif

// ---------- GameFrame()
#define TASK_CHECKTIME 5.0			// This is how often the plugin checks if any tasks should be executed

// SayMessage id
//#define MESSAGE_CLIENT	3

#define OPTION_DISABLED 0
#define OPTION_MENUBASED 1
#define OPTION_CHATBASED 2
#define OPTION_ONLYCHATRESPOND 3
#define OPTION_ESCMENUBASED 4

#define MENUSYSTEM_NOTWORKING -1

enum MenuPages
{
	MOD_NONE=0,
	MOD_CSTRIKE,
	MOD_DOD,
	MOD_HL2MP,
	MOD_HL2CTF,
};
typedef struct 
{
	int MenuMsg;		// The id of the MenuMsg
	int HintText;		// HintText
	int TextMsg;		// TextMsg
	int HudMsg;			// HudMsg
	int SayText;		// SayMsg
	bool MenuSupport;
}ModSettingsStruct;
typedef struct 
{
	int MaxPing; // The highest allowed ping
	int ExtraCountPing; // If the ping is this much above the max ping, the user gets 5 high ping ticks instead of 1
	int MaxFailedChecks; // How many failed ping checks before your kicked
}HPKOptionsStruct;
typedef struct 
{
	char steamid[MAX_NETWORKID_LENGTH+1];
	int userid;
	int HPKCount; // How many times this user has gotten a high ping count
	uint GracePeriod; // How many checks to skip, normally given when a player connects.
	bool IsBot; // If its a human player, and not a bot/hltv
	edict_t *PlayerEdict;
}ConstPlayerInfo;
#endif //_INCLUDE_CONST_H