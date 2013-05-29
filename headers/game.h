/*
*   TODO:
*   need to add a game class copy constructer
*   impliment tests of (in)equality opperator for Areas
*   added a func to make a move (maybe called doMove(Line,Coord))
*       use find to determin if the Coord is on a corner
*       (is it in the Line vector)
*       also this func needs to keep track of how many moves have been made
*   add a move count func... maybe? if using doMove
*   in a valid line func only check lines in the areasets of the start node
*       in the process keep track of & of checked lines, dont recheck
*   end game function
*
*   TONEVERDO list (wish list)
*   added a func to check validity of current game data state
*/

#ifndef H_Game
#define H_Game

#include <map>
#include <vector>
#include <iostream>
#include <algorithm>
#include "node.h"
#include "structs.h"

using namespace std;

// Used for std::find since our vectors contain pointers
template<class T> class PointerFind
{
    public:
        const T& item;

        PointerFind(const T& item) :item(item) { }

        bool operator()(const T* search) const
        {
            return item == *search;
        }
};

// We'll throw this when trying to use connectable() with outdated areasets
class AreasOutdated { };

// Thrown in doMove() if the center point isn't on the line or the line
// doesn't end in two nodes (or runs through more than two).
class InvalidMove { };

class Game
{
    // Vectors of addresses since addresses of an element in a vector will
    // change as it grows
    private:
        bool updated;
        vector<Area*> areas;
        vector<Areaset*> areasets;
    protected:
        vector<Node*> nodes;
        vector<Line*> lines;
    public:
        Game();
        Game(const Game&);
        void updateAreas(); //will call node.walk in its process
        void doMove(const Line&, Coord middle); // This is the function you'll use a LOT.
        bool connectable(const Node&,const Node&) const;
        bool isInArea(const Area&,Coord) const;

        // TODO: Make these private eventually? Use doMove() instead.
        Node& insertNode(Coord, Connection = Connection(), Connection = Connection());
        Line& insertLine(const Line&);

        // Used for debugging
        friend ostream& operator<<(ostream&, const Game&);
        friend class TestSuite;
    private:
        void clearAreas(); // empty areas/areasets and delete items pointed to
    public:
        ~Game();
};

ostream& operator<<(ostream&, const Game&);

#endif
