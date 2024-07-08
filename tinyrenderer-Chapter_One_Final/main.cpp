#include <vector>
#include <cmath>
#include "tgaimage.h"
#include "model.h"
#include "geometry.h"
#include <limits>

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red   = TGAColor(255, 0,   0,   255);
const TGAColor green = TGAColor(0,   255, 0,   255);
Model *model = NULL;
const int width  = 800;
const int height = 800;

void draw_line_high(int x1, int y1, int x2, int y2, TGAColor color, TGAImage& image) {
    int deltaX = x2 - x1;
    int deltaY = y2 - y1;
    int step = 1;

    if (deltaX < 0) {
        step = -1;
        deltaX = -deltaX;
    }

    int decision = 2 * deltaX - deltaY;
    int x = x1;

    for (int y = y1; y <= y2; y++) {
        image.set(x, y, color);
        if (decision > 0) {
            x += step;
            decision += 2 * (deltaX - deltaY);
        } else {
            decision += 2 * deltaX;
        }
    }
}

void draw_line_low(int x1, int y1, int x2, int y2, TGAColor color, TGAImage& image) {
    int deltaX = x2 - x1;
    int deltaY = y2 - y1;
    int step = 1;

    if (deltaY < 0) {
        step = -1;
        deltaY = -deltaY;
    }

    int decision = 2 * deltaY - deltaX;
    int y = y1;

    for (int x = x1; x <= x2; x++) {
        image.set(x, y, color);
        if (decision > 0) {
            y += step;
            decision += 2 * (deltaY - deltaX);
        } else {
            decision += 2 * deltaY;
        }
    }
}

void triangle(Vec2i t0, Vec2i t1, Vec2i t2, TGAImage &image, TGAColor color) {
    if (t0.y==t1.y && t0.y==t2.y) return; // i dont care about degenerate triangles
    if (t0.y>t1.y) std::swap(t0, t1);
    if (t0.y>t2.y) std::swap(t0, t2);
    if (t1.y>t2.y) std::swap(t1, t2);
    int total_height = t2.y-t0.y;
    for (int i=0; i<total_height; i++) {
        bool second_half = i>t1.y-t0.y || t1.y==t0.y;
        int segment_height = second_half ? t2.y-t1.y : t1.y-t0.y;
        float alpha = (float)i/total_height;
        float beta  = (float)(i-(second_half ? t1.y-t0.y : 0))/segment_height; // be careful: with above conditions no division by zero here
        Vec2i A =               t0 + (t2-t0)*alpha;
        Vec2i B = second_half ? t1 + (t2-t1)*beta : t0 + (t1-t0)*beta;
        if (A.x>B.x) std::swap(A, B);
        for (int j=A.x; j<=B.x; j++) {
            image.set(j, t0.y+i, color); // attention, due to int casts t0.y+i != A.y
        }
    }
}

void line(int x1, int y1, int x2, int y2, TGAColor color, TGAImage& image) {
    if (std::abs(y2 - y1) < std::abs(x2 - x1)) {
        if (x1 > x2)
            draw_line_low(x2, y2, x1, y1, color, image);
        else
            draw_line_low(x1, y1, x2, y2, color, image);
    } else {
        if (y1 > y2)
            draw_line_high(x2, y2, x1, y1, color, image);
        else
            draw_line_high(x1, y1, x2, y2, color, image);
    }
}

int main(int argc, char** argv) {
  if (2==argc) {
        model = new Model(argv[1]);
    } else {
        model = new Model("obj/african_head.obj");
    }

    TGAImage image(width, height, TGAImage::RGB);
    Vec3f light_dir(0,0,-1);
    for (int i=0; i<model->nfaces(); i++) {
        std::vector<int> face = model->face(i);
        Vec2i screen_coords[3];
        Vec3f world_coords[3];
        for (int j=0; j<3; j++) {
            Vec3f v = model->vert(face[j]);
            screen_coords[j] = Vec2i((v.x+1.)*width/2., (v.y+1.)*height/2.);
            world_coords[j]  = v;
        }
        Vec3f n = (world_coords[2]-world_coords[0])^(world_coords[1]-world_coords[0]);
        n.normalize();
        float intensity = n*light_dir;
        if (intensity>0) {
            triangle(screen_coords[0], screen_coords[1], screen_coords[2], image, TGAColor(intensity*255, intensity*255, intensity*255, 255));
        }
    }

    image.flip_vertically(); // i want to have the origin at the left bottom corner of the image
    image.write_tga_file("output.tga");
    delete model;
    return 0;
}