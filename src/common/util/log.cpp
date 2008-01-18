/*
* psplog.cpp -- Christophe Painchaud aka Atanor, DaSH <dash@ionblast.net>
*
* Copyright (C) 2001 Atomic Blue (info@planeshift.it, http://www.atomicblue.org)
*
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* as published by the Free Software Foundation (version 2 of the License)
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*
*/
#include <psconfig.h>

#include <csutil/csstring.h>
#include <iutil/objreg.h>
#include <ivaria/reporter.h>
#include "util/consoleout.h"
#include "util/log.h"
#include <iutil/cfgmgr.h>


namespace pslog
{

iObjectRegistry* logger;
bool disp_flag[MAX_FLAGS];
uint32 filters_id[MAX_FLAGS];

const char *flagnames[] = {
                        "LOG_ANY",
                        "LOG_WEATHER",
                        "LOG_SPAWN",
                        "LOG_CELPERSIST",
                        "LOG_PAWS",
                        "LOG_GROUP",
                        "LOG_CHEAT",
                        "LOG_LINMOVE",
                        "LOG_SPELLS",
                        "LOG_NEWCHAR",
                        "LOG_SUPERCLIENT",
                        "LOG_EXCHANGES",
                        "LOG_ADMIN",
                        "LOG_STARTUP",
                        "LOG_CHARACTER",
                        "LOG_CONNECTIONS",
                        "LOG_CHAT",
                        "LOG_NET",
                        "LOG_LOAD",
                        "LOG_NPC",
                        "LOG_TRADE",
                        "LOG_SOUND",
                        "LOG_COMBAT",
                        "LOG_SKILLXP",
                        "LOG_QUESTS",
                        "LOG_SCRIPT",
                        "LOG_MARRIAGE",
                        "LOG_MESSAGES",
                        "LOG_CACHE",
                        "LOG_PETS",
                        "LOG_USER",
                        "LOG_LOOT",
                        "LOG_DUELS",
                        "LOG_TRIBES"
}; // End of flagnames

const char *flagsetting[] = {
                        "Planeshift.Log.Any",
                        "Planeshift.Log.Weather",
                        "Planeshift.Log.Spawn",
                        "Planeshift.Log.Cel",
                        "Planeshift.Log.PAWS",
                        "Planeshift.Log.Group",
                        "Planeshift.Log.Cheat",
                        "Planeshift.Log.Linmove",
                        "Planeshift.Log.Spells",
                        "Planeshift.Log.Newchar",
                        "Planeshift.Log.Superclient",
                        "Planeshift.Log.Exchanges",
                        "Planeshift.Log.Admin",
                        "Planeshift.Log.Startup",
                        "Planeshift.Log.Character",
                        "Planeshift.Log.Connections",
                        "Planeshift.Log.Chat",
                        "Planeshift.Log.Net",
                        "Planeshift.Log.Load",
                        "Planeshift.Log.NPC",
                        "Planeshift.Log.Trade",
                        "Planeshift.Log.Sound",
                        "Planeshift.Log.Combat",
                        "Planeshift.Log.SkillXP",
                        "Planeshift.Log.Quests",
                        "Planeshift.Log.Script",
                        "Planeshift.Log.Marriage",
                        "Planeshift.Log.Messages",
                        "Planeshift.Log.Cache",
                        "Planeshift.Log.Pets",
                        "Planeshift.Log.User",
                        "Planeshift.Log.Loot",
                        "Planeshift.Log.Duels",
                        "Planeshift.Log.Tribes"
}; // End of flagsettings

bool DoLog(int severity, LOG_TYPES type, uint32 filter_id)
{
    if (logger == 0)
        return false;
    if (!disp_flag[type] && severity > CS_REPORTER_SEVERITY_WARNING)
        return false;
    if (filters_id[type]!=0 && filter_id!=0 && filters_id[type]!=filter_id)
        return false;
    return true;
}


void LogMessage (const char* file, int line, const char* function,
             int severity, LOG_TYPES type, uint32 filter_id, const char* msg, ...)
{
    if (!DoLog(severity,type,filter_id)) return;

    va_list arg;

    ConsoleOutMsgClass con = CON_SPAM;
    switch (severity)
    {
        case CS_REPORTER_SEVERITY_WARNING: con = CON_WARNING; break;
        case CS_REPORTER_SEVERITY_NOTIFY: con = CON_NOTIFY; break;
        case CS_REPORTER_SEVERITY_ERROR: con = CON_ERROR; break;
        case CS_REPORTER_SEVERITY_BUG: con = CON_BUG; break;
        case CS_REPORTER_SEVERITY_DEBUG: con = CON_DEBUG; break;
    }

    // Log to file
    va_start(arg, msg);
    CVPrintfLog (con, msg, arg);
    va_end(arg);

    if(con <= ConsoleOut::GetMaximumOutputClassStdout())
    {
        char msgid[5000];
        snprintf (msgid, 5000, "<%s:%d %s>\n",
            file, line, function);

        char description[5001];
        va_start(arg, msg);
        cs_vsnprintf (description, 5000, msg, arg);
        va_end(arg);
        size_t len = strlen(description);
        description[len++] = '\n';
        description[len] = '\0';

        // Disable use of csReport while testing using CPrintf
        //        va_start(arg, msg);
        //        csReportV (logger, severity, msgid, msg, arg);
        //        va_end(arg);
        CPrintf(con,msgid);
        CPrintf(con,description);

        /* ERR and BUG will be loged to errorLog by the CPrintf
        // Log errors to a file so they can be emailed to devs daily.
        if (severity == CS_REPORTER_SEVERITY_ERROR ||
            severity == CS_REPORTER_SEVERITY_BUG)
        {
            if(!errorLog)
                errorLog = fopen("errorlog.txt","w");
            fprintf(errorLog,msgid);
            fprintf(errorLog," ");
            va_start(arg, msg);
            vfprintf(errorLog,msg,arg);
            va_end(arg);
            fprintf(errorLog, "\n");
            fflush(errorLog);
        }
        */
    }
}


void Initialize(iObjectRegistry* object_reg)
{
    logger = object_reg;

    static bool initialized = false;
    if (initialized) return;
    initialized = true;

    for (int i = 0; i < MAX_FLAGS; i++)
    {
        disp_flag[i] = false;
    }

    disp_flag[LOG_ANY]         = true;
    disp_flag[LOG_CONNECTIONS] = true;
    disp_flag[LOG_CHAT]        = true;
    disp_flag[LOG_NET]         = true;
    disp_flag[LOG_CHARACTER]   = true;
    disp_flag[LOG_NEWCHAR]     = true;
}

void SetFlag(int index, bool flag, uint32 filter)
{
    csString str;

    disp_flag[index] = flag;
    
    str.AppendFmt("%s flag %s ",flagnames[index],flag?"activated":"deactivated");
    if (filter!=0 && filter!=(uint32)-1)
    {
        filters_id[index]=filter;
        str.AppendFmt("with filter %d.\n",filter);
    } else
    {
        filters_id[index]=0;
        str.AppendFmt("with no filter.\n");
    }
    CPrintf(CON_CMDOUTPUT,str.GetDataSafe());
}


void SetFlag(const char *name, bool flag, uint32 filter)
{
    bool all = !strcasecmp(name, "all");
    bool unique = true;
    int index = -1;
    
    for (int i=0; i<MAX_FLAGS; i++)
    {
        if (!flagnames[i])
            continue;

        if (all)
        {
            SetFlag(i, flag, filter);
        }

        if (!all && strcasestr(flagnames[i], name))
        {
            if (unique && index != -1)
            {
                // We have found another  match already.
                CPrintf(CON_CMDOUTPUT, "'%s' isn't an unique flag name!\n",name);
                unique = false;
                CPrintf(CON_CMDOUTPUT, "%s\n", flagnames[index]);
            }
            index = i;
            if (!unique)
            {
                CPrintf(CON_CMDOUTPUT, "%s\n", flagnames[index]);
            }
        }

    }
    if (all)
    {
        return;
    }
    
    if (index == -1)
    {
        CPrintf(CON_CMDOUTPUT, "No flag found with the name '%s'!\n",name);
        return;
    }

    if (unique)
    {
        SetFlag(index, flag, filter);
    }
}

void DisplayFlags(const char *name)
{
    for (int i=0; i<MAX_FLAGS; i++)
    {
        if (!name || !strcmp(flagnames[i],name))
        {
            csString str;
            str.AppendFmt("%s = %s ",flagnames[i],disp_flag[i]?"true":"false");
            if (filters_id[i]!=0)
            {
                str.AppendFmt("with filter %d \n",filters_id[i]);
            }
            else
            {
                str.AppendFmt("\n");
            }
            CPrintf(CON_CMDOUTPUT,str.GetDataSafe());
        }
    }
}

bool GetValue(const char *name)
{
    for (int i=0; i< MAX_FLAGS; i++)
    {
        if (flagnames[i] && !strcmp(flagnames[i],name))
        {
            return disp_flag[i];            
        }
    }
    return false;
}

const char* GetName(int id)
{
    return flagnames[id];
}

const char* GetSettingName(int id)
{
    return flagsetting[id];
}


} // end of namespace pslog

LogCSV::LogCSV(iConfigManager* configmanager, iVFS* vfs)
{
    csString filename[MAX_CSV];
    csString header;

    size_t maxSize = configmanager->GetInt("Planeshift.LogCSV.MaxSize", 10*1024*1024);
    filename[CSV_PALADIN] = configmanager->GetStr("Planeshift.LogCSV.File.Paladin");
    filename[CSV_EXCHANGES] = configmanager->GetStr("Planeshift.LogCSV.File.Exchanges");
    filename[CSV_AUTHENT] = configmanager->GetStr("Planeshift.LogCSV.File.Authent");
    filename[CSV_STATUS] = configmanager->GetStr("Planeshift.LogCSV.File.Status");
    filename[CSV_ADVICE] = configmanager->GetStr("Planeshift.LogCSV.File.Advice");
    filename[CSV_ECONOMY] = configmanager->GetStr("Planeshift.LogCSV.File.Economy");
    filename[CSV_STUCK] = configmanager->GetStr("Planeshift.LogCSV.File.Stuck");

    for(int i = 0;i < MAX_CSV;i++)
    {
		bool writeHeader = false;
        if(filename[i].IsEmpty())
        {
            csvFile[i] = NULL;
            continue;
        }
        if (!(vfs->Exists(filename[i])))
        {
            csvFile[i] = vfs->Open(filename[i],VFS_FILE_WRITE);
			writeHeader = true;
            
        }
        else
        {

            csvFile[i] = vfs->Open(filename[i],VFS_FILE_APPEND);

			// Need to rotate log
            if (csvFile[i] && csvFile[i]->GetSize() > maxSize)
            {
                CPrintf(CON_ERROR, "Log File %s is too big! Current size is: %u. Rotating log.", filename[i].GetData(), csvFile[i]->GetSize());

				csvFile[i] = NULL;

				// Rolling history
				for (int index = 4; index > 0; index--)
				{
                    csString src(filename[i]), dst(filename[i]);
                    src.Append(index);
                    dst.Append(index + 1);
					// Rotate the files (move file[index] to file[index+1])
					if (vfs->Exists(src))
					{
						csRef<iDataBuffer> existingData = vfs->ReadFile(src, false);
						vfs->WriteFile(dst, existingData->GetData(), existingData->GetSize());
					}
				}

				csRef<iDataBuffer> existingData = vfs->ReadFile(filename[i], false);
				vfs->WriteFile(filename[i] + "1", existingData->GetData(), existingData->GetSize());
                csvFile[i] = vfs->Open(filename[i],VFS_FILE_WRITE);

				writeHeader = true;
            }
        }
		if(writeHeader)
		{
			switch(i)
            {
            case CSV_PALADIN: 
                header = "Date/Time, Client, Type, Sector, Start pos (xyz), Maximum displacement, Real displacement, Start velocity, Angular velocity, Paladin version\n";
                break;
            case CSV_EXCHANGES:
                header = "Date/Time, Source Client, Target Client, Type, Item, Quantity, Cost\n";
                break;
            case CSV_AUTHENT:
                header = "Date/Time, Client, Client ID, Details\n";
                break;
            case CSV_STATUS:
                header = "Date/Time, Status\n";
                break;
            case CSV_ADVICE:
                header = "Date/Time, Source Client, Target Client, Message\n";
                break;
            case CSV_ECONOMY:
                header = "Action, Count, Item, Quality, From, To, Price, Time\n";
                break;
            case CSV_STUCK:
                header = "Date/Time, Client, Race, Gender, Sector, PosX, PosY,"
                         " PosZ, Direction\n";
                break;
            }
            csvFile[i]->Write(header.GetData(), header.Length());
            csvFile[i]->Flush();
		}
    }
}

void LogCSV::Write(int type, csString& text)
{
    if (!csvFile[type])
        return;


    time_t curtime = time(NULL);
    struct tm *loctime;
    loctime = localtime (&curtime);
    csString buf(asctime(loctime));
    buf.Truncate(buf.Length()-1);

    buf.Append(", ");
    buf.Append(text);
    buf.Append("\n");

    csvFile[type]->Write(buf, buf.Length());

    static unsigned count = 0;

    if (type == CSV_STATUS)
        csvFile[type]->Flush();
    else
    {
        // Unfair flushing mechanism for the time being.
        count++;
        if(count % 5 == 0)
        {
            count = 0;
            csvFile[type]->Flush();
        }
    }
}
