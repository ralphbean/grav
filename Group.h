#ifndef GROUP_H_
#define GROUP_H_

/*
 * @file Group.h
 * Represents a group of objects. The objects contained can be videos or other
 * groups (anything that is a RectangleBase). Objects in the group will be
 * drawn and moved together.
 * @author Andrew Ford
 */

#include "RectangleBase.h"

#include <vector>

class Group : public RectangleBase
{

public:
    Group( float _x, float _y );
    ~Group();
    
    void draw();
    
    void add( RectangleBase* object );
    void remove( RectangleBase* object );
    void removeAll();
    
    void rearrange();
    
    void move( float _x, float _y );
    void setPos( float _x, float _y );
    
private:
    std::vector<RectangleBase*> objects;
    float buffer;
    
};

#endif /*GROUP_H_*/
