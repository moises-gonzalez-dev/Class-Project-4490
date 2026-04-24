// program: UNCANNY VALLEY
// date: FALL 2026
// Framework for a 3D game.

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <GL/gl.h>
#include <GL/glx.h>
#include <GL/glu.h>
#include "defs.h"
#include "log.h"
#include "fonts.h"
#include <unistd.h>
#include <ctime>
#include "image.h"

typedef float Flt;
typedef Flt Vec[3];
typedef Flt Matrix[4][4];

void identity33(Matrix m);
void yy_transform(const Vec rotate, Matrix a);
void trans_vector(Matrix mat, const Vec in, Vec out);
//void screenShot();
void cube(float w1, float h1, float d1);
void tube(int n, float rad, float len);

#define VecMake(a,b,c,d) d[0]=a; d[1]=b; d[2]=c

Image scary_face("scary_face.png");

class Camera {
public:
  Vec position;
  Vec last_position;
  Vec direction;
	Vec force;
	Vec vel;
public:
    Camera() {
        VecMake(0, 1, 8, position);
        VecMake(0, 25, 22, position);
        VecMake(0, 0, -1, direction);
    }
    void translate(float x, float y, float z) {
        position[0] += x;
        position[1] += y;
        position[2] += z;
    }
    void lookLeftRight(float angle) {
        Matrix mat;
        identity33(mat);
        Vec rot = {0.0, angle, 0.0};
        yy_transform(rot, mat);
        trans_vector(mat, direction, direction);
    }
    void lookUpDown(float angle) {
		/*
        Matrix mat;
        identity33(mat);
        Vec rot = {angle, 0.0, 0.0};
        yy_transform(rot, mat);
        trans_vector(mat, direction, direction); 
		*/
		        
		void matrixFromAxisAngle(const Vec v, Flt ang, Matrix m);

        Vec left;
        Vec up = { 0, 1, 0 };
        VecCross(direction, up, left);
        //printf("LookUpDown: left: %f %f %f\n", left[0], left[1], left[2]);
 
        Matrix mat;
        identity33(mat);
        matrixFromAxisAngle(left, angle, mat);

        trans_vector(mat, direction, direction);
    }
    void moveLeftRight(float dist) {
        //get perpendicular vector to direction/up
        Vec left;
		Vec up = {0,1,0};
        VecCross(direction, up, left); 
        position[0] += left[0] * dist;
        position[1] += left[1] * dist;
        position[2] += left[2] * dist;
    }
    void moveUpDown(float dist) {
        //just move straight up or down
        position[1] += dist;
    }
};

//Class for a vector object.
class Myvec {
public:
	Flt x, y, z;
	Myvec(Flt a, Flt b, Flt c) {
		x = a;
		y = b;
		z = c;
	}
	void make(Flt a, Flt b, Flt c) {
		x = a;
		y = b;
		z = c;
	}
	void negate() {
		x = -x;
		y = -y;
		z = -z;
	}
	void zero() {
		x = y = z = 0.0;
	}
	Flt dot(Myvec v) {
		return (x*v.x + y*v.y + z*v.z);
	}
	Flt lenNoSqrt() {
		return (x*x + y*y + z*z);
	}
	Flt len() {
		return sqrtf(lenNoSqrt());
	}
	void copy(Myvec b) {
		b.x = x;
		b.y = y;
		b.z = z;
	}
	void add(Myvec b) {
		x = x + b.x;
		y = y + b.y;
		z = z + b.z;
	}
	void sub(Myvec b) {
		x = x - b.x;
		y = y - b.y;
		z = z - b.z;
	}
	void scale(Flt s) {
		x *= s;
		y *= s;
		z *= s;
	}
	void addS(Myvec b, Flt s) {
		x = x + b.x * s;
		y = y + b.y * s;
		z = z + b.z * s;
	}
};

class Enemy {
public:
    Vec pos;
    float yaw;
    bool eyesOn;
    int blinkTimer;

    Enemy() {
        VecMake(20.0f, 0.0f, -50.0f, pos);
        yaw = 0.0f;
        eyesOn = true;
        blinkTimer = 0;
    }

    void update(Camera &cam) {

      
        float dx = cam.position[0] - pos[0];
        float dz = cam.position[2] - pos[2];

        // face the player (note: atan2(y,x) order used to produce degrees)
        yaw = atan2f(dx, dz) * 180.0f / (float)M_PI;

        float dist = sqrtf(dx*dx + dz*dz);

        // follow but keep distance
        if (dist > 16.0f) {
            pos[0] += dx * 0.01f;
            pos[2] += dz * 0.01f;
        }

        blinkTimer++;

      if (blinkTimer > 300 + rand() % 300) {
          eyesOn = !eyesOn;
          blinkTimer = 0;
      }
    }

    void draw() {
        glPushMatrix();

        // position & orientation
        glTranslatef(pos[0], pos[1], pos[2]);
        glRotatef(yaw, 0.0f, 1.0f, 0.0f);

        // --- HORROR DISTORTION (controlled, readable) ---
        float t = (float)clock() * 0.0006f;  // MUCH slower

        // subtle breathing instead of wild scaling
        float stretchY = 1.0f + sinf(t) * 0.15f;
        float squashX  = 1.0f - sinf(t) * 0.08f;
        float squashZ  = 1.0f + cosf(t * 0.8f) * 0.05f;

        // very slight instability (barely noticeable but creepy)
        glRotatef(sinf(t * 1.2f) * 2.5f, 1, 0, 0);
        glRotatef(cosf(t * 1.0f) * 2.0f, 0, 0, 1);

        // apply scaling
        glScalef(squashX, stretchY, squashZ);

        // --- BODY (elongated, unnatural) ---
        glColor3ub(90, 90, 90);
        glPushMatrix();

        // slight sway offset so it doesn't feel rigid
        float sway = sinf(t * 3.0f) * 0.3f;

        glTranslatef(sway, 2.8f, 0.0f);

        // taller + thinner body
        cube(1.0f, 4.5f, 0.8f);

        glPopMatrix();


        // --- HEAD (too small + jittery = uncanny) ---
        glColor3ub(110, 110, 110);
        glPushMatrix();

        // jitter makes it feel alive
        float jitterX = sinf(t * 3.0f) * 0.03f;
        float jitterY = cosf(t * 2.5f) * 0.03f;

        glTranslatef(jitterX, 5.2f + jitterY, 0.0f);

        // slight independent rotation (VERY important)
        glRotatef(sinf(t * 5.0f) * 10.0f, 0, 1, 0);

        // smaller head = unsettling proportions
        cube(0.9f, 0.9f, 0.9f);

        glPopMatrix();

        // EYES
        glPushAttrib(GL_ENABLE_BIT | GL_CURRENT_BIT);
        glDisable(GL_LIGHTING);
        // ensure depth test remains enabled:
        glEnable(GL_DEPTH_TEST);
        if (eyesOn)
            glColor3ub(255,0,0);
        else
            glColor3ub(0,0,0);

        // left eye
        glPushMatrix();
        glTranslatef(-0.25f, 4.6f, 0.8f);
        GLUquadric *q = gluNewQuadric();
        gluSphere(q, 0.15, 12, 12);
        gluDeleteQuadric(q);
        glPopMatrix();

        // right eye
        glPushMatrix();
        glTranslatef(0.25f, 4.6f, 0.8f);
        q = gluNewQuadric();
        gluSphere(q, 0.15, 12, 12);
        gluDeleteQuadric(q);
        glPopMatrix();

        glPopAttrib();

        glPopMatrix();
    }
};

class Global {
public:
	int xres, yres;
	Flt aspectRatio;
	//Vec cameraPosition;
	Camera camera;
  Enemy enemy;
	GLfloat lightPosition[4];
	int state;
	int start_screen;
	int menu_screen;
	time_t timeStart;
	time_t timeCurrent;
	int fps;

  const float HALLWAY_WIDTH  = 12.0f;
  const float HALLWAY_HEIGHT = 10.0f;
  const float HALLWAY_LENGTH = 500.0f;
	Global() {
		//constructor
		xres=800;
		yres=600;
		aspectRatio = (GLfloat)xres / (GLfloat)yres;
		//VecMake(0.0, 1.0, 8.0, cameraPosition);
		//light is up high, right a little, toward a little
		VecMake(100.0f, 240.0f, 40.0f, lightPosition);
		lightPosition[3] = 1.0f;
		//init_opengl();
		state = 1;
		fps = 0;
		//state = 1 is player 
		//state = 0 is free move
		start_screen = 1;
		menu_screen = 0;
	}
	void init_opengl();
	void init();
	void check_mouse(XEvent *e);
	int check_keys(XEvent *e);
	void physics();
	void render();

  void clampPlayerToHallway(Camera &cam) {
        float margin = 0.5f; // half a unit so player doesn't poke into walls

        // X bounds (left/right walls)
        if (cam.position[0] < -HALLWAY_WIDTH + margin) cam.position[0] = -HALLWAY_WIDTH + margin;
        if (cam.position[0] >  HALLWAY_WIDTH - margin) cam.position[0] =  HALLWAY_WIDTH - margin;

        // Y bounds (floor/ceiling)
        if (cam.position[1] < 1.0f) cam.position[1] = 1.0f;           // eye height above floor
        if (cam.position[1] > HALLWAY_HEIGHT - 1.0f) cam.position[1] = HALLWAY_HEIGHT - 1.0f;

        // Z bounds (back wall / end of hallway)
        if (cam.position[2] < 0.1f) cam.position[2] = 0.1f;            // behind player wall
        if (cam.position[2] > HALLWAY_LENGTH - margin) cam.position[2] = HALLWAY_LENGTH - margin;
    }
} g;


void matrixFromAxisAngle(const Vec v, Flt ang, Matrix m)
{
    struct Axisangle {
        Flt angle;
        Flt x,y,z;
    } a1;
    a1.x = v[0];
    a1.y = v[1];
    a1.z = v[2];
    a1.angle = ang;
    //
    Flt c = cos(a1.angle);
    Flt s = sin(a1.angle);
    Flt t = 1.0 - c;
    m[0][0] = c + a1.x * a1.x * t;
    m[1][1] = c + a1.y * a1.y * t;
    m[2][2] = c + a1.z * a1.z * t;
    //
    Flt tmp1 = a1.x * a1.y * t;
    Flt tmp2 = a1.z * s;
    m[1][0] = tmp1 + tmp2;
    m[0][1] = tmp1 - tmp2;
    //
    tmp1 = a1.x * a1.z * t;
    tmp2 = a1.y * s;
    m[2][0] = tmp1 - tmp2;
    m[0][2] = tmp1 + tmp2;
    tmp1 = a1.y * a1.z * t;
    tmp2 = a1.x * s;
    m[2][1] = tmp1 + tmp2;
    m[1][2] = tmp1 - tmp2;
}

class X11_wrapper {
private:
	Display *dpy;
	Window win;
	GLXContext glc;
public:
	X11_wrapper() {
		Window root;
		GLint att[] = { GLX_RGBA,
						GLX_STENCIL_SIZE, 2,
						GLX_DEPTH_SIZE, 24,
						GLX_DOUBLEBUFFER, None };
		//GLint att[] = { GLX_RGBA, GLX_DEPTH_SIZE, 24, GLX_DOUBLEBUFFER, None };
		//GLint att[] = { GLX_RGBA, GLX_DEPTH_SIZE, 24, None };
		//XVisualInfo *vi;
		Colormap cmap;
		XSetWindowAttributes swa;
		setup_screen_res(640, 480);
		dpy = XOpenDisplay(NULL);
		if (dpy == NULL) {
			printf("\n\tcannot connect to X server\n\n");
			exit(EXIT_FAILURE);
		}
		root = DefaultRootWindow(dpy);
		XVisualInfo *vi = glXChooseVisual(dpy, 0, att);
		if (vi == NULL) {
			printf("\n\tno appropriate visual found\n\n");
			exit(EXIT_FAILURE);
		} 
		cmap = XCreateColormap(dpy, root, vi->visual, AllocNone);
		swa.colormap = cmap;
		swa.event_mask = ExposureMask | KeyPressMask | KeyReleaseMask |
						PointerMotionMask |
						MotionNotify | ButtonPressMask | ButtonReleaseMask |
						StructureNotifyMask | SubstructureNotifyMask;
		win = XCreateWindow(dpy, root, 0, 0, g.xres, g.yres, 0,
								vi->depth, InputOutput, vi->visual,
								CWColormap | CWEventMask, &swa);
		set_title();
		glc = glXCreateContext(dpy, vi, NULL, GL_TRUE);
		glXMakeCurrent(dpy, win, glc);
	}
	~X11_wrapper() {
		XDestroyWindow(dpy, win);
		XCloseDisplay(dpy);
	}
	void setup_screen_res(const int w, const int h) {
		g.xres = w;
		g.yres = h;
		g.aspectRatio = (GLfloat)g.xres / (GLfloat)g.yres;
	}
	void check_resize(XEvent *e) {
		//The ConfigureNotify is sent by the
		//server if the window is resized.
		if (e->type != ConfigureNotify)
			return;
		XConfigureEvent xce = e->xconfigure;
		if (xce.width != g.xres || xce.height != g.yres) {
			//Window size did change.
			reshape_window(xce.width, xce.height);
		}
	}
	void reshape_window(int width, int height) {
		//window has been resized.
		setup_screen_res(width, height);
		//
		glViewport(0, 0, (GLint)width, (GLint)height);
		glMatrixMode(GL_PROJECTION); glLoadIdentity();
		glMatrixMode(GL_MODELVIEW); glLoadIdentity();
		glOrtho(0, g.xres, 0, g.yres, -1, 1);
		set_title();
	}
	void set_title() {
		//Set the window title bar.
		XMapWindow(dpy, win);
		XStoreName(dpy, win, "UNCANNY VALLEY");
	}
	bool getXPending() {
		return XPending(dpy);
	}
	XEvent getXNextEvent() {
		XEvent e;
		XNextEvent(dpy, &e);
		return e;
	}
	void swapBuffers() {
		glXSwapBuffers(dpy, win);
	}
} x11;

int take_ss = 0;

int main()
{
	g.init_opengl();
	scary_face.upload();
  	srand(time(NULL));
	int done = 0;
	int nframes = 0;
	g.timeStart = time(nullptr);
	int starttime = time(NULL);

	while (!done) {
		while (x11.getXPending()) {
			XEvent e = x11.getXNextEvent();
			x11.check_resize(&e);
			g.check_mouse(&e);
			done = g.check_keys(&e);
		}
		g.physics();
		g.render();
		/*if (take_ss) {
			screenShot();
		}*/
		++nframes;
		int currtime = time(NULL);
		if (currtime > starttime) {
			starttime = currtime;
			g.fps = nframes;
			nframes = 0;
		}
		x11.swapBuffers();
	}
	//system("convert -loop 0 -coalesce - layers OptimizeFrame -delay 20 ./images/img*.jpg abc.gif");
	cleanup_fonts();
	return 0;
}

void Global::init() {
	//Place general program initializations here.
}

void Global::init_opengl()
{
    //OpenGL initialization
    glClearColor(0.0f, 0.1f, 0.3f, 0.0f);
    //Enable surface rendering priority using a Z-buffer.
    glClearDepth(1.0);
    glDepthFunc(GL_LESS);
    glEnable(GL_DEPTH_TEST);
    //Enable Phong shading of surfaces.
    glShadeModel(GL_SMOOTH);
    //Enable this so material colors are the same as vertex colors.
    glEnable(GL_COLOR_MATERIAL);
    // Make glColor affect ambient+diffuse material
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
    // Normalize normals after transforms (keeps lighting correct)
    glEnable(GL_NORMALIZE);
    //Enable diffuse lighting of surfaces with normals defined.
    glEnable(GL_LIGHTING);
    //Turn on a light (world light)
    glLightfv(GL_LIGHT0, GL_POSITION, g.lightPosition);
    glEnable(GL_LIGHT0);

    // Initialize LIGHT1 defaults (flashlight) — enabled/actualized per-frame in render()
    GLfloat l1ambient[]  = {0.0f, 0.0f, 0.0f, 1.0f};
    GLfloat l1diffuse[]  = {1.0f, 1.0f, 1.0f, 1.0f};
    GLfloat l1specular[] = {1.0f, 1.0f, 1.0f, 1.0f};
    glLightfv(GL_LIGHT1, GL_AMBIENT, l1ambient);
    glLightfv(GL_LIGHT1, GL_DIFFUSE, l1diffuse);
    glLightfv(GL_LIGHT1, GL_SPECULAR, l1specular);
    // Note: GL_LIGHT1 will be enabled each frame and its position/direction set in render()

    //Do this to allow fonts
    glEnable(GL_TEXTURE_2D);
    initialize_fonts();
}

Flt vecNormalize(Vec vec) {
	Flt len = vec[0]*vec[0] + vec[1]*vec[1] + vec[2]*vec[2];
	if (len == 0.0) {
		VecMake(0.0,0.0,1.0,vec);
		return 1.0;
	}
	len = sqrt(len);
	Flt tlen = 1.0 / len;
	vec[0] *= tlen;
	vec[1] *= tlen;
	vec[2] *= tlen;
	return len;
}

void Global::check_mouse(XEvent *e)
{
	//Did the mouse move?
	//Was a mouse button clicked?
	static int savex = 0;
	static int savey = 0;
	//
	if (e->type == ButtonRelease) {
		return;
	}
	if (e->type == ButtonPress) {
		if (e->xbutton.button==1) {
			//Left button is down
			g.camera.position[0] += g.camera.direction[0];
			g.camera.position[1] += g.camera.direction[1];
			g.camera.position[2] += g.camera.direction[2];
			if (g.state == 1) {
				g.camera.force[1] += 0.09;
			}
		}
		if (e->xbutton.button==3) {
			//Right button is down
			g.camera.position[0] -= g.camera.direction[0];
			g.camera.position[1] -= g.camera.direction[1];
			g.camera.position[2] -= g.camera.direction[2];
		}
	}

	if (savex != e->xbutton.x || savey != e->xbutton.y) {
		//Mouse moved
		if (e->xbutton.x  - savex < 0) {
			g.camera.lookLeftRight(0.01);
		} else {
			g.camera.lookLeftRight(-0.01);
		}
		if (e->xbutton.y  - savey < 0) {
			g.camera.lookUpDown(-0.01);
		} else {
			g.camera.lookUpDown(0.01);
		}
		savex = e->xbutton.x;
		savey = e->xbutton.y;
	}
}

int Global::check_keys(XEvent *e)
{
    //static int ctrl = 0;
    static bool keys[65536] = {};

    if (e->type == KeyPress) {
        int key = (XLookupKeysym(&e->xkey, 0) & 0x0000ffff);
        keys[key] = true;

        //if (key == XK_Control_L) { ctrl = 1; return 0; }
        
        // Single-press actions (non-movement)
        switch(key) {
            case XK_4:      
				g.start_screen = 0; 
				g.menu_screen = 1;
				break;
			case XK_5:
				g.menu_screen = 0;
				break;
            case XK_space:
                if (g.state == 1 && g.camera.position[1] <= 3.2)
                    g.camera.force[1] += 0.2;
                break;
            case XK_Escape: return 1;
        }
    }

    if (e->type == KeyRelease) {
        int key = (XLookupKeysym(&e->xkey, 0) & 0x0000ffff);
        keys[key] = false;
        //if (key == XK_Control_L) { ctrl = 0; return 0; }
    }

    // --- Movement: process all held keys together ---
    bool moved = false;

    // Forward/back
    if (keys[XK_w]) {
        g.camera.last_position[0] = g.camera.position[0];
        g.camera.last_position[1] = g.camera.position[1];
        g.camera.last_position[2] = g.camera.position[2];
        g.camera.position[0] += g.camera.direction[0];
        g.camera.position[1] += g.camera.direction[1];
        g.camera.position[2] += g.camera.direction[2];
        if (g.state == 1) g.camera.force[1] += 0.09;
        moved = true;
    }
    if (keys[XK_s]) {
        g.camera.last_position[0] = g.camera.position[0];
        g.camera.last_position[1] = g.camera.position[1];
        g.camera.last_position[2] = g.camera.position[2];
        g.camera.position[0] -= g.camera.direction[0];
        g.camera.position[1] -= g.camera.direction[1];
        g.camera.position[2] -= g.camera.direction[2];
        moved = true;
    }

    // Strafe
    if (keys[XK_d]) { g.camera.moveLeftRight(1.0);  moved = true; }
    if (keys[XK_a]) { g.camera.moveLeftRight(-1.0); moved = true; }

    // Vertical
    if (keys[XK_u])       g.camera.translate(0.0, 0.2, 0.0);
    if (keys[XK_Shift_L]) g.camera.position[1] -= 0.2;

    // Look
    if (keys[XK_Down])  g.camera.lookUpDown(0.1);
    if (keys[XK_Up])    g.camera.lookUpDown(-0.1);
    if (keys[XK_Left])  g.camera.lookLeftRight(0.1);
    if (keys[XK_Right]) g.camera.lookLeftRight(-0.1);

    if (moved) clampPlayerToHallway(g.camera);

    return 0;
}

void identity33(Matrix m)
{
	m[0][0] = m[1][1] = m[2][2] = 1.0f;
	m[0][1] = m[0][2] = m[1][0] = m[1][2] = m[2][0] = m[2][1] = 0.0f;
}

void yy_transform(const Vec rotate, Matrix a)
{
	//This function applies a rotation to a matrix.
	//It actually concatenates a transformation to the matrix.
	//Call this function first, then call trans_vector() to apply the
	//rotations to an object or vertex.
	//
	if (rotate[0] != 0.0f) {
		Flt ct = cos(rotate[0]), st = sin(rotate[0]);
		Flt t10 = ct*a[1][0] - st*a[2][0];
		Flt t11 = ct*a[1][1] - st*a[2][1];
		Flt t12 = ct*a[1][2] - st*a[2][2];
		Flt t20 = st*a[1][0] + ct*a[2][0];
		Flt t21 = st*a[1][1] + ct*a[2][1];
		Flt t22 = st*a[1][2] + ct*a[2][2];
		a[1][0] = t10;
		a[1][1] = t11;
		a[1][2] = t12;
		a[2][0] = t20;
		a[2][1] = t21;
		a[2][2] = t22;
		return;
	}
	if (rotate[1] != 0.0f) {
		Flt ct = cos(rotate[1]), st = sin(rotate[1]);
		Flt t00 = ct*a[0][0] - st*a[2][0];
		Flt t01 = ct*a[0][1] - st*a[2][1];
		Flt t02 = ct*a[0][2] - st*a[2][2];
		Flt t20 = st*a[0][0] + ct*a[2][0];
		Flt t21 = st*a[0][1] + ct*a[2][1];
		Flt t22 = st*a[0][2] + ct*a[2][2];
		a[0][0] = t00;
		a[0][1] = t01;
		a[0][2] = t02;
		a[2][0] = t20;
		a[2][1] = t21;
		a[2][2] = t22;
		return;
	}
	if (rotate[2] != 0.0f) {
		Flt ct = cos(rotate[2]), st = sin(rotate[2]);
		Flt t00 = ct*a[0][0] - st*a[1][0];
		Flt t01 = ct*a[0][1] - st*a[1][1];
		Flt t02 = ct*a[0][2] - st*a[1][2];
		Flt t10 = st*a[0][0] + ct*a[1][0];
		Flt t11 = st*a[0][1] + ct*a[1][1];
		Flt t12 = st*a[0][2] + ct*a[1][2];
		a[0][0] = t00;
		a[0][1] = t01;
		a[0][2] = t02;
		a[1][0] = t10;
		a[1][1] = t11;
		a[1][2] = t12;
		return;
	}
}

void trans_vector(Matrix mat, const Vec in, Vec out)
{
	Flt f0 = mat[0][0] * in[0] + mat[1][0] * in[1] + mat[2][0] * in[2];
	Flt f1 = mat[0][1] * in[0] + mat[1][1] * in[1] + mat[2][1] * in[2];
	Flt f2 = mat[0][2] * in[0] + mat[1][2] * in[1] + mat[2][2] * in[2];
	out[0] = f0;
	out[1] = f1;
	out[2] = f2;
}

void cube(float w1, float h1, float d1)
{
	float w = w1 * 0.5;
	float d = d1 * 0.5;
	float h = h1 * 0.5;
	//notice the normals being set
	glBegin(GL_QUADS);
		// top
		glNormal3f( 0.0f, 1.0f, 0.0f);
		glVertex3f( w, h,-d);
		glVertex3f(-w, h,-d);
		glVertex3f(-w, h, d);
		glVertex3f( w, h, d);
		// bottom
		glNormal3f( 0.0f, -1.0f, 0.0f);
		glVertex3f( w,-h, d);
		glVertex3f(-w,-h, d);
		glVertex3f(-w,-h,-d);
		glVertex3f( w,-h,-d);
		// front
		glNormal3f( 0.0f, 0.0f, 1.0f);
		glVertex3f( w, h, d);
		glVertex3f(-w, h, d);
		glVertex3f(-w,-h, d);
		glVertex3f( w,-h, d);
		// back
		glNormal3f( 0.0f, 0.0f, -1.0f);
		glVertex3f( w,-h,-d);
		glVertex3f(-w,-h,-d);
		glVertex3f(-w, h,-d);
		glVertex3f( w, h,-d);
		// left side
		glNormal3f(-1.0f, 0.0f, 0.0f);
		glVertex3f(-w, h, d);
		glVertex3f(-w, h,-d);
		glVertex3f(-w,-h,-d);
		glVertex3f(-w,-h, d);
		// right side
		glNormal3f( 1.0f, 0.0f, 0.0f);
		glVertex3f( w, h,-d);
		glVertex3f( w, h, d);
		glVertex3f( w,-h, d);
		glVertex3f( w,-h,-d);
	glEnd();
}

void tube(int n, float rad, float len)
{
	//Tube is centered at the origin.
	//Base of tube is at { 0, 0, 0 }.
	//Top of tube is at { 0, len, 0 }.
	//
	const int MAX_POINTS = 100;
	static float pts[MAX_POINTS][3];
	static int firsttime = 1;
	if (firsttime) {
		firsttime = 0;
		double angle = 0.0;
		double inc = (3.1415926535 * 2.0) / (double)n;
		for (int i=0; i<n; i++) {
			pts[i][0] = cos(angle) * rad;
			pts[i][2] = sin(angle) * rad;
			pts[i][1] = 0.0f;
			angle += inc;
		}
	}
	glBegin(GL_QUADS);
		for (int i=0; i<n; i++) {
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

void drawHallway()
{
    float width = 12.0f;
    float height = 10.0f;
    float length = 500.0f;

    int tilesX = 10; // horizontal subdivisions
    int tilesY = 10; // vertical subdivisions for walls
    int tilesZ = 50; // length subdivisions
    float tileW = (2 * width) / tilesX;
    float tileH = height / tilesY;
    float tileL = length / tilesZ;

    // --- FLOOR ---
    for (int i = 0; i < tilesX; i++) {
        for (int j = 0; j < tilesZ; j++) {
            bool even = (i + j) % 2 == 0;
            glColor3ub(even ? 60 : 100, even ? 60 : 100, even ? 60 : 100);

            float x0 = -width + i * tileW;
            float x1 = x0 + tileW;
            float z0 = j * tileL;
            float z1 = z0 + tileL;

            glBegin(GL_QUADS);
                glNormal3f(0,1,0);
                glVertex3f(x0, 0, z0);
                glVertex3f(x1, 0, z0);
                glVertex3f(x1, 0, z1);
                glVertex3f(x0, 0, z1);
            glEnd();
        }
    }

    // --- CEILING ---
    for (int i = 0; i < tilesX; i++) {
        for (int j = 0; j < tilesZ; j++) {
            bool even = (i + j) % 2 == 0;
            glColor3ub(even ? 40 : 80, even ? 40 : 80, even ? 40 : 80);

            float x0 = -width + i * tileW;
            float x1 = x0 + tileW;
            float z0 = j * tileL;
            float z1 = z0 + tileL;

            glBegin(GL_QUADS);
                glNormal3f(0,-1,0);
                glVertex3f(x0, height, z0);
                glVertex3f(x1, height, z0);
                glVertex3f(x1, height, z1);
                glVertex3f(x0, height, z1);
            glEnd();
        }
    }

    // --- LEFT WALL ---
    for (int j = 0; j < tilesZ; j++) {
        for (int i = 0; i < tilesY; i++) {
            bool even = (i + j) % 2 == 0;
            glColor3ub(even ? 60 : 100, even ? 60 : 100, even ? 60 : 100);

            float y0 = i * tileH;
            float y1 = y0 + tileH;
            float z0 = j * tileL;
            float z1 = z0 + tileL;

            glBegin(GL_QUADS);
                glNormal3f(1,0,0);
                glVertex3f(-width, y0, z0);
                glVertex3f(-width, y0, z1);
                glVertex3f(-width, y1, z1);
                glVertex3f(-width, y1, z0);
            glEnd();
        }
    }

    // --- RIGHT WALL ---
    for (int j = 0; j < tilesZ; j++) {
        for (int i = 0; i < tilesY; i++) {
            bool even = (i + j) % 2 == 0;
            glColor3ub(even ? 60 : 100, even ? 60 : 100, even ? 60 : 100);

            float y0 = i * tileH;
            float y1 = y0 + tileH;
            float z0 = j * tileL;
            float z1 = z0 + tileL;

            glBegin(GL_QUADS);
                glNormal3f(-1,0,0);
                glVertex3f(width, y0, z0);
                glVertex3f(width, y0, z1);
                glVertex3f(width, y1, z1);
                glVertex3f(width, y1, z0);
            glEnd();
        }
    }

    // --- BACK WALL (behind player, z=0) ---
    for (int i = 0; i < tilesX; i++) {
        for (int j = 0; j < tilesY; j++) {
            bool even = (i + j) % 2 == 0;
            glColor3ub(even ? 80 : 120, even ? 80 : 120, even ? 80 : 120);

            float x0 = -width + i * tileW;
            float x1 = x0 + tileW;
            float y0 = j * tileH;
            float y1 = y0 + tileH;

            glBegin(GL_QUADS);
                glNormal3f(0,0,1);
                glVertex3f(x0, y0, 0);
                glVertex3f(x1, y0, 0);
                glVertex3f(x1, y1, 0);
                glVertex3f(x0, y1, 0);
            glEnd();
        }
    }

    // --- FRONT WALL (far end, z=length) ---
    for (int i = 0; i < tilesX; i++) {
        for (int j = 0; j < tilesY; j++) {
            bool even = (i + j) % 2 == 0;
            glColor3ub(even ? 80 : 120, even ? 80 : 120, even ? 80 : 120);

            float x0 = -width + i * tileW;
            float x1 = x0 + tileW;
            float y0 = j * tileH;
            float y1 = y0 + tileH;

            glBegin(GL_QUADS);
                glNormal3f(0,0,-1);
                glVertex3f(x0, y0, length);
                glVertex3f(x1, y0, length);
                glVertex3f(x1, y1, length);
                glVertex3f(x0, y1, length);
            glEnd();
        }
    }
}

const Flt GRAVITY = -0.02f;

void Global::physics()
{
//	g.cameraPosition[2] -= 0.1;
//	g.cameraPosition[0] = 1.0 + sin(g.cameraPosition[2]*0.3);

	if (g.state == 1) {
		g.camera.vel[1] += GRAVITY;
		g.camera.vel[1] += g.camera.force[1];
		g.camera.force[1] = 0.0;
		g.camera.position[1] += g.camera.vel[1];
		if (g.camera.position[1] < 3.0) {
			g.camera.position[1] = 3.0;
			g.camera.vel[1] = 0.0;
		}
	}
  enemy.update(camera);
}

void Global::render()
{
    g.timeCurrent = time(nullptr);
    double elapsed = difftime(timeCurrent, timeStart);

    if (g.start_screen && (elapsed < 9.0)) {
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
        glViewport(0, 0, g.xres, g.yres);
        glMatrixMode(GL_PROJECTION); glLoadIdentity();
        gluOrtho2D(0, g.xres, 0, g.yres);
        glMatrixMode(GL_MODELVIEW); glLoadIdentity();
        glPushAttrib(GL_ENABLE_BIT);
        glDisable(GL_LIGHTING);

        if (elapsed > 5.0) {
            // goes from 0.0 to 1.0 over the last 4 seconds
            float t = (float)(elapsed - 5.0) / 4.0f;
            if (t > 1.0f) t = 1.0f;

            // scale from tiny (10px) to full size
            float maxW = 300.0f;
            float maxH = 300.0f;
            float w = 10.0f + (maxW - 10.0f) * t;
            float h = 10.0f + (maxH - 10.0f) * t;

            float cx = g.xres / 2.0f;
            float cy = g.yres / 2.0f;
            float x0 = cx - w * 0.5f;
            float x1 = cx + w * 0.5f;
            float y0 = cy - h * 0.5f;
            float y1 = cy + h * 0.5f;

            glEnable(GL_TEXTURE_2D);
            glBindTexture(GL_TEXTURE_2D, scary_face.texid); 
            glColor3f(1.0f, 1.0f, 1.0f);
            glBegin(GL_QUADS);
                glTexCoord2f(0, 1); glVertex2f(x0, y0);
				glTexCoord2f(1, 1); glVertex2f(x1, y0);
				glTexCoord2f(1, 0); glVertex2f(x1, y1);
				glTexCoord2f(0, 0); glVertex2f(x0, y1);
            glEnd();
            glDisable(GL_TEXTURE_2D);
        }

        // --- text ---
        Rect r;
        r.bot = g.yres - 50;
        r.left = g.xres / 2;
        r.center = 20;
        ggprint16(&r, 16, 0x00ffffff, "UNCANNY VALLEY");

        Rect r2;
		r2.bot = (int)(g.yres * 0.15f);  
		r2.left = g.xres / 2;
		r2.center = 20;
		ggprint12(&r2, 16, 0x00ffffff, "Press 4 to Skip");

        glPopAttrib();
		
	}else if (g.start_screen) {
		g.start_screen = 0;
		g.menu_screen = 1;
    } else if (g.menu_screen) {
		
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
		glViewport(0, 0, g.xres, g.yres);
		glMatrixMode(GL_PROJECTION); glLoadIdentity();
		gluOrtho2D(0, g.xres, 0, g.yres);
		glMatrixMode(GL_MODELVIEW); glLoadIdentity();
		glPushAttrib(GL_ENABLE_BIT);
		glDisable(GL_LIGHTING);

		int bw = 200, bh = 60;
		int cx = g.xres / 2;
		int cy = g.yres / 2;
		glDisable(GL_DEPTH_TEST);

		// draw box
		glColor4f(0.1f, 0.1f, 0.1f, 0.85f);
		glBegin(GL_QUADS);
			glVertex2f(cx - bw/2, cy - bh/2);
			glVertex2f(cx + bw/2, cy - bh/2);
			glVertex2f(cx + bw/2, cy + bh/2);
			glVertex2f(cx - bw/2, cy + bh/2);
		glEnd();

		// border
		glColor3f(1.0f, 1.0f, 1.0f);
		glLineWidth(2.0f);
		glBegin(GL_LINE_LOOP);
			glVertex2f(cx - bw/2, cy - bh/2);
			glVertex2f(cx + bw/2, cy - bh/2);
			glVertex2f(cx + bw/2, cy + bh/2);
			glVertex2f(cx - bw/2, cy + bh/2);
		glEnd();

		// all text
		Rect r;
		r.bot = g.yres - 50;
		r.left = g.xres / 2;
		r.center = 20;
		ggprint16(&r, 16, 0x00ffffff, "UNCANNY VALLEY");

		Rect r2;
		r2.bot = cy;
		r2.left = cx - 50;
		r2.center = 0;
		ggprint12(&r2, 16, 0x00ffffff, "Press 5 to Play");

		glEnable(GL_DEPTH_TEST);
		glPopAttrib();

	} else {
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		Rect r;

		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
		//3D mode
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		gluPerspective(45.0f, g.aspectRatio, 0.5f, 1000.0f);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		//for documentation...
		Vec up = { 0.0, 1.0, 0.0 };
		gluLookAt(
			g.camera.position[0], g.camera.position[1], g.camera.position[2],
			g.camera.position[0]+g.camera.direction[0],
			g.camera.position[1]+g.camera.direction[1],
			g.camera.position[2]+g.camera.direction[2],
			up[0], up[1], up[2]);
		glLightfv(GL_LIGHT0, GL_POSITION, g.lightPosition);

			// We set the light in eye coordinates so it always follows the camera exactly.
			// Push/pop so we don't disturb the modelview used for world drawing.
			glPushMatrix();
			glLoadIdentity(); // now coordinates are eye coords

			// place the flashlight at the eye origin
			GLfloat flashlightPos_eye[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
			// in eye coords the camera looks down -Z
			GLfloat flashlightDir_eye[3] = { 0.0f, 0.0f, -1.0f };

			// Apply to LIGHT1
			glLightfv(GL_LIGHT1, GL_POSITION, flashlightPos_eye);
			glLightfv(GL_LIGHT1, GL_SPOT_DIRECTION, flashlightDir_eye);
			glLightf(GL_LIGHT1, GL_SPOT_CUTOFF, 25.0f);      // cone angle
			glLightf(GL_LIGHT1, GL_SPOT_EXPONENT, 50.0f);    // concentration

			glLightf(GL_LIGHT1, GL_CONSTANT_ATTENUATION, 1.0f);
			glLightf(GL_LIGHT1, GL_LINEAR_ATTENUATION, 0.01f);
			glLightf(GL_LIGHT1, GL_QUADRATIC_ATTENUATION, 0.001f);

			// Make sure LIGHT1 is enabled
			glEnable(GL_LIGHT1);

			glPopMatrix();
		
		drawHallway();
		enemy.draw();
		
		//switch to 2D mode
		glViewport(0, 0, g.xres, g.yres);
		glMatrixMode(GL_MODELVIEW);   glLoadIdentity();
		glMatrixMode (GL_PROJECTION); glLoadIdentity();
		gluOrtho2D(0, g.xres, 0, g.yres);
		glPushAttrib(GL_ENABLE_BIT);
		glDisable(GL_LIGHTING);
		r.bot = g.yres - 20;
		r.left = 10;
		r.center = 0;
		ggprint8b(&r, 16, 0x00887766, "UNCANNY VALLEY");
		ggprint8b(&r, 16, 0x00ff00ff, "use WASD to MOVE");
		ggprint8b(&r, 16, 0x00ff00ff, "use SPACE to JUMP");
		ggprint8b(&r, 16, 0x00ff00ff, "use ARROW KEYS to LOOK AROUND");
		ggprint8b(&r, 16, 0x0000ff00, "FPS: %i", g.fps);
		glPopAttrib();
	}
}
