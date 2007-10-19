/*
* pawsframedrawable.cpp
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

#include <csutil/xmltiny.h>
#include <iutil/databuff.h>
#include <igraphic/imageio.h>
#include <igraphic/image.h>

#include <ivideo/txtmgr.h>
#include <ivideo/graph2d.h>
#include <csgeom/math.h>


#include "util/log.h"

#include "pawstexturemanager.h"
#include "pawsimagedrawable.h"
#include "pawsframedrawable.h"
#include "pawsmanager.h"

void pawsFrameDrawable::LoadPiece(csRef<iDocumentNode> node, FRAME_DRAWABLE_PIECE piece)
{
    bool tiled = node->GetAttributeValueAsBool("tile");
    csRect textureRect;
    textureRect.SetPos(node->GetAttributeValueAsInt("tx"), node->GetAttributeValueAsInt("ty"));
    textureRect.SetSize(node->GetAttributeValueAsInt("tw"), node->GetAttributeValueAsInt("th"));
    pieces[piece].drawable.AttachNew(new pawsImageDrawable(imageFileLocation, resourceName, tiled, textureRect, defaultAlphaValue, defaultTransparentColourRed, defaultTransparentColourGreen, defaultTransparentColourBlue));
    pieces[piece].offsetx = node->GetAttributeValueAsInt("offsetx");
    pieces[piece].offsety = node->GetAttributeValueAsInt("offsety");
}

pawsFrameDrawable::pawsFrameDrawable(csRef<iDocumentNode> node)
: scfImplementationType (this)
{
    defaultTransparentColourBlue  = -1;
    defaultTransparentColourGreen = -1;
    defaultTransparentColourRed   = -1;

    defaultAlphaValue = 0;

    // Read off the image and file vars
    imageFileLocation = node->GetAttributeValue("file");
    resourceName = node->GetAttributeValue("resource");

    const char * typeStr = node->GetAttributeValue("type");
    type = FDT_FULL;
    if (!strcmp(typeStr, "horizontal"))
        type = FDT_HORIZONTAL;
    else if (!strcmp(typeStr, "vertical"))
        type = FDT_VERTICAL;

    csRef<iDocumentNodeIterator> iter = node->GetNodes();
    while ( iter->HasNext() )
    {
        csRef<iDocumentNode> childNode = iter->Next();       

        // Read the default alpha value.
        if (strcmp(childNode->GetValue(), "alpha") == 0)
            defaultAlphaValue = childNode->GetAttributeValueAsInt("level");            

        // Read the default transparent colour.
        else if (strcmp(childNode->GetValue(), "trans") == 0)
        {
            defaultTransparentColourRed   = childNode->GetAttributeValueAsInt("r");            
            defaultTransparentColourGreen = childNode->GetAttributeValueAsInt("g");            
            defaultTransparentColourBlue  = childNode->GetAttributeValueAsInt("b");                                    
        }

        else if (strcmp(childNode->GetValue(), "top_left") == 0)
            LoadPiece(childNode, FDP_TOP_LEFT);
        else if (strcmp(childNode->GetValue(), "top") == 0)
            LoadPiece(childNode, FDP_TOP);
        else if (strcmp(childNode->GetValue(), "top_right") == 0)
            LoadPiece(childNode, FDP_TOP_RIGHT);
        else if (strcmp(childNode->GetValue(), "left") == 0)
            LoadPiece(childNode, FDP_LEFT);
        else if (strcmp(childNode->GetValue(), "middle") == 0)
            LoadPiece(childNode, FDP_MIDDLE);
        else if (strcmp(childNode->GetValue(), "right") == 0)
            LoadPiece(childNode, FDP_RIGHT);
        else if (strcmp(childNode->GetValue(), "bottom_left") == 0)
            LoadPiece(childNode, FDP_BOTTOM_LEFT);
        else if (strcmp(childNode->GetValue(), "bottom") == 0)
            LoadPiece(childNode, FDP_BOTTOM);
        else if (strcmp(childNode->GetValue(), "bottom_right") == 0)
            LoadPiece(childNode, FDP_BOTTOM_RIGHT);
    }

}

pawsFrameDrawable::~pawsFrameDrawable()
{

}

const char * pawsFrameDrawable::GetName() const
{
    return resourceName;
}

void pawsFrameDrawable::Draw(int x, int y, int alpha)
{
    Draw(x, y, GetWidth(), GetHeight(), alpha);
}

void pawsFrameDrawable::Draw(csRect rect, int alpha)
{
    Draw(rect.xmin, rect.ymin, rect.Width(), rect.Height(), alpha);
}

void pawsFrameDrawable::Draw(int x, int y, int newWidth, int newHeight, int alpha)
{
    if (alpha < 0)
        alpha = defaultAlphaValue;

    int w1, w2;
    switch (type)
    {
    case FDT_HORIZONTAL:
        w1 = pieces[FDP_LEFT].drawable->GetWidth();
        w2 = pieces[FDP_RIGHT].drawable->GetWidth();
        DrawPiece(FDP_LEFT, x, y, w1, newHeight, alpha, false, false);
        DrawPiece(FDP_RIGHT, x+newWidth-w2, y, w2, newHeight, alpha, false, false);
        DrawPiece(FDP_MIDDLE, x+w1, y, newWidth-w1-w2, newHeight, alpha, true, false);
        break;
    case FDT_VERTICAL:
        w1 = pieces[FDP_TOP].drawable->GetHeight();
        w2 = pieces[FDP_BOTTOM].drawable->GetHeight();
        DrawPiece(FDP_TOP, x, y, newWidth, w1, alpha);
        DrawPiece(FDP_BOTTOM, x, y+newHeight-w2, newWidth, w2, alpha);
        DrawPiece(FDP_MIDDLE, x, y+w1, newWidth, newHeight-w1-w2, alpha);
        break;
    case FDT_FULL:
        {
            int midLeft   = x + pieces[FDP_TOP_LEFT].drawable->GetWidth();
            int midTop    = y + pieces[FDP_TOP_LEFT].drawable->GetHeight();
            int midRight  = x + newWidth - pieces[FDP_BOTTOM_RIGHT].drawable->GetWidth();
            int midBottom = y + newHeight - pieces[FDP_BOTTOM_RIGHT].drawable->GetHeight();
            DrawPiece(FDP_TOP_LEFT, x, y, alpha);
            DrawPiece(FDP_TOP, midLeft, midTop-pieces[FDP_TOP].drawable->GetHeight(), midRight-midLeft, pieces[FDP_TOP].drawable->GetHeight(), alpha, true, false);
            DrawPiece(FDP_TOP_RIGHT, midRight, y, alpha);
            DrawPiece(FDP_LEFT, midLeft-pieces[FDP_LEFT].drawable->GetWidth(), midTop, pieces[FDP_LEFT].drawable->GetWidth(), midBottom-midTop, alpha, false, true);
            DrawPiece(FDP_MIDDLE, midLeft, midTop, midRight-midLeft, midBottom-midTop, alpha, true, true);
            DrawPiece(FDP_RIGHT, midRight, midTop, pieces[FDP_RIGHT].drawable->GetWidth(), midBottom-midTop, alpha, false, true);
            DrawPiece(FDP_BOTTOM_LEFT, midLeft-pieces[FDP_BOTTOM_LEFT].drawable->GetWidth(), midBottom, alpha);
            DrawPiece(FDP_BOTTOM, midLeft, midBottom, midRight-midLeft, pieces[FDP_BOTTOM].drawable->GetHeight(), alpha, true, false);
            DrawPiece(FDP_BOTTOM_RIGHT, midRight, midBottom, alpha);
            break;
        }
    case FDT_COUNT:
        break;
    }
}

int pawsFrameDrawable::GetWidth() const
{
    switch (type)
    {
    case FDT_HORIZONTAL:
        return pieces[FDP_LEFT].drawable->GetWidth() + pieces[FDP_MIDDLE].drawable->GetWidth() + pieces[FDP_RIGHT].drawable->GetWidth();
    case FDT_VERTICAL:
        return csMax<int>(pieces[FDP_TOP].drawable->GetWidth(), csMax<int>(pieces[FDP_MIDDLE].drawable->GetWidth(), pieces[FDP_BOTTOM].drawable->GetWidth()));
    case FDT_FULL:
        {
            int t = pieces[FDP_TOP_LEFT].drawable->GetWidth() + pieces[FDP_TOP].drawable->GetWidth() + pieces[FDP_TOP_RIGHT].drawable->GetWidth();
            int m = pieces[FDP_LEFT].drawable->GetWidth() + pieces[FDP_MIDDLE].drawable->GetWidth() + pieces[FDP_RIGHT].drawable->GetWidth();
            int b = pieces[FDP_BOTTOM_LEFT].drawable->GetWidth() + pieces[FDP_BOTTOM].drawable->GetWidth() + pieces[FDP_BOTTOM_RIGHT].drawable->GetWidth();
            return csMax<int>(t, csMax<int>(m, b));
        }
    case FDT_COUNT:
    break;
    }
    return 0;
}

int pawsFrameDrawable::GetHeight() const
{
    switch (type)
    {
    case FDT_HORIZONTAL:
        return csMax<int>(pieces[FDP_LEFT].drawable->GetHeight(), csMax<int>(pieces[FDP_MIDDLE].drawable->GetHeight(), pieces[FDP_RIGHT].drawable->GetHeight()));
    case FDT_VERTICAL:
        return pieces[FDP_TOP].drawable->GetHeight() + pieces[FDP_MIDDLE].drawable->GetHeight() + pieces[FDP_BOTTOM].drawable->GetHeight();
    case FDT_FULL:
        {
            int l = pieces[FDP_TOP_LEFT].drawable->GetHeight() + pieces[FDP_LEFT].drawable->GetHeight() + pieces[FDP_BOTTOM_LEFT].drawable->GetHeight();
            int m = pieces[FDP_TOP].drawable->GetHeight() + pieces[FDP_MIDDLE].drawable->GetHeight() + pieces[FDP_BOTTOM].drawable->GetHeight();
            int r = pieces[FDP_TOP_RIGHT].drawable->GetHeight() + pieces[FDP_RIGHT].drawable->GetHeight() + pieces[FDP_BOTTOM_RIGHT].drawable->GetHeight();
            return csMax<int>(l, csMax<int>(m, r));
        }
    case FDT_COUNT:
        break;
    }
    return 0;
}

int pawsFrameDrawable::GetDefaultAlpha() const
{
    return defaultAlphaValue;
}
