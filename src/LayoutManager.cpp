/**
 * @file LayoutManager.h
 * Definition of the LayoutManager, which takes objects and arranges them into
 * grid, perimeter, fullscreen, etc.
 * @author Andrew Ford
 *         Ralph Bean
 */

#include <cstdio>
#include <cmath>
#include <algorithm>

#include "LayoutManager.h"
#include "RectangleBase.h"

// Little utilities... should be phased out with a better usage of std::map
bool str2bool(std::string str) { return str.compare( "True" ) == 0; }
int str2int(std::string str) { return atoi(str.c_str()); }
float str2fl(std::string str) { return atof(str.c_str()); }

LayoutManager::LayoutManager()
{ }

bool LayoutManager::arrange( std::string method,
                             RectangleBase outerRect,
                             RectangleBase innerRect,
                             std::map<std::string, std::vector<RectangleBase*> > data,
                             std::map<std::string, std::string> options)
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
                             std::map<std::string, std::string> options)
{
    typedef bool (LayoutManager::*fn_ptr)(
        float oL, float oR, float oU, float oD,
        float iL, float iR, float iU, float iD,
        std::map<std::string, std::vector<RectangleBase*> > dat,
        std::map<std::string, std::string> opts);

    std::map<std::string, fn_ptr> lookup;
    lookup["perimeter"] = &LayoutManager::perimeterArrange;
    lookup["grid"]      = &LayoutManager::gridArrange;
    lookup["tiling"]    = &LayoutManager::tilingArrange;
    lookup["focus"]     = &LayoutManager::focus;
    lookup["aspectFocus"] = &LayoutManager::aspectFocus;

    if ( lookup.find(method) == lookup.end() )
    {
        printf( "error: No such layout method '%s'.\n", method.c_str() );
        return false; // double false !!!
    }
    return (this->*lookup[method])( outerL, outerR, outerU, outerD,
                                    innerL, innerR, innerU, innerD,
                                    data, options );
}

bool LayoutManager::perimeterArrange( RectangleBase outerRect,
                                        RectangleBase innerRect,
                                        std::map<std::string, std::vector<RectangleBase*> > data,
                                        std::map<std::string, std::string> opts)
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
    //printf( "LayoutManager::perimeter: outer inners: %f,%f %f,%f\n",
    //        outerL, outerR, outerU, outerD );
    if ( data.find("objects") == data.end() )
    {
        printf( "ZOMG:::: perimeterArrange was not passed an 'objects'\n" );
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

    //printf( "LayoutManager::perimeter: ratios of area: %f %f, %i %i\n",
    //        topRatio, sideRatio, topNum, sideNum );

    // create lists of objects on top,right,down,left areas and send them
    // to be arranged
    std::vector<RectangleBase*> topObjs, rightObjs, bottomObjs, leftObjs;
    int end = topNum;

    if ( topNum > 0 )
    {
        //printf( "arranging objects %d to %d to top\n", 0, end-1 );
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
        //printf( "arranging objects %d to %d to right\n", topNum, end-1 );
        for ( int i = topNum; i < end; i++ )
            rightObjs.push_back( objects[i] );

        data["objects"] = rightObjs;

        gridArrange( innerR, outerR, outerU, outerD, false, true, true,
                     data, 1, sideNum );
    }

    end = topNum + sideNum + bottomNum;

    if ( bottomNum > 0 )
    {
        //printf( "arranging objects %d to %d to bottom\n", topNum + sideNum,
        //        end-1 );
        for ( int i = end-1; i >= topNum + sideNum; i-- )
            bottomObjs.push_back( objects[i] );

        data["objects"] = bottomObjs;

        gridArrange( innerL, innerR, innerD, outerD, true, false, true,
                     data, bottomNum, 1 );
    }

    if ( sideNum > 0 )
    {
        //printf( "arranging objects %d to %d to left\n",
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

bool LayoutManager::tilingArrange( RectangleBase outerRect,
                                   std::map<std::string, std::vector<RectangleBase*> > data)
{
    float outerL = outerRect.getLBound();
    float outerR = outerRect.getRBound();
    float outerU = outerRect.getUBound();
    float outerD = outerRect.getDBound();

    return tilingArrange( outerL, outerR, outerU, outerD, data );
}

bool LayoutManager::tilingArrange(float outerL, float outerR,
                                float outerU, float outerD,
                                float innerL, float innerR,
                                float innerU, float innerD,
                                std::map<std::string, std::vector<RectangleBase*> > data,
                                std::map<std::string, std::string> opts )
{
    // Setup opts defaults
    std::map<std::string, std::string> dflt = \
        std::map<std::string, std::string>();

    // NOTE: no defaults are really added here (none are specified).
    //       this code is left here from being copied from gridArrange to make
    //       it simple to add options and their defaults in the future.

    // Apply the opts defaults to opts
    for (std::map<std::string,std::string>::iterator i = dflt.begin();
         i != dflt.end(); i++)
    {
        // If the value wasn't specified by our caller, then load the default
        if ( opts.find(i->first) == opts.end() )
            opts[i->first] = i->second;
    }

    return tilingArrange( outerL, outerR, outerU, outerD, data );
}

float maximum_y(std::vector<RectangleBase*> rects, std::vector<bool> is_positioned)
{
    float max = 0;
    for ( int i = 0; i < rects.size(); i++ )
    {
        if ( is_positioned[i] )
        {
            float y = (rects[i])->getDestHeight() + (rects[i])->getDestY();
            if ( y > max ) max = y;
        }
    }
    return max;
}

void fillBoundedArea(
                float xmin, float xmax, float ymin, float ymax,
                int num_unpositioned,
                std::vector<RectangleBase*> & rects,
                std::vector<bool> & is_positioned)
{

}


void fillUnboundedArea(
                float xmin, float xmax, float ymin,
                int num_unpositioned,
                std::vector<RectangleBase*> & rects,
                std::vector<bool> & is_positioned)
{
    if ( num_unpositioned <= 0 ) return;

    int best_num_unpositioned = num_unpositioned;
    std::vector<RectangleBase*> best_rects = rects;
    std::vector<bool> best_is_positioned = is_positioned;

    int best_max_y = 10000000; // infinity

    for ( int i = 0; i < rects.size(); i++ )
    {
        printf("Looking at rectangle %i\n", i);
        if ( ! is_positioned[i] && (rects[i])->getDestWidth() <= xmax - xmin )
        {
            // It will fit.  Try it.
            int test1_num_unpositioned = num_unpositioned;
            std::vector<RectangleBase*> test1_rects = rects;
            std::vector<bool> test1_is_positioned = is_positioned;

            (test1_rects[i])->setDestX( xmin );
            (test1_rects[i])->setDestY( ymin );
            test1_is_positioned[i] = true;

            // Fill the area on the right
            fillUnboundedArea(xmin + rects[i]->getDestWidth(), xmax, //Bounded?
                            ymin,// ymin + rects[i]->getDestHeight(),//Bounded?
                            test1_num_unpositioned, test1_rects, test1_is_positioned );
            // Fill the area on the bottom
            fillUnboundedArea(xmin, xmax, ymin + rects[i]->getDestHeight(),
                            test1_num_unpositioned, test1_rects, test1_is_positioned);

            float test1_max_y = maximum_y(test1_rects, test1_is_positioned);

            if ((test1_num_unpositioned == 0) && (test1_max_y < best_max_y))
            {
                best_max_y = test1_max_y;
                best_rects = test1_rects;
                best_is_positioned = test1_is_positioned;
                best_num_unpositioned = test1_num_unpositioned;
            }

            // Divide the remaining area vertically.
            int test2_num_unpositioned = num_unpositioned;
            std::vector<RectangleBase*> test2_rects = rects;
            std::vector<bool> test2_is_positioned = is_positioned;

            (test2_rects[i])->setDestX( xmin );
            (test2_rects[i])->setDestY( ymin );
            test2_is_positioned[i] = true;

            fillUnboundedArea(xmin + rects[i]->getDestWidth(), xmax, ymin,
                            test2_num_unpositioned, test2_rects, test2_is_positioned);
            fillUnboundedArea(xmin, xmin + rects[i]->getDestWidth(),
                            ymin + rects[i]->getDestHeight(),
                            test2_num_unpositioned, test2_rects, test2_is_positioned);

            float test2_max_y = maximum_y(test2_rects, test2_is_positioned);

            if ((test2_num_unpositioned == 0) && (test2_max_y < best_max_y))
            {
                // This test is better than the other one.  save it.
                best_max_y = test2_max_y;
                best_rects = test2_rects;
                best_is_positioned = test2_is_positioned;
                best_num_unpositioned = test2_num_unpositioned;
            }
        } // end trying this rectangle.
    } // end looping through the rectangles.

    // return the best we found
    is_positioned = best_is_positioned;
    num_unpositioned = best_num_unpositioned;
    rects = best_rects;
    for ( int i = 0; i < rects.size(); i++ )
    {
        (rects[i])->move((rects[i])->getDestX(), (rects[i])->getDestY());
    }
}

bool LayoutManager::tilingArrange(
                float outerL, float outerR, float outerU, float outerD,
                std::map<std::string, std::vector<RectangleBase*> > data )
{
    printf( "Trying to do tiling\n");
    // Extract object data
    if ( data.find("objects") == data.end() )
    {
        printf( "ZOMG:::: tilingArrange was not passed an 'objects'\n" );
        return false;
    }
    std::vector<RectangleBase*> objects = data["objects"];

    if ( objects.size() == 0 )
        return false;

    // if we only have one object, just fullscreen it to the area
    if ( objects.size() == 1 )
    {
        objects[0]->fillToRect( outerL, outerR, outerU, outerD );
        return true;
    }

    sort(objects.begin(), objects.end(), RectangleHeightComparator);

    // TODO -- to do focus, we can scale up selected rects here before we binpack

    std::vector<bool> is_positioned;
    for ( int i = 0; i < objects.size(); i++ ) is_positioned.push_back(false);

    float xmin = -10;
    float xmax = 10;
    float ymin = 0;

    fillUnboundedArea( xmin, xmax, ymin, is_positioned.size(), objects, is_positioned);
    printf("Outties from tiling arrange.\n");

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

bool LayoutManager::gridArrange(float outerL, float outerR,
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
        printf( "ZOMG:::: gridArrange was not passed an 'objects'\n" );
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
        //printf( "layout: doing grid arrangement with %i objects (%ix%i)\n",
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

    //printf( "grid:outers: %f,%f %f,%f\n", outerL, outerR, outerU, outerD );
    //printf( "grid:numx %i numy %i\n", numX, numY );

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
        //printf( "grid: edges are %f,%f\n", edgeL, edgeR );
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

    //printf( "grid: starting at %f,%f\n", curX, curY );
    //printf( "grid: stride is %f\n", stride );
    //printf( "grid: span is %f\n", span );

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
                //printf( "layout setting height to %f\n", newHeight );
                objects[i]->setTotalHeight( newHeight );
            }
            else
            {
                //printf( "layout setting width to %f\n", newWidth );
                objects[i]->setTotalWidth( newWidth );
            }
        }
    }

    for ( unsigned int i = 0; i < objects.size(); i++ )
    {
        //printf( "grid: moving object %i to %f,%f\n", i, curX, curY );
        objects[i]->move( curX, curY - objects[i]->getCenterOffsetY() );
        int objectsLeft = (int)objects.size() - i - 1;

        if ( horiz )
        {
            curX += stride;
            if ( (i+1) % numX == 0 )
            {
                //printf( "grid: changing to new row\n" );
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
        printf( "ZOMG:::: focus was not passed an 'outers'\n" );
        return false;
    }
    if ( data.find("inners") == data.end() )
    {
        printf( "ZOMG:::: focus was not passed an 'inners'\n" );
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

bool LayoutManager::aspectFocus( RectangleBase outerRect, RectangleBase innerRect,
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

bool LayoutManager::aspectFocus( float outerL, float outerR, float outerU, float outerD,
            float innerL, float innerR, float innerU, float innerD,
            std::map<std::string, std::vector<RectangleBase*> > data,
            std::map<std::string, std::string> opts )
{
    // Extract object data
    if ( data.find("outers") == data.end() )
    {
        printf( "ZOMG:::: aspectfocus was not passed an 'outers'\n" );
        return false;
    }
    if ( data.find("inners") == data.end() )
    {
        printf( "ZOMG:::: aspectfocus was not passed an 'inners'\n" );
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
