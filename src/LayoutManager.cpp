/*
 * @file LayoutManager.h
 *
 * Definition of the LayoutManager, which takes objects and arranges them into
 * grid, perimeter, fullscreen, etc.
 *
 * @author Andrew Ford
 * @author Ralph Bean
 * Copyright (C) 2011 Rochester Institute of Technology
 *
 * This file is part of grav.
 *
 * grav is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * grav is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with grav.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <cstdio>
#include <cmath>

#include "LayoutManager.h"
#include "RectangleBase.h"

#include "gravUtil.h"

LayoutManager::LayoutManager()
{ }

bool LayoutManager::arrange( std::string method,
        RectangleBase outerRect,
        RectangleBase innerRect,
        std::map<std::string, std::vector<RectangleBase*> > data,
        std::map<std::string, std::string> options )
{
    float outerL = outerRect.getLBound();
    float outerR = outerRect.getRBound();
    float outerU = outerRect.getUBound();
    float outerD = outerRect.getDBound();

    float innerL = innerRect.getLBound();
    float innerR = innerRect.getRBound();
    float innerU = innerRect.getUBound();
    float innerD = innerRect.getDBound();

    return arrange(method,
            outerL, outerR, outerU, outerD,
            innerL, innerR, innerU, innerD,
            data, options);
}

bool LayoutManager::arrange( std::string method,
        float outerL, float outerR,
        float outerU, float outerD,
        float innerL, float innerR,
        float innerU, float innerD,
        std::map<std::string, std::vector<RectangleBase*> > data,
        std::map<std::string, std::string> options )
{
    typedef bool (LayoutManager::*fn_ptr)(
        float oL, float oR, float oU, float oD,
        float iL, float iR, float iU, float iD,
        std::map<std::string, std::vector<RectangleBase*> > dat,
        std::map<std::string, std::string> opts);

    std::map<std::string, fn_ptr> lookup;
    lookup["perimeter"]   = &LayoutManager::perimeterArrange;
    lookup["grid"]        = &LayoutManager::gridArrange;
    lookup["focus"]       = &LayoutManager::focus;
    lookup["aspectFocus"] = &LayoutManager::aspectFocus;

    if ( lookup.find(method) == lookup.end() )
    {
        gravUtil::logError( "InputHandler::arrange: method %s not found\n",
                method.c_str() );
        return false;
    }
    return (this->*lookup[method])( outerL, outerR, outerU, outerD,
                                    innerL, innerR, innerU, innerD,
                                    data, options );
}

bool LayoutManager::perimeterArrange( RectangleBase outerRect,
        RectangleBase innerRect,
        std::map<std::string, std::vector<RectangleBase*> > data,
        std::map<std::string, std::string> opts )
{
    float outerL = outerRect.getLBound();
    float outerR = outerRect.getRBound();
    float outerU = outerRect.getUBound();
    float outerD = outerRect.getDBound();

    float innerL = innerRect.getLBound();
    float innerR = innerRect.getRBound();
    float innerU = innerRect.getUBound();
    float innerD = innerRect.getDBound();

    return perimeterArrange( outerL, outerR, outerU, outerD, innerL, innerR,
                                innerU, innerD, data );
}

bool LayoutManager::perimeterArrange( float outerL, float outerR,
        float outerU, float outerD,
        float innerL, float innerR,
        float innerU, float innerD,
        std::map<std::string, std::vector<RectangleBase*> > data,
        std::map<std::string, std::string> opts)
{
    //gravUtil::logVerbose( "LayoutManager::perimeter: outer inners: %f,%f %f,%f\n",
    //        outerL, outerR, outerU, outerD );
    if ( data.find("objects") == data.end() )
    {
        gravUtil::logError( "InputHandler::perimeterArrange was not passed an "
                "'objects'\n" );
        return false;
    }
    std::vector<RectangleBase*> objects = data["objects"];

    float topRatio = (innerR-innerL) / ((outerU-outerD)+(innerR-innerL));
    float sideRatio = (outerU-outerD) / ((outerU-outerD)+(innerR-innerL));
    int topNum, sideNum, bottomNum;

    if ( objects.size() == 1 )
    {
        topNum = 1; sideNum = 0; bottomNum = 0;
    }
    else
    {
        topNum = floor( topRatio * (float)objects.size() / 2.0f );
        sideNum = ceil( sideRatio * (float)objects.size() / 2.0f );
        bottomNum = std::max( (int)objects.size() - topNum - (sideNum*2), 0 );
    }

    //gravUtil::logVerbose( "LayoutManager::perimeter: ratios of area: %f %f, %i %i\n",
    //        topRatio, sideRatio, topNum, sideNum );

    // create lists of objects on top,right,down,left areas and send them
    // to be arranged
    std::vector<RectangleBase*> topObjs, rightObjs, bottomObjs, leftObjs;
    int end = topNum;

    if ( topNum > 0 )
    {
        //gravUtil::logVerbose( "arranging objects %d to %d to top\n", 0, end-1 );
        for ( int i = 0; i < end; i++ )
            topObjs.push_back( objects[i] );

        data["objects"] = topObjs;

        // constant on top is for space for text
        gridArrange( innerL, innerR, outerU-0.8f, innerU, true, false, true,
                     data, topNum, 1 );
    }

    end = topNum + sideNum;

    if ( sideNum > 0 )
    {
        //gravUtil::logVerbose( "arranging objects %d to %d to right\n", topNum, end-1 );
        for ( int i = topNum; i < end; i++ )
            rightObjs.push_back( objects[i] );

        data["objects"] = rightObjs;

        gridArrange( innerR, outerR, outerU, outerD, false, true, true,
                     data, 1, sideNum );
    }

    end = topNum + sideNum + bottomNum;

    if ( bottomNum > 0 )
    {
        //gravUtil::logVerbose( "arranging objects %d to %d to bottom\n", topNum + sideNum,
        //        end-1 );
        for ( int i = end-1; i >= topNum + sideNum; i-- )
            bottomObjs.push_back( objects[i] );

        data["objects"] = bottomObjs;

        gridArrange( innerL, innerR, innerD, outerD, true, false, true,
                     data, bottomNum, 1 );
    }

    if ( sideNum > 0 )
    {
        //gravUtil::logVerbose( "arranging objects %d to %d to left\n",
        //        topNum + sideNum + bottomNum, objects.size()-1 );
        for ( int i = objects.size()-1; i >= topNum + sideNum + bottomNum; i-- )
            leftObjs.push_back( objects[i] );

        data["objects"] = leftObjs;

        gridArrange( outerL, innerL, outerU, outerD, false, true, true,
                     data, 1, sideNum );
    }
    // TODO - return the conjunction of the above gridArrange return values
    return true;
}

bool LayoutManager::gridArrange( RectangleBase outerRect,
        bool horiz, bool edge, bool resize,
        std::map<std::string, std::vector<RectangleBase*> > data,
        int numX, int numY )
{
    float outerL = outerRect.getLBound();
    float outerR = outerRect.getRBound();
    float outerU = outerRect.getUBound();
    float outerD = outerRect.getDBound();

    return gridArrange( outerL, outerR, outerU, outerD, horiz, edge,
                            resize, data, numX, numY );
}

// Little utility... should be phased out with a better usage of std::map
bool str2bool(std::string str) { return str.compare( "True" ) == 0; }
int str2int(std::string str) { return atoi(str.c_str()); }
float str2fl(std::string str) { return atof(str.c_str()); }

bool LayoutManager::gridArrange( float outerL, float outerR,
        float outerU, float outerD,
        float innerL, float innerR,
        float innerU, float innerD,
        std::map<std::string, std::vector<RectangleBase*> > data,
        std::map<std::string, std::string> opts )
{
    // Setup opts defaults
    std::map<std::string, std::string> dflt = \
        std::map<std::string, std::string>();
    dflt["horiz"] = "True"; dflt["edge"] = "False"; dflt["resize"] = "True";
    dflt["numX"] = "0";     dflt["numY"] = "0";

    // Apply the opts defaults to opts
    for (std::map<std::string,std::string>::iterator i = dflt.begin();
         i != dflt.end(); i++)
    {
        // If the value wasn't specified by our caller, then load the default
        if ( opts.find(i->first) == opts.end() )
            opts[i->first] = i->second;
    }

    return gridArrange(outerL, outerR, outerU, outerD,
                       str2bool(opts["horiz"]),
                       str2bool(opts["edge"]),
                       str2bool(opts["resize"]),
                       data,
                       str2int(opts["numX"]),
                       str2int(opts["numY"]));
}

bool LayoutManager::gridArrange( float outerL, float outerR, float outerU,
        float outerD,
        bool horiz, bool edge, bool resize,
        std::map<std::string, std::vector<RectangleBase*> > data,
        int numX, int numY )
{
    // Extract object data
    if ( data.find("objects") == data.end() )
    {
        gravUtil::logError( "InputHandler::gridArrange was not passed an "
                "'objects'\n" );
        return false;
    }
    std::vector<RectangleBase*> objects = data["objects"];

    if ( objects.size() == 0 )
        return false;

    // both of these being 0 (also the default vals) means we should figure out
    // the proper numbers here
    if ( numX == 0 && numY == 0 )
    {
        numX = ceil( sqrt( objects.size() ) );
        numY = objects.size() / numX + ( objects.size() % numX > 0 );
        //gravUtil::logVerbose( "layout: doing grid arrangement with %i objects (%ix%i)\n",
        //            objects.size(), numX, numY );
    }

    // if there's too many objects, fail
    if (  objects.size() > (unsigned int)(numX * numY) )
        return false;

    // if we only have one object, just fullscreen it to the area
    if ( objects.size() == 1 )
    {
        objects[0]->fillToRect( outerL, outerR, outerU, outerD );
        return true;
    }

    //gravUtil::logVerbose( "grid:outers: %f,%f %f,%f\n", outerL, outerR, outerU, outerD );
    //gravUtil::logVerbose( "grid:numx %i numy %i\n", numX, numY );

    float span; // height of rows if going horizontally,
                // width of columns if going vertically
    float stride; // distance to move each time
    float curX, curY;
    float edgeL = outerL, edgeR = outerR, edgeU = outerU, edgeD = outerD;

    // set up span and stride, etc differently for horizontal vs vertical
    // arrangement
    if ( horiz )
    {
        span = (outerU-outerD) / numY;
        stride = (outerR-outerL) / numX;

        edgeL = outerL + 0.2f + (stride / 2);
        edgeR = outerR - 0.2f - (stride / 2);
        //gravUtil::logVerbose( "grid: edges are %f,%f\n", edgeL, edgeR );
        if ( edge ) stride = (edgeR-edgeL) / std::max(1, (numX-1));

        curY = outerU - (span/2.0f);

        if ( numX == 1 )
            curX = (outerR+outerL)/2.0f;
        else
        {
            if ( edge )
                curX = edgeL;
            else
                curX = outerL + (stride/2.0f);
        }
    }
    else
    {
        span = (outerR-outerL) / numX;
        stride = (outerU-outerD) / numY;

        edgeU = outerU - 0.2f - (stride / 2);
        edgeD = outerD + 0.2f + (stride / 2);
        if ( edge ) stride = (edgeU-edgeD) / std::max(1, (numY-1));

        curX = outerL + (span/2.0f);

        if ( numY == 1 )
            curY = (outerU+outerD)/2.0f;
        else
        {
            if ( edge )
                curY = edgeU;
            else
                curY = outerU - (stride/2.0f);
        }
    }

    //gravUtil::logVerbose( "grid: starting at %f,%f\n", curX, curY );
    //gravUtil::logVerbose( "grid: stride is %f\n", stride );
    //gravUtil::logVerbose( "grid: span is %f\n", span );

    // if we're resizing them, do it on a first pass so the calculations later
    // are correct
    if ( resize )
    {
        for ( unsigned int i = 0; i < objects.size(); i++ )
        {
            float objectAspect =
                       objects[i]->getTotalWidth()/objects[i]->getTotalHeight();
            float aspect = 1.0f;
            float newWidth = 1.0f;
            float newHeight = 1.0f;
            if ( horiz )
            {
                aspect = stride / span;
                // the .95s are to push things away from the edges, which can
                // cut close due to roundoff error etc.
                newHeight = span * 0.95f;
                newWidth = stride * 0.95f;
            }
            else
            {
                aspect = span / stride;
                newHeight = stride * 0.95f;
                newWidth = span * 0.95f;
            }
            if ( aspect > objectAspect )
            {
                //gravUtil::logVerbose( "layout setting height to %f\n", newHeight );
                objects[i]->setTotalHeight( newHeight );
            }
            else
            {
                //gravUtil::logVerbose( "layout setting width to %f\n", newWidth );
                objects[i]->setTotalWidth( newWidth );
            }
        }
    }

    for ( unsigned int i = 0; i < objects.size(); i++ )
    {
        //gravUtil::logVerbose( "grid: moving object %i to %f,%f\n", i, curX, curY );
        objects[i]->move( curX, curY - objects[i]->getCenterOffsetY() );
        int objectsLeft = (int)objects.size() - i - 1;

        if ( horiz )
        {
            curX += stride;
            if ( (i+1) % numX == 0 )
            {
                //gravUtil::logVerbose( "grid: changing to new row\n" );
                curY -= span;
                // if the number of objects we have left is less than a full
                // row/column, change stride such that it's evenly spaced
                if ( objectsLeft < numX )
                {
                    if ( edge )
                        stride = (edgeR-edgeL) / std::max(1, (objectsLeft-1));
                    else
                        stride = (outerR-outerL) / (objectsLeft);
                }
                curX = outerL + (stride/2.0f);
            }
        }
        else
        {
            curY -= stride;
            if ( (i+1) % numY == 0 )
            {
                curX += span;
                // if the number of objects we have left is less than a full
                // row/column, change stride such that it's evenly spaced
                if ( objectsLeft < numY )
                {
                    if ( edge )
                        stride = (edgeU-edgeD) / std::max(1, (objectsLeft-1));
                    else
                        stride = outerU-outerD / (objectsLeft+1);
                }
                curY = outerU - (stride/2.0f);
            }
        }
    }

    return true;
}

bool LayoutManager::focus( RectangleBase outerRect, RectangleBase innerRect,
        std::map<std::string, std::vector<RectangleBase*> > data,
        std::map<std::string, std::string> opts)
{
    float innerL = innerRect.getLBound();
    float innerR = innerRect.getRBound();
    float innerU = innerRect.getUBound();
    float innerD = innerRect.getDBound();

    float outerL = outerRect.getLBound();
    float outerR = outerRect.getRBound();
    float outerU = outerRect.getUBound();
    float outerD = outerRect.getDBound();

    return focus( outerL, outerR, outerU, outerD,
                  innerL, innerR, innerU, innerD,
                  data, opts );
}

bool LayoutManager::focus( float outerL, float outerR,
        float outerU, float outerD,
        float innerL, float innerR,
        float innerU, float innerD,
        std::map<std::string, std::vector<RectangleBase*> > data,
        std::map<std::string, std::string> opts)
{
    // Extract object data
    if ( data.find("outers") == data.end() )
    {
        gravUtil::logError( "InputHandler::focus was not passed an "
                "'outers'\n" );
        return false;
    }
    if ( data.find("inners") == data.end() )
    {
        gravUtil::logError( "InputHandler::focus was not passed an "
                "'inners'\n" );
        return false;
    }
    std::vector<RectangleBase*> outers = data["outers"];
    std::vector<RectangleBase*> inners = data["inners"];

    // Setup opts defaults
    std::map<std::string, std::string> dflt =
        std::map<std::string, std::string>();
    dflt["scaleX"] = "0.65";   dflt["scaleY"] = "0.6";

    // Apply the opts defaults to opts
    for (std::map<std::string,std::string>::iterator i = dflt.begin();
         i != dflt.end(); i++)
    {
        // If the value wasn't specified by our caller, then load the default
        if ( opts.find(i->first) == opts.end() )
            opts[i->first] = i->second;
    }

    float gridBoundL;
    float gridBoundR;
    float gridBoundU;
    float gridBoundD;
    float perimeterInnerL;
    float perimeterInnerR;
    float perimeterInnerU;
    float perimeterInnerD;

    // if there aren't any objects in the outside, just size the inner objects
    // fully to the center as a grid
    if ( outers.empty() )
    {
        gridBoundL = outerL;
        gridBoundR = outerR;
        gridBoundU = outerU;
        gridBoundD = outerD;
    }
    else
    {
        float centerX = ( innerL + innerR ) / 2.0f;
        float centerY = ( innerD + innerU ) / 2.0f;
        float Xdist = ( innerR - innerL ) / 2.0f;
        float Ydist = ( innerU - innerD ) / 2.0f;
        // .95f to give some extra room
        // TODO make this an argument?
        // could be easier now with the opts map?
        gridBoundL = centerX - (Xdist*0.95f);
        gridBoundR = centerX + (Xdist*0.95f);
        gridBoundU = centerY + (Ydist*0.95f);
        gridBoundD = centerY - (Ydist*0.95f);
        perimeterInnerL = centerX - Xdist;
        perimeterInnerR = centerX + Xdist;
        perimeterInnerU = centerY + Ydist;
        perimeterInnerD = centerY - Ydist;
    }

    std::map<std::string, std::vector<RectangleBase*> > a_data =
        std::map<std::string, std::vector<RectangleBase*> >();
    a_data["objects"] = inners;

    bool gridRes = gridArrange( gridBoundL, gridBoundR, gridBoundU, gridBoundD,
                                 true, false, true,
                                 a_data );

    bool perimRes = true;
    if ( !outers.empty() )
    {
        a_data["objects"] = outers;
        perimRes = perimeterArrange( outerL, outerR, outerU, outerD,
                            perimeterInnerL, perimeterInnerR,
                            perimeterInnerU, perimeterInnerD,
                            a_data );
    }

    return gridRes && perimRes;
}

bool LayoutManager::aspectFocus( RectangleBase outerRect,
        RectangleBase innerRect,
        std::map<std::string, std::vector<RectangleBase*> > data,
        std::map<std::string, std::string> opts )
{
    float innerL = innerRect.getLBound();
    float innerR = innerRect.getRBound();
    float innerU = innerRect.getUBound();
    float innerD = innerRect.getDBound();

    float outerL = outerRect.getLBound();
    float outerR = outerRect.getRBound();
    float outerU = outerRect.getUBound();
    float outerD = outerRect.getDBound();

    return aspectFocus( outerL, outerR, outerU, outerD,
                  innerL, innerR, innerU, innerD,
                  data, opts );
}

bool LayoutManager::aspectFocus( float outerL, float outerR,
        float outerU, float outerD,
        float innerL, float innerR,
        float innerU, float innerD,
        std::map<std::string, std::vector<RectangleBase*> > data,
        std::map<std::string, std::string> opts )
{
    // Extract object data
    if ( data.find("outers") == data.end() )
    {
        gravUtil::logError( "InputHandler::aspectfocus was not passed an "
                "'outers'\n" );
        return false;
    }
    if ( data.find("inners") == data.end() )
    {
        gravUtil::logError( "InputHandler::aspectfocus was not passed an "
                "'inners'\n" );
        return false;
    }
    std::vector<RectangleBase*> outers = data["outers"];
    std::vector<RectangleBase*> inners = data["inners"];

    // Setup opts defaults
    std::map<std::string, std::string> dflt =
        std::map<std::string, std::string>();
    dflt["aspect"] = "1.5555";
    dflt["scale"] = "0.65";

    // Apply the opts defaults to opts
    for (std::map<std::string,std::string>::iterator i = dflt.begin();
         i != dflt.end(); i++)
    {
        // If the value wasn't specified by our caller, then load the default
        if ( opts.find(i->first) == opts.end() )
            opts[i->first] = i->second;
    }

    float outerAspect = ( outerR - outerL ) / ( outerU - outerD );
    float aspect = str2fl( opts["aspect"] );
    float scale = str2fl( opts["scale"] );
    float centerX = ( outerL + outerR ) / 2.0f;
    float centerY = ( outerD + outerU ) / 2.0f;
    float width = outerR - outerL;
    float height = outerU - outerD;
    float xScale = 1.0f;
    float yScale = 1.0f;

    // use the scale for the dimension that the inner rect would be closer on,
    // so that the inner rect doesn't cross the outer rect
    // this will also size it such that the inner rect aspect ratio matches the
    // aspect value
    if ( aspect >= outerAspect )
    {
        xScale = scale * width / 2.0f;
        yScale = xScale / aspect;
    }
    else
    {
        yScale = scale * height / 2.0f;
        xScale = yScale * aspect;
    }

    innerL = centerX - xScale;
    innerR = centerX + xScale;
    innerU = centerY + yScale;
    innerD = centerY - yScale;

    return focus( outerL, outerR, outerU, outerD,
                  innerL, innerR, innerU, innerD,
                  data, opts );
}
