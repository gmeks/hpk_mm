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
* ============================
	
	Reconnect looses TA TK count
*/
#include "hl2sdk/BATInterface.h"
#include "ForgiveTK.h"
#include "meta_hooks.h"
#include "cvars.h"
#include "const.h"
#include <time.h>

#include "INetChannelInfo.h"
#include "shareddefs.h"

#include "hl2sdk/recipientfilters.h"
#include <bitbuf.h>

//This has all of the necessary hook declarations.  Read it!
#include "meta_hooks.h"

int g_IndexOfLastUserCmd;
int g_MaxClients;
int g_ModIndex;

int g_ForgiveTKMenu;

float g_FlTime;
bool g_IsConnected[MAXPLAYERS+1];
bool g_HasMenuOpen[MAXPLAYERS+1];
bool g_FirstLoad;

HPK g_HPKCore;
ConstPlayerInfo g_UserInfo[MAXPLAYERS+1];
HPKOptionsStruct g_HPKSettings;
ModSettingsStruct g_ModSettings;
AdminInterface *m_AdminManager;
bool g_EventHookActive=false;

#if BAT_ORANGEBOX == 1
ICvar *g_pCVar = NULL;
#else
ICvar *g_pCVar = NULL;
#endif

PLUGIN_EXPOSE(HPK, g_HPKCore);

void HPK::ServerActivate(edict_t *pEdictList, int edictCount, int clientMax)
{
	g_FlTime = 0.0f;
	LoadPluginSettings(clientMax);
	RETURN_META(MRES_IGNORED);
}
void HPK::ClientDisconnect(edict_t *pEntity)
{
	int id = m_Engine->IndexOfEdict(pEntity);
	g_IsConnected[id] = false;
	RETURN_META(MRES_IGNORED);
}
void HPK::ClientPutInServer(edict_t *pEntity, char const *playername)
{
	int id = m_Engine->IndexOfEdict(pEntity);
	g_IsConnected[id] = true;
	g_UserInfo[id].PlayerEdict = pEntity;
	g_UserInfo[id].userid = m_Engine->GetPlayerUserId(pEntity);

	g_UserInfo[id].HPKCount = 0;
	g_UserInfo[id].GracePeriod = 10;
	sprintf(g_UserInfo[id].steamid,m_Engine->GetPlayerNetworkIDString(pEntity));

	if(strcmp(g_UserInfo[id].steamid,"BOT") == 0 || strcmp(g_UserInfo[id].steamid,"HLTV") == 0 ) // || strcmp(g_UserInfo[id].steamid,"STEAM_ID_LAN") == 0
		g_UserInfo[id].IsBot = true;
	else 
	{
		g_UserInfo[id].IsBot = false;
	}
	RETURN_META(MRES_IGNORED);
}

void HPK::SetCommandClient(int index)
{
	//META_LOG(g_PLAPI, "SetCommandClient() called: index=%d", index);
	g_IndexOfLastUserCmd = index +1;
	RETURN_META(MRES_IGNORED);
}
void HPK::GameFrame(bool simulating) // We dont hook GameFrame, we leave it in here incase we ever need some timer system 
{
	float CurTime = g_SMAPI->GetCGlobals()->curtime;
	if(CurTime - g_FlTime >= TASK_CHECKTIME)
	{
		g_FlTime = CurTime;
		if(!g_HPKVars.DoCvarChecks())
			return;

		int PlayerPing[MAXPLAYERS+1];
		int PlayerGettingPingWarrning=0;
		int PlayersBeingChecked = 0;

		for(int i = 1; i <= g_MaxClients; i++) 
		{ 
			PlayerPing[i] = -1;
			if(g_IsConnected[i] == false || g_UserInfo[i].IsBot)
				continue;
			
			if(g_UserInfo[i].GracePeriod > 0)
			{
				g_UserInfo[i].GracePeriod--;
				continue;
			}

			INetChannelInfo *nci = m_Engine->GetPlayerNetInfo(i);
			float ping = nci->GetAvgLatency(0);
			const char * szCmdRate = m_Engine->GetClientConVarValue( i, "cl_cmdrate" );
			int nCmdRate = max( 20, atoi( szCmdRate ) );
			if(!nci || !szCmdRate)
				continue;

			ping -= (0.5f/nCmdRate) + (g_SMAPI->GetCGlobals()->interval_per_tick * 1.0f); // correct latency

			// in GoldSrc we had a different, not fixed tickrate. so we have to adjust
			// Source pings by half a tick to match the old GoldSrc pings.
			ping -= (g_SMAPI->GetCGlobals()->interval_per_tick * 0.5f);
			ping = ping * 1000.0f; // as msecs			

#if HPK_DEBUG == 1
			ServerCommand("echo [HPK Debug] %d) ping %0.0f",i,ping);
#endif
			
			if(ping > g_HPKSettings.MaxPing)
			{
				PlayerPing[i] = ping;
				PlayerGettingPingWarrning++;
			}
			PlayersBeingChecked++;
		}

		if(GetPrecentageOfPlayers(PlayerGettingPingWarrning,PlayersBeingChecked) >= 40 || g_HPKVars.GetMinimumPlayers() >= PlayersBeingChecked)
		{
#if HPK_DEBUG == 1
			ServerCommand("echo [HPK Debug] PlayersBeingChecked %d : PlayersGettingWarrnings: %d (%d)",PlayersBeingChecked,GetPrecentageOfPlayers(PlayerGettingPingWarrning,PlayersBeingChecked),PlayerGettingPingWarrning);
#endif
			return;
		}

		for(int i = 1; i <= g_MaxClients; i++) 
		{ 
			if(PlayerPing[i] == -1)
				continue;

			if(PlayerPing[i] > g_HPKSettings.MaxPing)
			{
				RegisterHighPingCount(i,PlayerPing[i]);
			}
		}
	}
	RETURN_META(MRES_IGNORED);
}
void HPK::ClientCommand(edict_t *pEntity)
{
	
	RETURN_META(MRES_IGNORED);
}
/*
void HPK::FireGameEvent(IGameEvent *event)
{
	if (!event || !event->GetName() || g_FTKVars.GetFTKOption() == OPTION_DISABLED )
		return;

}
*/
ICvar *GetICVar()
{
#if BAT_ORANGEBOX == 1
	return (ICvar *)((g_SMAPI->GetEngineFactory())(CVAR_INTERFACE_VERSION, NULL));
#else
	return (ICvar *)((g_SMAPI->GetEngineFactory())(VENGINE_CVAR_INTERFACE_VERSION, NULL));
#endif
}
bool HPK::RegisterConCommandBase(ConCommandBase *pVar)
{
	return META_REGCVAR(pVar);;
}
void HPK::LoadPluginSettings(int clientMax)
{	
	if(g_FirstLoad == false)
	{
		SetupModSpesficInfo();


		g_FirstLoad = true;
		g_MaxClients = clientMax;
		g_HPKVars.SetupCallBacks();		
	}
	for(int i=1;i<=g_MaxClients;i++)
		g_IsConnected[i] = false;
	
	g_HPKVars.CvarCallback();
}
bool HPK::Load(PluginId id, ISmmAPI *ismm, char *error, size_t maxlen, bool late)
{
	PLUGIN_SAVEVARS();

	GET_V_IFACE_ANY(GetServerFactory, m_ServerDll, IServerGameDLL, INTERFACEVERSION_SERVERGAMEDLL);
	GET_V_IFACE_ANY(GetServerFactory, m_ServerClients, IServerGameClients, INTERFACEVERSION_SERVERGAMECLIENTS);
	GET_V_IFACE_ANY(GetServerFactory, m_InfoMngr, IPlayerInfoManager, INTERFACEVERSION_PLAYERINFOMANAGER);

	GET_V_IFACE_CURRENT(GetEngineFactory, m_Engine, IVEngineServer, INTERFACEVERSION_VENGINESERVER);
	GET_V_IFACE_CURRENT(GetEngineFactory, m_GameEventManager, IGameEventManager2, INTERFACEVERSION_GAMEEVENTSMANAGER2);
	GET_V_IFACE_CURRENT(GetEngineFactory, m_Helpers, IServerPluginHelpers, INTERFACEVERSION_ISERVERPLUGINHELPERS);
	GET_V_IFACE_CURRENT(GetEngineFactory, m_Sound, IEngineSound, IENGINESOUND_SERVER_INTERFACE_VERSION);

	//We're hooking the following things as POST, in order to seem like Server Plugins.
	//However, I don't actually know if Valve has done server plugins as POST or not.
	//Change the last parameter to 'false' in order to change this to PRE.
	//SH_ADD_HOOK_MEMFUNC means "SourceHook, Add Hook, Member Function".

	m_hooks.push_back(SH_ADD_HOOK(IServerGameDLL, ServerActivate, m_ServerDll, SH_MEMBER(this, &HPK::ServerActivate), true));
	m_hooks.push_back(SH_ADD_HOOK(IServerGameDLL, GameFrame, m_ServerDll, SH_MEMBER(this, &HPK::GameFrame), true));
	m_hooks.push_back(SH_ADD_HOOK(IServerGameClients, ClientDisconnect, m_ServerClients, SH_MEMBER(this, &HPK::ClientDisconnect), true));
	m_hooks.push_back(SH_ADD_HOOK(IServerGameClients, ClientPutInServer, m_ServerClients, SH_MEMBER(this, &HPK::ClientPutInServer), true));

	//The following functions are pre handled, because that's how they are in IServerPluginCallbacks
	m_Engine_CC = SH_GET_CALLCLASS(m_Engine); // invoking their hooks (when needed).

	g_pCVar = GetICVar();

#if BAT_ORANGEBOX == 1
	ConVar_Register(0,this);
#else
	ConCommandBaseMgr::OneTimeInit(this);
#endif
	
	SH_CALL(m_Engine_CC, &IVEngineServer::LogPrint)("All hooks started!\n");
	if(late)
	{
		LoadPluginSettings(ismm->GetCGlobals()->maxClients);
		ServerCommand("echo [HPK] Late load is not really supported, plugin will function properly after 1 mapchange");
	}
	return true;
}

bool HPK::Unload(char *error, size_t maxlen)
{
	
	//IT IS CRUCIAL THAT YOU REMOVE CVARS.
	//As of Metamod:Source 1.00-RC2, it will automatically remove them for you.
	//But this is only if you've registered them correctly!

	//Make sure we remove any hooks we did... this may not be necessary since
	//SourceHook is capable of unloading plugins' hooks itself, but just to be safe.
	
	for (size_t i = 0; i < m_hooks.size(); i++)
	{
		if (m_hooks[i] != 0)
		{
			SH_REMOVE_HOOK_ID(m_hooks[i]);
		}
	}
	m_hooks.clear();

	//this, sourcehook does not keep track of.  we must do this.
	SH_RELEASE_CALLCLASS(m_Engine_CC);


	return true;
}
void HPK::AllPluginsLoaded()
{
	//AdminInterfaceListner *AdminInterfacePointer;

	if(m_AdminManager) // We dont need to find the AdminInterface again, we allready found it once.
		return;

	//we don't really need this for anything other than interplugin communication
	//and that's not used in this plugin.
	//If we really wanted, we could override the factories so other plugins can request
	// interfaces we make.  In this callback, the plugin could be assured that either
	// the interfaces it requires were either loaded in another plugin or not.
	PluginId id2; 

	void *ptr = g_SMAPI->MetaFactory("AdminInterface", NULL, &id2); 
	
	if (!ptr) 
	{
		ServerCommand("echo [HPK Error] Did not find AdminInterface, plugin will not check admin rights"); 
	} else if(id2 > 100) { // This is to prevent a crash bug, If the id2 is to high, then the pointer can be pass the null check, but its still just garbage and crashes
		m_AdminManager = (AdminInterface *)ptr;
		m_AdminManager->AddEventListner(this);
		int InterfaceVersion = m_AdminManager->GetInterfaceVersion();
		
		if(InterfaceVersion == ADMININTERFACE_VERSION)
		{
			ServerCommand("echo [HPK] Found AdminInterface via %s (Interface version: %d)",m_AdminManager->GetModName(),InterfaceVersion); 
#if HPK_DEBUG == 1
			ServerCommand("echo [HPK] Found AdminInterface[%d] at (%p), via %s (Interface version: %d)", id2, ptr,m_AdminManager->GetModName(),InterfaceVersion); 
#endif
		}
		else
			ServerCommand("echo [HPK] Found AdminInterface via %s (Interface version: %d), but internal version is %d",m_AdminManager->GetModName(),InterfaceVersion,ADMININTERFACE_VERSION); 
	}
}
void HPK::OnAdminInterfaceUnload() // Called when the admin interface is getting unloaded, this happens if a admin uses meta unload or the server is shutting down.
{
	m_AdminManager = NULL;
	//g_HPKCore.ServerCommand("echo AdminInterface was unloaded");
}

void HPK::Client_Authorized(int id) // This is called when a client is fully connected and authed by the admin tool, this normaly happens after the client has gotten a valid steamid & the admin tool has checked it against its internal user lists.
{
#if HPK_DEBUG == 1
	ServerCommand("echo [HPK Debug]Client_Authorized: %d",id);
#endif
}