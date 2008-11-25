/*
* psminigameboard.h
*
* Copyright (C) 2008 Atomic Blue (info@planeshift.it, http://www.atomicblue.org)
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

#ifndef __MINIGAMEBOARD_H
#define __MINIGAMEBOARD_H

//=============================================================================
// Crystal Space Includes
//=============================================================================
#include <csutil/weakref.h>

//=============================================================================
// Project Includes
//=============================================================================
#include "psstdint.h" 

//=============================================================================
// Local Includes
//=============================================================================

class Client;

/**
 * Game board limits
 */
#define GAMEBOARD_MIN_PLAYERS 1
#define GAMEBOARD_MAX_PLAYERS 2
#define GAMEBOARD_DEFAULT_PLAYERS 2

/**
 * Following enums define values to represent simple game rules
 */
enum Rule_PlayerTurn
{
    RELAXED,
    ORDERED
};
enum Rule_MovePieceType
{
    PLACE_OR_MOVE,
    PLACE_ONLY,
    MOVE_ONLY
};
enum Rule_MoveablePieces
{
    ANY_PIECE,
    OWN_PIECES_ONLY,
};
enum Rule_MovePiecesTo
{
    ANYWHERE,
    VACANCY_ONLY
};

/**
 * Game board definition class.
 */
class psMiniGameBoardDef
{
    friend class psMiniGameBoard;

public:

    psMiniGameBoardDef(const uint8_t defCols, const uint8_t defRows,
                       const char *defLayout, const char *defPieces,
                       const uint8_t defPlayers, const int16_t options);

    ~psMiniGameBoardDef();

    /// Returns the number of columns.
    uint8_t GetCols() const { return cols; }
    
    /// Returns the number of rows.
    uint8_t GetRows() const { return rows; }

    /// returns layout size
    int GetLayoutSize() const { return layoutSize; };

    /// pack string layout into binary array
    void PackLayoutString(const char *layoutStr, uint8_t *packedLayout);

    /// returns gameboard layout options
    uint16_t GetGameboardOptions(void) { return gameboardOptions; };

    /// decipher simple game rules from XML
    bool DetermineGameRules(csString rulesXMLstr, csString name);

private:
    /// layout size
    int layoutSize;

    /// The initial game board layout with tiles and pieces.
    uint8_t *layout;
        
    /// The number of columns.
    uint8_t cols;

    /// The number of rows.
    uint8_t rows;

    /// The number of available pieces.
    uint8_t numPieces;

    /// The package list of available game pieces.
    uint8_t *pieces;

    uint8_t numPlayers;

    /// gameboard options flags
    int16_t gameboardOptions;

    /// simple game rule variables
    Rule_PlayerTurn playerTurnRule;
    Rule_MovePieceType movePieceTypeRule;
    Rule_MoveablePieces moveablePiecesRule;
    Rule_MovePiecesTo movePiecesToRule;
};


/**
 * Wrapper class for the game board layout buffer.
 */
class psMiniGameBoard
{
public:

    psMiniGameBoard();

    ~psMiniGameBoard();

    /// Sets up the game board layout.
    void Setup(psMiniGameBoardDef *newGameDef, uint8_t *preparedLayout);

    /// Returns the number of columns.
    uint8_t GetCols() const { return gameBoardDef->cols; }
    
    /// Returns the number of rows.
    uint8_t GetRows() const { return gameBoardDef->rows; }

    /// Returns the packed game board layout.
    uint8_t *GetLayout() const { return layout; };

    /// Returns the number of available pieces.
    uint8_t GetNumPieces() const { return gameBoardDef->numPieces; }

    /// Returns the package list of available pieces.
    uint8_t *GetPieces() const { return gameBoardDef->pieces; }

    /// Gets the tile state from the specified column and row.
    uint8_t Get(uint8_t col, uint8_t row) const;

    /// Sets the tile state at the specified column and row.
    void Set(uint8_t col, uint8_t row, uint8_t state);

    /// returns number of players
    uint8_t GetNumPlayers(void) { return gameBoardDef->numPlayers; };

    /// return Game rules
    Rule_PlayerTurn GetPlayerTurnRule(void) { return gameBoardDef->playerTurnRule; };
    Rule_MovePieceType GetMovePieceTypeRule(void) { return gameBoardDef->movePieceTypeRule; };
    Rule_MoveablePieces GetMoveablePiecesRule(void) { return gameBoardDef->moveablePiecesRule; };
    Rule_MovePiecesTo GetMovePiecesToRule (void) { return gameBoardDef->movePiecesToRule; };

protected:

    /// The current game board layout with tiles and pieces.
    uint8_t *layout;
        
    /// game board definition
    psMiniGameBoardDef *gameBoardDef;
};

#endif

