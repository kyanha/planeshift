/* cmdadmin.cpp
 *
 * Copyright (C) 2005 Atomic Blue (info@planeshift.it, http://www.atomicblue.org)
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
#include <iutil/document.h>

//=============================================================================
// Project Includes
//=============================================================================
#include "net/cmdhandler.h"
#include "net/msghandler.h"
#include "net/messages.h"

#include "util/strutil.h"

#include "gui/chatwindow.h"

#include "paws/pawsmanager.h"

//=============================================================================
// Local Includes
//=============================================================================
#include "cmdadmin.h"
#include "pscelclient.h"
#include "globals.h"

class pawsWidget;

psAdminCommands::psAdminCommands(MsgHandler* mh,
                                 CmdHandler *ch,
                                 iObjectRegistry* obj )
  : psCmdBase(mh,ch,obj)
{
    msgqueue->Subscribe(this, MSGTYPE_ADMINCMD);
    cmdsource->Subscribe( "/petition", this );
    cmdsource->Subscribe( "/deputize", this );
}

psAdminCommands::~psAdminCommands()
{
    msgqueue->Unsubscribe(this, MSGTYPE_ADMINCMD);
    cmdsource->UnsubscribeAll(this);
}

const char *psAdminCommands::HandleCommand(const char *cmd)
{
    WordArray words(cmd);

    if (words.GetCount() == 0)
        return NULL;

    if ( words[0] == "/show_gm" )
    {
        pawsWidget* widget = PawsManager::GetSingleton().FindWidget("GmGUI");
        if (!widget)
        {
            Error1("Failure on a /show_gm command. GmGUI isn't loaded\n");
            return NULL;
        }
        else
        {
            widget->Show();
        }
    }
    else
    {
        psAdminCmdMessage cmdmsg(cmd,0);
        cmdmsg.SendMessage();        
    }
    return NULL;  // don't display anything here
}

void psAdminCommands::HandleMessage(MsgEntry *me)
{
    if ( me->GetType() == MSGTYPE_ADMINCMD )
    {        
        psAdminCmdMessage msg(me);

        if( !msg.cmd)
        {
            psSystemMessage sysMsg( 0, MSG_INFO, "You lack administrator access" );
            msgqueue->Publish( sysMsg.msg );
            return;
        }

        psSystemMessage sysMsg( 0, MSG_INFO, "You now have the following admin commands available:" );
        msgqueue->Publish( sysMsg.msg );

        
        iDocumentSystem* xml = psengine->GetXMLParser ();
        csRef<iDocument> doc = xml->CreateDocument();
        const char* error = doc->Parse(msg.cmd);

        if ( error )
        {
            Error3("Failure to parse XML string %s Error %s\n", msg.cmd.GetData(), error);            
        }
        else
        {
            csRef<iDocumentNodeIterator> cmdIter = doc->GetRoot()->GetNodes("command");
        
            csString commands = "";
            while ( cmdIter->HasNext() )
            {
                csRef<iDocumentNode> commandNode = cmdIter->Next();
                csString cmdString = commandNode->GetAttributeValue("name");
                commands.Append( cmdString );
                commands.Append( "  " );
                cmdsource->Subscribe( cmdString, this );
            }
            
            psSystemMessage commandMsg( 0, MSG_INFO, commands.GetData() );
            msgqueue->Publish( commandMsg.msg );

            // Update the auto-complete list
            pawsChatWindow* chat = static_cast<pawsChatWindow*>(PawsManager::GetSingleton().FindWidget("ChatWindow"));
            chat->RefreshCommandList();
        }            
    }
}
