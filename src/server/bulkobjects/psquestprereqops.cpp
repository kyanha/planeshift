/*
 * psquestprereqopts.cpp
 *
 * Copyright (C) 2005 Atomic Blue (info@planeshift.it, http://www.atomicblue.org)
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
//=============================================================================
// Crystal Space Includes
//=============================================================================

//=============================================================================
// Project Includes
//=============================================================================
#include "rpgrules/factions.h"

#include "../gem.h"
#include "../globals.h"

//=============================================================================
// Local Includes
//=============================================================================
#include "psquestprereqops.h"
#include "pscharacter.h"
#include "psquest.h"
#include "weathermanager.h"
#include "cachemanager.h"
#include "psraceinfo.h"
#include "psguildinfo.h"

///////////////////////////////////////////////////////////////////////////////////////////

csString psQuestPrereqOp::GetScript()
{
    csString script;
    script.Append("<pre>");
    script.Append(GetScriptOp());
    script.Append("</pre>");
    return script;
}


///////////////////////////////////////////////////////////////////////////////////////////

void psQuestPrereqOpList::Push(csRef<psQuestPrereqOp> prereqOp)
{
    prereqlist.Push(prereqOp);
}

void psQuestPrereqOpList::Insert(size_t n, csRef<psQuestPrereqOp> prereqOp)
{
    prereqlist.Insert(n,prereqOp);
}

///////////////////////////////////////////////////////////////////////////////////////////

bool psQuestPrereqOpAnd::Check(psCharacter * character)
{
    //Requirements are always valid for quest testers
    if(character->GetActor() && character->GetActor()->questtester)
        return true;

    // Check if all prereqs are valid
    for (size_t i = 0; i < prereqlist.GetSize(); i++)
    {
        if (!prereqlist[i]->Check(character))
        {
            return false;
        }
    }

    return true;
}

csString psQuestPrereqOpAnd::GetScriptOp()
{
    csString script;
    script.Append("<and>");
    for (size_t i = 0; i < prereqlist.GetSize(); i++)
    {
        script.Append(prereqlist[i]->GetScriptOp());
    }
    script.Append("</and>");
    return script;
}

csPtr<psQuestPrereqOp> psQuestPrereqOpAnd::Copy()
{
    csRef<psQuestPrereqOpAnd> copy;
    copy.AttachNew(new psQuestPrereqOpAnd());
    for (size_t i = 0; i < prereqlist.GetSize(); i++)
    {
        copy->Push(prereqlist[i]->Copy());
    }
    return csPtr<psQuestPrereqOp>(copy);
}

///////////////////////////////////////////////////////////////////////////////////////////

bool psQuestPrereqOpOr::Check(psCharacter * character)
{
    //Requirements are always valid for quest testers
    if(character->GetActor() && character->GetActor()->questtester)
        return true;

    // Check if any of the prereqs are valid
    for (size_t i = 0; i < prereqlist.GetSize(); i++)
    {
        if (prereqlist[i]->Check(character))
        {
            return true;
        }
    }

    return false;
}

csString psQuestPrereqOpOr::GetScriptOp()
{
    csString script;
    script.Append("<or>");
    for (size_t i = 0; i < prereqlist.GetSize(); i++)
    {
        script.Append(prereqlist[i]->GetScriptOp());
    }
    script.Append("</or>");
    return script;
}

csPtr<psQuestPrereqOp> psQuestPrereqOpOr::Copy()
{
    csRef<psQuestPrereqOpOr> copy;
    copy.AttachNew(new psQuestPrereqOpOr());
    for (size_t i = 0; i < prereqlist.GetSize(); i++)
    {
        copy->Push(prereqlist[i]->Copy());
    }
    return csPtr<psQuestPrereqOp>(copy);
}

///////////////////////////////////////////////////////////////////////////////////////////

psQuestPrereqOpRequire::psQuestPrereqOpRequire(int min_required,int max_required)
{
    min = min_required;
    max = max_required;
}

bool psQuestPrereqOpRequire::Check(psCharacter * character)
{
    //Requirements are always valid for quest testers
    if(character->GetActor() && character->GetActor()->questtester)
        return true;

    // Count the number of prereqs that is valid.
    int count=0;
    for (size_t i = 0; i < prereqlist.GetSize(); i++)
    {
        if (prereqlist[i]->Check(character))
        {
            count++;
        }
    }
    // Verify that the appropiate numbers of prereqs was counted.
    return ((min == -1 || count >= min) && (max == -1 || count <= max));
}

csString psQuestPrereqOpRequire::GetScriptOp()
{
    csString script;
    script.Append("<require");
    if (min != -1)
    {
        script.AppendFmt(" min=\"%d\"",min);
    }

    if (max != -1)
    {
        script.AppendFmt(" max=\"%d\"",max);
    }

    script.Append(">");
    for (size_t i = 0; i < prereqlist.GetSize(); i++)
    {
        script.Append(prereqlist[i]->GetScriptOp());
    }
    script.Append("</require>");
    return script;
}

csPtr<psQuestPrereqOp> psQuestPrereqOpRequire::Copy()
{
    csRef<psQuestPrereqOpRequire> copy;
    copy.AttachNew(new psQuestPrereqOpRequire(min,max));
    for (size_t i = 0; i < prereqlist.GetSize(); i++)
    {
        copy->Push(prereqlist[i]->Copy());
    }
    return csPtr<psQuestPrereqOp>(copy);
}

///////////////////////////////////////////////////////////////////////////////////////////

bool psQuestPrereqOpNot::Check(psCharacter * character)
{
    //Requirements are always valid for quest testers
    if(character->GetActor() && character->GetActor()->questtester)
        return true;

    return (prereqlist.GetSize() && !prereqlist[0]->Check(character));
}

csString psQuestPrereqOpNot::GetScriptOp()
{
    csString script;
    script.Append("<not>");
    if (prereqlist.GetSize())
    {
        script.Append(prereqlist[0]->GetScriptOp());
    }
    script.Append("</not>");
    return script;
}

csPtr<psQuestPrereqOp> psQuestPrereqOpNot::Copy()
{
    csRef<psQuestPrereqOpNot> copy;
    copy.AttachNew(new psQuestPrereqOpNot());
    copy->Push(prereqlist[0]->Copy());
    return csPtr<psQuestPrereqOp>(copy);
}

///////////////////////////////////////////////////////////////////////////////////////////

psQuestPrereqOpQuestCompleted::psQuestPrereqOpQuestCompleted(csString questName)
{
    quest = NULL;
    name = questName;
}

bool psQuestPrereqOpQuestCompleted::Check(psCharacter * character)
{
    //Requirements are always valid for quest testers
    if(character->GetActor() && character->GetActor()->questtester)
        return true;

    if (quest == NULL)
        quest = CacheManager::GetSingleton().GetQuestByName(name);
    return character->CheckQuestCompleted(quest);
}

csString psQuestPrereqOpQuestCompleted::GetScriptOp()
{
    csString script;

    script.AppendFmt("<completed quest=\"%s\"/>",quest->GetName());

    return script;
}

csPtr<psQuestPrereqOp> psQuestPrereqOpQuestCompleted::Copy()
{
    csRef<psQuestPrereqOpQuestCompleted> copy;

    if (quest==NULL)
        copy.AttachNew(new psQuestPrereqOpQuestCompleted(name));
    else
        copy.AttachNew(new psQuestPrereqOpQuestCompleted(quest));
    return csPtr<psQuestPrereqOp>(copy);
}

///////////////////////////////////////////////////////////////////////////////////////////

bool psQuestPrereqOpQuestAssigned::Check(psCharacter * character)
{
    //Requirements are always valid for quest testers
    if(character->GetActor() && character->GetActor()->questtester)
        return true;

    return character->CheckQuestAssigned(quest);
}

csString psQuestPrereqOpQuestAssigned::GetScriptOp()
{
    csString script;

    script.AppendFmt("<assigned quest=\"%s\"/>",quest->GetName());

    return script;
}

csPtr<psQuestPrereqOp> psQuestPrereqOpQuestAssigned::Copy()
{
    csRef<psQuestPrereqOpQuestAssigned> copy;
    copy.AttachNew(new psQuestPrereqOpQuestAssigned(quest));
    return csPtr<psQuestPrereqOp>(copy);
}

///////////////////////////////////////////////////////////////////////////////////////////

bool psQuestPrereqOpQuestCompletedCategory::Check(psCharacter * character)
{
    //Requirements are always valid for quest testers
    if(character->GetActor() && character->GetActor()->questtester)
        return true;

    int count = character->NumberOfQuestsCompleted(category);

    Debug5(LOG_QUESTS, character->GetPID().Unbox(), "Check for category %s in range %d <= %d <= %d",
           category.GetDataSafe(),min,count,max);

    // Verify that the appropiate numbers of quest of given category is done.
    return ((min == -1 || min <= count) && (max == -1 || count <= max));
}

csString psQuestPrereqOpQuestCompletedCategory::GetScriptOp()
{
    csString script;

    script.AppendFmt("<completed category=\"%s\"",category.GetDataSafe());
    if (min != -1)
    {
        script.AppendFmt(" min=\"%d\"",min);
    }

    if (max != -1)
    {
        script.AppendFmt(" max=\"%d\"",max);
    }
    script.Append("/>");

    return script;
}

csPtr<psQuestPrereqOp> psQuestPrereqOpQuestCompletedCategory::Copy()
{
    csRef<psQuestPrereqOpQuestCompletedCategory> copy;
    copy.AttachNew(new psQuestPrereqOpQuestCompletedCategory(category,min,max));
    return csPtr<psQuestPrereqOp>(copy);
}

///////////////////////////////////////////////////////////////////////////////////////////

bool psQuestPrereqOpFaction::Check(psCharacter * character)
{
    //Requirements are always valid for quest testers
    if(character->GetActor() && character->GetActor()->questtester)
        return true;

    if(max)
    {
        // If value is max, make sure we're below it
        return !character->CheckFaction(faction,value);
    }
    return character->CheckFaction(faction,value);
}

csString psQuestPrereqOpFaction::GetScriptOp()
{
    csString script;

    script.AppendFmt("<faction name=\"%s\" value=\"%d\" max=\"%d\"/>",faction->name.GetData(),value,max);

    return script;
}

csPtr<psQuestPrereqOp> psQuestPrereqOpFaction::Copy()
{
    csRef<psQuestPrereqOpFaction> copy;
    copy.AttachNew(new psQuestPrereqOpFaction(faction,value,max));
    return csPtr<psQuestPrereqOp>(copy);
}

///////////////////////////////////////////////////////////////////////////////////////////

bool psQuestPrereqOpActiveMagic::Check(psCharacter * character)
{
    //Requirements are always valid for quest testers
    if(character->GetActor() && character->GetActor()->questtester)
        return true;

    if (character->GetActor())
    {
        return character->GetActor()->IsMagicCategoryActive(activeMagic);
    }
    return false;
}

csString psQuestPrereqOpActiveMagic::GetScriptOp()
{
    csString script;

    script.Format("<activemagic name=\"%s\"/>", activeMagic.GetData());

    return script;
}

csPtr<psQuestPrereqOp> psQuestPrereqOpActiveMagic::Copy()
{
    csRef<psQuestPrereqOpActiveMagic> copy;
    copy.AttachNew(new psQuestPrereqOpActiveMagic(activeMagic));
    return csPtr<psQuestPrereqOp>(copy);
}

///////////////////////////////////////////////////////////////////////////////////////////

bool psQuestPrereqOpRace::Check(psCharacter * character)
{
    //Requirements are always valid for quest testers
    if(character->GetActor() && character->GetActor()->questtester)
        return true;

    if (character->GetRaceInfo())
    {
        return ((csString)character->GetRaceInfo()->GetRace() == race);
    }
    return false;
}

csString psQuestPrereqOpRace::GetScriptOp()
{
    csString script;

    script.Format("<race name=\"%s\"/>", race.GetData());

    return script;
}

csPtr<psQuestPrereqOp> psQuestPrereqOpRace::Copy()
{
    csRef<psQuestPrereqOpRace> copy;
    copy.AttachNew(new psQuestPrereqOpRace(race));
    return csPtr<psQuestPrereqOp>(copy);
}

///////////////////////////////////////////////////////////////////////////////////////////

bool psQuestPrereqOpGender::Check(psCharacter * character)
{
    //Requirements are always valid for quest testers
    if(character->GetActor() && character->GetActor()->questtester)
        return true;

    if (character->GetRaceInfo())
    {
        return ((csString)character->GetRaceInfo()->GetGender() == gender);
    }
    return false;
}

csString psQuestPrereqOpGender::GetScriptOp()
{
    csString script;

    script.Format("<gender type=\"%s\"/>", gender.GetData());

    return script;
}

csPtr<psQuestPrereqOp> psQuestPrereqOpGender::Copy()
{
    csRef<psQuestPrereqOpGender> copy;
    copy.AttachNew(new psQuestPrereqOpGender(gender));
    return csPtr<psQuestPrereqOp>(copy);
}

///////////////////////////////////////////////////////////////////////////////////////////

bool psQuestPrereqOpGuild::Check(psCharacter * character)
{
    //Requirements are always valid for quest testers
    if(character->GetActor() && character->GetActor()->questtester)
        return true;

    if (!character->GetGuild()) //the player isn't in a guild
    {
        return (guildtype == "none"); //it was what we where looking for?
    }
    else
    {
        if(guildtype == "both") //no need to check for the case it's in a guild
            return true;
        if(character->GetGuild()->IsSecret()) //the guild is secret
            return (guildtype == "secret");
        else //the guild is public
            return (guildtype == "public");
    }
    return false;
}

csString psQuestPrereqOpGuild::GetScriptOp()
{
    csString script;

    script.Format("<guild type=\"%s\"/>", guildtype.GetData());

    return script;
}

csPtr<psQuestPrereqOp> psQuestPrereqOpGuild::Copy()
{
    csRef<psQuestPrereqOpGuild> copy;
    copy.AttachNew(new psQuestPrereqOpGuild(guildtype));
    return csPtr<psQuestPrereqOp>(copy);
}

///////////////////////////////////////////////////////////////////////////////////////////

bool psQuestPrereqOpTimeOnline::Check(psCharacter * character)
{
    //Requirements are always valid for quest testers
    if(character->GetActor() && character->GetActor()->questtester)
        return true;

    if(type == "min")
        return (character->GetTotalOnlineTime() > minTime);
    if(type == "max")
        return (character->GetTotalOnlineTime() < maxTime);
    if(type == "both")
        return (character->GetTotalOnlineTime() > minTime && character->GetTotalOnlineTime() < maxTime);
    return false;
}

csString psQuestPrereqOpTimeOnline::GetScriptOp()
{
    csString script;

    script.Format("<onlinetime min=\"%d\" max=\"%d\" type=\"%s\"/>", minTime, maxTime, type.GetDataSafe());

    return script;
}

csPtr<psQuestPrereqOp> psQuestPrereqOpTimeOnline::Copy()
{
    csRef<psQuestPrereqOpTimeOnline> copy;
    copy.AttachNew(new psQuestPrereqOpTimeOnline(minTime, maxTime, type));
    return csPtr<psQuestPrereqOp>(copy);
}

///////////////////////////////////////////////////////////////////////////////////////////

bool psQuestPrereqOpAdvisorPoints::Check(psCharacter * character)
{
    //Requirements are always valid for quest testers
    if(character->GetActor())
    {
         if(character->GetActor()->questtester)
            return true;
        if(character->GetActor()->GetClient())
        {
            if(type == "min")
                return (character->GetActor()->GetClient()->GetAdvisorPoints() > minPoints);
            if(type == "max")
                return (character->GetActor()->GetClient()->GetAdvisorPoints() < maxPoints);
            if(type == "both")
                return (character->GetActor()->GetClient()->GetAdvisorPoints() > minPoints &&
                        character->GetActor()->GetClient()->GetAdvisorPoints() < maxPoints);
        }
    }
    return false;
}

csString psQuestPrereqOpAdvisorPoints::GetScriptOp()
{
    csString script;

    script.Format("<advisorpoints min=\"%d\" max=\"%d\" type=\"%s\"/>", minPoints, maxPoints, type.GetDataSafe());

    return script;
}

csPtr<psQuestPrereqOp> psQuestPrereqOpAdvisorPoints::Copy()
{
    csRef<psQuestPrereqOpAdvisorPoints> copy;
    copy.AttachNew(new psQuestPrereqOpAdvisorPoints(minPoints, maxPoints, type));
    return csPtr<psQuestPrereqOp>(copy);
}

///////////////////////////////////////////////////////////////////////////////////////////

bool psQuestPrereqOpTimeOfDay::Check(psCharacter * character)
{
    //Requirements are always valid for quest testers
    if(character->GetActor() && character->GetActor()->questtester)
        return true;

    int currTime = psserver->GetWeatherManager()->GetGameTODHour();

    if (minTime <= maxTime)
    {
        return (currTime <= maxTime) && (currTime >= minTime); // quests during the day
    }

    return (currTime >= maxTime) || (currTime <= minTime); // quests overnight
}

csString psQuestPrereqOpTimeOfDay::GetScriptOp()
{
    csString script;

    script.Format("<timeofday min=\"%d\" max=\"%d\"/>", minTime, maxTime);

    return script;
}

csPtr<psQuestPrereqOp> psQuestPrereqOpTimeOfDay::Copy()
{
    csRef<psQuestPrereqOpTimeOfDay> copy;
    copy.AttachNew(new psQuestPrereqOpTimeOfDay(minTime, maxTime));
    return csPtr<psQuestPrereqOp>(copy);
}

///////////////////////////////////////////////////////////////////////////////////////////

bool psQuestPrereqOpXor::Check(psCharacter * character)
{
    //Requirements are always valid for quest testers
    if(character->GetActor() && character->GetActor()->questtester)
        return true;

    // Check if any of the prereqs are valid
    bool flag = 0;
    for (size_t i = 0; i < prereqlist.GetSize(); i++)
    {
        flag ^= prereqlist[i]->Check(character);
    }

    return flag;
}

csString psQuestPrereqOpXor::GetScriptOp()
{
    csString script;
    script.Append("<xor>");
    for (size_t i = 0; i < prereqlist.GetSize(); i++)
    {
        script.Append(prereqlist[i]->GetScriptOp());
    }
    script.Append("</xor>");
    return script;
}

csPtr<psQuestPrereqOp> psQuestPrereqOpXor::Copy()
{
    csRef<psQuestPrereqOpXor> copy;
    copy.AttachNew(new psQuestPrereqOpXor());
    for (size_t i = 0; i < prereqlist.GetSize(); i++)
    {
        copy->Push(prereqlist[i]->Copy());
    }
    return csPtr<psQuestPrereqOp>(copy);
}
///////////////////////////////////////////////////////////////////////////////////////////

bool psQuestPrereqOpSkill::Check(psCharacter * character)
{
    //Requirements are always valid for quest testers
    if(character->GetActor() && character->GetActor()->questtester)
        return true;

    unsigned int skill_val = character->Skills().GetSkillRank(skill->id);

    if(max && skill_val > max)
    {
        return false;
    }
    if(min && skill_val < min)
    {
        return false;
    }
    return true;
}

csString psQuestPrereqOpSkill::GetScriptOp()
{
    csString script;

    script.AppendFmt("<skill name=\"%s\"", skill->name.GetData());
    if(min)
    {
        script.AppendFmt(" min=\"%d\"", min);
    }
    if(max)
    {
        script.AppendFmt(" max=\"%d\"", max);
    }
    script.Append(" />");

    return script;
}

csPtr<psQuestPrereqOp> psQuestPrereqOpSkill::Copy()
{
    csRef<psQuestPrereqOpSkill> copy;
    copy.AttachNew(new psQuestPrereqOpSkill(skill,min,max));
    return csPtr<psQuestPrereqOp>(copy);
}
