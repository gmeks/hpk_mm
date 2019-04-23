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

#include <metamod_oslink.h>
#include "ForgiveTK.h"
#include "time.h"
#include "cvars.h"
#include "convar.h"
#include "const.h"
#include <string.h>
#include <bitbuf.h>
#include <ctype.h>
#include "hl2sdk/recipientfilters.h"

//#pragma warning(disable:4996)

extern int g_MaxClients;
extern int g_ModIndex;
extern bool g_IsConnected[MAXPLAYERS+1];
extern ConstPlayerInfo g_UserInfo[MAXPLAYERS+1];
extern HPKOptionsStruct g_HPKSettings;
extern ModSettingsStruct g_ModSettings;
extern AdminInterface *m_AdminManager;

void HPK::RegisterHighPingCount(int id,int NewPing)
{
	if(NewPing >= (g_HPKSettings.MaxPing+g_HPKSettings.ExtraCountPing))
		g_UserInfo[id].HPKCount += 5;
	else
		g_UserInfo[id].HPKCount += 1;

	
	if(g_UserInfo[id].HPKCount >= g_HPKSettings.MaxFailedChecks)
	{
		// The user is getting kicked now
		g_IsConnected[id] = false;

		MessagePlayer(0,"[HPK] %s was kicked due to high ping ( Was %d, max is %d)",GetPlayerName(id),NewPing,g_HPKSettings.MaxPing);
		ServerLogPrint("[HPK] %s<%s> was kicked due to high ping ( %d )",GetPlayerName(id),g_UserInfo[id].steamid,NewPing);

		if(m_AdminManager)
		{
			if(!m_AdminManager->HasFlag(id,"immunity"))
				ServerCommand("kickid %d Kicked due to high ping violation",g_UserInfo[id].userid);
		}
		else
			ServerCommand("kickid %d Kicked due to high ping violation",g_UserInfo[id].userid);
	}
	else
	{
		MessagePlayer(id,"[HPK] %s your ping was to high %d (Max ping is: %d) warning %d/%d",GetPlayerName(id),NewPing,g_HPKSettings.MaxPing,g_UserInfo[id].HPKCount,g_HPKSettings.MaxFailedChecks);
		ServerLogPrint("[HPK] %s ping was to high %d (Max ping is: %d) warning %d/%d",GetPlayerName(id),NewPing,g_HPKSettings.MaxPing,g_UserInfo[id].HPKCount,g_HPKSettings.MaxFailedChecks);
	}
}
int HPK::GetPrecentageOfPlayers(int Players,int PlayerCount)
{
	if(Players == 0 || PlayerCount == 0)
		return 0;

	return int((Players * 100) / PlayerCount);
}
void HPK::SendCSay(int id,const char *msg, ...) 
{
	char vafmt[192];
	va_list ap;
	va_start(ap, msg);
	_vsnprintf(vafmt,191,msg,ap);
	va_end(ap);

	for(int i=1;i<=g_MaxClients;i++)	
	{
		if(IsUserConnected(i) && !g_UserInfo[i].IsBot)
			m_Engine->ClientPrintf(g_UserInfo[i].PlayerEdict,vafmt);
	}

	if(g_ModIndex != MOD_CSTRIKE && g_ModIndex != MOD_DOD)
	{
		RecipientFilter mrf;
		
		if(id == 0)
			mrf.AddAllPlayers(g_MaxClients);
		else
			mrf.AddPlayer(id);

		bf_write *msg = m_Engine->UserMessageBegin((RecipientFilter *)&mrf,g_ModSettings.TextMsg);
		//msg->WriteByte(4); //4
		//msg->WriteString(temp);
		msg->WriteByte( 0); //textparms.channel
		msg->WriteFloat( -1 ); // textparms.x ( -1 = center )
		msg->WriteFloat( -.25 ); // textparms.y ( -1 = center )
		msg->WriteByte( 0xFF ); //textparms.r1
		msg->WriteByte( 0x00 ); //textparms.g1
		msg->WriteByte( 0x00 ); //textparms.b1
		msg->WriteByte( 0xFF ); //textparms.a2
		msg->WriteByte( 0xFF ); //textparms.r2
		msg->WriteByte( 0xFF ); //textparms.g2
		msg->WriteByte( 0xFF ); //textparms.b2
		msg->WriteByte( 0xFF ); //textparms.a2
		msg->WriteByte( 0x00); //textparms.effect
		msg->WriteFloat( 0 ); //textparms.fadeinTime
		msg->WriteFloat( 0 ); //textparms.fadeoutTime
		msg->WriteFloat( 10 ); //textparms.holdtime
		msg->WriteFloat( 45 ); //textparms.fxtime
		msg->WriteString( vafmt ); //Message
		m_Engine->MessageEnd();
	}
	else
		MessagePlayer(id,vafmt);
}
bool HPK::IsUserAlive(int id)
{
	IPlayerInfo *pPlayerInfo = m_InfoMngr->GetPlayerInfo(g_UserInfo[id].PlayerEdict);
	if(!pPlayerInfo)
		return false;

	bool IsDead = pPlayerInfo->IsDead();

	if(!IsDead)
		return true;
	return false;
}
bool HPK::IsUserConnected(int id)
{
	if(!g_IsConnected[id])		// We save the poor cpu some work and check our internal bool first
		return false;

	edict_t *e = m_Engine->PEntityOfEntIndex(id);
	if(!e || e->IsFree() )
		return false;

	//IPlayerInfo *pInfo = g_BATCore.PlayerInfo()->GetPlayerInfo(e);	
	IPlayerInfo *pInfo = m_InfoMngr->GetPlayerInfo(e);
	if(!pInfo || !pInfo->IsConnected())
		return false;
	return true;
}
int HPK::GetUserTeam(int id)
{
	edict_t *e = m_Engine->PEntityOfEntIndex(id);
	if(!e || e->IsFree() || !IsUserConnected(id))
		return TEAM_NOT_CONNECTED;

	IPlayerInfo *pInfo = m_InfoMngr->GetPlayerInfo(e);
	return pInfo->GetTeamIndex();
}
void HPK::GetModName()		// This function gets the actual mod name, and not the full pathname
{
	char path[128];
	char ModName[48];
	m_Engine->GetGameDir(path,127);

	const char *modname = path;
	for (const char *iter = path; *iter; ++iter)
		{
		if (*iter == '/' || *iter == '\\') // or something
			modname = iter + 1;
		}
	strncpy(ModName,modname,47);

	if(strcmp(ModName,"cstrike") == 0)
		g_ModIndex = MOD_CSTRIKE;
	else if(strcmp(ModName,"dod") == 0 || stricmp(ModName,"DODC") == 0)
		g_ModIndex = MOD_DOD;
	else if(strcmp(ModName,"hl2mp") == 0)
		g_ModIndex = MOD_HL2MP;
	else 
		g_ModIndex = MOD_NONE;	
}
int HPK::GetMsgNum(const char *Msg)
{

	char temp[40];
	int dontdoit=0;
	int MaxScan= g_SMAPI->GetUserMessageCount();

	for (int i=0;i<MaxScan;i++) 
	{
		g_HPKCore.GetServerGameDll()->GetUserMessageInfo(i,temp,39,dontdoit);
		if(strcmp(Msg,temp) == 0)
			return i;
	}
	ServerLogPrint("ERROR: HPK failed to find the message number for %s , this could create all sorts of errors/crashes",Msg);
	return -1;
}
void HPK::SetupModSpesficInfo()	// Gets the menu message id, for the running mod
{
	GetModName();

	g_ModSettings.HintText = GetMsgNum("HintText");
	g_ModSettings.HudMsg = GetMsgNum("HudMsg");
	g_ModSettings.MenuMsg = GetMsgNum("ShowMenu");
	g_ModSettings.TextMsg = GetMsgNum("TextMsg");
	g_ModSettings.SayText = GetMsgNum("SayText");

	if(g_ModIndex == MOD_CSTRIKE || g_ModIndex == MOD_DOD || g_ModIndex == MOD_HL2CTF)
		g_ModSettings.MenuSupport = true;
	else 
		g_ModSettings.MenuSupport = false;		
}

void HPK::ServerLogPrint(const char *fmt, ...)
{
	char buffer[512];
	va_list ap;
	va_start(ap, fmt);
	_vsnprintf(buffer, sizeof(buffer)-2, fmt, ap);
	va_end(ap);
	strcat(buffer,"\n");
	META_CONPRINTF(buffer);
}
void HPK::ServerCommand(const char *fmt, ...)
{
	char buffer[512];
	va_list ap;
	va_start(ap, fmt);
	_vsnprintf(buffer, sizeof(buffer)-2, fmt, ap);
	va_end(ap);
	strcat(buffer,"\n");
	m_Engine->ServerCommand(buffer);
}
int HPK::FindPlayer(int UserID)	// Finds the player index based on userid.
{
	for(int i=1;i<=g_MaxClients;i++)
	{
		if(g_IsConnected[i] == true)
		{
			if(UserID == g_UserInfo[i].userid )	// Name or steamid matches TargetInfo so we return the index of the player
			{
				return i;
			}
		}
	}
	return -1;
}
void HPK::MessagePlayer(int index, const char *msg, ...)
{
	 RecipientFilter rf;
	 if (index > g_MaxClients || index < 0)
		 return;

	 if (index == 0)
	 {
		 rf.AddAllPlayers(g_MaxClients);
	 } else {
		rf.AddPlayer(index);
	 }
	 rf.MakeReliable();

	 char vafmt[192];
	 va_list ap;
	 va_start(ap, msg);
	 int len = _vsnprintf(vafmt, 189, msg, ap);
	 va_end(ap);
	 len += _snprintf(&(vafmt[len]),191-len," \n");

	 bf_write *buffer = m_Engine->UserMessageBegin(static_cast<IRecipientFilter *>(&rf), g_ModSettings.SayText);
	 buffer->WriteByte(0);
	 buffer->WriteString(vafmt);
	 buffer->WriteByte(1);
	 m_Engine->MessageEnd();
}
const char *HPK::GetPlayerName(int id)
{
	return m_Engine->GetClientConVarValue(id,"name");
}
bool HPK::HasAdminImmunity(int id)
{
	if(m_AdminManager == NULL)
		return false;

	return m_AdminManager->HasFlag(id,"immunity");
}
int HPK::str_replace(char *str, const char *from, const char *to, int maxlen) // Function from sslice 
{
	char  *pstr   = str;
	int   fromlen = Q_strlen(from);
	int   tolen   = Q_strlen(to);
	int	  RC=0;		// Removed count

	while (*pstr != '\0' && pstr - str < maxlen) {
		if (Q_strncmp(pstr, from, fromlen) != 0) {
			*pstr++;
			continue;
		}
		Q_memmove(pstr + tolen, pstr + fromlen, maxlen - ((pstr + tolen) - str) - 1);
		Q_memcpy(pstr, to, tolen);
		pstr += tolen;
		RC++;
	}
	return RC;
}