#ifndef H_Game
#define H_Game

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
        void updateAreas(); //will call node.walk in its process
        bool connectable(const Node&,const Node&) const;
        bool isInArea(const Area&,Coord) const;
        void clearAreas(); // empty areas/areasets and delete items pointed to
        Node& insertNode(Coord, Connection = Connection(), Connection = Connection());
        Line& insertLine(const Line&);

        // Used for debugging
        friend ostream& operator<<(ostream&, const Game&);
        friend class TestSuite;

        ~Game();
};

#endif
