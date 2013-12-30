/*
 * hiresession.cpp  creator <andersr@pvv.org>
 *
 * Copyright (C) 2013 Atomic Blue (info@planeshift.it, http://www.atomicblue.org) 
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
//====================================================================================
// Crystal Space Includes
//====================================================================================

//====================================================================================
// Project Includes
//====================================================================================
//====================================================================================
// Local Includes
//====================================================================================
#include "hiresession.h"
#include "gem.h"

HireSession::HireSession():
    guild(false)
{
}

HireSession::HireSession(gemActor* owner):
    guild(false)
{
    SetOwner(owner);
}

HireSession::~HireSession()
{
}

bool HireSession::Load(iResultRow &row)
{
    ownerPID = PID(row.GetInt("owner_id"));
    hiredPID = PID(row.GetInt("hired_npc_id"));
    guild = row["guild"][0] == 'Y';
 
    return true;
}

bool HireSession::Save(bool newSession)
{
    unsigned long result = 0;
    
    if (newSession)
    {
        result = db->CommandPump("INSERT INTO npc_hired_npcs (owner_id,hired_npc_id,guild) "
                                 "VALUES ('%u','%u','%s')",
                                 ownerPID.Unbox(), hiredPID.Unbox(), guild?"Y":"N");
    }
    else
    {
        result = db->CommandPump("UPDATE npc_hired_npcs SET guild='%s'"
                                 " WHERE owner_id=%u AND hired_npc_id=%u",
                                 guild?"Y":"N", ownerPID.Unbox(), hiredPID.Unbox());
    }
    
    if(result==QUERY_FAILED)
        return false;

    return true;
}

bool HireSession::Delete()
{
    unsigned long result = 0;
   
    result = db->CommandPump("DELETE FROM npc_hired_npcs WHERE owner_id=%u AND hired_npc_id=%u",
                             ownerPID.Unbox(), hiredPID.Unbox());

    if(result==QUERY_FAILED)
        return false;

    return true;
}


void HireSession::SetHireType(const csString& name, const csString& npcType)
{
    hireTypeName = name;
    hireTypeNPCType = npcType;
}

const csString& HireSession::GetHireTypeName() const
{
    return hireTypeName;
}

const csString& HireSession::GetHireTypeNPCType() const
{
    return hireTypeNPCType;
}

void HireSession::SetMasterPID(PID masterPID)
{
    this->masterPID = masterPID;
}

PID HireSession::GetMasterPID() const
{
    return masterPID;
}

void HireSession::SetHiredNPC(gemNPC* hiredNPC)
{
    hiredPID = hiredNPC->GetPID();
    this->hiredNPC = hiredNPC;
}

gemNPC* HireSession::GetHiredNPC() const
{
    return hiredNPC;
}

void HireSession::SetOwner(gemActor* owner)
{
    ownerPID = owner->GetCharacterData()->GetPID();
    this->owner = owner;
}

gemActor* HireSession::GetOwner() const
{
    return owner;
}

bool HireSession::VerifyPendingHireConfigured()
{
    return (masterPID.IsValid() && !hireTypeName.IsEmpty() && !hireTypeNPCType.IsEmpty());
}

PID HireSession::GetOwnerPID()
{
    return ownerPID;
}

PID HireSession::GetHiredPID()
{
    return hiredPID;
}


