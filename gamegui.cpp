#include "headers/gamegui.h"

GameGUI::GameGUI(SDL_Surface* screen)
    :GameAI(), screen(screen),
    font(TTF_OpenFont("images/LiberationSerif-Bold.ttf", 14)),
    state(Blank), nodeRadius(5), lineThick(1), selectRadius(10),
    player1(true), error(false)
{
    textCol.r = 255;
    textCol.g = 255;
    textCol.b = 255;
}

void GameGUI::init(int count, int radius1, int radius2, int thick)
{
    nodeRadius = radius1;
    selectRadius = radius2;
    lineThick = thick;
    player1 = true;
    error = false;
    if (nodes.size() == 0)
    {
        double theta = 0;

        // Create count nodes
        if (count > 0)
        {
            // Place first node in center
            insertNode(center());

            // Place the rest of the initial nodes along an ellipse at the center
            // of the screen
            for (int i = 1; i < count; i++)
            {
                insertNode(Coord(
                    screen->w/3 * cos(theta) + screen->w/2,
                    screen->h/3 * sin(theta) + screen->h/2));

                theta += 2*3.14/count;
            }
        }
    }
    updateAreas();
    redraw();
}

void GameGUI::redraw(bool lck)
{
    if (lck)
        lock();

    // Blank screen
    SDL_FillRect(screen, NULL, 0);

    // Draw nodes
    for (int i = 0; i < nodes.size(); i++)
        circle(nodes[i]->getLoci(), nodeRadius, nodeCol);

    // Draw lines
    for (int i = 0; i < lines.size(); i++)
        for (int j = 1; j < lines[i]->size(); j++)
            line((*lines[i])[j-1], (*lines[i])[j], lineCol);

    // Draw temporary line
    for (int i = 1; i < currentLine.size(); i++)
        line(currentLine[i-1], currentLine[i], (player1)?player1Col:player2Col);

    if (lck)
        unlock();
}

// Lock the screen so we can access it
void GameGUI::lock()
{
    SDL_LockSurface(screen);
}

// Unlock and show it (double buffering)
void GameGUI::unlock()
{
    SDL_UnlockSurface(screen);

    // This really should not be here since this function is to unlock the
    // screen and not display text. However, it was easy to put it here and it
    // works.
    if (error)
        displayError("Error: need to connect at 180 degrees.");

    SDL_Flip(screen);
}

void GameGUI::cancel()
{
    currentLine.clear();
    state = Blank;
    redraw();
}

State GameGUI::click(Coord location)
{
    Node* selected = selectedNode(location);
    error = false; //Initialize as false every click

    // Don't do anything if it's dead
    if (selected && selected->dead())
        return state;

    // Clicked on node to start, make sure this is the first node
    if (selected && currentLine.size() == 0)
    {
        currentLine.push_back(selected->getLoci());
        state = NodeClicked;
    }
    // Clicked on node to end, make sure this is at least the second line or
    // that the line is perfectly straight between nodes
    else if (selected && state == NodeClicked)
    {
        currentLine.push_back(selected->getLoci());

        // Only end here if we can come into the new node correctly
        if (fixEndpoints())
        {
            Coord middle = findMiddle();
            doMove(currentLine, middle);
            player1 = !player1;
            currentLine.clear();
            state = Blank;

            if (gameEnded())
                state = GameEnd;
        }
        // Reset state if the line got deleted in fixEndpoints due to a messed
        // up starting line.
        else if (currentLine.size() == 0)
        {
            redraw();
            state = Blank;
            return state;
        }
    }
    // Clicked to place a line
    else if (state == NodeClicked)
    {
        Coord straightened = straighten(currentLine.back(), location);
        currentLine.push_back(straightened);

        // Check start line on first line
        if (currentLine.size() == 2)
        {
            if (!fixEndpoints())
            {
                redraw();
                state = Blank;
                return state;
            }
        }

        // Avoid objects as possible and then delete anything that can't be
        // made valid.  Return the line to a valid state.
        objectAvoidance();
    }

    redraw();

    return state;
}

void GameGUI::cursor(Coord location)
{
    if (state == NodeClicked)
    {
        lock();
        redraw(false);
        line(currentLine.back(), straighten(currentLine.back(), location),
            (player1)?player1Col:player2Col);
        unlock();
    }
}

bool GameGUI::vertical(Coord last, Coord point) const
{
    if (((point.y<=(last.y+(last.x-point.x)))&&
         (point.y<=(last.y-(last.x-point.x))))||
        ((point.y>=(last.y+(last.x-point.x)))&&
         (point.y>=(last.y-(last.x-point.x)))))
         return true;
    else
        return false;
}

Coord GameGUI::straighten(Coord last, Coord point)
{
    // Determine to snap vertically or horizontally
    if (vertical(last, point))
        return Coord(last.x, point.y);
    else
        return Coord(point.x, last.y);
}

Coord GameGUI::center() const
{
    return Coord(screen->w/2, screen->h/2);
}

void GameGUI::line(Coord a, Coord b, Uint32 color)
{
    for (int i = -lineThick; i < lineThick; i++)
        for (int j = -lineThick; j < lineThick; j++)
            lineColor(screen, a.x+i, a.y+j, b.x+i, b.y+j, color);
}

void GameGUI::circle(Coord p, int radius, Uint32 color)
{
    circleColor(screen, p.x, p.y, radius, color);
}

// Select the closest node to the point if within the selectRadius, otherwise
// return NULL
Node* GameGUI::selectedNode(Coord point) const
{
    int closestIndex = -1;
    double minDist = numeric_limits<double>::infinity();

    for (int i = 0; i < nodes.size(); i++)
    {
        double currentDist = distance(nodes[i]->getLoci(), point);

        if (currentDist < minDist)
        {
            minDist = currentDist;
            closestIndex = i;
        }
    }

    if (closestIndex != -1 && minDist <= selectRadius)
        return nodes[closestIndex];
    else
        return NULL;
}

Node* GameGUI::findNode(Coord point) const
{
    for (int i = 0; i < nodes.size(); i++)
        if (nodes[i]->getLoci() == point)
            return nodes[i];

    return NULL;
}

double GameGUI::distance(Coord a, Coord b) const
{
    return sqrt(pow(1.0*a.x-b.x,2)+pow(1.0*a.y-b.y,2));
}

bool GameGUI::validSingleLine(const Line& line, Coord start, Coord end) const
{
    const int startX = start.x;
    const int startY = start.y;
    const int endX = end.x;
    const int endY = end.y;

    for (int j = 1; j < line.size(); j++)
    {
        const int A2 = line[j-1].x;
        const int B2 = line[j-1].y;
        const int A3 = line[j].x;
        const int B3 = line[j].y;

        if (endX == startX)
        {
            if(A2 != A3)
            {
                if(A2 > A3)
                {
                    if((startX > A3)&&(startX < A2))
                    {
                        if(startY > endY)
                        {
                            if((B2 < startY)&&(B2 > endY))
                                return false;
                        }
                        else
                            if((B2 > startY)&&(B2 < endY))
                                return false;
                    }
                }
                else
                    if((startX < A3)&&(startX > A2))
                    {
                        if(startY > endY)
                        {
                            if((B2 < startY)&&(B2 > endY))
                                return false;
                        }
                        else
                            if((B2 > startY)&&(B2 < endY))
                                return false;
                    }
            }
            else
                if(startX == A2)
                {
                    if(B2 > B3)
                    {
                        if(((startY > B3)&&(startY <B2))||((endY > B3)&&(endY < B2)))
                            return false;
                        if(startY < endY)
                        {
                            if((startY < B3)&&(endY > B3))
                                return false;
                        }
                        else
                            if((startY > B2)&&(endY < B2))
                                return false;
                    }
                    else
                    {
                        if(((startY > B2)&&(startY < B3))||((endY > B2)&&(endY < B3)))
                            return false;
                        if(startY < endY)
                        {
                            if((startY < B2)&&(endY > B2))
                                return false;
                        }
                        else
                            if((startY > B3)&&(endY < B3))
                                return false;
                    }
                }

        }
        else
        {
            if(B2 != B3)
            {
                if(B2 > B3)
                {
                    if((startY < B2)&&(startY > B3))
                    {
                        if(startX > endX)
                        {
                            if((A2 < startX)&&(A2 > endX))
                                return false;
                        }
                        else
                            if((A2 > startX)&&(A2 < endX))
                                return false;
                    }
                }
                else
                    if((startY > B2)&&(startY < B3))
                    {
                        if(startX > endX)
                        {
                            if((A2 < startX)&&(A2 > endX))
                                return false;
                        }
                        else
                            if((A2 > startX)&&(A2 < endX))
                                return false;
                    }
            }
            else
                if(startY == B2)
                {
                    if(A2 > A3)
                    {
                        if(((startX > A3)&&(startX < A2))||((endX > A3)&&(endX < A2)))
                            return false;
                        if(startX < endX)
                        {
                            if((startX < A3)&&(endX > A3))
                                return false;
                        }
                        else
                            if((startX > A2)&&(endX < A2))
                                return false;
                    }
                    else
                    {
                        if(((startX > A2)&&(startX < A3))||((endX > A2)&&(endX < A3)))
                            return false;
                        if(startX < endX)
                        {
                            if((startX < A2)&&(endX > A2))
                                return false;
                        }
                        else
                            if((startX > A3)&&(endX < A3))
                                return false;
                    }
                }
        }
    }

    return true;
}

bool GameGUI::validLine(Coord start, Coord end) const
{
    if (!validSingleLine(currentLine, start, end))
        return false;

    for (int i = 0; i < lines.size(); i++)
    {
        const Line& line = *lines[i];

        if (!validSingleLine(line, start, end))
            return false;
    }

    return true;
}

void GameGUI::displayError(const string& msg)
{
    // Top left
    static SDL_Rect location;
    location.x = 0;
    location.y = 0;

    // Only update top left corner. size*10 works with the current font size.
    location.w = msg.size()*10;
    location.h = 20;

    SDL_Surface* error = TTF_RenderText_Blended(font, msg.c_str(), textCol);

    SDL_FillRect(screen, &location, 0);
    SDL_BlitSurface(error, NULL, screen , &location);
    SDL_FreeSurface(error);
}

void GameGUI::displayPosition(Coord c)
{
    ostringstream s;
    s << c;

    // Bottom left
    static SDL_Rect origin;
    origin.w = 60; // Approximate width and height
    origin.h = 20;
    origin.x = 0;
    origin.y = screen->h - origin.h;

    // Only update top left corner.

    SDL_Surface* hover = TTF_RenderText_Blended(font, s.str().c_str(), textCol);

    SDL_FillRect(screen, &origin, 0);
    SDL_BlitSurface(hover, NULL, screen , &origin);
    SDL_FreeSurface(hover);
}

bool GameGUI::playerTurn() const
{
    return player1;
}

bool GameGUI::fixEndpoints()
{
    // Don't call this if there's less than 2 coordinates. Nothing to fix.
    if (currentLine.size() < 2)
        throw "call fixEndpoints with at least two coordinates";

    const Coord startCoord = currentLine.front();
    const Coord startNext  = currentLine[1];
    const Coord endCoord   = currentLine.back();
    const Coord endPrev    = currentLine[currentLine.size()-2];
    const Node* startNode  = findNode(startCoord);
    const Node* endNode    = findNode(endCoord);

    // If this is run when the final node is clicked, fix the last line segment.
    if (endNode)
    {
        currentLine.back() = straighten(endPrev, endCoord);
        currentLine.push_back(endCoord);
    }

    // Force 180 degrees with 2 connections
    if (startNode && startNode->conCount() == 1)
    {
        // Determine direction
        if (!((!startNode->openUp()  && startNext.x == startCoord.x && startNext.y > startCoord.y) ||
            (!startNode->openDown()  && startNext.x == startCoord.x && startNext.y < startCoord.y) ||
            (!startNode->openLeft()  && startNext.y == startCoord.y && startNext.x > startCoord.x) ||
            (!startNode->openRight() && startNext.y == startCoord.y && startNext.x < startCoord.x)))
        {
            currentLine.clear();
            error = true;
            return false;
        }
    }

    // If the last point is the end Node
    if (endNode && endNode->conCount() == 1)
    {
        // All greater/less-than signs are flipped from startNode because we're
        // comparing it to the previous instead of the next coordinate.
        if (!((!endNode->openUp()  && endPrev.x == endCoord.x && endPrev.y < endCoord.y) ||
            (!endNode->openDown()  && endPrev.x == endCoord.x && endPrev.y > endCoord.y) ||
            (!endNode->openLeft()  && endPrev.y == endCoord.y && endPrev.x < endCoord.x) ||
            (!endNode->openRight() && endPrev.y == endCoord.y && endPrev.x > endCoord.x)))
        {
            currentLine.pop_back();
            error = true;
            return false;
        }
    }

    return true;
}

bool GameGUI::objectAvoidance()
{
    // Don't call this if there's less than 2 coordinates. Nothing to fix.
    if (currentLine.size() < 2)
        throw "call objectAvoidance with at least two coordinates";

    // It's already fine.
    if (validLine(currentLine[currentLine.size()-2], currentLine.back()))
        return true;

    // Try to dodge any objects in the way. Currently just get rid of the
    // last invalid point.
    currentLine.pop_back();

    return false;
}

Coord GameGUI::findMiddle() const
{
    int longestIndex = -1;
    double greatestDist = 0;

    for (int i = 1; i < currentLine.size(); i++)
    {
        double currentDist = distance(currentLine[i], currentLine[i-1]);

        if (currentDist > greatestDist)
        {
            greatestDist = currentDist;
            longestIndex = i;
        }
    }

    // Something went wrong, just pick the middle
    if (longestIndex == -1)
        longestIndex = currentLine.size()/2;

    return Coord((currentLine[longestIndex-1].x+currentLine[longestIndex].x)/2,
                 (currentLine[longestIndex-1].y+currentLine[longestIndex].y)/2);
}

GameGUI::~GameGUI()
{

}
