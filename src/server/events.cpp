/*
 * events.cpp
 *
 * Copyright (C) 2004 Atomic Blue (info@planeshift.it, http://www.atomicblue.org) 
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
 * Author: Keith Fulton <keith@planeshift.it>
 */

#include <psconfig.h>

#include <stdio.h>
#include <string.h>
#include <memory.h>
#include <stdlib.h>

#include <iutil/objreg.h>
#include <iutil/cfgmgr.h>
#include <csutil/csstring.h>

#include "globals.h"
#include "psserver.h"
#include "netmanager.h"
#include "util/strutil.h"
#include "economymanager.h"
#include "events.h"

PSF_IMPLEMENT_MSG_FACTORY(psDamageEvent,MSGTYPE_DAMAGE_EVENT);

psDamageEvent::psDamageEvent(gemActor *attack,gemActor *victim,float dmg)
{
    msg  = new MsgEntry(sizeof(gemActor*)*2 + sizeof(float) ,PRIORITY_LOW );

    msg->SetType(MSGTYPE_DAMAGE_EVENT);
    msg->clientnum      = 0; 
    msg->AddPointer( (uintptr_t) attack );
    msg->AddPointer( (uintptr_t) victim );
    msg->Add( dmg );

    valid=!(msg->overrun);
}

psDamageEvent::psDamageEvent( MsgEntry* event )
{
    if (!event)
        return;

    attacker = (gemActor*)event->GetPointer();
    target   = (gemActor*)event->GetPointer();
    damage   = event->GetFloat();
}

csString psDamageEvent::ToString(AccessPointers * /*access_ptrs*/)
{
    csString msgtext;
    
    msgtext.AppendFmt("Attacker: %p Target: %p Damage: %f",(void*)attacker,(void*)target,damage);

    return msgtext;
}

//------------------------------------------------------------------------------

PSF_IMPLEMENT_MSG_FACTORY(psDeathEvent,MSGTYPE_DEATH_EVENT);

psDeathEvent::psDeathEvent(gemActor *dead,gemActor *killer)
{
    msg  = new MsgEntry(sizeof(gemActor*)*2 ,PRIORITY_LOW );

    msg->SetType(MSGTYPE_DEATH_EVENT);
    msg->clientnum      = 0;
    msg->AddPointer( (uintptr_t) dead );
    msg->AddPointer( (uintptr_t) killer );


    valid=!(msg->overrun);
}

psDeathEvent::psDeathEvent( MsgEntry* event )
{
    if (!event)
        return;

    deadActor = (gemActor*)event->GetPointer();
    killer    = (gemActor*)event->GetPointer();
}

csString psDeathEvent::ToString(AccessPointers * /*access_ptrs*/)
{
    csString msgtext;
    
    msgtext.AppendFmt("DeadActor: %p Killer: %p",(void*)deadActor,(void*)killer);

    return msgtext;
}

//------------------------------------------------------------------------------

PSF_IMPLEMENT_MSG_FACTORY(psTargetChangeEvent,MSGTYPE_TARGET_EVENT);

psTargetChangeEvent::psTargetChangeEvent(gemActor *targeter, gemObject *targeted)
{
    msg = new MsgEntry(sizeof(gemActor*) + sizeof(gemObject*), PRIORITY_LOW);

    msg->SetType(MSGTYPE_TARGET_EVENT);
    msg->clientnum      = 0;
    msg->AddPointer( (uintptr_t) targeter );
    msg->AddPointer( (uintptr_t) targeted );    
    
    valid = ! (msg->overrun);
}

psTargetChangeEvent::psTargetChangeEvent(MsgEntry *event)
{
    if(!event)
        return;

    character = (gemActor*)event->GetPointer();
    target    = (gemObject*)event->GetPointer();
}

csString psTargetChangeEvent::ToString(AccessPointers * /*access_ptrs*/)
{
    csString msgtext;
    
    msgtext.AppendFmt("Character: %p Target: %p",(void*)character,(void*)target);

    return msgtext;
}

//------------------------------------------------------------------------------

PSF_IMPLEMENT_MSG_FACTORY(psZPointsGainedEvent,MSGTYPE_ZPOINT_EVENT);

psZPointsGainedEvent::psZPointsGainedEvent( gemActor* actor, const char *name, int gained, bool rankup )
{
    msg = new MsgEntry(sizeof(gemActor*) + 
                       sizeof(int) + 
                       sizeof(bool) +
                       strlen(name)+1, PRIORITY_LOW);
                       
    msg->SetType(MSGTYPE_ZPOINT_EVENT);                   
    msg->clientnum = 0;
    msg->Add( name );
    msg->AddPointer( (uintptr_t) actor );
    msg->Add( (uint32_t)gained );
    msg->Add( rankup );                             
 }
   
psZPointsGainedEvent::psZPointsGainedEvent( MsgEntry* event )
{
    if ( !event )
        return;

    skillName = event->GetStr();
    actor = (gemActor*)event->GetPointer();
    amountGained = event->GetUInt32();  
    rankUp = event->GetBool();    
}

csString psZPointsGainedEvent::ToString(AccessPointers * /*access_ptrs*/)
{
    csString msgtext;
    
    msgtext.AppendFmt("Skill: %s Actor: %p ZGained: %d RankUp: %s",
                      skillName.GetDataSafe(),(void*)actor,amountGained,(rankUp?"true":"false"));

    return msgtext;
}

//------------------------------------------------------------------------------

PSF_IMPLEMENT_MSG_FACTORY(psBuyEvent,MSGTYPE_BUY_EVENT);

psBuyEvent::psBuyEvent( int from, int to, const char* item, int stack, int quality,unsigned int price)
{
    // Merchant => Player

    msg = new MsgEntry( (sizeof(int) * 4) + sizeof(unsigned int)+
                       strlen(item)+1, PRIORITY_LOW);
                       
    msg->SetType(MSGTYPE_BUY_EVENT);                   
    msg->clientnum = 0;  
    msg->Add( (int32_t) from);
    msg->Add( (int32_t) to);
    msg->Add( item);
    msg->Add( (int32_t) stack);
    msg->Add( (int32_t) quality);
    msg->Add( (uint32_t) price);
}

psBuyEvent::psBuyEvent( MsgEntry* event)
{
    if(!event)
        return;

    trans = new TransactionEntity(); // needs to be handled by economy manager

    trans->from = event->GetInt32();
    trans->to = event->GetInt32();

    trans->item = event->GetStr();
    trans->count = event->GetInt32();
    trans->quality = event->GetInt32();
    trans->price = event->GetUInt32();
}

csString psBuyEvent::ToString(AccessPointers * /*access_ptrs*/)
{
    csString msgtext;
    
    msgtext.AppendFmt("From: %d To: %d Item: '%s' Count: %d Quality: %d Price %d",
                      trans->from,trans->to,trans->item.GetDataSafe(),
                      trans->count,trans->quality,trans->price);

    return msgtext;
}

//------------------------------------------------------------------------------

PSF_IMPLEMENT_MSG_FACTORY(psSellEvent,MSGTYPE_SELL_EVENT);

psSellEvent::psSellEvent( int from, int to, const char* item, int stack, int quality,unsigned int price)
{
    // Player => Merchant

    msg = new MsgEntry( (sizeof(int) * 4) + sizeof(unsigned int)+
                       strlen(item)+1, PRIORITY_LOW);
                       
    msg->SetType(MSGTYPE_SELL_EVENT);                   
    msg->clientnum = 0;  
    msg->Add( (int32_t) from);
    msg->Add( (int32_t) to);
    msg->Add( item);
    msg->Add( (int32_t) stack);
    msg->Add( (int32_t) quality);
    msg->Add( (uint32_t) price);
}

psSellEvent::psSellEvent( MsgEntry* event)
{
    if(!event)
        return;

    trans = new TransactionEntity(); // needs to be handled by economy manager

    trans->from = event->GetInt32();
    trans->to = event->GetInt32();

    trans->item = event->GetStr();
    trans->count = event->GetInt32();
    trans->quality = event->GetInt32();
    trans->price = event->GetUInt32();
}

csString psSellEvent::ToString(AccessPointers * /*access_ptrs*/)
{
    csString msgtext;
    
    msgtext.AppendFmt("From: %d To: %d Item: '%s' Count: %d Quality: %d Price %d",
                      trans->from,trans->to,trans->item.GetDataSafe(),
                      trans->count,trans->quality,trans->price);

    return msgtext;
}

//------------------------------------------------------------------------------

PSF_IMPLEMENT_MSG_FACTORY(psConnectEvent,MSGTYPE_CONNECT_EVENT);

psConnectEvent::psConnectEvent( int clientID )
{
    // Player => Merchant

    msg = new MsgEntry( sizeof(int), PRIORITY_LOW);
                       
    msg->SetType(MSGTYPE_CONNECT_EVENT);                   
    msg->clientnum = clientID;  
    msg->Add( (int32_t) clientID );
}

psConnectEvent::psConnectEvent( MsgEntry* event)
{
    if(!event)
        return;

    client_id = event->GetInt32();
}

csString psConnectEvent::ToString(AccessPointers * /*access_ptrs*/)
{
    csString msgtext;
    
    msgtext.AppendFmt("ClientID: %d", client_id);

    return msgtext;
}

//------------------------------------------------------------------------------

PSF_IMPLEMENT_MSG_FACTORY(psMovementEvent,MSGTYPE_MOVEMENT_EVENT);

psMovementEvent::psMovementEvent( int clientID )
{
    msg = new MsgEntry( sizeof(int), PRIORITY_LOW);
                       
    msg->SetType(MSGTYPE_MOVEMENT_EVENT);                   
    msg->clientnum = clientID;  
    msg->Add( (int32_t) clientID );
}

psMovementEvent::psMovementEvent( MsgEntry* event)
{
    if(!event)
        return;

    client_id = event->GetInt32();
}

csString psMovementEvent::ToString(AccessPointers * /*access_ptrs*/)
{
    csString msgtext;
    
    msgtext.AppendFmt("ClientID: %d", client_id);

    return msgtext;
}



//------------------------------------------------------------------------------

PSF_IMPLEMENT_MSG_FACTORY(psGenericEvent,MSGTYPE_GENERIC_EVENT);

psGenericEvent::psGenericEvent( int clientID, psGenericEvent::Type type )
{
    msg = new MsgEntry( sizeof(int)*2, PRIORITY_LOW);
                       
    msg->SetType(MSGTYPE_GENERIC_EVENT);                   
    msg->clientnum = clientID;  
    msg->Add( (int32_t) type );
    msg->Add( (int32_t) clientID );
}

psGenericEvent::psGenericEvent( MsgEntry* event)
{
    if(!event)
        return;

    eventType = (psGenericEvent::Type) event->GetInt32();
    client_id = event->GetInt32();
}

csString psGenericEvent::ToString(AccessPointers * /*access_ptrs*/)
{
    csString msgtext;
    
    msgtext.AppendFmt("Type: %d, ClientID: %d", eventType, client_id);

    return msgtext;
}
