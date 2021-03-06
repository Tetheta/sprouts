#include "headers/node.h"

Node::Node(Coord point, Connection con1, Connection con2)
    : loci(point)
{
    areasets[0] = NULL;
    areasets[1] = NULL;
    connections[0] = con1;
    connections[1] = con2;
    connections[2] = Connection();

    // Update openings for the initial connections if specified
    fill(&open[0], &open[0]+4, true); // By default every direction is open
    updateOpen();
}

Connection* Node::getConnAddr()
{
    return connections;
}

// NOTE: We MUST add keep these in order so that the last one filled is [2]
bool Node::dead() const
{
    return (connections[2].exists());
}

bool Node::vertical() const
{
    return !open[Up]; // if the node has a line coming in from the top then it is vertical
}

// The initial call. There are two of these since we have to pass in the
// original node's address so that we can later find if we have returned to
// this.
void Node::walk(vector<Area*>& areas)
{
    // Walk each connection
    for (int i = 0; i < 3; i++)
    {
        // If a connection is filled we have not already been to it since this
        // is the initial function call, recurse. Note we pass in this nodes
        // address so we'll know when we return to it.
        if (connections[i].exists())
            connections[i].dest->walk(areas, Area(), &connections[i], this);
    }
}

// Note: we could pass in history by reference if we delete added entries at
// the end of each iteration
void Node::walk(vector<Area*>& areas, Area history, Connection* connection, Node* initial)
{
    history.push_back(connection);

    // We have a circuit/loop if we're back to the start node
    if (connection->dest == initial)
    {
        //rotate the area to allow for uniqueness comparison
        Area::iterator iter;
        int oSize=history.size();
        Area rotatedHist(oSize);
        iter = min_element(history.begin(),history.end(),LineCmp);

        if (iter==history.end()) //this should never happen
            throw "Node::walk() didn't find minimum";

        for (int i=0;i<oSize;i++)
            rotatedHist[i]=history[(iter-history.begin()+i)%oSize];

        // Add a copy of the rotated history to areas vector if it isn't
        // already there
        if (find_if(areas.begin(), areas.end(), AreaFind(rotatedHist)) == areas.end())
        {
            Area* keep = new Area(rotatedHist);
            areas.push_back(keep);
        }

        return;
    }

    // Walk each connection
    for (int i = 0; i < 3; i++)
    {
        // If a connection is filled and we have not already been to it, recurse
        // Note we use the connection's node for the current node, not *this, which
        // is where we started.
        if (connections[i].exists() &&
            find_if(history.begin(), history.end(),
            LineFind(connections[i].line)) == history.end())
            connections[i].dest->walk(areas, history, &connections[i], initial);
    }
}

void Node::setAreasets(Areaset* sets[2])
{
    areasets[0]=sets[0];
    areasets[1]=sets[1];
}

bool Node::addConnection(const Connection& con)
{
    for (int i = 0; i < 3; i++)
    {
        if (!connections[i].exists())
        {
            connections[i] = con;
            updateOpen();
            return true;
        }
    }

    return false;
}

void Node::updateOpen()
{
    int count = 0;

    // Blank it again
    fill(&open[0], &open[0]+4, true); // By default every direction is open

    // For each of the connections, set open[dir] to false
    for (int i = 0; i < 3; i++)
    {
        if (connections[i].exists())
        {
            const Coord* other;

            // Keep track so we can check if it's valid afterwards
            ++count;

            // Note that this can be simplified if we always make sure a line
            // ends with the node pointed to by dest, but until then, check
            // based on coordinates
            const Line& line = *(connections[i].line);

            // A line must be at least the beginning and ending node
            if (line.size() < 2)
                throw InvalidLine(line);

            if (line.front() == loci) // At beginning
                other = &line[1];
            else if (line.back() == loci) // At end
                other = &line[line.size()-2];
            else // In the middle? It should be at the beginning or end!
                throw InvalidLine(line);

            // It can't be the same point
            if (loci == *other)
                throw InvalidLine(line);

            // Determine direction
            if (loci.x == other->x) // Vertical
            {
                if (loci.y < other->y) // Down
                {
                    if (open[Down])
                        open[Down] = false;
                    else
                        throw NodeEntryCollision();
                }
                else // Up
                {
                    if (open[Up])
                        open[Up] = false;
                    else
                        throw NodeEntryCollision();
                }
            }
            else if (loci.y == other->y) // Horizontal
            {
                if (loci.x < other->x) // Right
                {
                    if (open[Right])
                        open[Right] = false;
                    else
                        throw NodeEntryCollision();
                }
                else // Left
                {
                    if (open[Left])
                        open[Left] = false;
                    else
                        throw NodeEntryCollision();
                }
            }
            else // Neither, so invalid
            {
                throw InvalidLine(line);
            }
        }
    }

    // If there's only two, they must be 180 degrees from each other
    if (count == 2 &&
        !((open[Left] == false && open[Right] == false) ||
         (open[Up]   == false && open[Down]  == false)))
        throw InvalidCorner();
}

int Node::conCount() const
{
    int count = 0;

    for (int i = 0; i < 3; i++)
        if (connections[i].exists())
            ++count;

    return count;
}

Node::~Node()
{

}

ostream& operator<<(ostream& os, const Connection& con)
{
    if (con.exists())
        os << "Node " << con.dest << " @ " << con.dest->loci
           << " via " << con.line << ":{ " << con.line << ":" << *con.line << " }";
    else
        os << "default";

    return os;
}

ostream& operator<<(ostream& os, const InvalidLine& o)
{
    return os << "Invalid line: " << o.line;
}

bool LineCmp(Connection* a, Connection* b)
{
    return a->line < b->line;
}
