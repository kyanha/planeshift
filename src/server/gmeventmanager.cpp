/** gmeventmanager.cpp
 *
 * Copyright (C) 2006 Atomic Blue (info@planeshift.it, http://www.atomicblue.org)
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation (version 2 of the License)
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 * Manages Game Master events for players.
 */

#include <psconfig.h>
//=============================================================================
// Crystal Space Includes
//=============================================================================

//=============================================================================
// Project Includes
//=============================================================================
#include "util/psdatabase.h"
#include "util/log.h"
#include "util/eventmanager.h"
#include "util/strutil.h"

#include "net/messages.h"

//=============================================================================
// Local Includes
//=============================================================================
#include "gmeventmanager.h"
#include "clients.h"
#include "gem.h"
#include "cachemanager.h"
#include "entitymanager.h"
#include "adminmanager.h"
#include "globals.h"

GMEventManager::GMEventManager()
{
    nextEventID = 0;

    // initialise gmEvents
    gmEvents.DeleteAll();
    
    psserver->GetEventManager()->Subscribe(this, MSGTYPE_GMEVENT_INFO, REQUIRE_READY_CLIENT);
}

GMEventManager::~GMEventManager()
{
    psserver->GetEventManager()->Unsubscribe(this,MSGTYPE_GMEVENT_INFO);

    for (size_t e = 0; e < gmEvents.GetSize(); e++)
    {
        gmEvents[e]->playerID.DeleteAll();
        delete gmEvents[e];
    }
    gmEvents.DeleteAll();
}

bool GMEventManager::Initialise(void)
{
    GMEvent* ongoingGMEvent;

    // load any existing gm events from database
    Result events(db->Select("SELECT * from gm_events order by id"));
    if (events.IsValid())
    {
        for (unsigned long e=0; e<events.Count(); e++)
        {
            ongoingGMEvent = new GMEvent;
            ongoingGMEvent->id = events[e].GetInt("id");
            ongoingGMEvent->status = static_cast<GMEventStatus>(events[e].GetInt("status"));
            ongoingGMEvent->gmID = events[e].GetInt("gm_id");
            ongoingGMEvent->eventName = csString(events[e]["name"]);
            ongoingGMEvent->eventDescription = csString(events[e]["description"]);
            gmEvents.Push(ongoingGMEvent);

            // setup next available id
            if (ongoingGMEvent->id >= nextEventID)
                nextEventID = ongoingGMEvent->id + 1;
        }

        // load registered players from database
        Result registeredPlayers(db->Select("SELECT * from character_events order by event_id"));
        if (registeredPlayers.IsValid())
        {
            int eventPlayerID;
            int eventID;
            for (unsigned long rp=0; rp<registeredPlayers.Count(); rp++)
            {
                eventID = registeredPlayers[rp].GetInt("event_id");
                eventPlayerID = registeredPlayers[rp].GetInt("player_id");
                if ((ongoingGMEvent = GetGMEventByID(eventID)) != NULL)
                    ongoingGMEvent->playerID.Push(eventPlayerID);
                else
                {
                    Error1("GMEventManager: gm_events / character_events table mismatch.");
                    return false;        // ermm.. somethings gone wrong with the DB!!!
                }
            }
        }
        else
        {
            Error1("GMEventManager: character_events table is not valid.");
            return false;
        }

        return true;
    }

    Error1("GMEventManager: gm_events table is not valid.");
    return false;
}

/// GameMaster creates a new event for players.
bool GMEventManager::AddNewGMEvent (Client* client, csString eventName, csString eventDescription)
{
    int newEventID, zero=0;
    unsigned int gmID = client->GetPlayerID();
    int clientnum = client->GetClientNum();
    GMEvent* gmEvent;
    
    // if this GM already has an active event, he/she cant start another
    if ((gmEvent = GetGMEventByGM(gmID, RUNNING, zero)) != NULL)
    {
        psserver->SendSystemInfo(clientnum, 
                                 "You are already running the \'%s\' event.",
                                 gmEvent->eventName.GetDataSafe());
        return false;
    }

    if (eventName.Length() > MAX_EVENT_NAME_LENGTH)
    {
        eventName.Truncate(MAX_EVENT_NAME_LENGTH);
        psserver->SendSystemInfo(client->GetClientNum(), 
                                 "Event name truncated to \'%s\'.",
                                 eventName.GetDataSafe());
    }

    
    // GM can register his/her event
    newEventID = GetNextEventID();

    // remove undesirable characters from the name & description
    csString escEventName, escEventDescription;
    db->Escape(escEventName, eventName.GetDataSafe());
    db->Escape(escEventDescription, eventDescription.GetDataSafe());

    // first in the database
    db->Command("INSERT INTO gm_events(id, gm_id, name, description, status) VALUES (%i, %i, '%s', '%s', %i)",
        newEventID,
        gmID,
       	escEventName.GetDataSafe(),
       	escEventDescription.GetDataSafe(),
        RUNNING);

    // and in the local cache.
    gmEvent = new GMEvent;
    gmEvent->id = newEventID;
    gmEvent->gmID = gmID;
    gmEvent->eventName = escEventName;
    gmEvent->eventDescription = escEventDescription;
    gmEvent->status = RUNNING;
    gmEvents.Push(gmEvent);

    // keep psCharacter upto date
    client->GetActor()->GetCharacterData()->AssignGMEvent(gmEvent->id,true);

    psserver->SendSystemInfo(client->GetClientNum(), 
                             "You have initiated a new event, \'%s\'.",
                             eventName.GetDataSafe());

    return true;
}

/// GM registers player into his/her event
bool GMEventManager::RegisterPlayerInGMEvent (int clientnum, unsigned int gmID, Client* target)
{
    unsigned int playerID;
    int zero=0;
    GMEvent *gmEvent;
    
    // make sure GM is running an event, the player is valid and available. 
    if ((gmEvent = GetGMEventByGM(gmID, RUNNING, zero)) == NULL)
    {
        psserver->SendSystemInfo(clientnum, "You are not running an event.");
        return false;
    }
    if (!target)
    {
        psserver->SendSystemInfo(clientnum, "Invalid target for registering in event.");
        return false;
    }
    if ((playerID = target->GetPlayerID()) == gmID)
    {
        psserver->SendSystemInfo(clientnum, "You cannot register yourself in your own event.");
        return false;
    }
    zero = 0;
    if (GetGMEventByPlayer(playerID, RUNNING, zero) != NULL)
    {
        psserver->SendSystemInfo(clientnum, "%s is already registered in an event.", target->GetName());
        return false;
    }
    if (gmEvent->playerID.GetSize() == MAX_PLAYERS_PER_EVENT)
    {
        psserver->SendSystemInfo(clientnum,
                                 "There are already %d players in the \'%s\' event.",
                                 MAX_PLAYERS_PER_EVENT,
                                 gmEvent->eventName.GetDataSafe());
        return false;
    }

    // store the registration in db
    db->Command("INSERT INTO character_events(event_id, player_id) VALUES (%i, %i)",
        gmEvent->id,
        playerID);
    gmEvent->playerID.Push(playerID);

    // keep psCharacter up to date
    target->GetActor()->GetCharacterData()->AssignGMEvent(gmEvent->id,false);

    // player is available to join the valid GM event
    psserver->SendSystemInfo(clientnum, "%s is registered in the \'%s\' event.",
                             target->GetName(),
                             gmEvent->eventName.GetDataSafe());
    psserver->SendSystemInfo(target->GetClientNum(), "You are registered in the \'%s\' event.",
                             gmEvent->eventName.GetDataSafe());
    return true;
}

/// Register all players within range (upto 100m)
bool GMEventManager::RegisterPlayersInRangeInGMEvent (Client* client, float range)
{
    int clientnum = client->GetClientNum(), zero = 0;

    // make sure GM is running an event
    if (GetGMEventByGM(client->GetPlayerID(), RUNNING, zero) == NULL)
    {
        psserver->SendSystemInfo(clientnum, "You are not running an event.");
        return false;
    }
    // check range is within max permissable
    if (range <= 0.0 || range > MAX_REGISTER_RANGE)
    {
       psserver->SendSystemInfo(clientnum,
                                "Range should be greater than 0m upto %.2fm to register participants.",
                                MAX_REGISTER_RANGE);
       return false;
    }

    csArray<PublishDestination>& participants = client->GetActor()->GetMulticastClients();

    // search for players in range & register them
    bool regResult;
    for (size_t i = 0; i < participants.GetSize(); i++)
    {
        Client *target = psserver->GetConnections()->Find(participants[i].client);
        if (target && target != client && target->IsReady() && participants[i].dist <= range)
        {
            regResult = RegisterPlayerInGMEvent (clientnum, client->GetPlayerID(), target);
        }
    }

    return true;
}

bool GMEventManager::ListGMEvents (Client* client)
{
    psserver->SendSystemInfo(client->GetClientNum(), "Event list");
    psserver->SendSystemInfo(client->GetClientNum(), "--------------------");
    for (unsigned int i = 0 ; i < gmEvents.GetSize() ; i++)
    {
        if (gmEvents[i]->status == RUNNING)
        {
            psserver->SendSystemInfo(client->GetClientNum(), "%s", gmEvents[i]->eventName.GetData());
        }            
    }
    return true;
}

bool GMEventManager::CompleteGMEvent (Client* client, csString eventName)
{
    int zero = 0;
    GMEvent* theEvent = GetGMEventByName(eventName, RUNNING, zero);
    if (!theEvent)
    {
        psserver->SendSystemInfo(client->GetClientNum(), "Event %s wasn't found.", eventName.GetData());
        return false;
    }
    return CompleteGMEvent(client, theEvent->gmID);
}

/// GM completes their event
bool GMEventManager::CompleteGMEvent (Client* client, unsigned int gmID)
{
    int zero = 0;

    // if this GM does not have an active event, he/she can't end it.
    GMEvent* theEvent;
    int clientnum = client->GetClientNum();
    
    if ((theEvent = GetGMEventByGM(gmID, RUNNING, zero)) == NULL)
    {
        psserver->SendSystemInfo(clientnum, "You are not running an event.");
        return false;
    }

    // inform players
    ClientConnectionSet* clientConnections = psserver->GetConnections();
    Client* target;
    for (size_t p = 0; p < theEvent->playerID.GetSize(); p++)
    {
        if ((target = clientConnections->FindPlayer(theEvent->playerID[p])))
        {
            // psCharacter
            target->GetActor()->GetCharacterData()->CompleteGMEvent(false);

            psserver->SendSystemInfo(target->GetClientNum(),
                                     "Event '%s' complete.",
                                     theEvent->eventName.GetDataSafe());
        }
    }

    // GMs psCharacter    
    client->GetActor()->GetCharacterData()->CompleteGMEvent(true);

    // flag the event complete
    db->Command("UPDATE gm_events SET status = %d WHERE id = %d", COMPLETED, theEvent->id);
    theEvent->status = COMPLETED;
    
    psserver->SendSystemInfo(clientnum, "Event '%s' complete.", theEvent->eventName.GetDataSafe());

    return true;
}

/// GM removes player from incomplete event
bool GMEventManager::RemovePlayerFromGMEvent (int clientnum, unsigned int gmID, Client* target)
{
    unsigned int playerID;
    int zero=0;
    GMEvent* gmEvent;
    GMEvent* playerEvent;
    
    // make sure GM is running an event, the player is valid and registered.
    if ((gmEvent = GetGMEventByGM(gmID, RUNNING, zero)) == NULL)
    {
        psserver->SendSystemInfo(clientnum, "You are not running an event.");
        return false;
    }
    if (!target)
    {
        psserver->SendSystemInfo(clientnum, "Invalid target - cannot remove.");
        return false;
    }
    playerID = target->GetPlayerID();
    zero = 0;
    playerEvent = GetGMEventByPlayer(playerID, RUNNING, zero);
    if (!playerEvent || gmEvent->id != playerEvent->id)
    {
        psserver->SendSystemInfo(clientnum,
                                 "%s is not registered in your \'%s\' event.",
                                 target->GetName(),
                                 gmEvent->eventName.GetDataSafe());
        return false;
    }

    // all tests pass, drop the player
    RemovePlayerRefFromGMEvent(gmEvent, target, playerID);

    psserver->SendSystemInfo(clientnum, "%s has been removed from the \'%s\' event.",
                             target->GetName(),
                             gmEvent->eventName.GetDataSafe());
    psserver->SendSystemInfo(target->GetClientNum(), "You have been removed from the \'%s\' event.",
                             gmEvent->eventName.GetDataSafe());

    return true;
}

/// Reward player(s). Create reward item(s) and put in player's inventory if possible.
bool GMEventManager::RewardPlayersInGMEvent (Client* client,
                                             RangeSpecifier rewardRecipient,
                                             float range,
                                             Client* target,
                                             short stackCount,
                                             csString itemName)
{
    GMEvent* gmEvent;
    int clientnum = client->GetClientNum(), zero = 0;
    WordArray rewardDesc(itemName);
    RewardType rewardType = REWARD_ITEM;

    // make sure GM is running an event
    if ((gmEvent = GetGMEventByGM(client->GetPlayerID(), RUNNING, zero)) == NULL)
    {
        psserver->SendSystemInfo(clientnum, "You are not running an event.");
        return false;
    }
    // check range is within max permissable
    if (rewardRecipient == IN_RANGE && (range <= 0.0 || range > MAX_REGISTER_RANGE))
    {
       psserver->SendSystemInfo(clientnum,
                                "Range should be greater than 0m upto %.2fm to reward participants.",
                                MAX_REGISTER_RANGE);
       return false;
    }

    // identify reward type: experience, faction points or an item
    if (rewardDesc[0] == "exp" && rewardDesc.GetCount() == 1)
        rewardType = REWARD_EXPERIENCE;
    else if (rewardDesc[0] == "faction" && rewardDesc.GetCount() > 1)
        rewardType = REWARD_FACTION_POINTS;

    // retrieve base stats item
    psItemStats *basestats;
    if (rewardType == REWARD_ITEM)
    {
        basestats = CacheManager::GetSingleton().GetBasicItemStatsByName(itemName.GetDataSafe());
        if (basestats == NULL)
        {
            psserver->SendSystemInfo(clientnum, "Reward \'%s\' not recognised.", itemName.GetDataSafe());
            Error2("'%s' was not found as a valid base item.", itemName.GetDataSafe());
            return false;
        }
        if (stackCount <= 0)
        {
           psserver->SendSystemInfo(clientnum,
                                "You must reward at least 1 item to participant(s).");
           return false;
        }
    }
    else
        basestats = NULL;


    if (rewardRecipient == INDIVIDUAL)
    {
        if (!target)
            psserver->SendSystemInfo(clientnum, "Invalid target for reward.");
        else
        {
            zero = 0;
            GMEvent* playersEvent = GetGMEventByPlayer(target->GetPlayerID(), RUNNING, zero);
            if (playersEvent && playersEvent->id == gmEvent->id)
            {
                if (rewardType == REWARD_EXPERIENCE)
                    psserver->GetAdminManager()->AwardExperienceToTarget(clientnum, target, target->GetName(), stackCount);
                else if (rewardType == REWARD_FACTION_POINTS)
                    psserver->GetAdminManager()->AdjustFactionStandingOfTarget(clientnum, target, rewardDesc.GetTail(1), stackCount);
                else
                    RewardPlayer(clientnum, target, stackCount, basestats);
            }
            else
            {
                psserver->SendSystemInfo(clientnum,
                                         "%s is not registered in your \'%s\' event.",
                                         target->GetName(),
                                         gmEvent->eventName.GetDataSafe());
            }
        }
    }
    else
    {
        // reward ALL players (incl. within range)
        ClientConnectionSet* clientConnections = psserver->GetConnections();
        gemActor* clientActor = client->GetActor();
        for (size_t p = 0; p < gmEvent->playerID.GetSize(); p++)
        {
            if ((target = clientConnections->FindPlayer(gmEvent->playerID[p])))
            {
                if (rewardRecipient == ALL || clientActor->RangeTo(target->GetActor()) <= range)
                {
                    if (rewardType == REWARD_EXPERIENCE)
                        psserver->GetAdminManager()->AwardExperienceToTarget(clientnum, target, target->GetName(), stackCount);
                    else if (rewardType == REWARD_FACTION_POINTS)
                        psserver->GetAdminManager()->AdjustFactionStandingOfTarget(clientnum, target, rewardDesc.GetTail(1), stackCount);
                    else
                        RewardPlayer(clientnum, target, stackCount, basestats);
                }
            }
        }
    }

    return true;
}
 
/// return all events, running & complete, for a specified player
int GMEventManager::GetAllGMEventsForPlayer (unsigned int playerID,
                                             csArray<int>& completedEvents,
                                             int& runningEventAsGM,
                                             csArray<int>& completedEventsAsGM)
{
    int runningEvent = -1, id, index=0;
    GMEvent* gmEvent;
    
    completedEvents.DeleteAll();
    completedEventsAsGM.DeleteAll();

    if ((gmEvent = GetGMEventByGM(playerID, RUNNING, index)))
    {
        runningEventAsGM = gmEvent->id;
    }
    else
    {
        runningEventAsGM = -1;
    }

    index = 0;
    do
    {
        gmEvent = GetGMEventByGM(playerID, COMPLETED, index);
        if (gmEvent)
        {
            id = gmEvent->id;
            completedEventsAsGM.Push(id);
        }
    }
    while (gmEvent);
   
    index = 0; 
    gmEvent = GetGMEventByPlayer(playerID, RUNNING, index);
    if (gmEvent)
        runningEvent = gmEvent->id;

    index = 0;
    do
    {
        gmEvent = GetGMEventByPlayer(playerID, COMPLETED, index);
        if (gmEvent)
        {
            id = gmEvent->id;
            completedEvents.Push(id);
        }
    }
    while (gmEvent);

    return (runningEvent);
}

/// handle message from client
void GMEventManager::HandleMessage(MsgEntry* me, Client* client)
{
    // request from client for the events description
    if (me->GetType() == MSGTYPE_GMEVENT_INFO)
    {
        //int gmID;
        psGMEventInfoMessage msg(me);

        if (msg.command == psGMEventInfoMessage::CMD_QUERY)
        {
            GMEvent* theEvent;    
            if ((theEvent = GetGMEventByID(msg.id)) == NULL)
            {
                Error3("Client %s requested unavailable GM Event %d", client->GetName(), msg.id);
                return;
            }

            csString eventDesc(theEvent->eventDescription);

            if (theEvent->status != EMPTY)
            {
                ClientConnectionSet* clientConnections = psserver->GetConnections();
                Client* target;

                // if this client is the GM, list the participants too
                if (client->GetPlayerID() == theEvent->gmID)
                {		    
                    eventDesc.AppendFmt(". Participants: %zu. Online: ", theEvent->playerID.GetSize());
                    csArray<unsigned int>::Iterator iter = theEvent->playerID.GetIterator();
                    while (iter.HasNext())
                    {
                        if ((target = clientConnections->FindPlayer(iter.Next())))
                        {
                            eventDesc.AppendFmt("%s, ", target->GetName());
                        }
                    }
                }
                else // and name the running GM
                {
                    if (theEvent->gmID == UNDEFINED_GMID)
                    {
                        eventDesc.AppendFmt(" (No GM)");
                    }
                    if ((target = clientConnections->FindPlayer(theEvent->gmID)))
                    {
                        eventDesc.AppendFmt(" (%s)", target->GetName());
                    }		    
                }
		
                psGMEventInfoMessage response(me->clientnum, 
                                              psGMEventInfoMessage::CMD_INFO,
                                              msg.id,
                                              theEvent->eventName.GetDataSafe(),
                                              eventDesc.GetDataSafe());
                response.SendMessage();
            }
            else
            {
                Error3("Client %s requested unavailable GM Event %d", client->GetName(), msg.id);
            }
        }
        else if (msg.command == psGMEventInfoMessage::CMD_DISCARD)
        {
            DiscardGMEvent(client, msg.id);
        }
    }
}
     
/// remove players complete references to GM events they were involved with,
/// e.g. if a character is deleted.
bool GMEventManager::RemovePlayerFromGMEvents(unsigned int playerID)
{
    int runningEventIDAsGM;
    int runningEventID, gmEventID;
    csArray<int> completedEventIDsAsGM;
    csArray<int> completedEventIDs;
    GMEvent* gmEvent;
    bool eventsFound = false;

    runningEventID = GetAllGMEventsForPlayer(playerID,
                                             completedEventIDs,
                                             runningEventIDAsGM,
                                             completedEventIDsAsGM);

    // remove if partaking in an ongoing event
    if (runningEventID >= 0)
    {
        gmEvent = GetGMEventByID(runningEventID);
        if (gmEvent)
        {
            gmEvent->playerID.Delete(playerID);
            eventsFound = true;
        }
        else
            Error3("Cannot remove player %d from GM Event %d.", playerID, runningEventID);
    }
    // remove ref's to old completed events
    csArray<int>::Iterator evIter = completedEventIDs.GetIterator();
    while(evIter.HasNext())
    {
        gmEventID = evIter.Next();
        gmEvent = GetGMEventByID(gmEventID);
        if (gmEvent)
        {
            gmEvent->playerID.Delete(playerID);
            eventsFound = true;
         }
         else
            Error3("Cannot remove player %d from GM Event %d.", playerID, runningEventID);
    }
    // ...and from the DB too
    if (eventsFound)
        db->Command("DELETE FROM character_events WHERE player_id = %d", playerID);

    // if this is a GM thats being deleted, set its GMID to UNDEFINED_GMID.
    if (runningEventIDAsGM >= 0)
    {
        gmEvent = GetGMEventByID(runningEventIDAsGM);
        if (gmEvent)
        {
            gmEvent->gmID = UNDEFINED_GMID;
            db->Command("UPDATE gm_events SET gm_id = %d WHERE id = %d", UNDEFINED_GMID, gmEvent->id);
        }
        else
            Error3("Cannot remove GM %d from Event %d.", playerID, runningEventID);
    }
    evIter = completedEventIDsAsGM.GetIterator();
    while(evIter.HasNext())
    {
        gmEventID = evIter.Next();
        gmEvent = GetGMEventByID(gmEventID);
        if (gmEvent)
        {
            gmEvent->gmID = UNDEFINED_GMID;
            db->Command("UPDATE gm_events SET gm_id = %d WHERE id = %d", UNDEFINED_GMID, gmEvent->id);
        }
        else
            Error3("Cannot remove GM %d from Event %d.", playerID, gmEventID);
    }

    return true;
}

/// GM assumes control of ongoing event.
bool GMEventManager::AssumeControlOfGMEvent(Client* client, csString eventName)
{
    int zero=0;
    unsigned int newGMID = client->GetPlayerID();
    int clientnum = client->GetClientNum();
    GMEvent* gmEvent;

    // if this GM already has an active event, he/she cant control another
    if ((gmEvent = GetGMEventByGM(newGMID, RUNNING, zero)))
    {
        psserver->SendSystemInfo(clientnum, 
                                 "You are already running the \'%s\' event.",
                                 gmEvent->eventName.GetDataSafe());
        return false;
    }

    // find the requested event
    zero = 0;
    if (!(gmEvent = GetGMEventByName(eventName, RUNNING, zero)))
    {
        psserver->SendSystemInfo(clientnum, 
                                 "The \'%s\' event is not recognised or not running.",
                                 eventName.GetDataSafe());
        return false;
    }

    // look for the current GM if there is one
    ClientConnectionSet* clientConnections = psserver->GetConnections();
    Client* target;
    if (gmEvent->gmID != UNDEFINED_GMID)
    {
        if ((target = clientConnections->FindPlayer(gmEvent->gmID)))
        {
            psserver->SendSystemInfo(clientnum, 
                                     "The \'%s\' event's GM, %s, is online: you cannot assume control.",
                                     gmEvent->eventName.GetDataSafe(),
                                     target->GetName());
            return false;
        }
    }

    // if the GM is a participant, then remove the reference
    RemovePlayerRefFromGMEvent(gmEvent, client, newGMID);

    // OK, assume control
    gmEvent->gmID = newGMID;
    db->Command("UPDATE gm_events SET gm_id = %d WHERE id = %d", newGMID, gmEvent->id);
    client->GetActor()->GetCharacterData()->AssignGMEvent(gmEvent->id,true);
    psserver->SendSystemInfo(clientnum, 
                             "You now control the \'%s\' event.",
                             gmEvent->eventName.GetDataSafe());

    csArray<unsigned int>::Iterator iter = gmEvent->playerID.GetIterator();
    while (iter.HasNext())
    {
        if ((target = clientConnections->FindPlayer(iter.Next())))
        {
            psserver->SendSystemInfo(target->GetClientNum(),
                                     "The GM %s is now controlling the \'%s\' event.",
                                     client->GetName(),
                                     gmEvent->eventName.GetDataSafe());
        }
    }

    return true;
}

/// returns details of an event
GMEventStatus GMEventManager::GetGMEventDetailsByID (int id, 
                                                     csString& name,
                                                     csString& description)
{
    GMEvent* gmEvent = GetGMEventByID(id);

    if (gmEvent)
    {
        name = gmEvent->eventName;
        description = gmEvent->eventDescription;
        return gmEvent->status;
    }

    // ooops - some kinda cockup
    name = "?";
    description = "?";
    return EMPTY;
}

/// returns the specified GM Event
GMEventManager::GMEvent* GMEventManager::GetGMEventByID(int id)
{
    for (size_t e = 0; e < gmEvents.GetSize(); e++)
    {
        if (gmEvents[e]->id == id)
            return gmEvents[e];
    }

    return NULL;
}

/// returns a RUNNING event for a GM, or NULL
GMEventManager::GMEvent* GMEventManager::GetGMEventByGM(unsigned int gmID, GMEventStatus status, int& startIndex)
{
    for (size_t e = startIndex; e < gmEvents.GetSize(); e++)
    {
        startIndex++;
        if (gmEvents[e]->gmID == gmID && gmEvents[e]->status == status)
            return gmEvents[e];
    }

    return NULL;
}

/// returns a RUNNING event by name or NULL
GMEventManager::GMEvent* GMEventManager::GetGMEventByName(csString eventName, GMEventStatus status, int& startIndex)
{
    for (size_t e = startIndex; e < gmEvents.GetSize(); e++)
    {
        startIndex++;
        if (gmEvents[e]->eventName == eventName && gmEvents[e]->status == status)
            return gmEvents[e];
    }

    return NULL;
}

/// get the index into the gmEvents array for the next event of a specified
/// status for a particular player. Note startIndex will be modified upon
/// return.
GMEventManager::GMEvent* GMEventManager::GetGMEventByPlayer(unsigned int playerID, GMEventStatus status, int& startIndex)
{    
    for (size_t e = startIndex; e < gmEvents.GetSize(); e++)
    {
        startIndex++;
        if (gmEvents[e]->status == status)
        {
            csArray<unsigned int>::Iterator iter = gmEvents[e]->playerID.GetIterator();
            while (iter.HasNext())
            {
                if (iter.Next() == playerID)
                {
                    return gmEvents[e];
                }
            }
        }
    }

    return NULL;
}

/// reward an individual player.
void GMEventManager::RewardPlayer(int clientnum, Client* target, short stackCount, psItemStats* basestats)
{
    // generate the prize item
    psItem* newitem = basestats->InstantiateBasicItem(true);
    if (newitem == NULL)
    {
        Error1("Could not instantiate from base item.");
        psserver->SendSystemInfo(clientnum, "%s has not been rewarded.", target->GetName());
        return;
    }
    newitem->SetItemQuality(basestats->GetQuality());
    newitem->SetStackCount(stackCount);

    // spawn the item into the recipients inventory
    newitem->SetLoaded();  // Item is fully created
    if (target->GetActor()->GetCharacterData()->Inventory().Add(newitem))
    {
        // inform recipient of their prize
        psserver->SendSystemInfo(target->GetClientNum(), 
                                 "You have been rewarded for participating in this GM event.");        psserver->SendSystemInfo(clientnum, "%s has been rewarded.", target->GetName());
        return;
    }

    // failed to stash item, so remove it
    CacheManager::GetSingleton().RemoveInstance(newitem);
    psserver->SendSystemInfo(clientnum, "%s has not been rewarded.", target->GetName());
}

/// allocates the next GM event id
int GMEventManager::GetNextEventID(void)
{
    // TODO this is just too simple
    return nextEventID++;
}

/// player discards GM event
void GMEventManager::DiscardGMEvent(Client* client, int eventID)
{
    int runningEventIDAsGM;
    int runningEventID;
    unsigned int playerID;
    csArray<int> completedEventIDsAsGM;
    csArray<int> completedEventIDs;
    GMEvent* gmEvent;
 
    // look for this event in the player's events
    runningEventID = GetAllGMEventsForPlayer((playerID = client->GetPlayerID()),
                                             completedEventIDs,
                                             runningEventIDAsGM,
                                             completedEventIDsAsGM);

    // cannot discard an event if the player was/is the GM of it
    if (runningEventIDAsGM == eventID || completedEventIDsAsGM.Find(eventID) != csArrayItemNotFound)
    {
        psserver->SendSystemError(client->GetClientNum(), "You cannot discard an event for which you are/were the GM");
        return;
    }

    // attempt to remove player from event...
    if ((runningEventID == eventID || completedEventIDs.Find(eventID) != csArrayItemNotFound) &&
        (gmEvent = GetGMEventByID(eventID)))
    {
        RemovePlayerRefFromGMEvent(gmEvent, client, playerID);
    }
    else
    {
        psserver->SendSystemError(client->GetClientNum(), "Cannot find this event...");
        Error2("Cannot find event ID %d.", eventID);
    }
}

/// Actually remove player reference from GMEvent.
bool GMEventManager::RemovePlayerRefFromGMEvent(GMEvent* gmEvent, Client* client, unsigned int playerID)
{
    if (gmEvent->playerID.Delete(playerID))
    {
        // ...from the database
        db->Command("DELETE FROM character_events WHERE event_id = %d AND player_id = %d", gmEvent->id, playerID);
        // ...from psCharacter
        client->GetActor()->GetCharacterData()->RemoveGMEvent(gmEvent->id);

        return true;
    }

    return false;
}
