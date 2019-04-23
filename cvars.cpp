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

#include "ForgiveTK.h"
#include "cvars.h"

extern HPKOptionsStruct g_HPKSettings;
extern HPK g_HPKCore;

HPKVars g_HPKVars;

ConVar g_VarHPKVersion("hpk_version",HPK_VERSION, FCVAR_SPONLY|FCVAR_REPLICATED|FCVAR_NOTIFY, "The version of High Ping Kicker");
ConVar g_VarHPKOption("hpk_option", "1", 0, "\n0 = System Disabled\n1 = Enable the HPK system");

ConVar g_VarHPKMaxPing("hpk_maxping", "150", 0, "The maximum ping allowed on the server");
ConVar g_VarHPKMaxPingAdd("hpk_maxping_add", "50", 0, "If the ping goes above this, the user gets a extra 5 bad ping counts");
ConVar g_VarHPKMaxPingCount("hpk_maxping_count", "15", 0, "The maximum number of high ping count a user can get in 1 map (1 check pr second)");
ConVar g_VarHPKMinPlayers("hpk_minimumplayers", "4", 0, "The minimum players needed before ping checks happen");


bool HPKVars::RegisterConCommandBase(ConCommandBase *pVar)
{
	//this will work on any type of concmd!
	return META_REGCVAR(pVar);
}
bool HPKVars::DoCvarChecks()
{
	if(g_VarHPKOption.GetInt() == 1)
		return true;

	return false;
}

void HPKVars::SetupCallBacks() // Yes i know we read all the cvars, when just 1 changed ( Or we can end up reading all 4 chars 4 times. But who cares, this should only happen on a mapchange and im lazy )
{
	g_VarHPKMaxPingAdd.InstallChangeCallback((FnChangeCallback_t) &HPKVars::CvarCallback);
	g_VarHPKMaxPingCount.InstallChangeCallback((FnChangeCallback_t) &HPKVars::CvarCallback);
	g_VarHPKMaxPing.InstallChangeCallback((FnChangeCallback_t) &HPKVars::CvarCallback);
}
void HPKVars::CvarCallback()
{
	g_HPKSettings.ExtraCountPing = g_VarHPKMaxPingAdd.GetInt();
	g_HPKSettings.MaxFailedChecks = g_VarHPKMaxPingCount.GetInt();
	g_HPKSettings.MaxPing = g_VarHPKMaxPing.GetInt();
}
/*
CON_COMMAND(admin_test, "Sample Plugin command")
{
	META_LOG(g_PLAPI, "This sentence is in Spanish when you're not looking.");
}
*/