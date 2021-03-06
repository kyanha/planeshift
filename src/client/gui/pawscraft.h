/*
 * pawscraft.h - Author: Andrew Craig <acraig@planeshift.it> 
 *
 * Copyright (C) 2003-2005 Atomic Blue (info@planeshift.it, http://www.atomicblue.org)
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
#ifndef PAWS_CRAFT_WINDOW_HEADER
#define PAWS_CRAFT_WINDOW_HEADER

#include "paws/pawswidget.h"
#include "net/cmdbase.h"

class pawsSimpleTree;
class pawsMultiLineTextBox;
class pawsEditTextBox;


/** Window Widget that displays information about the mind item to 
 *  be used in crafting.
 *  Crafting window which allows to filter recipes by text. Alternative way to book read, used with /study command
 *
 * This handles the incoming network traffic and displays the information 
 * for the client in a tree about all the items available in the current
 * mind item.
 */
class pawsCraftWindow : public pawsWidget, public psClientNetSubscriber
{
public:
    bool PostSetup(); 
    virtual void Show();
    void HandleMessage( MsgEntry* me );
    bool OnSelected(pawsWidget* widget);
    csString craftText;  // contains the complete set of available carfting recipes
    csString filter;  // contains the currently used filter for the crafting recipes
    /** Draw
     *
     * overwriting this to be able to do some reformating of the text is necessary
     */
    virtual void Draw();
    /** Format method for the output text
     *
     * This method sets the actual on-screen text by filtering all unwanted lines
     */
    void Format();
    
protected:
    pawsMultiLineTextBox* textBox;
    pawsEditTextBox* filterEditTextBox;

};

CREATE_PAWS_FACTORY( pawsCraftWindow );


#endif
