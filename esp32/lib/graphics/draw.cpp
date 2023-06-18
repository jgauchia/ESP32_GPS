#include <Arduino.h>
#include <TFT_eSPI.h>
#include "graphics.h"
#include "maps.h"
#include "../conf.h"


void draw( TFT_eSPI& tft, ViewPort& viewPort, MemBlocks& memblocks)
{
    tft.fillScreen( BACKGROUND_COLOR);
    for( MapBlock* mblock: memblocks.blocks){
        if( !mblock || !mblock->inView) continue;
        log_v("Polylines: %i Polygons: %i", mblock->polylines.size(), mblock->polygons.size());
        Point32 screen_center_mc = viewPort.center - mblock->offset;  // screen center with features coordinates
        BBox screen_bbox_mc = viewPort.bbox - mblock->offset;  // screen boundaries with features coordinates
        
        ////// Polygons 
        for( Polygon polygon : mblock->polygons){
            if( polygon.color == TFT_YELLOW) log_w("Polygon type unknown");
            std::vector<Point16> points2;
            bool hit = false;
            for( Point16 p : polygon.points){
                if( screen_bbox_mc.contains_point( p)) hit = true;
                points2.push_back( toScreenCoords( p, screen_center_mc));
                log_v("polygon: %s (%i,%i)", polygon.color, p.x, p.y);
            }
            if( hit) fill_polygon( tft, points2, polygon.color);
        }
        
        ////// Lines 
        for( Polyline polyline : mblock->polylines){
            for( int i=0; i < (polyline.points.size() - 1); i++) {
                Point16 p1 = polyline.points[i];
                Point16 p2 = polyline.points[i+1];
                if( !screen_bbox_mc.contains_point( p1) && !screen_bbox_mc.contains_point( p2)) continue; // TODO: could still cut the screen area!
                p1 = toScreenCoords( p1, screen_center_mc);  // TODO: clipping
                p2 = toScreenCoords( p2, screen_center_mc);
                log_v(" %i (%i,%i) (%i,%i) ", polyline.color, p1.x, p1.y, p2.x, p2.y);
                tft.drawWideLine(
                    p1.x, SCREEN_HEIGHT - p1.y,
                    p2.x, SCREEN_HEIGHT - p2.y,
                    polyline.width/PIXEL_SIZE ?: 1, polyline.color, polyline.color);  
            }
        }
    }

    tft.fillTriangle( 
        SCREEN_WIDTH/2 - 4, SCREEN_HEIGHT/2 + 5, 
        SCREEN_WIDTH/2 + 4, SCREEN_HEIGHT/2 + 5, 
        SCREEN_WIDTH/2,     SCREEN_HEIGHT/2 - 6, 
        RED);
    log_v("Draw done!");
}

void stats( ViewPort& viewPort, MapBlock* mblock)
{
    Point32 screen_center_mc = viewPort.center - mblock->offset;  // screen center with features coordinates
    BBox screen_bbox_mc = viewPort.bbox - mblock->offset;  // screen boundaries with features coordinates
    BBox map_bbox_mc = mblock->bbox - mblock->offset;  // screen boundaries with features coordinates

    ////// Polygons 
    int in_screen = 0, in_map = 0,  points_total = 0;
    for( Polygon polygon : mblock->polygons){
        bool hit = false;
        for( Point16 p : polygon.points){
            points_total++;
            if( screen_bbox_mc.contains_point( p)) in_screen++;
            if( map_bbox_mc.contains_point( p)) in_map++;
        }
    }
    log_i("Polygons points.  in_screen: %i, in_map: %i,  total: %i", in_screen, in_map, points_total);
    
    ////// Lines 
    in_screen = 0;
    in_map = 0;
    points_total = 0;
    for( Polyline polyline : mblock->polylines){
        for( Point16 p : polyline.points){
            points_total++;
            if( screen_bbox_mc.contains_point( p)) in_screen++;
            if( map_bbox_mc.contains_point( p)) in_map++;
        }
    }
    log_i("Lines points. in_screen: %i,  in_map: %i,  total: %i", in_screen, in_map, points_total);
}