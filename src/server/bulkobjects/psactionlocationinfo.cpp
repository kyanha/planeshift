/*
* psactionlocationinfo.cpp
*
* Copyright (C) 2005 Atomic Blue (info@planeshift.it, http://www.atomicblue.org)
*
* Credits :
*            Michael Cummings <cummings.michael@gmail.com>
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* as published by the Free Software Foundation (version 2
* of the License).
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*
* Creation Date: 1/20/2005
* Description : Container for the action_locations db table.
*
*/


#include <psconfig.h>
//=============================================================================
// Crystal Space Includes
//=============================================================================
#include <csutil/randomgen.h>
#include <csutil/xmltiny.h>
#include <imesh/object.h>
#include <imesh/spritecal3d.h>
#include <iengine/mesh.h>
#include <csgeom/math3d.h>

#include <physicallayer/propclas.h>
#include <physicallayer/entity.h>
#include <propclass/mesh.h>

//=============================================================================
// Project Includes
//=============================================================================
#include "util/psxmlparser.h"
#include "util/log.h"
#include "util/psstring.h"
#include "util/serverconsole.h"

#include "../globals.h"
#include "../psserver.h"
#include "../cachemanager.h"
#include "../gem.h"

//=============================================================================
// Local Includes
//=============================================================================
#include "pssectorinfo.h"
#include "psactionlocationinfo.h"


// Definition of the itempool for psItemStats
PoolAllocator<psActionLocation> psActionLocation::actionpool;

void *psActionLocation::operator new(size_t allocSize)
{
    CS_ASSERT( allocSize <= sizeof( psActionLocation ) );
    return (void *)actionpool.CallFromNew();
}

void psActionLocation::operator delete(void *releasePtr)
{
    actionpool.CallFromDelete( (psActionLocation *)releasePtr );
}


psActionLocation::psActionLocation() : gemAction( NULL )
{
    id = 0;
    master_id = 0;
    name = "";
    sectorname = "";
    meshname = "";
    polygon.Format("%d", 0);
    position = csVector3(0.0F);
    radius = 0.0F;
    triggertype  = "";
    responsetype = "";
    response = "";
    isContainer = false;
    isGameBoard = false;
    isEntrance = false;
    isLockable = false;
    isReturn = false;
    enterScript = "";
    entranceType = "";
    instanceID = 0;
    entrancePosition.x = 0;
    entrancePosition.y = 0;
    entrancePosition.z = 0;
    entranceRot = 0.0;
    returnSector = "";
    returnPosition.x = 0;
    returnPosition.y = 0;
    returnPosition.z = 0;
    returnRot = 0.0;
    returnSector = "";
    description = "";
    isActive = true;
}

psActionLocation::~psActionLocation()
{
}


bool psActionLocation::Load(iResultRow& row)
{
    id = row.GetInt( "id" );
    master_id = row.GetInt( "master_id" );
    name = row[ "name" ];
    sectorname = row[ "sectorname" ];
    meshname = row[ "meshname" ];
    polygon.Format("%d",row.GetInt( "polygon" ));
    radius = row.GetFloat( "radius" );
    float x = row.GetFloat( "pos_x" );
    float y = row.GetFloat( "pos_y" );
    float z = row.GetFloat( "pos_z" );
    position = csVector3( x, y, z );

    if ( !master_id )
    {
        triggertype = row[ "triggertype" ];
        responsetype = row[ "responsetype" ];
        response = row[ "response" ];
    }
    else
    {
        responsetype = row[ "master_triggertype" ];
        responsetype = row[ "master_responsetype" ];
        response = row[ "master_response" ];
    }

    isActive = (row["active_ind"][0]=='Y') ? true : false;

    return true;
}

bool psActionLocation::Load( csRef<iDocumentNode> root )
{
    csRef<iDocumentNode> topNode;
    csRef<iDocumentNode> node;

    node = root->GetNode( "id" );
    if ( node ) id = node->GetContentsValueAsInt();

    node = root->GetNode( "masterid" );
    if ( node ) master_id = node->GetContentsValueAsInt();

    node = root->GetNode( "name" );
    if ( node ) name = node->GetContentsValue();

    node = root->GetNode( "sector" );
    if ( node ) sectorname = node->GetContentsValue();

    node = root->GetNode( "mesh" );
    if ( node ) meshname = node->GetContentsValue();

    node = root->GetNode( "polygon" );
    if ( node ) polygon = node->GetContentsValue();

    topNode = root->GetNode( "position" );
    if ( topNode )
    {
        float posx=0.0f, posy=0.0f, posz=0.0f;

        node = topNode->GetNode( "x" );
        if ( node ) posx = node->GetContentsValueAsFloat();

        node = topNode->GetNode( "y" );
        if ( node ) posy = node->GetContentsValueAsFloat();

        node = topNode->GetNode( "z" );
        if ( node ) posz = node->GetContentsValueAsFloat();

        position = csVector3( posx, posy, posz );
    }

    node = root->GetNode( "radius" );
    if ( node ) radius = node->GetContentsValueAsFloat();

    node = root->GetNode( "triggertype" );
    if ( node ) triggertype = node->GetContentsValue();

    node = root->GetNode( "responsetype" );
    if ( node ) responsetype = node->GetContentsValue();

    node = root->GetNode( "response" );
    if ( node ) response = node->GetContentsValue();

    node = root->GetNode( "active" );
    if (node)
    {
        const char *active = node->GetContentsValue();
        isActive = (active[0] == 'Y');
    }

    return true;
}

bool psActionLocation::Save()
{
    psStringArray fields;
    const char *fieldnames[]= {
        "master_id",
        "name",
        "sectorname",
        "meshname",
        "polygon",
        "pos_x",
        "pos_y",
        "pos_z",
        "radius",
        "triggertype",
        "responsetype",
        "response",
        "active_ind"
    };

    fields.FormatPush( "%u", master_id );
    fields.Push( name );
    fields.Push( sectorname );
    fields.Push( meshname );
    fields.FormatPush( "%s", polygon.GetData() );
    fields.FormatPush( "%f", position.x );
    fields.FormatPush( "%f", position.y );
    fields.FormatPush( "%f", position.z );
    fields.FormatPush( "%f", radius );
    //csString escpxml_response = EscpXML(response);

    if ( !master_id )
    {
        fields.FormatPush( "%s", triggertype.GetData() );
        fields.FormatPush( "%s", responsetype.GetData() );
        fields.FormatPush( "%s", response.GetData() );
    }
    else
    {
        fields.Push(NULL);
        fields.Push(NULL);
        fields.Push(NULL);
    }
    fields.FormatPush("%c",(isActive) ? 'Y' : 'N');

    if ( id == 0 ) // Insert New
    {
        uint32 newid = Insert( "action_locations" , fieldnames, fields );
        if (newid == 0)
        {
            Error2("Failed to create new action location. Error %s", db->GetLastError() );
            return false;
        }
        id = newid;
    }
    else
    {
        csString idStr;
        idStr.Format( "%d", id );
        if ( !UpdateByKey( "action_locations", "id", idStr, fieldnames, fields ) )
        {
            Error3("Failed to update action location %u. Error %s", id, db->GetLastError() );
            return false;
        }
    }
    return true;
}

bool psActionLocation::Delete()
{
    csString idStr;
    idStr.Format("%u", id);

    if (!DeleteByKey("action_locations", "id", idStr.GetData()))
    {
        Error3("Failed to delete action location %u. Error %s", id, db->GetLastError() );
        return false;
    }
    return true;
}

int psActionLocation::IsMatch( psActionLocation *compare )
{
    int result = 0;

    // match on sectorName and Mesh
    if ( ( compare->sectorname == sectorname ) && ( compare->meshname == meshname ) )
    {
        result++;
        if ( polygon != "0" )
        {
            if ( compare->polygon == polygon )
            {
                result++;
            }
            else // If polygon specified but does not match , no match
            {
                return 0;
            }
        }
        if ( !position.IsZero() )
        {
            if ( csSquaredDist::PointPoint( compare->position, position ) < (radius * radius) )
            {
                result++;
            }
            else // If position specified but does not match , no match
            {
                return 0;
            }
        }
    }

    return result;
}

void psActionLocation::SetGemObject( gemActionLocation *gemAction )
{
    this->gemAction = gemAction;
}

gemActionLocation *psActionLocation::GetGemObject( void )
{
   return this->gemAction;
}

void psActionLocation::GetLocationInWorld(const char **sectorname, float &loc_x, float &loc_y, float &loc_z, float &loc_yrot)
{
    *sectorname = this->sectorname.GetData();
    loc_x = this->position.x;
    loc_y = this->position.y;
    loc_z = this->position.z;
    loc_yrot = 0;
}

void psActionLocation::Send( int clientnum)
{
    this->gemAction->Send( clientnum, false, false );
}

csString psActionLocation::ToXML() const
{
    csString xml;
    const char* formatXML = "<location><id>%u</id><masterid>%u</masterid><name>%s</name><sector>%s</sector><mesh>%s</mesh><polygon>%s</polygon><position><x>%f</x><y>%f</y><z>%f</z></position><radius>%f</radius><triggertype>%s</triggertype><responsetype>%s</responsetype><response>%s</response><active>%s</active></location>";
    csString escpxml_name = EscpXML(name);
    csString escpxml_sectorname = EscpXML(sectorname);
    csString escpxml_meshname = EscpXML(meshname);
    csString escpxml_polygon = EscpXML(polygon);
    csString escpxml_triggertype = EscpXML(triggertype);
    csString escpxml_responsetype = EscpXML(responsetype);
    csString escpxml_response = EscpXML(response);
    xml.Format( formatXML,
                id,
                master_id,
                escpxml_name.GetData(),
                escpxml_sectorname.GetData(),
                escpxml_meshname.GetData(),
                escpxml_polygon.GetData(),
                position.x,
                position.y,
                position.z,
                radius,
                escpxml_triggertype.GetData(),
                escpxml_responsetype.GetData(),
                escpxml_response.GetData(),
                (isActive) ? "Y" : "N");

    return xml;
}

// DB Helper Operations
unsigned int psActionLocation::Insert( const char *table, const char **fieldnames, psStringArray& fieldvalues )
{
    csString command;
    int count = fieldvalues.GetSize();
    int i;
    command = "INSERT INTO ";
    command.Append(table);
    command.Append(" (");
    for (i=0;i<count;i++)
    {
        if (i>0)
            command.Append(",");
        command.Append(fieldnames[i]);
    }

    command.Append(") VALUES (");
    for (i=0;i<count;i++)
    {
        if (i>0)
            command.Append(",");
        if (fieldvalues[i]!=NULL)
        {
            command.Append("'");
            csString escape;
            db->Escape( escape, fieldvalues[i] );
            command.Append(escape);
            command.Append("'");
        }
        else
        {
            command.Append("NULL");
        }
    }
    command.Append(")");

    if (db->Command(command)!=1)
        return 0;

    return db->GetLastInsertID();
}

bool psActionLocation::UpdateByKey( const char *table, const char *idname, const char *idvalue, const char **fieldnames, psStringArray& fieldvalues )
{
    int i;
    int count = fieldvalues.GetSize();
    csString command;

    command.Append("UPDATE ");
    command.Append(table);
    command.Append(" SET ");

    for (i=0;i<count;i++)
    {
        if (i>0)
            command.Append(",");
        command.Append(fieldnames[i]);
        if (fieldvalues[i]!=NULL)
        {
            command.Append("='");
            csString escape;
            db->Escape(escape, fieldvalues[i]);
            command.Append(escape);
            command.Append("'");
        }
        else
        {
            command.Append("=NULL");
        }

    }

    command.Append(" where ");
    command.Append(idname);
    command.Append("='");
    csString escape;
    db->Escape( escape, idvalue );
    command.Append(escape);
    command.Append("'");

    if (db->Command(command)==QUERY_FAILED)
    {
        return false;
    }

    return true;
}


bool psActionLocation::DeleteByKey( const char *table, const char *idname, const char *idvalue )
{
    csString command;

    command.Append("DELETE FROM ");
    command.Append(table);
    command.Append(" WHERE ");
    command.Append(idname);
    command.Append("='");
    csString escape;
    db->Escape( escape, idvalue );
    command.Append(escape);
    command.Append("'");

    if (db->Command(command)==QUERY_FAILED)
    {
        return false;
    }
    return true;
}

// Parse action locations response XML and set member variables.
bool psActionLocation::ParseResponse()
{
    // Only check XML if response starts with examine
    if ( response.StartsWith( "<Examine>", false ) )
    {
        // load response into XML doc
        csRef<iDocument> doc = ParseString( response );
        if(!doc)
        {
            Error1("Parse error in action response");
            return false;
        }

        csRef<iDocumentNode> root = doc->GetRoot();
        if(!root)
        {
            Error1("No XML root in action response");
            return false;

        }
        
        csRef<iDocumentNode> topNode = root->GetNode( "Examine" );
        if(!topNode)
        {
            Error1("No <Examine> tag in action response");
            return false;
        }
        
        // Do Entrances
        csRef<iDocumentNode> entranceNode = topNode->GetNode( "Entrance" );
        if ( entranceNode ) 
        {
            SetupEntrance(entranceNode);
        }

        // Do Entrance Returns
        csRef<iDocumentNode> returnNode = topNode->GetNode( "Return" );
        if ( returnNode ) 
        {
            SetupReturn(returnNode);
        }

        // Do Containers
        csRef<iDocumentNode> containerNode = topNode->GetNode( "Container" );
        if ( containerNode ) 
        {
            SetupContainer(containerNode);
        }

        // Do Gameboards
        csRef<iDocumentNode> boardNode = topNode->GetNode("GameBoard");
        if ( boardNode ) 
        {
            SetupGameboard(boardNode);
        }

        // Do Descriptions
        csRef<iDocumentNode> descriptionNode = topNode->GetNode( "Description" );
        if ( descriptionNode ) 
        {
            SetupDescription(descriptionNode);
        }
    }
    return true;
}

// Set up entrance tag
//
// Action ID Exit.
//   Send player to location specified by coordinates in actionID (see above)
// <Examine>
//   <Entrance Type=ExitActionID />
//   <Description>Way back home</Description>
//  </Examine>
//
// Prime instance entrance example with a scripted lock:
//   Send player to x=2,y=3.2,z=0.4,rot=35.0 in NPCroom2 after running EnterScript
// <Examine>
//   <Entrance Type=Prime Script=EnterScript Sector=NPCroom2 X=2.0 Y=3.2 Z=0.4 Rot=35.0/>
//   <Description>A new place to visit.</Description>
// </Examine>
void psActionLocation::SetupEntrance(csRef<iDocumentNode> entranceNode)
{
    isEntrance = true;

    // All entrances have types
    SetEntranceType(entranceNode->GetAttributeValue( "Type" ));

    // Set lock instance ID if any 
    uint32 instance_id = (uint32)entranceNode->GetAttributeValueAsInt( "LockID" );
    SetInstanceID(instance_id);
    if ( instance_id  != 0 )
    {
        isLockable = true;
    }

    // Set entrance script if any
    SetEnterScript(entranceNode->GetAttributeValue( "Script" ));

    // Set entrance coordinates
    csVector3 entrancePos;
    entrancePos.x = entranceNode->GetAttributeValueAsFloat( "X" );
    entrancePos.y = entranceNode->GetAttributeValueAsFloat( "Y" );
    entrancePos.z = entranceNode->GetAttributeValueAsFloat( "Z" );
    SetEntrancePosition(entrancePos);
    SetEntranceRotation(entranceNode->GetAttributeValueAsFloat( "Rot" ));
    SetEntranceSector(entranceNode->GetAttributeValue( "Sector" ));
}

// Setup Return tag
//
// Action ID Entrance example with a lock and return to primary instance information.
//   Send player to x=-58,y=0,z=-115,rot=280 in NPCroom2 using action ID instance if they can open lock
//   When return action location is selected use these co-ordinates to return player to prime instance
// <Examine>
//   <Entrance Type=ActionID LockID=95 X=-58 Y=0 Z=-115 Rot=280 Sector=NPCroom2 />
//   <Return X=-56 Y=0 Z=-148 Rot=130 Sector=hydlaa_plaza />
//   <Description>A new place</Description>
//  </Examine>
void psActionLocation::SetupReturn(csRef<iDocumentNode> returnNode)
{
    isReturn = true;

    // Set return coordinates
    csVector3 returnPos;
    returnPos.x = returnNode->GetAttributeValueAsFloat( "X" );
    returnPos.y = returnNode->GetAttributeValueAsFloat( "Y" );
    returnPos.z = returnNode->GetAttributeValueAsFloat( "Z" );
    SetReturnPosition(returnPos);
    SetReturnRotation(returnNode->GetAttributeValueAsFloat( "Rot" ));
    SetReturnSector(returnNode->GetAttributeValue( "Sector" ));
}

// Setup Container tag
//
// Container example:
// <Examine>
//   <Container ID=71/>
//   <Description>A smith forge, ready to heat materials</Description>
// </Examine>
void psActionLocation::SetupContainer(csRef<iDocumentNode> containerNode)
{
    isContainer = true;

    // Set container instance ID if any 
    SetInstanceID((uint32)containerNode->GetAttributeValueAsInt( "ID" ));
}

// Setup game tag
//
// Game board example:
// <Examine>
//   <GameBoard Cols=6 Rows=6 Layout=FF00FFF0000F000000000000F0000FFF00FF Pieces=123456789ABCDE />
//   <Description>A minigame board for testing.</Description>
// </Examine>
//  Note: Some gameboard XML parsing is done in minigamemanager
void psActionLocation::SetupGameboard(csRef<iDocumentNode> boardNode)
{
    isGameBoard = true;
}

// Setup description tag
//
// Exit example using no script for prime instance
//   Send player back to x=56,y=0.2,z=110.4,rot=215.0 in NPCroom
// <Examine>
//   <Entrance Type=Prime Sector=NPCroom X=56.0 Y=0.2 Z=110.4 Rot=215.0/>
//   <Description>A door out.</Description>
// </Examine>
void psActionLocation::SetupDescription(csRef<iDocumentNode> descriptionNode)
{
    // Set description if any
    SetDescription(descriptionNode->GetContentsValue());
}
