#include "geometry.h"
#include <GL/gl.h>
#include <math.h>

void cube(float w1, float h1, float d1)
{
    float w = w1 * 0.5;
    float d = d1 * 0.5;
    float h = h1 * 0.5;
    glBegin(GL_QUADS);
        // top
        glNormal3f( 0.0f, 1.0f, 0.0f);
        glVertex3f( w, h,-d); glVertex3f(-w, h,-d);
        glVertex3f(-w, h, d); glVertex3f( w, h, d);
        // bottom
        glNormal3f( 0.0f,-1.0f, 0.0f);
        glVertex3f( w,-h, d); glVertex3f(-w,-h, d);
        glVertex3f(-w,-h,-d); glVertex3f( w,-h,-d);
        // front
        glNormal3f( 0.0f, 0.0f, 1.0f);
        glVertex3f( w, h, d); glVertex3f(-w, h, d);
        glVertex3f(-w,-h, d); glVertex3f( w,-h, d);
        // back
        glNormal3f( 0.0f, 0.0f,-1.0f);
        glVertex3f( w,-h,-d); glVertex3f(-w,-h,-d);
        glVertex3f(-w, h,-d); glVertex3f( w, h,-d);
        // left
        glNormal3f(-1.0f, 0.0f, 0.0f);
        glVertex3f(-w, h, d); glVertex3f(-w, h,-d);
        glVertex3f(-w,-h,-d); glVertex3f(-w,-h, d);
        // right
        glNormal3f( 1.0f, 0.0f, 0.0f);
        glVertex3f( w, h,-d); glVertex3f( w, h, d);
        glVertex3f( w,-h, d); glVertex3f( w,-h,-d);
    glEnd();
}

void tube(int n, float rad, float len)
{
    const int MAX_POINTS = 100;
    static float pts[MAX_POINTS][3];
    static int firsttime = 1;
    if (firsttime) {
        firsttime = 0;
        double angle = 0.0;
        double inc = (3.1415926535 * 2.0) / (double)n;
        for (int i = 0; i < n; i++) {
            pts[i][0] = cos(angle) * rad;
            pts[i][2] = sin(angle) * rad;
            pts[i][1] = 0.0f;
            angle += inc;
        }
    }
    glBegin(GL_QUADS);
        for (int i = 0; i < n; i++) {
            int j = (i+1) % n;
            glNormal3f(pts[i][0], 0.0, pts[i][2]);
            glVertex3f(pts[i][0], 0.0, pts[i][2]);
            glVertex3f(pts[i][0], len, pts[i][2]);
            glNormal3f(pts[j][0], 0.0, pts[j][2]);
            glVertex3f(pts[j][0], len, pts[j][2]);
            glVertex3f(pts[j][0], 0.0, pts[j][2]);
        }
    glEnd();
}

// ===== SHARED CONSTANTS (IMPORTANT) =====
const int GRID = 12;
const float ROOM_SIZE = 120.0f;
const float HEIGHT = 10.0f;

float getCellSize() {
    return ROOM_SIZE / GRID;
}

// SAME pattern used EVERYWHERE (render + collision)
int getPattern(int x, int z) {
    return (x * 13 + z * 7) % 5;
}

// Convert grid → world position (CENTER of cell)
void getCellCenter(int x, int z, float &cx, float &cz) {
    float cell = getCellSize();
    cx = -ROOM_SIZE/2 + x * cell + cell/2;
    cz = -ROOM_SIZE/2 + z * cell + cell/2;
}

void drawHallway()
{
    float cell = getCellSize();

    // FLOOR
    for (int x = 0; x < GRID; x++) {
        for (int z = 0; z < GRID; z++) {
            bool even = (x + z) % 2 == 0;
            glColor3ub(even ? 90 : 120, even ? 90 : 120, even ? 70 : 90);

            float x0 = -ROOM_SIZE/2 + x * cell;
            float x1 = x0 + cell;
            float z0 = -ROOM_SIZE/2 + z * cell;
            float z1 = z0 + cell;

            glBegin(GL_QUADS);
                glNormal3f(0,1,0);
                glVertex3f(x0,0,z0);
                glVertex3f(x1,0,z0);
                glVertex3f(x1,0,z1);
                glVertex3f(x0,0,z1);
            glEnd();
        }
    }

    // CEILING
    for (int x = 0; x < GRID; x++) {
        for (int z = 0; z < GRID; z++) {
            glColor3ub(70,70,50);

            float x0 = -ROOM_SIZE/2 + x * cell;
            float x1 = x0 + cell;
            float z0 = -ROOM_SIZE/2 + z * cell;
            float z1 = z0 + cell;

            glBegin(GL_QUADS);
                glNormal3f(0,-1,0);
                glVertex3f(x0,HEIGHT,z0);
                glVertex3f(x1,HEIGHT,z0);
                glVertex3f(x1,HEIGHT,z1);
                glVertex3f(x0,HEIGHT,z1);
            glEnd();
        }
    }

    // WALLS (MAZE)
    for (int x = 0; x < GRID; x++) {
        for (int z = 0; z < GRID; z++) {

            int pattern = getPattern(x,z);

            if (pattern == 0 || pattern == 1) {
                float cx, cz;
                getCellCenter(x,z,cx,cz);

                glColor3ub(110,110,80);
                glPushMatrix();
                glTranslatef(cx, HEIGHT/2, cz);

                if (pattern == 0)
                    cube(cell * 0.2f, HEIGHT, cell);

                if (pattern == 1)
                    cube(cell, HEIGHT, cell * 0.2f);

                glPopMatrix();
            }
        }
    }

    // PILLARS (FIXED ALIGNMENT BUG HERE)
    for (int x = 1; x < GRID; x += 2) {
        for (int z = 1; z < GRID; z += 2) {

            float cx, cz;
            getCellCenter(x,z,cx,cz);

            glColor3ub(130,130,90);
            glPushMatrix();
            glTranslatef(cx, HEIGHT/2, cz); // ✅ FIXED (was misaligned before)
            cube(cell * 0.3f, HEIGHT, cell * 0.3f);
            glPopMatrix();
        }
    }

    // OUTER WALLS
    glColor3ub(100,100,70);

    glPushMatrix();
    glTranslatef(0, HEIGHT/2, -ROOM_SIZE/2);
    cube(ROOM_SIZE, HEIGHT, 1.0f);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(0, HEIGHT/2, ROOM_SIZE/2);
    cube(ROOM_SIZE, HEIGHT, 1.0f);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(-ROOM_SIZE/2, HEIGHT/2, 0);
    cube(1.0f, HEIGHT, ROOM_SIZE);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(ROOM_SIZE/2, HEIGHT/2, 0);
    cube(1.0f, HEIGHT, ROOM_SIZE);
    glPopMatrix();
}