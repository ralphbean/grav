#ifndef RECTANGLEBASE_H_
#define RECTANGLEBASE_H_

/*
 * @file RectangleBase.h
 * RectangleBase defines an object that has position & dimensions and can be
 * drawn, moved, and clicked on. The base class for VideoSource, Group and any
 * future object that needs to be clickable.
 * @author Andrew Ford
 */

#include <string>

#include "GLUtil.h"

typedef struct {
    float R;
    float G;
    float B;
    float A;
} RGBAColor;

// we need to do forward declaration since rectanglebase and group circularly
// reference each other
class Group;

class RectangleBase
{

public:
    RectangleBase();
    RectangleBase( float _x, float _y );
    RectangleBase( const RectangleBase& other );
    virtual ~RectangleBase();
    
    void setDefaults();

    /*
     * Returns the width or height of the object. These are virtual because
     * the dimensions of a VideoSource, e.g., are based on the video's aspect
     * ratio in addition to how much it's being scaled. In objects without
     * aspect ratio, these will most likely be equal to the scale factors.
     */
    virtual float getWidth(); virtual float getHeight();
    float getTotalWidth(); float getTotalHeight();
    virtual float getDestWidth(); virtual float getDestHeight();
    float getBorderSize(); float getDestBorderSize();
    float getBorderScale();
    float getLBound(); float getRBound(); float getUBound(); float getDBound();

    /*
     * Note that total text height includes offset from border (getTextOffset())
     * Parts of letters that fall under the line (y, g, j, etc) will go in that
     * space.
     */
    float getTextHeight(); float getTextWidth();
    float getTextScale();
    float getTextOffset();

    /*
     * Offset of the center of just the video/inner rectangle content to the
     * center of the whole, meaning the borders and text are included.
     */
    float getCenterOffsetX(); float getCenterOffsetY();

    /*
     * Change the position of the object. Move may or may not have animation
     * (based on the animation switch), set will never have animation.
     */
    virtual void move( float _x, float _y );
    virtual void setPos( float _x, float _y );
    
    /*
     * Change the size of the object.
     */
    virtual void setScale( float xs, float ys );
    /*
     * The bool is so we can have a different version for groups, so they can
     * resize their members or not.
     */
    virtual void setScale( float xs, float ys, bool resizeMembers );
    
    /*
     * Change the selected size to be equal to the argument given, preserving
     * aspect ratio.
     */
    virtual void setWidth( float w );
    virtual void setHeight( float h );
    
    /*
     * Change the total size, ie, including borders and text, while preserving
     * aspect ratio.
     */
    void setTotalWidth( float w );
    void setTotalHeight( float h );

    /*
     * Set the background texture for this object.
     */
    void setTexture( GLuint tex, int width, int height );

    float getX(); float getY(); float getZ();
    float getDestX(); float getDestY();
    float getScaleX(); float getScaleY();
    float getLat(); float getLon();
    
    void setName( std::string s );
    void setSiteID( std::string sid );
    std::string getName();
    std::string getSubName();
    std::string getAltName();
    std::string getSiteID();
    
    bool isSelected();
    bool isSelectable();
    void setSelect( bool select );
    virtual void setSelectable( bool s );
    void setEffectVal( float f );
    void setAnimation( bool anim );
    
    /*
     * Is this object a member of a group?
     */
    bool isGrouped();
    
    /*
     * Is this object a group itself?
     */
    virtual bool isGroup();

    void setGroup( Group* g );
    Group* getGroup();

    RGBAColor getColor();
    RGBAColor getBaseColor();
    void setColor( RGBAColor c );
    void setSecondaryColor( RGBAColor c );
    /*
     * Resets color back to regular gray color.
     */
    void resetColor();

    /*
     * For now locking only has meaning for groups, but is needed in the generic
     * drawing code.
     */
    bool isLocked();
    void changeLock();

    /*
     * Return whether or not the current name can be said to be 'final'.
     * For video sources, this is the SDES_NAME as opposed to SDES_CNAME, and
     * for groups it signifies whether or not it has figured out a common
     * name from its members.
     */
    bool usingFinalName();
    
    virtual bool updateName();
    void updateTextBounds( bool reset = false );
    void setSubstring( int start, int end );
    
    /*
     * Checks whether this object intersects with another rectangle, defined
     * either by its specific points or an existing rectangle object.
     * 
     * @param L,R,U,D   floating-point values that determine the EDGES
     *                  (left, right, up, down) of the rectangle to be
     *                  checked - not the points.
     */
    bool intersect( float L, float R, float U, float D );
    bool intersect( RectangleBase* other );
    
    // set/get enable/disable for rendering
    // this means different things for different subclasses - in base it means
    // nothing, videosource uses it to disable rendering, runway makes it
    // invisible
    virtual void setRendering( bool r );
    bool getRendering();

    /*
     * GL draw function to render the object.
     */
    virtual void draw();
    
protected:
    // position in world space (center of the object)
    float x,y,z;
    // x/y destinations for movement/animation
    float destX, destY;
    float xAngle, yAngle, zAngle;
    float scaleX, scaleY;
    float destScaleX, destScaleY;
    
    // value for the amplitude of the audio connection
    float effectVal;
    
    // for global positioning
    float lat, lon;
    
    RGBAColor borderColor;
    RGBAColor destBColor;
    RGBAColor baseBColor;
    RGBAColor secondaryColor;
    RGBAColor destSecondaryColor;
    
    std::string name;
    std::string altName;
    std::string siteID;
    // substring of the name to render
    int nameStart, nameEnd;
    bool finalName;
    // if name ends up being wider than the object itself, cut off with ellipsis
    int cutoffPos;
    
    FTFont* font;
    FTBBox textBounds;
    // amount to scale the text relative to the total size
    float relativeTextScale;

    // size of the border relative to total size
    float borderScale;
    GLuint borderTex;
    // width/height of our border/background texture in
    // pixels
    int twidth, theight;
    
    bool selected;
    bool selectable;
    bool grouped;
    Group* myGroup;
    bool locked;
    bool showLockStatus;

    bool enableRendering;
    bool debugDraw;
    
    bool animated;
    void animateValues();
    
};

#endif /*RECTANGLEBASE_H_*/
