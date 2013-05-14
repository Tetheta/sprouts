#include "sprout.h"

Sprout::Sprout(SDL_Surface *sf, SDL_Surface *buf, int numSprouts)
    : surface(sf), buffer(buf), doLockSurface(true), highlighted(true)
{
	double theta = 0;

	/* Make first sprout centered */
	sprout tmpSprout;
	tmpSprout.degree = 0;
	tmpSprout.xPoint = surface->w/2;
	tmpSprout.yPoint = surface->h/2;
	sprouts.push_back(tmpSprout);

	/* Put sprouts along an ellipse at the center of the screen */
	for (int i = 1; i < numSprouts; i++)
	{
		tmpSprout = sprout();
		tmpSprout.degree = 0;

		tmpSprout.xPoint = surface->w/3 * std::cos(theta) + surface->w/2;
		tmpSprout.yPoint = surface->h/3 * std::sin(theta) + surface->h/2;
		theta += 2*3.14 / numSprouts;

		sprouts.push_back(tmpSprout);
	}

	drawSprouts();
}

void Sprout::thickLine(SDL_Surface *sf, int x0, int y0, int x, int y, Uint32 color)
{
	for (int i = -thickness; i <= thickness; i++)
	{
		for (int j = -thickness; j <= thickness; j++)
		{
			lineColor(sf, x0+i, y0+j, x+i, y+j, color);
		}
	}
}

bool Sprout::connect()
{
	SDL_Event event;			// dump event polls into this

	//int firstSprout;
	int index = 0;
	int xVec, yVec;
	double xUnit, yUnit;
	bool run = true, planted = false;

	sprout tmpSprout;
	connection tmpConnection;

	while (run)
	{
		while (SDL_PollEvent(&event))
		{
			switch (event.type)
			{
                case SDL_QUIT:
                    return false;
				case SDL_KEYUP:					// keyboard released
					if (event.key.keysym.sym == SDLK_ESCAPE)
					{
						drawLines();
						return true;	// true?
					}
					break;
				case SDL_MOUSEBUTTONDOWN:	break;	// mouse pressed
				case SDL_MOUSEBUTTONUP:		// mouse released
					if (event.button.button == SDL_BUTTON_LEFT)
					{
						doLockSurface = false;
						SDL_LockSurface(surface);

                        if (index == 0)	// TODO: rewrite this using a bool for started or not started already
                        {
                            if (select(event.button.x, event.button.y))
                            {
                                if (sprouts[activeSprout].degree < 3)
                                {
                                    //firstSprout = activeSprout;
                                    tmpConnection = connection();
                                    tmpConnection.xPoints.push_back(sprouts[activeSprout].xPoint);
                                    tmpConnection.yPoints.push_back(sprouts[activeSprout].yPoint);
                                    index++;
                                    sprouts[activeSprout].degree++;
                                }
                            }
                        }
                        else
                        {
                            if (select(event.button.x, event.button.y) && planted)		// clicked last point on path
                            {
                                if (sprouts[activeSprout].degree < 3)
                                {
                                    tmpConnection.xPoints.push_back(sprouts[activeSprout].xPoint);
                                    tmpConnection.yPoints.push_back(sprouts[activeSprout].yPoint);
                                    lines.push_back(tmpConnection);
                                    sprouts[activeSprout].degree++;
                                    drawLines();
                                    run = false;
                                }
                            }
                            else	// clicking control points
                            {
                                // check if valid to add point there
                                drawLines();
                                drawConnection(tmpConnection);

                                if (lineValid(tmpConnection.xPoints.back(), tmpConnection.yPoints.back(), event.button.x, event.button.y))
                                {
                                    // If the mouse is too close to another node (if we can select it, there's
                                    // one nearby) or another line
                                    if (select(event.button.x, event.button.y) || nearLine(event.button.x, event.button.y))
                                    {
                                        xVec = event.button.x - sprouts[activeSprout].xPoint;
                                        yVec = event.button.y - sprouts[activeSprout].yPoint;

                                        xUnit = xVec / std::sqrt(xVec*xVec + yVec*yVec);
                                        yUnit = yVec / std::sqrt(xVec*xVec + yVec*yVec);

                                        xVec = xUnit * selectRadius * 1.5;
                                        yVec = yUnit * selectRadius * 1.5;

                                        // Push the mouse away
                                        SDL_WarpMouse(event.button.x+xVec, event.button.y+yVec);
                                    }
                                    else
                                    {
                                        tmpConnection.xPoints.push_back(event.button.x);
                                        tmpConnection.yPoints.push_back(event.button.y);
                                    }
                                }
                            }

                            drawConnection(tmpConnection);
                        }

						SDL_UnlockSurface(surface);
						SDL_Flip(surface);
						doLockSurface = true;
					}
					else if (event.button.button == SDL_BUTTON_RIGHT)
					{
						if (index > 0)
						{
							if (!planted)
							{
								doLockSurface = false;
								SDL_LockSurface(surface);
                                drawLines();
                                drawConnection(tmpConnection);

                                if (lineValid(tmpConnection.xPoints.back(), tmpConnection.yPoints.back(), event.button.x, event.button.y))
                                {
                                    tmpSprout = sprout();
                                    tmpSprout.xPoint = event.button.x;
                                    tmpSprout.yPoint = event.button.y;
                                    tmpSprout.degree = 2;
                                    sprouts.push_back(tmpSprout);
                                    // TODO: isn't sprouts.back().xPoint event.button.x, the thing we just pushed onto the vector!?
                                    tmpConnection.xPoints.push_back(sprouts.back().xPoint);
                                    tmpConnection.yPoints.push_back(sprouts.back().yPoint);
                                    drawConnection(tmpConnection);
                                    planted = true;
                                }

                                drawLines();
                                drawConnection(tmpConnection);
								SDL_UnlockSurface(surface);
								SDL_Flip(surface);
								doLockSurface = true;
							}
						}
					}
					break;
				case SDL_MOUSEMOTION:		// mouse moved
					doLockSurface = false;
					SDL_LockSurface(surface);

                    if (index > 0)
                    {
                        drawLines();
                        drawConnection(tmpConnection);
                        lineValid(tmpConnection.xPoints.back(), tmpConnection.yPoints.back(), event.motion.x, event.motion.y);
                        circleColor(surface, event.motion.x, event.motion.y, selectRadius, grayedColor);
                    }

                    highlightNear(event.motion.x, event.motion.y);
					SDL_UnlockSurface(surface);
					SDL_Flip(surface);
					doLockSurface = true;
					break;
				default:
					break;
			}
		}
	}

	drawLines();

	/* Check if all nodes have degree 3 */
	int twos = 0;

	for (int i = 0; i < sprouts.size(); i++)
	{
		if (sprouts[i].degree == 2)
		{
			twos++;
		}
        else if (sprouts[i].degree == 1)	// still nodes left to connect
		{
			i = sprouts.size();
			twos = 0;
		}
	}

	if (twos == 1)
		return false;

	return true;
}

void Sprout::connect(int firstSprout, int secondSprout, int x, int y)
{
	int middleSprout;

	sprout tmpSprout;
    connection one, two;

	/* Create new sprout at x,y for snapping */
	tmpSprout.xPoint = x;
	tmpSprout.yPoint = y;
	tmpSprout.degree = 2;
	middleSprout = sprouts.size();
	sprouts.push_back(tmpSprout);
	drawSprouts();

	/* Split line at x,y */
	one.xPoints.push_back(sprouts[firstSprout ].xPoint);
	one.xPoints.push_back(sprouts[middleSprout].xPoint);
	one.yPoints.push_back(sprouts[firstSprout ].yPoint);
	one.yPoints.push_back(sprouts[middleSprout].yPoint);

	two.xPoints.push_back(sprouts[secondSprout].xPoint);
	two.xPoints.push_back(sprouts[middleSprout].xPoint);
	two.yPoints.push_back(sprouts[secondSprout].yPoint);
	two.yPoints.push_back(sprouts[middleSprout].yPoint);

	lines.push_back(two);
	lines.push_back(one);
}

void Sprout::drawSprouts()
{
	if (doLockSurface)
		SDL_LockSurface(surface);

	for (int i = 0; i < sprouts.size(); i++)
		circleColor(surface, sprouts[i].xPoint, sprouts[i].yPoint, sproutRadius, grayedColor);

	if (doLockSurface)
	{
		SDL_UnlockSurface(surface);
		SDL_Flip(surface);
	}
}

void Sprout::drawLines()
{
    if (doLockSurface)
        SDL_LockSurface(surface);

    //Blanks the screen
	SDL_FillRect(buffer, NULL, 0);
    SDL_FillRect(surface, NULL, 0);

	/* Draw lines */
	for (int it = 0; it < lines.size(); it++)
	{
		// iterate through each pair of control points defining the line
		for (int i = 1; i < lines[it].xPoints.size(); i++)	// what happens if there is only one xPoint?
		{
			thickLine(surface, lines[it].xPoints[i], lines[it].yPoints[i], lines[it].xPoints[i-1], lines[it].yPoints[i-1], defaultColor);
            thickLine(buffer,  lines[it].xPoints[i], lines[it].yPoints[i], lines[it].xPoints[i-1], lines[it].yPoints[i-1], bufferColor);
		}
	}

    drawSprouts();

	if (doLockSurface)
    {
        SDL_UnlockSurface(surface);
        SDL_Flip(surface);
    }
}

void Sprout::drawConnection(const connection& tmpConnection)
{
	if(doLockSurface)
        SDL_LockSurface(surface);

	for (int i = 1; i < tmpConnection.xPoints.size(); i++)	// what happens if there is only one xPoint?
	{
		thickLine(surface, tmpConnection.xPoints[i], tmpConnection.yPoints[i], tmpConnection.xPoints[i-1], tmpConnection.yPoints[i-1], defaultColor);
		thickLine(buffer,  tmpConnection.xPoints[i], tmpConnection.yPoints[i], tmpConnection.xPoints[i-1], tmpConnection.yPoints[i-1], bufferColor);
	}

	if (doLockSurface)
    {
        SDL_UnlockSurface(surface);
        SDL_Flip(surface);
    }
}

// WARNING! Only call this function if the line from index to x,y has not been drawn (only call this once per drawLines() call)
// lock the screen, blank the screen, draw the other lines, then call this function twice (for two lines), then unlock and flip the screen
bool Sprout::lineValid(int x0, int y0, int x, int y)
{
	double step;
	bool clear = true;
	int xNew, yNew, xOld, yOld;
	std::vector<int> xCollisions, yCollisions;

	/* Check line segment between x0,y0 and x,y */
	xOld = x;
	yOld = y;
	step = 1.0 / dist(x0, y0, x, y);

	for (double t = 0+step; t <= 1; t += step)
	{
		xNew = x0 * t + x * (1 - t);		// xNew = sprouts[index]->xPoint * t + x * ( 1 - t );
		yNew = y0 * t + y * (1 - t);		// yNew = sprouts[index]->yPoint * t + y * ( 1 - t );

		if (xOld == xNew && yOld == yNew)
			continue;

		if (getpixel(buffer, xNew, yNew) > 0) //!= SDL_MapRGBA(buffer->format, 0, 0, 0, 0))		// pixel plotted is already there (intersecting another line)
		{
            if (dist2(xNew, yNew, x0, y0) > sproutRadius*sproutRadius)//selectRadius*selectRadius)
            {
                xCollisions.push_back(xNew);
                yCollisions.push_back(yNew);
                clear = false;
            }
		}

		xOld = xNew;
		yOld = yNew;
	}

	/* Draw line */
	for (double t = 0+step; t <= 1; t += step)
	{
		xNew = x0 * t + x * (1 - t);		// xNew = sprouts[index]->xPoint * t + x * ( 1 - t );
		yNew = y0 * t + y * (1 - t);		// yNew = sprouts[index]->yPoint * t + y * ( 1 - t );

        for (int i = -thickness; i <= thickness; i++)
        {
            for (int j = -thickness; j <= thickness; j++)
            {
                pixelColor(surface, xNew+i, yNew+j, defaultColor);
            }
        }
	}

	/* Draw circles at collisions */
	for (int it = 0; it < xCollisions.size(); it++)
	{
		filledCircleColor(surface, xCollisions[it], yCollisions[it], 5, highlightColor);
	}

	return clear;
}

bool Sprout::select(int x, int y)
{
	if(sprouts.empty())
		return false;		// no sprouts, so nothing to try to select

	int closestPoint, closestRadius = surface->w, tmpRadius;

	for (int it = 0; it < sprouts.size(); it++)
	{
		tmpRadius = dist2(x, y, sprouts[it].xPoint, sprouts[it].yPoint);
		if (tmpRadius < closestRadius)
		{
			closestRadius = tmpRadius;
			closestPoint = it;
		}
	}

	if (closestRadius <= selectRadius * selectRadius)		// found a sprout withing selectRadius of x,y
	{
		activeSprout = closestPoint;
		return true;
	}

	return false;
}

// See if on the buffer (just the lines) there is a line nearby
bool Sprout::nearLine(int x, int y)
{
    int y_min = (y-nearLineRadius >= 0)?(y-nearLineRadius):0;
    int y_max = (y+nearLineRadius < surface->h)?(y+nearLineRadius):(surface->h-1); // Should this be -1?
    int x_min = (x-nearLineRadius >= 0)?(x-nearLineRadius):0;
    int x_max = (x+nearLineRadius < surface->w)?(x+nearLineRadius):(surface->w-1);

    // Go through the square encompassing the circle of radius nearLineRadius
    for (int y_search = y_min; y_search < y_max; y_search++)
    {
        for (int x_search = x_min; x_search < x_max; x_search++)
        {
            // We only care about points within the actual circle (note that dist, not dist2, is
            // the distance formula).
            if (dist(x, y, x_search, y_search) <= nearLineRadius)
            {
                // If this is not blank
                if (getpixel(buffer, x_search, y_search) != SDL_MapRGBA(buffer->format, 0, 0, 0, 0))
                {
                    return true;
                }
            }
        }
    }

    return false;
}

/* TODO: make this so it will draw properly if two points are too close together */
bool Sprout::highlightNear(int x, int y)
{
	if (select(x, y))
	{
		// if(!highlighted)	// not previously highlighted, now mouse near a sprout
		{
			circleColor(surface, sprouts[activeSprout].xPoint, sprouts[activeSprout].yPoint, sproutRadius, highlightColor);
			highlighted = true;
		}
	}
	else if (highlighted)	// was highlighted, now mouse not near that node
	{
        if (activeSprout > 0 && activeSprout < sprouts.size())
        {
            circleColor(surface, sprouts[activeSprout].xPoint, sprouts[activeSprout].yPoint, sproutRadius, grayedColor);
            highlighted = false;
        }
	}

	if (doLockSurface)
		SDL_Flip(surface);

    return highlighted;
}

inline double Sprout::dist(int x0, int y0, int x1, int y1)
{
    return std::sqrt((double)dist2(x0, y0, x1, y1));
}
//This is not a distance function.
//TODO: Change name if possible to what it is actualy being used for
inline int Sprout::dist2(int x0, int y0, int x1, int y1)
{
    return (x1-x0)*(x1-x0) + (y1-y0)*(y1-y0);
}

Sprout::~Sprout()
{
}