//============================================================================
//	Johns Hopkins University Engineering Programs for Professionals
//	605.467 Computer Graphics and 605.767 Applied Computer Graphics
//	Instructor:	David W. Nesbitt
//
//	Author:  Sam Du, Miles Gapcynski, and Chad Pournaras
//	File:    FinalProject.cpp
//	Purpose: OpenGL and GLUT program to draw a simple 3-D scene. It starts
//           with some simple object modeling and representation, adds camera
//           and projection controls, adds lighting and shading, then adds
//           texture mapping.
//
//============================================================================

#include <iostream>
#include <vector>

#include <GL/gl3w.h>
#include <GL/freeglut.h>
#include "geometry/geometry.h"
#include "shader_support/glsl_shader.h"
#include "scene/scene.h"

// NOTE - moved object geometry nodes to scene directory
#include "lighting_shader_node.h"

#include "TroughSurface.h"

// Root of the scene graph and scene state
SceneNode* SceneRoot;
SceneNode* tvNode;
SceneState MySceneState;

// Global camera node (so we can change view)
CameraNode* MyCamera;

// Lamp light
LightNode* lamplight1;

// Animated presentation node (global so we can toggle the tv power)
PresentationNode* Video;

// While mouse button is down, the view will be updated
bool  Animate = false;
bool  Forward = true;
float Velocity = 1.0f;
int   MouseX;
int   MouseY;
int   RenderWidth = 800;
int   RenderHeight = 600;
const float FrameRate = 72.0f;
const float VideoFrameRate = 21.0f;

int useRealistic = 0;
bool useOutline = 0;
int useBumpMap = 0;

PresentationNode* floorMaterial;
PresentationNode* wallMaterial;
PresentationNode* ceilMaterial;

PresentationNode* chairFabric;
PresentationNode* chairWood;

PresentationNode* couchFabric;
PresentationNode* couchWood;

PresentationNode* metal;
PresentationNode* shadeMaterial;

PresentationNode* rugMaterial;

// Simple logging function
void logmsg(const char *message, ...)
{
	// Open file if not already opened
	static FILE *lfile = NULL;
	if (lfile == NULL)
		lfile = fopen("FinalProject.log", "w");

	va_list arg;
	va_start(arg, message);
	vfprintf(lfile, message, arg);
	putc('\n', lfile);
	fflush(lfile);
	va_end(arg);
}

/**
 * Updates the view given the mouse position and whether to move
 * forward or backward.
 * @param  x        Window x position.
 * @param  y        Window y position.
 * @param  forward  Forward flag (true if moving forward).
 */
void UpdateView(const int x, const int y, bool forward) {
	// Find relative dx and dy relative to center of the window
	float dx = 4.0f * ((x - (static_cast<float>(RenderWidth * 0.5f))) / static_cast<float>(RenderWidth));
	float dy = 4.0f * (((static_cast<float>(RenderHeight * 0.5f) - y)) / static_cast<float>(RenderHeight));
	float dz = (forward) ? Velocity : -Velocity;
	MyCamera->MoveAndTurn(dx * Velocity, dy * Velocity, dz);
	glutPostRedisplay();
}

/**
 * Display callback. Clears the prior scene and draws a new one.
 */
void display() {
	// Clear the framebuffer and the depth buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Fill the stencil buffer for the TV
    glEnable(GL_STENCIL_TEST);
    glClearStencil(0);
    glClear(GL_STENCIL_BUFFER_BIT);

    glStencilFunc(GL_NEVER, 1, 1);
    glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);

    MySceneState.Init();
    tvNode->Draw(MySceneState);

    glStencilFunc(GL_EQUAL, 1, 1);
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

    // Draw the reflected scene
    glFrontFace(GL_CW);

    MySceneState.Init();

    MySceneState.model_matrix.Translate(0.0f, 200.0f, 0.0f);
    MySceneState.model_matrix.Scale(1.0f, -1.0f, 1.0f);

    HPoint3 lampLightPos = lamplight1->getPosition();
    lamplight1->SetPosition(MySceneState.model_matrix * lampLightPos);

    SceneRoot->Draw(MySceneState);

	// Draw the scene as normal
    glDisable(GL_STENCIL_TEST);
    glFrontFace(GL_CCW);

	MySceneState.Init();

    lamplight1->SetPosition(lampLightPos);

    tvNode->Draw(MySceneState);
	SceneRoot->Draw(MySceneState);

	// Swap buffers
	glutSwapBuffers();
}

/**
 * Use a timer method to try to do a consistent update rate or 72Hz.
 * Without using a timer, the speed of movement will depend on how fast
 * the program runs (fast movement on a fast PC and slow movement on a
 * slower PC)
 */
void timerFunction(int value) {
	// If mouse button is down, generate another view
	if (Animate) {
		UpdateView(MouseX, MouseY, Forward);
		glutTimerFunc((int)(1000.0f / FrameRate), timerFunction, 0);
	}
}

/**
 * Mouse callback (called when a mouse button state changes)
 */
void mouse(int button, int state, int x, int y) {
	// On a left button up event MoveAndTurn the view with forward motion
	if (button == GLUT_LEFT_BUTTON) {
		if (state == GLUT_DOWN) {
			MouseX = x;
			MouseY = y;
			Forward = true;
			Animate = true;
			UpdateView(x, y, true);

			// Set update
			glutTimerFunc((int)(1000.0f / FrameRate), timerFunction, 0);
		}
		else {
			Animate = false;  // Disable animation when the button is released
		}
	}

	// On a right button up event MoveAndTurn the view with reverse motion
	if (button == GLUT_RIGHT_BUTTON) {
		if (state == GLUT_DOWN) {
			MouseX = x;
			MouseY = y;
			Forward = false;
			Animate = true;
			UpdateView(x, y, true);

			// Set update
			glutTimerFunc((int)(1000.0f / FrameRate), timerFunction, 0);
		}
		else {
			Animate = false;  // Disable animation when the button is released
		}
	}
}

/**
 * Mouse motion callback (called when mouse button is depressed)
 */
void mouseMotion(int x, int y) {
	// Update position used for changing the view and force a new view
	MouseX = x;
	MouseY = y;
	UpdateView(x, y, true);
}

/**
* Callback to update the tv screen in the scene
*/
void screenTimer(int value)
{
	if (Video->GetPoweredOn())
	{
		Video->UpdateFrame();
		// Recursively call timer func 16 times per second for video frame rate
		glutTimerFunc(1000.0f / VideoFrameRate, screenTimer, 0);
		glutPostRedisplay();
	}
}

/**
 * Toggles textures and realistic vs non realistic shading
 */
void toggleShaderModes()
{
	useRealistic = (useRealistic == 0) ? 1 : 0;

	// Toggle textures and normals
	floorMaterial->useTextureAndNormal(useRealistic, useBumpMap);
	wallMaterial->useTextureAndNormal(useRealistic, useBumpMap);
	ceilMaterial->useTextureAndNormal(useRealistic, useBumpMap);

	chairFabric->useTextureAndNormal(useRealistic, useBumpMap);
	chairWood->useTextureAndNormal(useRealistic, useBumpMap);

	couchFabric->useTextureAndNormal(useRealistic, useBumpMap);
	couchWood->useTextureAndNormal(useRealistic, useBumpMap);

    shadeMaterial->useTextureAndNormal(useRealistic, useBumpMap);
    metal->useTextureAndNormal(useRealistic, useBumpMap);

    rugMaterial->useTextureAndNormal(useRealistic, useBumpMap);

	// Toggle realistic lighting
	glUniform1i(MySceneState.usereallighting_loc, useRealistic);
}
void toggleNormalMapModes()
{
		useBumpMap = (useBumpMap == 0) ? 1 : 0;

		// Toggle textures and normals
		floorMaterial->useTextureAndNormal(useRealistic, useBumpMap);
		wallMaterial->useTextureAndNormal(useRealistic, useBumpMap);
		ceilMaterial->useTextureAndNormal(useRealistic, useBumpMap);

		chairFabric->useTextureAndNormal(useRealistic, useBumpMap);
		chairWood->useTextureAndNormal(useRealistic, useBumpMap);

		couchFabric->useTextureAndNormal(useRealistic, useBumpMap);
		couchWood->useTextureAndNormal(useRealistic, useBumpMap);

		shadeMaterial->useTextureAndNormal(useRealistic, useBumpMap);
		metal->useTextureAndNormal(useRealistic, useBumpMap);

		rugMaterial->useTextureAndNormal(useRealistic, useBumpMap);

		// Toggle realistic lighting
		glUniform1i(MySceneState.usenormalmap_loc, useBumpMap);
}

/**
 * Toggle outlines for objects within the scene
 */
void toggleOutlines()
{
	useOutline = (useOutline == 0) ? 1 : 0;
	glUniform1i(MySceneState.outlineEnable_loc, useOutline);
}

/**
 * Keyboard callback.
 */
void keyboard(unsigned char key, int x, int y) {
	switch (key) {
	// Escape key
	case 27:
		exit(0);
		break;

	// Reset the view
	case 'i':
		MyCamera->SetPosition(Point3(0.0f, -100.0f, 60.0f));
		MyCamera->SetLookAtPt(Point3(0.0f, 0.0f, 35.0f));
		MyCamera->SetViewUp(Vector3(0.0, 0.0, 1.0));
		glutPostRedisplay();
		break;

	// Roll the camera by 5 degrees
	case 'r':
		MyCamera->Roll(5);
		glutPostRedisplay();
		break;

	// Roll the camera by 5 degrees (clockwise)
	case 'R':
		MyCamera->Roll(-5);
		glutPostRedisplay();
		break;

	// Change the pitch of the camera by 5 degrees
	case 'p':
		MyCamera->Pitch(5);
		glutPostRedisplay();
		break;

	// Change the pitch of the camera by 5 degrees (clockwise)
	case 'P':
		MyCamera->Pitch(-5);
		glutPostRedisplay();
		break;

	// Change the heading of the camera by 5 degrees
	case 'h':
		MyCamera->Heading(5);
		glutPostRedisplay();
		break;

	// Change the heading of the camera by 5 degrees (clockwise)
	case 'H':
		MyCamera->Heading(-5);
		glutPostRedisplay();
		break;

	// Go faster
	case 'V':
		Velocity += 0.2f;
		break;

	// Go slower
	case 'v':
		Velocity -= 0.2f;
		if (Velocity < 0.2f)
			Velocity = 0.1f;
		break;

	// Slide right
	case 'X':
		MyCamera->Slide(5.0f, 0.0f, 0.0f);
		glutPostRedisplay();
		break;

	// Slide left
	case 'x':
		MyCamera->Slide(-5.0f, 0.0f, 0.0f);
		glutPostRedisplay();
		break;

	// Slide up
	case 'Y':
		MyCamera->Slide(0.0f, 5.0f, 0.0f);
		glutPostRedisplay();
		break;

	// Slide down
	case 'y':
		MyCamera->Slide(0.0f, -5.0f, 0.0f);
		glutPostRedisplay();
		break;

	// Move forward
	case 'F':
		MyCamera->Slide(0.0f, 0.0f, -5.0f);
		glutPostRedisplay();
		break;

	// Move backward
	case 'f':
		MyCamera->Slide(0.0f, 0.0f, 5.0f);
		glutPostRedisplay();
		break;

	// Toggle tv on/off
	case '1':
		Video->TogglePower();
		if (Video->GetPoweredOn())
		{
			glutTimerFunc(1000.0f / VideoFrameRate, screenTimer, 0);
		}
		glutPostRedisplay();
		break;

	// Toggle shader modes
	case '2':
		toggleShaderModes();
		glutPostRedisplay();
		break;

	//toggle outlines
	case '3':
		toggleOutlines();
		glutPostRedisplay();
		break;
	case '4':
			toggleNormalMapModes();
			glutPostRedisplay();
	default:
		break;
	}
}

/**
 * Reshape callback. Update projection to reflect new aspect ratio.
 * @param  width  Window width
 * @param  height Window height
 */
void reshape(int width, int height) {
	RenderWidth = width;
	RenderHeight = height;

	// Reset the viewport
	glViewport(0, 0, width, height);

	// Reset the perspective projection to reflect the change of aspect ratio 
	// Make sure we cast to float so we get a fractional aspect ratio.
	MyCamera->ChangeAspectRatio(static_cast<float>(width) / static_cast<float>(height));
}

/**
 * Convenience method to add a material, then a transform, then a
 * geometry node as a child to a specified parent node.
 * @param  parent    Parent scene node.
 * @param  material  Presentation node.
 * @param  transform Transformation node.
 * @param  geometry  Geometry node.
 */
void AddSubTree(SceneNode* parent, SceneNode* material, SceneNode* transform, SceneNode* geometry) {
	parent->AddChild(material);
	material->AddChild(transform);
	transform->AddChild(geometry);
}

/**
 * Construct room as a child of the specified node
 * @param  parent       Parent node
 * @param  unit_square  Geometry node to use
 * @return Returns a scene node that describes the room.
 */
SceneNode* ConstructRoom(UnitSquareSurface* unit_square, TexturedUnitSquareSurface* textured_square) {
	// Contruct transform nodes for the walls. Perform rotations so the 
	// walls face inwards
	TransformNode* floor_transform = new TransformNode;
	floor_transform->Scale(200.0f, 200.0f, 1.0f);

	// Back wall is rotated +90 degrees about x: (y -> z)
	TransformNode* backwall_transform = new TransformNode;
	backwall_transform->Translate(0.0f, 100.0f, 40.0f);
	backwall_transform->RotateX(90.0f);
	backwall_transform->Scale(200.0f, 80.0f, 1.0f);

	// Front wall is rotated -90 degrees about x: (z -> y)
	TransformNode* frontwall_transform = new TransformNode;
	frontwall_transform->Translate(0.0f, -100.0f, 40.0f);
	frontwall_transform->RotateZ(180.0f);
	frontwall_transform->RotateX(90.0f);
	frontwall_transform->Scale(200.0f, 80.0f, 1.0f);

	// Left wall is rotated 90 degrees about y: (z -> x)
	TransformNode* leftwall_transform = new TransformNode;
	leftwall_transform->Translate(-100.0f, 0.0f, 40.0f);
	leftwall_transform->RotateZ(90.0f);
	leftwall_transform->RotateX(90.0f);
	leftwall_transform->Scale(200.0f, 80.0f, 1.0f);

	// Right wall is rotated -90 about y: (z -> -x)
	TransformNode* rightwall_transform = new TransformNode;
	rightwall_transform->Translate(100.0f, 0.0f, 40.0f);
	rightwall_transform->RotateZ(-90.0f);
	rightwall_transform->RotateX(90.0f);
	rightwall_transform->Scale(200.0f, 80.0f, 1.0f);

	// Ceiling is rotated 180 about x so it faces inwards
	TransformNode* ceiling_transform = new TransformNode;
	ceiling_transform->Translate(0.0f, 0.0f, 80.0f);
	ceiling_transform->RotateX(180.0f);
	ceiling_transform->Scale(200.0f, 200.0f, 1.0f);

	// Use a texture for the floor
	floorMaterial = new PresentationNode(Color4(0.15f, 0.15f, 0.15f),
		Color4(0.4f, 0.4f, 0.4f),
		Color4(0.2f, 0.2f, 0.2f),
		Color4(0.0f, 0.0f, 0.0f),
		5.0f);
	floorMaterial->SetTexture("wood-floor-texture.jpg", GL_REPEAT, GL_REPEAT, GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR);
	floorMaterial->setNormalMap("wood-floor-normal.jpg", GL_REPEAT, GL_REPEAT, GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR);
	floorMaterial->setTextureScale(4.0f);

	// Use a texture for the walls
	wallMaterial = new PresentationNode(Color4(0.35f, 0.225f, 0.275f),
		Color4(0.7f, 0.55f, 0.55f),
		Color4(0.4f, 0.4f, 0.4f),
		Color4(0.0f, 0.0f, 0.0f),
		16.0f);
	wallMaterial->SetTexture("masonry-wall-texture.jpg", GL_REPEAT, GL_REPEAT, GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR);
	wallMaterial->setNormalMap("masonry-wall-normal.jpg", GL_REPEAT, GL_REPEAT, GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR);
	wallMaterial->setTextureScale(4.0f);

	// Use a texture for the ceiling
	ceilMaterial = new PresentationNode(Color4(0.75f, 0.75f, 0.75f),
		Color4(1.0f, 1.0f, 1.0f),
		Color4(0.9f, 0.9f, 0.9f),
		Color4(0.0f, 0.0f, 0.0f),
		64.0);
	ceilMaterial->SetTexture("ceiling-texture.jpg", GL_REPEAT, GL_REPEAT, GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR);
	ceilMaterial->setNormalMap("ceiling-normal.jpg", GL_REPEAT, GL_REPEAT, GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR);
	ceilMaterial->setTextureScale(8.0f);

	// Walls. We can group these all under a single presentation node.
	SceneNode* room = new SceneNode;
	room->AddChild(wallMaterial);
	wallMaterial->AddChild(backwall_transform);
	backwall_transform->AddChild(textured_square);
	wallMaterial->AddChild(leftwall_transform);
	leftwall_transform->AddChild(textured_square);
	wallMaterial->AddChild(rightwall_transform);
	rightwall_transform->AddChild(textured_square);
	wallMaterial->AddChild(frontwall_transform);
	frontwall_transform->AddChild(textured_square);

	// Add floor and ceiling to the parent. Use convenience method to add
	// presentation, then transform, then geometry.
	AddSubTree(room, floorMaterial, floor_transform, textured_square);
	AddSubTree(room, ceilMaterial, ceiling_transform, textured_square);

	return room;
}

/**
* Construct a a unit box with outward facing normals.
* @param  unit_square  Geometry node to use
*/
SceneNode* ConstructUnitBox(TexturedUnitSquareSurface* textured_square) {
	// Contruct transform nodes for the sides of the box.
	// Perform rotations so the sides face outwards

	// Bottom is rotated 180 degrees so it faces outwards
	TransformNode* bottom_transform = new TransformNode;
	bottom_transform->Translate(0.0f, 0.0f, -0.5f);
	bottom_transform->RotateX(180.0f);

	// Back is rotated -90 degrees about x: (z -> y)
	TransformNode* back_transform = new TransformNode;
	back_transform->Translate(0.0f, 0.5f, 0.0f);
	back_transform->RotateX(-90.0f);

	// Front wall is rotated 90 degrees about x: (y -> z)
	TransformNode* front_transform = new TransformNode;
	front_transform->Translate(0.0f, -0.5f, 0.0f);
	front_transform->RotateX(90.0f);

	// Left wall is rotated -90 about y: (z -> -x)
	TransformNode* left_transform = new TransformNode;
	left_transform->Translate(-0.5f, 0.0f, 00.0f);
	left_transform->RotateY(-90.0f);

	// Right wall is rotated 90 degrees about y: (z -> x)
	TransformNode* right_transform = new TransformNode;
	right_transform->Translate(0.5f, 0.0f, 0.0f);
	right_transform->RotateY(90.0f);

	// Top 
	TransformNode* top_transform = new TransformNode;
	top_transform->Translate(0.0f, 0.0f, 0.50f);

	// Create a SceneNode and add the 6 sides of the box.
	SceneNode* box = new SceneNode;
	box->AddChild(back_transform);
	back_transform->AddChild(textured_square);
	box->AddChild(left_transform);
	left_transform->AddChild(textured_square);
	box->AddChild(right_transform);
	right_transform->AddChild(textured_square);
	box->AddChild(front_transform);
	front_transform->AddChild(textured_square);
	box->AddChild(bottom_transform);
	bottom_transform->AddChild(textured_square);
	box->AddChild(top_transform);
	top_transform->AddChild(textured_square);

	return box;
}

/**
 * Construct a TV
 * @param unit_square Geometry node to use
 */
SceneNode* ConstructTV(UnitSquareSurface* unit_square, TexturedUnitSquareSurface* textured_square) {
	SceneNode* box = ConstructUnitBox(textured_square);

	// Create bezels around the screen
	TransformNode* left = new TransformNode;
	left->Translate(-24.5f, -1.0f, 14.5f);
	left->Scale(1.0f, 2.0f, 29.0f);

	TransformNode* right = new TransformNode;
	right->Translate(24.5f, -1.0f, 14.5f);
	right->Scale(1.0f, 2.0f, 29.0f);

	TransformNode* top = new TransformNode;
	top->Translate(0.0f, -1.0f, 28.5f);
	top->Scale(48.0f, 2.0f, 1.0f);

	TransformNode* bottom = new TransformNode;
	bottom->Translate(0.0f, -1.0f, 0.5f);
	bottom->Scale(48.0f, 2.0f, 1.0f);

	TransformNode* screen = new TransformNode;
	screen->Translate(0.0f, -0.85f, 14.5f);
	screen->RotateX(90.0f);
	screen->Scale(48.0f, 27.0f, 0.0f);

    PresentationNode* plastic = new PresentationNode();
    plastic->SetMaterialAmbient(Color4(0.0f, 0.0f, 0.0f, 1.0f));
    plastic->SetMaterialDiffuse(Color4(0.2f, 0.2f, 0.2f, 1.0f));
    plastic->SetMaterialSpecular(Color4(0.5f, 0.5f, 0.5f, 1.0f));
    plastic->SetMaterialShininess(75.0f);

	Video = new PresentationNode();
    Video->SetMaterialAmbient(Color4(0.9f, 0.9f, 0.9f, 0.9f));
    Video->SetMaterialDiffuse(Color4(1.0f, 1.0f, 1.0f, 0.9f));
    Video->SetMaterialSpecular(Color4(0.4f, 0.4f, 0.4f, 0.9f));
    Video->SetMaterialEmission(Color4(1.0f, 1.0f, 1.0f, 0.75f));
    Video->SetMaterialShininess(15.0f);
	Video->SetAnimatedTexture("Video/futurama00",
		GL_CLAMP_TO_EDGE,
		GL_CLAMP_TO_EDGE,
		GL_LINEAR_MIPMAP_LINEAR,
		GL_LINEAR,
		336,
		".jpg");
	Video->useTextureAndNormal(true, true);
	glutTimerFunc(1000.0f / VideoFrameRate, screenTimer, 0);

	SceneNode* tv = new SceneNode;

    tv->AddChild(plastic);
    plastic->AddChild(left);
	left->AddChild(box);

    plastic->AddChild(right);
	right->AddChild(box);

    plastic->AddChild(top);
	top->AddChild(box);

    plastic->AddChild(bottom);
	bottom->AddChild(box);

	tv->AddChild(Video);
	Video->AddChild(screen);
	screen->AddChild(textured_square);

	return tv;
}

SceneNode* ConstructCouch(TexturedUnitSquareSurface* textured_square) {
	SceneNode* box = ConstructUnitBox(textured_square);

	TransformNode* base = new TransformNode;
	base->Translate(0.0f, 0.0f, 10.5f);
	base->Scale(45.0f, 15.0f, 12.0f);

	TransformNode* leg1 = new TransformNode;
	leg1->Translate(25.5f, 10.5f, 2.75f);
	leg1->Scale(3.0f, 3.0f, 4.5f);

	TransformNode* leg2 = new TransformNode;
	leg2->Translate(25.5f, -4.5f, 2.75f);
	leg2->Scale(3.0f, 3.0f, 4.5f);

	TransformNode* leg3 = new TransformNode;
	leg3->Translate(-25.5f, 10.5f, 2.75f);
	leg3->Scale(3.0f, 3.0f, 4.5f);

	TransformNode* leg4 = new TransformNode;
	leg4->Translate(-25.5f, -4.5f, 2.75f);
	leg4->Scale(3.0f, 3.0f, 4.5f);

	TransformNode* left = new TransformNode;
	left->Translate(25.5f, 0.0f, 13.5f);
	left->Scale(6.0f, 15.0f, 18.0f);

	TransformNode* right = new TransformNode;
	right->Translate(-25.5f, 0.0f, 13.5f);
	right->Scale(6.0f, 15.0f, 18.0f);

	TransformNode* back = new TransformNode;
	back->Translate(0.0f, 10.5f, 18.0f);
	back->Scale(57.0f, 6.0f, 27.0f);

    couchFabric = new PresentationNode();
    couchFabric->SetMaterialAmbient(Color4(0.1f, 0.0f, 0.2f));
    couchFabric->SetMaterialDiffuse(Color4(0.2f, 0.0f, 0.4f));
    couchFabric->SetMaterialSpecular(Color4(0.6f, 0.6f, 0.6f));
    couchFabric->SetMaterialShininess(3);
    couchFabric->SetTexture("fabric-texture.jpg", GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR);
    couchFabric->setNormalMap("fabric-normal.jpg", GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR);

	// Wood material for table
	couchWood = new PresentationNode(Color4(0.275f, 0.225f, 0.075f),
									 Color4(0.55f, 0.45f, 0.15f), 
									 Color4(0.3f, 0.3f, 0.3f), 
									 Color4(0.0f, 0.0f, 0.0f), 
									 64.0f);
    couchWood->SetTexture("grainy-wood-texture.jpg", GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR);

	SceneNode* couch = new SceneNode;
	// Add pieces of couch with fabri material
	couch->AddChild(couchFabric);
	couchFabric->AddChild(base);
	base->AddChild(box);
	couchFabric->AddChild(left);
	left->AddChild(box);
	couchFabric->AddChild(right);
	right->AddChild(box);
	couchFabric->AddChild(back);
	back->AddChild(box);

	// Add legs with wood material
	couch->AddChild(couchWood);
	couchWood->AddChild(leg1);
	leg1->AddChild(box);
	couchWood->AddChild(leg2);
	leg2->AddChild(box);
	couchWood->AddChild(leg3);
	leg3->AddChild(box);
	couchWood->AddChild(leg4);
	leg4->AddChild(box);

	return couch;
}

/**
 * Construct a chair.
 * @param unit_square Geometry node to use
 */
SceneNode* ConstructChair(TexturedUnitSquareSurface* textured_square) {
	SceneNode* box = ConstructUnitBox(textured_square);

	TransformNode* base = new TransformNode;
	base->Translate(0.0f, 0.0f, 10.5f);
	base->Scale(15.0f, 15.0f, 12.0f);

	TransformNode* leg1 = new TransformNode;
	leg1->Translate(10.5f, 10.5f, 2.75f);
	leg1->Scale(3.0f, 3.0f, 4.5f);

	TransformNode* leg2 = new TransformNode;
	leg2->Translate(10.5f, -4.5f, 2.75f);
	leg2->Scale(3.0f, 3.0f, 4.5f);

	TransformNode* leg3 = new TransformNode;
	leg3->Translate(-10.5f, 10.5f, 2.75f);
	leg3->Scale(3.0f, 3.0f, 4.5f);

	TransformNode* leg4 = new TransformNode;
	leg4->Translate(-10.5f, -4.5f, 2.75f);
	leg4->Scale(3.0f, 3.0f, 4.5f);

	TransformNode* left = new TransformNode;
	left->Translate(10.5f, 0.0f, 13.5f);
	left->Scale(6.0f, 15.0f, 18.0f);

	TransformNode* right = new TransformNode;
	right->Translate(-10.5f, 0.0f, 13.5f);
	right->Scale(6.0f, 15.0f, 18.0f);

	TransformNode* back = new TransformNode;
	back->Translate(0.0f, 10.5f, 18.0f);
	back->Scale(27.0f, 6.0f, 27.0f);

	chairFabric = new PresentationNode();
	chairFabric->SetMaterialAmbient(Color4(0.1f, 0.0f, 0.2f));
	chairFabric->SetMaterialDiffuse(Color4(0.2f, 0.0f, 0.4f));
	chairFabric->SetMaterialSpecular(Color4(0.6f, 0.6f, 0.6f));
	chairFabric->SetMaterialShininess(3);
	chairFabric->SetTexture("fabric-texture.jpg", GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR);
	chairFabric->setNormalMap("fabric-normal.jpg", GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR);

	// Wood material for table
	chairWood = new PresentationNode(Color4(0.275f, 0.225f, 0.075f),
									 Color4(0.55f, 0.45f, 0.15f),
									 Color4(0.3f, 0.3f, 0.3f),
									 Color4(0.0f, 0.0f, 0.0f),
									 64.0f);
	chairWood->SetTexture("grainy-wood-texture.jpg", GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR);

	SceneNode* chair = new SceneNode;
	// Add pieces of couch with fabri material
	chair->AddChild(chairFabric);
	chairFabric->AddChild(base);
	base->AddChild(box);
	chairFabric->AddChild(left);
	left->AddChild(box);
	chairFabric->AddChild(right);
	right->AddChild(box);
	chairFabric->AddChild(back);
	back->AddChild(box);

	// Add legs with wood material
	chair->AddChild(chairWood);
	chairWood->AddChild(leg1);
	leg1->AddChild(box);
	chairWood->AddChild(leg2);
	leg2->AddChild(box);
	chairWood->AddChild(leg3);
	leg3->AddChild(box);
	chairWood->AddChild(leg4);
	leg4->AddChild(box);

	return chair;
}

SceneNode* ConstructLamp(int position_loc, int normal_loc, int texture_loc, int tangent_loc, int bitangent_loc)
{
	TexturedConicSurface* base = new TexturedConicSurface(7.0f, 0.5f, 20, 4, position_loc, normal_loc, texture_loc, tangent_loc, bitangent_loc);
	TexturedConicSurface* post = new TexturedConicSurface(0.5f, 0.5f, 20, 20, position_loc, normal_loc, texture_loc, tangent_loc, bitangent_loc);
	TexturedSphereSection* cap = new TexturedSphereSection(0.0f, 360.0f, 36, 0.0f, 360.0f, 18, 0.5f, position_loc, normal_loc, texture_loc, tangent_loc, bitangent_loc);
	TexturedTroughSurface* shade = new TexturedTroughSurface(20, 20, position_loc, normal_loc, texture_loc, tangent_loc, bitangent_loc);

	TransformNode* baseTransform = new TransformNode;
	baseTransform->Translate(0.0f, 0.0f, 1.0f);
	baseTransform->Scale(1.0f, 1.0f, 2.0f);

	TransformNode* postTransform = new TransformNode;
	postTransform->Translate(0.0f, 0.0f, 21.0f);
	postTransform->Scale(1.0f, 1.0f, 38.0f);

	TransformNode* capTransform = new TransformNode;
	capTransform->Translate(0.0f, 0.0f, 40.0f);

	TransformNode* shadeTransform = new TransformNode;
	shadeTransform->Translate(0.0f, 0.0f, 39.5f);
	shadeTransform->Scale(5.0f, 5.0f, 20.0f);

	metal = new PresentationNode(Color4(0.15f, 0.15f, 0.2f), 
                                 Color4(0.3f, 0.3f, 0.4f), 
                                 Color4(0.2f, 0.2f, 0.2f), 
                                 Color4(0.0f, 0.0f, 0.0f), 
                                 15.0f);

	shadeMaterial = new PresentationNode(Color4(0.4f, 0.4f, 0.2f),
                                         Color4(0.8f, 0.8f, 0.4f), 
                                         Color4(0.3f, 0.3f, 0.3f), 
                                         Color4(0.2f, 0.2f, 0.2f), 
                                         5.0f);
    shadeMaterial->SetTexture("lampshade-texture.jpg", GL_REPEAT, GL_REPEAT, GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR);

	SceneNode* lamp = new SceneNode;
	lamp->AddChild(metal);
	metal->AddChild(baseTransform);
	baseTransform->AddChild(base);

	metal->AddChild(postTransform);
	postTransform->AddChild(post);

	metal->AddChild(capTransform);
	capTransform->AddChild(cap);

	lamp->AddChild(shadeMaterial);
	shadeMaterial->AddChild(shadeTransform);
	shadeTransform->AddChild(shade);

	return lamp;
}

/**
 * Construct lighting for this scene. Note that it is hard coded
 * into the shader node for this exercise.
 * @param  lighting  Pointer to the lighting shader node.
 */
LightNode* ConstructLighting(LightingShaderNode* lighting) {
	// Set the global light ambient
	Color4 globalAmbient(0.4f, 0.4f, 0.4f, 1.0f);
	lighting->SetGlobalAmbient(globalAmbient);

	// Light 0 - point light source in back right corner
	lamplight1 = new LightNode(0);
	lamplight1->SetDiffuse(Color4(0.5f, 0.5f, 0.5f, 1.0f));
	lamplight1->SetSpecular(Color4(0.5f, 0.5f, 0.5f, 1.0f));
	lamplight1->SetPosition(HPoint3(0.0f, -40.0f, 38.0f, 1.0f));
	lamplight1->Enable();

	// Light1 - directional light from the ceiling
	LightNode* light1 = new LightNode(2);
	light1->SetDiffuse(Color4(0.5f, 0.5f, 0.5f, 1.0f));
	light1->SetSpecular(Color4(0.5f, 0.5f, 0.5f, 1.0f));
	light1->SetPosition(HPoint3(0.0f, 0.0f, 1.0f, 0.0f));
	light1->Enable();

	// Lights are children of the camera node
	MyCamera->AddChild(lamplight1);
	lamplight1->AddChild(light1);

	return light1;
}

/**
 * Construct the scene
 */
void ConstructScene() {
	// Shader node
	LightingShaderNode* shader = new LightingShaderNode;
	if (!shader->Create("pixel_lighting.vert", "pixel_lighting.frag") || !shader->GetLocations())
	{
		exit(-1);
	}

	// Get the position, texture, and normal locations to use when constructing VAOs
	int position_loc = shader->GetPositionLoc();
	int normal_loc = shader->GetNormalLoc();
	int texture_loc = shader->GetTextureLoc();
	int tangent_loc = shader->getTangentLoc();
	int bitangent_loc = shader->getBitangentLoc();

	// Add the camera to the scene
	// Initialize the view and set a perspective projection
	MyCamera = new CameraNode();
	MyCamera->SetPosition(Point3(0.0f, -100.0f, 60.0f));
	MyCamera->SetLookAtPt(Point3(0.0f, 0.0f, 35.0f));
	MyCamera->SetViewUp(Vector3(0.0f, 0.0f, 1.0f));
	MyCamera->SetPerspective(50.0f, 1.0f, 1.0f, 1000.0f);

	// Construct scene lighting - make lighting nodes children of the camera node
	LightNode* light = ConstructLighting(shader);

	// Construct subdivided square - subdivided 10x in both x and y
	UnitSquareSurface* unit_square = new UnitSquareSurface(2, position_loc, normal_loc);

	// Construct a textured square for the floor
	TexturedUnitSquareSurface* textured_square = new TexturedUnitSquareSurface(2,
		position_loc,
		normal_loc,
		texture_loc,
		tangent_loc,
		bitangent_loc);

	// Construct the room as a child of the root node
	SceneNode* room = ConstructRoom(unit_square, textured_square);

	// Construct the chair with transform
	TransformNode* chairTransform = new TransformNode();
	chairTransform->Translate(20.0f, -15.0f, 0.0f);
	chairTransform->RotateZ(225.0f);
	SceneNode* chair = ConstructChair(textured_square);

	// Construct the couch with transform
	TransformNode* couchTransform = new TransformNode();
	couchTransform->Translate(-30.0f, -10.0f, 0.0f);
	couchTransform->RotateZ(135.0f);
	SceneNode* couch = ConstructCouch(textured_square);

	// Construct the tv with transform
	TransformNode* tvTransform = new TransformNode();
	tvTransform->Translate(0.0f, 99.0f, 45.0f);

	TexturedUnitSquareSurface* screen_square = new TexturedUnitSquareSurface(1,
		position_loc,
		normal_loc,
		texture_loc,
		tangent_loc,
		bitangent_loc);

	SceneNode* tv = ConstructTV(unit_square, screen_square);

	TransformNode* lampTransform = new TransformNode();
	lampTransform->Translate(0.0f, -40.0f, 0.1f);
	SceneNode* lamp = ConstructLamp(position_loc, normal_loc, texture_loc, tangent_loc, bitangent_loc);

	// Use a texture for the rug
	TransformNode* rugTransform = new TransformNode();
	rugTransform->Translate(0.0f, 20.0f, 1.0f);
	rugTransform->RotateZ(45.0f);
	rugTransform->Scale(60.0, 60.0, 1.0f);

	rugMaterial = new PresentationNode(Color4(0.4f, 0.4f, 0.4f),
		Color4(0.75f, 0.75f, 0.75f),
		Color4(0.2f, 0.2f, 0.2f),
		Color4(0.0f, 0.0f, 0.0f),
		5.0);
	rugMaterial->SetTexture("rug-texture.jpg", GL_REPEAT, GL_REPEAT, GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR);
	rugMaterial->setNormalMap("rug-normal.jpg", GL_REPEAT, GL_REPEAT, GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR);
	rugMaterial->setTextureScale(2.0f);

	// Construct the scene layout
	SceneRoot = new SceneNode;
	SceneRoot->AddChild(shader);
	shader->AddChild(MyCamera);

	// Construct a base node for the rest of the scene, it will be a child
	// of the last light node (so entire scene is under influence of all 
	// lights)
	SceneNode* myscene = new SceneNode;
	light->AddChild(myscene);

	// Add the room (walls, floor, ceiling)
	myscene->AddChild(room);

	// Add the chair, couch, tv, lamp, and rug
	myscene->AddChild(chairTransform);
	chairTransform->AddChild(chair);

	myscene->AddChild(couchTransform);
	couchTransform->AddChild(couch);

	myscene->AddChild(lampTransform);
	lampTransform->AddChild(lamp);

	myscene->AddChild(rugMaterial);
	rugMaterial->AddChild(rugTransform);
	rugTransform->AddChild(textured_square);

    tvNode = new SceneNode();
    tvNode->AddChild(tvTransform);
    tvTransform->AddChild(tv);
}

/**
 * Main
 */
int main(int argc, char** argv) {
	// Print the keyboard commands
	std::cout << "i - Reset to initial view" << std::endl;
	std::cout << "R - Roll    5 degrees clockwise   r - Counter-clockwise" << std::endl;
	std::cout << "P - Pitch   5 degrees clockwise   p - Counter-clockwise" << std::endl;
	std::cout << "H - Heading 5 degrees clockwise   h - Counter-clockwise" << std::endl;
	std::cout << "X - Slide camera right            x - Slide camera left" << std::endl;
	std::cout << "Y - Slide camera up               y - Slide camera down" << std::endl;
	std::cout << "F - Move camera forward           f - Move camera backwards" << std::endl;
	std::cout << "V - Faster mouse movement         v - Slower mouse movement" << std::endl;
	std::cout << "-----------------------------------------------------------" << std::endl;
	std::cout << "1 - Toggle TV Power" << std::endl;
	std::cout << "2 - Toggle textures and realistic vs non realistic shading" << std::endl;
	std::cout << "3 - Toggle outlines" << std::endl;
	std::cout << "4 - Toggle normal bump map" << std::endl;
	std::cout << "-----------------------------------------------------------" << std::endl;
	std::cout << "ESC - Exit Program" << std::endl;

	// Initialize free GLUT
	glutInit(&argc, argv);
	glutInitContextVersion(3, 2);
	glutInitContextProfile(GLUT_CORE_PROFILE);

	// Double buffer with depth buffer and MSAA
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH | GLUT_MULTISAMPLE | GLUT_STENCIL);
	glutInitWindowPosition(100, 100);
	glutInitWindowSize(800, 600);
	glutCreateWindow("Final Project by Sam Du, Miles Gapcynski, and Chad Pournaras");

	// Add GLUT callbacks
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutKeyboardFunc(keyboard);
	glutMouseFunc(mouse);
	glutMotionFunc(mouseMotion);

	// Initialize Open 3.2 core profile
	if (gl3wInit()) {
		fprintf(stderr, "gl3wInit: failed to initialize OpenGL\n");
		return -1;
	}
	if (!gl3wIsSupported(3, 2)) {
		fprintf(stderr, "OpenGL 3.2 not supported\n");
		return -1;
	}
	printf("OpenGL %s, GLSL %s\n", glGetString(GL_VERSION), glGetString(GL_SHADING_LANGUAGE_VERSION));

	// Set the clear color to black. Any part of the window outside the
	// viewport should appear black
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

	// Initialize DevIL
	ilInit();

	// Construct scene.
	ConstructScene();
	CheckError("After ConstructScene");

	// Enable multi-sample anti-aliasing
	glEnable(GL_MULTISAMPLE);

	// Enable depth testing
	glEnable(GL_DEPTH_TEST);

    // Turn on blending
    glEnable(GL_BLEND);

    // Combine what is to be rendered with what's already in the color buffers.
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

	// Enable back face polygon removal
	glFrontFace(GL_CCW);
	glCullFace(GL_BACK);
	glEnable(GL_CULL_FACE);

	glutMainLoop();
	return 0;
}