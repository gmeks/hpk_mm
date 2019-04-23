/* ======== Basic Admin tool ========
* Copyright (C) 2004-2005 Erling K. Sæterdal
* No warranties of any kind
*
* License: zlib/libpng
*
* Author(s): Erling K. Sæterdal ( EKS )
* Credits:
*	Menu code based on code from CSDM ( http://www.tcwonline.org/~dvander/cssdm ) Created by BAILOPAN
*	Adminloading function from Xaphan ( http://www.sourcemod.net/forums/viewtopic.php?p=25807 ) Created by Devicenull
* Helping on misc errors/functions: BAILOPAN,sslice,devicenull,PMOnoTo,cybermind ( most who idle in #sourcemod on GameSurge realy )
* ============================ */

#ifndef _INCLUDE_SAMPLEPLUGIN_H
#define _INCLUDE_SAMPLEPLUGIN_H

#include <ISmmPlugin.h>
#include <sourcehook/sourcehook.h>
#include "hl2sdk/BATInterface.h"
#include <igameevents.h>
#include "ienginesound.h"
#include <iplayerinfo.h>
#include "convar.h"
#include "cvars.h"
#include "const.h"
#include <sh_vector.h>


//SH_DECL_HOOK0_void(ConCommand, Dispatch, SH_NOATTRIB, 0);

class HPK : public ISmmPlugin, public IMetamodListener, public AdminInterfaceListner, public IConCommandBaseAccessor //  public IGameEventListener2
{
public:
	bool Load(PluginId id, ISmmAPI *ismm, char *error, size_t maxlen, bool late);
	bool Unload(char *error, size_t maxlen);
	void AllPluginsLoaded();
	bool Pause(char *error, size_t maxlen)
	{
		return true;
	}
	bool Unpause(char *error, size_t maxlen)
	{
		return true;
	}
public:
#if BAT_ORANGEBOX == 0
	int GetApiVersion() { return PLAPI_VERSION; }
#else
	int GetApiVersion() { return METAMOD_PLAPI_VERSION; }
#endif

public:
	const char *GetAuthor()
	{
		return "EKS";
	}
	const char *GetName()
	{
		return "High Ping Kicker";
	}
	const char *GetDescription()
	{
		return "A High ping kicker";
	}
	const char *GetURL()
	{
		return "http://www.sourcemm.net/";
	}
	const char *GetLicense()
	{
		return "zlib/libpng";
	}
	const char *GetVersion()
	{
		return HPK_VERSION;
	}
	const char *GetDate()
	{
		return __DATE__;
	}
	const char *GetLogTag()
	{
		return "HPK";
	}
public:
	//These functions are from IServerPluginCallbacks
	//Note, the parameters might be a little different to match the actual calls!


	//Called on ServerActivate.  Same definition as server plugins
	void ServerActivate(edict_t *pEdictList, int edictCount, int clientMax);
	//Called when a client uses a command.  Unlike server plugins, it's void.
	// You can still supercede the gamedll through RETURN_META(MRES_SUPERCEDE).
	void ClientCommand(edict_t *pEntity);
	//Called on a game tick.  Same definition as server plugins
	void GameFrame(bool simulating);
	//Client disconnects - same as server plugins
	void ClientDisconnect(edict_t *pEntity);
	//Client is put in server - same as server plugins
	void ClientPutInServer(edict_t *pEntity, char const *playername);
	//Sets the client index - same as server plugins
	void SetCommandClient(int index);
	
	IVEngineServer *GetEngine() { return m_Engine; }
	IPlayerInfoManager *PlayerInfo() { return m_InfoMngr; }
	IServerPluginHelpers *GetHelpers() {return m_Helpers;}
	IServerGameDLL *GetServerGameDll(){ return m_ServerDll; }

	HPKVars  GetFTKVar() { return g_HPKVars; }
	bool HookCvars();
	void MessagePlayer(int index, const char *msg, ...);
	void SendCSay(int id,const char *msg, ...);
	void ServerCommand(const char *fmt, ...);
	void ServerLogPrint(const char *fmt, ...);
			
	int GetMenuMsgId();
	int GetUserTeam(int id);
	int FindPlayer(int UserID);
	bool IsUserConnected(int id);
	bool IsUserAlive(int id);
	bool HasAdminImmunity(int id);

	const char *GetPlayerName(int id);
	void RemoveTeamAttack(int id);
	int GetPrecentageOfPlayers(int Players,int PlayerCount);
	
	void OnAdminInterfaceUnload();
	void Client_Authorized(int id);
	int str_replace(char *str, const char *from, const char *to, int maxlen); // Function from sslice 
	virtual bool RegisterConCommandBase(ConCommandBase *pVar);

private:
	IGameEventManager2 *m_GameEventManager;	
	IVEngineServer *m_Engine;
	IServerGameDLL *m_ServerDll;
	IServerGameClients *m_ServerClients;
	IPlayerInfoManager *m_InfoMngr;
	IServerPluginHelpers *m_Helpers;
	IEngineSound *m_Sound;
	ICvar *m_ICvar;
	SourceHook::CallClass<IVEngineServer> *m_Engine_CC;
	ConCommand *pSayCmd;
	ConVar *pFriendlyFire;
	SourceHook::CVector<int> m_hooks;

	void LoadPluginSettings(int clientMax);
	void GetModName();
	void CheckIfPlayerReconnected(int id);
	void SaveReconnectInfo(int id);
	void RegisterHighPingCount(int id,int NewPing);

	int GetMsgNum(const char *Msg);
	void SetupModSpesficInfo();
};

extern HPK g_HPKCore;
extern HPKVars g_HPKVars;
extern ConstPlayerInfo g_UserInfo[MAXPLAYERS+1];
PLUGIN_GLOBALVARS();

#if BAT_ORANGEBOX == 0
class CCommand
{
public:
	const char *ArgS()
	{
		return g_ForgiveTKCore.GetEngine()->Cmd_Args();
	}
	int ArgC()
	{
		return g_ForgiveTKCore.GetEngine()->Cmd_Argc();
	}

	const char *Arg(int index)
	{
		return g_ForgiveTKCore.GetEngine()->Cmd_Argv(index);
	}
};
extern ICvar *g_pCVar;
ICvar * GetICVar();
#endif
extern CCommand g_LastCCommand;
extern bool g_IsConnected[MAXPLAYERS+1];

#endif //_INCLUDE_SAMPLEPLUGIN_H
