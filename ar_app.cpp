//This is a code Sample taken from my Playstation Vita AR game that I created back in 4th year.
//The object of the game was to guide a ball using the rear touch pad of the Vita into one of 4
//goals that can only be seen by moving the Vita into different positions around an AR marker so
//you could see which goal is open.

#include "ar_app.h"
#include <system/platform.h>
#include <graphics/sprite_renderer.h>
#include <graphics/texture.h>
#include <graphics/mesh.h>
#include <graphics/primitive.h>
#include <assets/png_loader.h>
#include <assets\obj_loader.h>
#include <graphics/image_data.h>
#include <graphics/font.h>
#include <input/touch_input_manager.h>
#include <maths/vector2.h>
#include <input/input_manager.h>
#include <maths/math_utils.h>
#include <graphics/renderer_3d.h>
#include <graphics/render_target.h>
#include <input/sony_controller_input_manager.h>
#include <sony_sample_framework.h>
#include <sony_tracking.h>

using namespace std;


ARApp::ARApp(gef::Platform& platform) :
	Application(platform),
	input_manager_(NULL),
	sprite_renderer_(NULL),
	font_(NULL),
	renderer_3d_(NULL)
{
}

void ARApp::Init()
{
	input_manager_ = platform_.CreateInputManager();
	sprite_renderer_ = platform_.CreateSpriteRenderer();
	renderer_3d_ = platform_.CreateRenderer3D();

	// Enable the back touch pad
	if (input_manager_ && input_manager_->touch_manager() && (input_manager_->touch_manager()->max_num_panels() > 0))
	{
		input_manager_->touch_manager()->EnablePanel(1);
	}
	active_touch_id_ = -1;

	InitFont();

	gef::OBJLoader obj_loader;

	//Set up marker ids
	marker_id[0] = 0;
	marker_id[1] = 1;

	//Set the game time for 60 seconds, the time for the goal to move to 10 and set the first goal position to be the top goal
	goalMoveTimer = 10;
	gameTimer = 60;
	goalsScored = 0;

	//Game setup
	og = UPGOAL;
	gs = SETTINGSCALE;
	markerNotFound = true;
	gameOver = false;
	scaleSet = false;

	// initialise sony framework
	sampleInitialize();
	smartInitialize();

	// reset marker tracking
	AppData* dat = sampleUpdateBegin();
	smartTrackingReset();
	sampleUpdateEnd(dat);

	CameraSetup();

	GoalSetUp();

	//arena setup
	arenaPlane_.gameobject_mesh_ = CreatePlaneMesh();
	arenaPlane_.set_mesh(arenaPlane_.gameobject_mesh_);

	//ball setup
	obj_loader.Load("ball1.obj", platform_, Ball_.gameobject_model_);
	Ball_.set_mesh(Ball_.gameobject_model_.mesh());

	SetupLights();
}

void ARApp::CleanUp()
{
	smartRelease();
	sampleRelease();

	CleanUpFont();
	delete sprite_renderer_;
	sprite_renderer_ = NULL;

	delete renderer_3d_;
	renderer_3d_ = NULL;

	delete input_manager_;
	input_manager_ = NULL;

	Ball_.gameobject_model_.Release();

	
}

bool ARApp::Update(float frame_time)
{
	fps_ = 1.0f / frame_time;

	//Handle input
	if (input_manager_)
	{
		input_manager_->Update();

		ProcessControllerInput();
		ProcessTouchInput();
	}

	AppData* dat = sampleUpdateBegin();

	// use the tracking library to try and find markers
	smartUpdate(dat->currentImage);

		switch (gs)
		{
		case SETTINGSCALE:	//user will choose a scale by moving the two markers

			SetScale();

			break;

		case PLAYING:	//game is being played

			Gameplay(frame_time);

			break;
		case GAMEOVER:	//game is over

			if (gameOver == false)
			{
				gs = SETTINGSCALE;
			}

			break;

		default:
			break;
		}

	sampleUpdateEnd(dat);

	return true;
}

void ARApp::Render()
{
	AppData* dat = sampleRenderBegin();

	//
	// Render the camera feed
	//
	gef::Matrix44 identity;
	identity.SetIdentity();

	sprite_renderer_->set_projection_matrix(camOrtho);
	renderer_3d_->set_view_matrix(identity);
	renderer_3d_->set_projection_matrix(finalProjMatrix);

	// REMEMBER AND SET THE PROJECTION MATRIX HERE

	sprite_renderer_->Begin(true);

	// DRAW CAMERA FEED SPRITE HERE

	if (dat->currentImage)
	{
		cameraTexture.set_texture(dat->currentImage->tex_yuv);
		sprite_renderer_->DrawSprite(cameraSprite);
	}

	sprite_renderer_->End();

	// Begin rendering 3D meshes, don't clear the frame buffer
	renderer_3d_->Begin(false);

	// DRAW 3D MESHES HERE
	if (markerNotFound == false && gameOver == false)
	{
		renderer_3d_->DrawMesh(arenaPlane_);
		renderer_3d_->DrawMesh(Ball_);
		renderer_3d_->DrawMesh(GoalPlane_);

		//render each wall of the goals
		for (int i = 0; i <= 3; i++)
		{
			for (int p = 0; p <= 2; p++)
			{
				renderer_3d_->DrawMesh(Goal_[i].goal_walls_[p]);
			}	
		}
	}

	renderer_3d_->End();

	RenderOverlay();

	sampleRenderEnd();
}

void ARApp::RenderOverlay()
{
	//
	// render 2d hud on top
	//
	gef::Matrix44 proj_matrix2d;

	proj_matrix2d = platform_.OrthographicFrustum(0.0f, platform_.width(), 0.0f, platform_.height(), -1.0f, 1.0f);
	sprite_renderer_->set_projection_matrix(proj_matrix2d);
	sprite_renderer_->Begin(false);
	DrawHUD();
	sprite_renderer_->End();
}

void ARApp::CameraSetup()
{
	//Camera set up
	camOrtho.SetIdentity();
	camOrtho.OrthographicFrustumGL(-1, 1, -1, 1, -1, 1);
	yScalingFactor = ((platform_.width() / platform_.height()) * (camWidth / camHeight));
	cameraSprite.set_position(gef::Vector4(0.0f, 0.0f, 1.0f, 0.0f));
	cameraSprite.set_height(yScalingFactor * 2.0f);
	cameraSprite.set_width(2.0f);
	cameraSprite.set_texture(&cameraTexture);

	projMatrix.PerspectiveFovGL(SCE_SMART_IMAGE_FOV, camWidth / camHeight, 0.01f, 10.0f);
	finalProjMatrix = projMatrix;
	gef::Matrix44 scaleMatrix;
	scaleMatrix.Scale(gef::Vector4(1, yScalingFactor, 1, 1));
	finalProjMatrix = projMatrix * scaleMatrix;
}

void ARApp::SetScale()
{
	if (sampleIsMarkerFound(marker_id[0]) && sampleIsMarkerFound(marker_id[1]))	//find markers 1 and 2
	{
		markerNotFound = false;

		sampleGetTransform(marker_id[0], &temp_marker_transform1);
		sampleGetTransform(marker_id[1], &temp_marker_transform2);
		transform1 = temp_marker_transform1.GetTranslation();
		transform2 = temp_marker_transform2.GetTranslation();
		dBetween = transform2 - transform1;

		//////////////////Set the scale for each object based on the distance between the two markers ////////////////////// 

		if (scaleSet == false)
		{
			arenaPlane_.Scale(abs(dBetween.x()), abs(dBetween.y()), markerScale);
			arenaPlane_.LocalMove(dBetween.x() / 2, dBetween.y() / 2, 0);
			arenaPlane_.BuildTransform(temp_marker_transform1);

			Ball_.Scale(ballScale, ballScale, ballScale);
			Ball_.LocalMove(Ball_.tempx + dBetween.x() / 2, Ball_.tempy + dBetween.y() / 2, 0.05f);
			Ball_.BuildTransform(temp_marker_transform1);

			GoalPlane_.Scale(goalScale * abs(dBetween.x()), goalScale * abs(dBetween.y()), goalScale);
			GoalPlane_.BuildTransform(temp_marker_transform1);

			Goal_[0].LocalMove(0.0f + dBetween.x() / 2, goalPosition * abs(dBetween.y()) + dBetween.y() / 2, 0.05f);
			Goal_[1].LocalMove(goalPosition * abs(dBetween.x()) + dBetween.x() / 2, 0.0f + dBetween.y() / 2, 0.05f);
			Goal_[2].LocalMove(0.0f + dBetween.x() / 2, -goalPosition * abs(dBetween.y()) + dBetween.y() / 2, 0.05f);
			Goal_[3].LocalMove(-goalPosition * abs(dBetween.x()) + dBetween.x() / 2, 0.0f + dBetween.y() / 2, 0.05f);

			for (int i = 0; i <= 3; i++)
			{
				Goal_[i].Scale(goalPlaneScale * abs(dBetween.x()), goalPlaneScale * abs(dBetween.y()), markerScale * 2.5);
				Goal_[i].BuildTransform(temp_marker_transform1);
			}
		}
		else
		{
			//once the scale has been set then move on to playing the game
			gs = PLAYING;
		}
	}
	else
	{
		markerNotFound = true;
	}
}

void ARApp::Gameplay(float frame_time)
{
	if (gameTimer > 0)
	{

		if (sampleIsMarkerFound(marker_id[0]))
		{
			////////////////Game set-up. will move the play area to the marker ////////////////////////
			markerNotFound = false;

			sampleGetTransform(marker_id[0], &temp_marker_transform1);

			arenaPlane_.LocalMove(0, 0, 0);
			arenaPlane_.BuildTransform(temp_marker_transform1);

			Ball_.LocalMove(Ball_.tempx, Ball_.tempy, 0.05f);
			Ball_.BuildTransform(temp_marker_transform1);
			Ball_.moveLeft = true;
			Ball_.moveRight = true;
			Ball_.moveUp = true;
			Ball_.moveDown = true;

			GoalPlane_.BuildTransform(temp_marker_transform1);

			Goal_[0].LocalMove(0.0f, goalPosition * abs(dBetween.y()), 0.05f);
			Goal_[1].LocalMove(goalPosition * abs(dBetween.x()), 0.0f, 0.05f);
			Goal_[2].LocalMove(0.0f, -goalPosition * abs(dBetween.y()), 0.05f);
			Goal_[3].LocalMove(-goalPosition * abs(dBetween.x()), 0.0f, 0.05f);

			for (int i = 0; i <= 3; i++)
			{
				Goal_[i].BuildTransform(temp_marker_transform1);
			}

			/////////////////////////////////Gameplay Starts Here//////////////////////////////////////

			//Game timer decreses only when the markers are found (game is being played)
			gameTimer -= frame_time;

			//Handles collisions
			Collisions();

			//slow the ball down
			gravity(frame_time);

			//moves the Goal around at set time intervals
			GoalMovement(frame_time, dBetween);

			//Move the ball based on the distance of the gesture on the back touch pad
			Ball_.TouchMove(touchDistance);

		}
		else
		{
			//If the markers are not found the the game will stop until the markers are found
			markerNotFound = true;
		}
	}
	else
	{
		//if game is over then change state to GAMEOVER
		gameOver = true;
		gs = GAMEOVER;
	}
}

void ARApp::InitFont()
{
	font_ = new gef::Font(platform_);
	font_->Load("comic_sans");
}

void ARApp::CleanUpFont()
{
	delete font_;
	font_ = NULL;
}

void ARApp::DrawHUD()
{
	if(font_)
	{
		//Display text depending what state the game is currently in
		switch (gs)
		{
		case ARApp::SETTINGSCALE:

			if (markerNotFound == true)
			{
				font_->RenderText(sprite_renderer_, gef::Vector4(425.0f, 255.0f, -0.9f), 1.5f, 0xffffffff, gef::TJ_CENTRE, "Please find markers 1 and 2");
			}

			font_->RenderText(sprite_renderer_, gef::Vector4(425.0f, 10.0f, -0.9f), 1.0f, 0xffffffff, gef::TJ_CENTRE, "Position the markers diagonally until you have the scale you want");
			font_->RenderText(sprite_renderer_, gef::Vector4(425.0f, 35.0f, -0.9f), 1.0f, 0xffffffff, gef::TJ_CENTRE, "then press X to accept");

			break;
		case ARApp::PLAYING:

			if (markerNotFound == true && gs == PLAYING)
			{
				font_->RenderText(sprite_renderer_, gef::Vector4(425.0f, 255.0f, -0.9f), 1.5f, 0xffffffff, gef::TJ_CENTRE, "Please find Marker 1");
			}

			font_->RenderText(sprite_renderer_, gef::Vector4(425.0f, 510.0f, -0.9f), 1.0f, 0xffffffff, gef::TJ_LEFT, "Goals Scored: %d", goalsScored);
			font_->RenderText(sprite_renderer_, gef::Vector4(175.0f, 10.0f, -0.9f), 1.0f, 0xffffffff, gef::TJ_CENTRE, "Press X to reset if Ball is stuck");

			break;
		case ARApp::GAMEOVER:

			if (gameOver == true)
			{
				font_->RenderText(sprite_renderer_, gef::Vector4(425.0f, 200.0f, -0.9f), 1.5f, 0xffffffff, gef::TJ_CENTRE, "Game Over : X to reset");
				font_->RenderText(sprite_renderer_, gef::Vector4(425.0f, 510.0f, -0.9f), 1.0f, 0xffffffff, gef::TJ_LEFT, "Goals Scored: %d", goalsScored);
			}

			break;

		default:
			break;
		}

		//font_->RenderText(sprite_renderer_, gef::Vector4(850.0f, 510.0f, -0.9f), 1.0f, 0xffffffff, gef::TJ_LEFT, "FPS: %.1f", fps_);

	}
}

void ARApp::ProcessControllerInput()
{
	//Handles all controller input and is called in update
	const gef::SonyControllerInputManager* controller_input = input_manager_->controller_input();

	if (controller_input)
	{
		const gef::SonyController* controller = controller_input->GetController(0);
		if (controller)
		{
			//The game will Handle button input differently depending on what state the game is in
			if (controller->buttons_pressed() & gef_SONY_CTRL_CROSS)
			{
				switch (gs)
				{
				case SETTINGSCALE:
					//Set the scale when X is pressed
					if (scaleSet == false)
					{
						scaleSet = true;
					}

					break;
				case PLAYING:

					//Reset Game
					gameTimer = 60;
					goalsScored = 0;
					og = UPGOAL;
					gs = SETTINGSCALE;
					goalMoveTimer = 10;
					Ball_.tempx = 0;
					Ball_.tempy = 0;
					Ball_.velocityX = 0;
					Ball_.velocityY = 0;
					gameOver = false;
					scaleSet = false;

					break;
				case GAMEOVER:

					//Restart Game
					gameTimer = 60;
					goalsScored = 0;
					og = UPGOAL;
					gs = SETTINGSCALE;
					goalMoveTimer = 10;
					Ball_.tempx = 0;
					Ball_.tempy = 0;
					Ball_.velocityX = 0;
					Ball_.velocityY = 0;
					gameOver = false;
					scaleSet = false;

					break;

				default:
					break;
				}
			}
		}
	}
}

void ARApp::ProcessTouchInput()
{
	//width 0-959
	//height 54-445 / 391
	const gef::TouchInputManager* touch_input = input_manager_->touch_manager();
	if (touch_input && (touch_input->max_num_panels() > 0))
	{
		// get the active touches for this panel
		const gef::TouchContainer& panel_touches = touch_input->touches(1);

		// go through the touches
		for (gef::ConstTouchIterator touch = panel_touches.begin(); touch != panel_touches.end(); touch++)
		{
			// if active touch id is -1, then we are currently processing a touch
			if (active_touch_id_ == -1)
			{
				// check for the start of a new touch
				if (touch->type == gef::TT_NEW)
				{
					active_touch_id_ = touch->id;

					// do any processing for a new touch here
					// we're just going to record the position of the touch
					touch_position_ = touch->position;
					//get start position
					touch_start_ = touch->position;
				}

			}

			else if (active_touch_id_ == touch->id)
			{
				//we are processing touch data with a matching id to the one we are looking for
				if (touch->type == gef::TT_ACTIVE)
				{
					// update an active touch here
					// we're just going to record the position of the touch
					touch_position_ = touch->position;
					touch_end_ = touch->position;
					VectorCalculations();
				}
				else if (touch->type == gef::TT_RELEASED)
				{
					// the touch we are tracking has been released
					// perform any actions that need to happen when a touch is released here
					// we're not doing anything here apart from resetting the active touch id
					active_touch_id_ = -1;

				}
			}
		}
	}
}

gef::Mesh* ARApp::CreateCubeMesh()
{
	gef::Mesh* mesh = platform_.CreateMesh();

	// initialise the vertex data to create a 1, 1, 1 cube
	const float half_size = 0.5f;
	const gef::Mesh::Vertex vertices[] = {
		{ half_size, -half_size, -half_size,  0.577f, -0.577f, -0.577f, 0.0f, 0.0f },
		{ half_size,  half_size, -half_size,  0.577f,  0.577f, -0.577f, 0.0f, 0.0f },
		{ -half_size,  half_size, -half_size, -0.577f,  0.577f, -0.577f, 0.0f, 0.0f },
		{ -half_size, -half_size, -half_size, -0.577f, -0.577f, -0.577f, 0.0f, 0.0f },
		{ half_size, -half_size,  half_size,  0.577f, -0.577f,  0.577f, 0.0f, 0.0f },
		{ half_size,  half_size,  half_size,  0.577f,  0.577f,  0.577f, 0.0f, 0.0f },
		{ -half_size,  half_size,  half_size, -0.577f,  0.577f,  0.577f, 0.0f, 0.0f },
		{ -half_size, -half_size,  half_size, -0.577f, -0.577f,  0.577f, 0.0f, 0.0f }
	};

	mesh->InitVertexBuffer(platform_, static_cast<const void*>(vertices), sizeof(vertices) / sizeof(gef::Mesh::Vertex), sizeof(gef::Mesh::Vertex));

	// we will create a single triangle list primitive to draw the triangles that make up the cube
	mesh->AllocatePrimitives(1);
	gef::Primitive* primitive = mesh->GetPrimitive(0);

	const UInt32 indices[] = {
		// Back
		0, 1, 2,
		2, 3, 0,
		// Front
		6, 5, 4,
		4, 7, 6,
		// Left
		2, 7, 3,
		2, 6, 7,
		// Right
		0, 4, 1,
		5, 1, 4,
		// Top
		6, 2, 1,
		5, 6, 1,
		// Bottom
		0, 3, 7,
		0, 7, 4
	};

	primitive->InitIndexBuffer(platform_, static_cast<const void*>(indices), sizeof(indices) / sizeof(UInt32), sizeof(UInt32));
	primitive->set_type(gef::TRIANGLE_LIST);

	// set size of bounds, we need this for collision detection routines
	gef::Aabb aabb(gef::Vector4(-half_size, -half_size, -half_size), gef::Vector4(half_size, half_size, half_size));
	gef::Sphere sphere(aabb);

	mesh->set_aabb(aabb);
	mesh->set_bounding_sphere(sphere);

	return mesh;
}

gef::Mesh* ARApp::CreatePlaneMesh()
{
	gef::Mesh* mesh = platform_.CreateMesh();

	const float half_size = 0.5f;

	const gef::Mesh::Vertex vertices[] = {
		{ half_size, -half_size, 0.0f,  0.577f, -0.577f, -0.577f, 0.0f, 0.0f },//0 - 0
		{ half_size,  half_size, 0.0f,  0.577f,  0.577f, -0.577f, 0.0f, 0.0f },//1 - 1
		{ -half_size,  half_size, 0.0f, -0.577f,  0.577f, -0.577f, 0.0f, 0.0f },//2 - 2
		{ -half_size, -half_size, 0.0f, -0.577f, -0.577f, -0.577f, 0.0f, 0.0f }//3 - 3
	};

	mesh->InitVertexBuffer(platform_, static_cast<const void*>(vertices), sizeof(vertices) / sizeof(gef::Mesh::Vertex), sizeof(gef::Mesh::Vertex));

	mesh->AllocatePrimitives(1);
	gef::Primitive* primitive = mesh->GetPrimitive(0);

	const UInt32 indices[] = {
		// Back
		0, 1, 2,
		2, 3, 0
	};

	primitive->InitIndexBuffer(platform_, static_cast<const void*>(indices), sizeof(indices) / sizeof(UInt32), sizeof(UInt32));
	primitive->set_type(gef::TRIANGLE_LIST);

	// set size of bounds, we need this for collision detection routines
	gef::Aabb aabb(gef::Vector4(-half_size, -half_size, -half_size), gef::Vector4(half_size, half_size, half_size));
	gef::Sphere sphere(aabb);

	mesh->set_aabb(aabb);
	mesh->set_bounding_sphere(sphere);

	return mesh;
}

gef::Mesh* ARApp::CreateGoalBlock1Mesh()
{
	//Left plane
	gef::Mesh* mesh = platform_.CreateMesh();

	const float half_size = 0.5f;

	const gef::Mesh::Vertex vertices[] = {
		{ half_size, -half_size, -half_size,  0.577f, -0.577f, -0.577f, 0.0f, 0.0f },
		{ half_size,  half_size, -half_size,  0.577f,  0.577f, -0.577f, 0.0f, 0.0f },
		{ -half_size,  half_size, -half_size, -0.577f,  0.577f, -0.577f, 0.0f, 0.0f },
		{ -half_size, -half_size, -half_size, -0.577f, -0.577f, -0.577f, 0.0f, 0.0f },
		{ half_size, -half_size,  half_size,  0.577f, -0.577f,  0.577f, 0.0f, 0.0f },
		{ half_size,  half_size,  half_size,  0.577f,  0.577f,  0.577f, 0.0f, 0.0f },
		{ -half_size,  half_size,  half_size, -0.577f,  0.577f,  0.577f, 0.0f, 0.0f },
		{ -half_size, -half_size,  half_size, -0.577f, -0.577f,  0.577f, 0.0f, 0.0f }
	};

	mesh->InitVertexBuffer(platform_, static_cast<const void*>(vertices), sizeof(vertices) / sizeof(gef::Mesh::Vertex), sizeof(gef::Mesh::Vertex));

	mesh->AllocatePrimitives(1);
	gef::Primitive* primitive = mesh->GetPrimitive(0);

	const UInt32 indices[] = {
		// Left
		2, 7, 3,
		2, 6, 7
	};

	primitive->InitIndexBuffer(platform_, static_cast<const void*>(indices), sizeof(indices) / sizeof(UInt32), sizeof(UInt32));
	primitive->set_type(gef::TRIANGLE_LIST);

	// set size of bounds, we need this for collision detection routines
	gef::Aabb aabb1(gef::Vector4(-half_size - 0.01, -half_size, -half_size), gef::Vector4(-half_size, half_size, half_size));
	gef::Sphere sphere(aabb1);

	mesh->set_aabb(aabb1);
	mesh->set_bounding_sphere(sphere);

	return mesh;
}

gef::Mesh* ARApp::CreateGoalBlock2Mesh()
{
	//middle plane
	gef::Mesh* mesh = platform_.CreateMesh();

	const float half_size = 0.5f;

	const gef::Mesh::Vertex vertices[] = {
		{ half_size, -half_size, -half_size,  0.577f, -0.577f, -0.577f, 0.0f, 0.0f },
		{ half_size,  half_size, -half_size,  0.577f,  0.577f, -0.577f, 0.0f, 0.0f },
		{ -half_size,  half_size, -half_size, -0.577f,  0.577f, -0.577f, 0.0f, 0.0f },
		{ -half_size, -half_size, -half_size, -0.577f, -0.577f, -0.577f, 0.0f, 0.0f },
		{ half_size, -half_size,  half_size,  0.577f, -0.577f,  0.577f, 0.0f, 0.0f },
		{ half_size,  half_size,  half_size,  0.577f,  0.577f,  0.577f, 0.0f, 0.0f },
		{ -half_size,  half_size,  half_size, -0.577f,  0.577f,  0.577f, 0.0f, 0.0f },
		{ -half_size, -half_size,  half_size, -0.577f, -0.577f,  0.577f, 0.0f, 0.0f }
	};

	mesh->InitVertexBuffer(platform_, static_cast<const void*>(vertices), sizeof(vertices) / sizeof(gef::Mesh::Vertex), sizeof(gef::Mesh::Vertex));

	mesh->AllocatePrimitives(1);
	gef::Primitive* primitive = mesh->GetPrimitive(0);

	const UInt32 indices[] = {
		// Bottom
		0, 3, 7,
		0, 7, 4
	};

	primitive->InitIndexBuffer(platform_, static_cast<const void*>(indices), sizeof(indices) / sizeof(UInt32), sizeof(UInt32));
	primitive->set_type(gef::TRIANGLE_LIST);

	// set size of bounds, we need this for collision detection routines
	gef::Aabb aabb2(gef::Vector4(-half_size, -half_size - 0.01, -half_size), gef::Vector4(half_size, -half_size, half_size));

	gef::Sphere sphere(aabb2);

	mesh->set_aabb(aabb2);

	mesh->set_bounding_sphere(sphere);

	return mesh;
}

gef::Mesh* ARApp::CreateGoalBlock3Mesh()
{
	//right plane
	gef::Mesh* mesh = platform_.CreateMesh();

	const float half_size = 0.5f;

	const gef::Mesh::Vertex vertices[] = {
		{ half_size, -half_size, -half_size,  0.577f, -0.577f, -0.577f, 0.0f, 0.0f },
		{ half_size,  half_size, -half_size,  0.577f,  0.577f, -0.577f, 0.0f, 0.0f },
		{ -half_size,  half_size, -half_size, -0.577f,  0.577f, -0.577f, 0.0f, 0.0f },
		{ -half_size, -half_size, -half_size, -0.577f, -0.577f, -0.577f, 0.0f, 0.0f },
		{ half_size, -half_size,  half_size,  0.577f, -0.577f,  0.577f, 0.0f, 0.0f },
		{ half_size,  half_size,  half_size,  0.577f,  0.577f,  0.577f, 0.0f, 0.0f },
		{ -half_size,  half_size,  half_size, -0.577f,  0.577f,  0.577f, 0.0f, 0.0f },
		{ -half_size, -half_size,  half_size, -0.577f, -0.577f,  0.577f, 0.0f, 0.0f }
	};

	mesh->InitVertexBuffer(platform_, static_cast<const void*>(vertices), sizeof(vertices) / sizeof(gef::Mesh::Vertex), sizeof(gef::Mesh::Vertex));

	mesh->AllocatePrimitives(1);
	gef::Primitive* primitive = mesh->GetPrimitive(0);

	const UInt32 indices[] = {
		// Right
		0, 4, 1,
		5, 1, 4
	};

	primitive->InitIndexBuffer(platform_, static_cast<const void*>(indices), sizeof(indices) / sizeof(UInt32), sizeof(UInt32));
	primitive->set_type(gef::TRIANGLE_LIST);

	// set size of bounds, we need this for collision detection routines

	gef::Aabb aabb3(gef::Vector4(half_size + 0.01, -half_size, -half_size), gef::Vector4(half_size, half_size, half_size));
	gef::Sphere sphere(aabb3);

	mesh->set_aabb(aabb3);

	mesh->set_bounding_sphere(sphere);

	return mesh;
}

void ARApp::SetupLights()
{
	gef::PointLight default_point_light;
	default_point_light.set_colour(gef::Colour(0.7f, 0.7f, 1.0f, 1.0f));
	default_point_light.set_position(gef::Vector4(0.0f, 100.0f, -500.0f));

	gef::Default3DShaderData& default_shader_data = renderer_3d_->default_shader_data();
	default_shader_data.set_ambient_light_colour(gef::Colour(0.5f, 0.5f, 0.5f, 1.0f));
	default_shader_data.AddPointLight(default_point_light);

	//default_point_light.set_position(gef::Vector4(-500.0f, 400.0f, 700.0f));
}

bool ARApp::CollisionAABB(gef::MeshInstance model1, gef::MeshInstance model2)
{
	gef::Aabb Aabb1 = model1.mesh()->aabb().Transform(model1.transform());
	gef::Aabb Aabb2 = model2.mesh()->aabb().Transform(model2.transform());

	return (Aabb1.max_vtx().x() > Aabb2.min_vtx().x() &&
		Aabb1.min_vtx().x() < Aabb2.max_vtx().x() &&	
		Aabb1.max_vtx().y() > Aabb2.min_vtx().y() &&	
		Aabb1.min_vtx().y() < Aabb2.max_vtx().y() &&	
		Aabb1.max_vtx().z() > Aabb2.min_vtx().z() &&	
		Aabb1.min_vtx().z() < Aabb2.max_vtx().z());		
	//if true, collision has occured		   
}

void ARApp::CollisionSide(Gameobject& model1, gef::MeshInstance& model2)
{
	gef::Aabb Aabb1 = model1.mesh()->aabb().Transform(model1.transform());
	gef::Aabb Aabb2 = model2.mesh()->aabb().Transform(model2.transform());

	if (Aabb1.max_vtx().x() > Aabb2.min_vtx().x() && Aabb1.min_vtx().x() < Aabb2.min_vtx().x())
	{
		//if the right side of the ball is colliding with an object then it cant move right
		model1.moveRight = false;
	}

	if (Aabb1.min_vtx().x() < Aabb2.max_vtx().x() && Aabb1.max_vtx().x() > Aabb2.max_vtx().x())
	{
		//if the left side of the ball is colliding with an object then it cant move left
		model1.moveLeft = false;
	}

	if (Aabb1.max_vtx().y() > Aabb2.min_vtx().y() && Aabb1.min_vtx().y() < Aabb2.min_vtx().y())
	{
		//if the top side of the ball is colliding with an object then it cant move up
		model1.moveUp = false;
	}

	if (Aabb1.min_vtx().y() < Aabb2.max_vtx().y() && Aabb1.max_vtx().y() > Aabb2.max_vtx().y())
	{
		//if the bottom side of the ball is colliding with an object then it cant move down
		model1.moveDown = false;
	}
}

void ARApp::GoalMovement(float frame_time, gef::Vector4 marker_distance)
{
	//This causes the Goal to change every few seconds
	if (goalMoveTimer < 0)
	{
		//randomly choose which goal to change to
		og = static_cast<openGoal>(rand() % 4 + 1);
		goalMoveTimer = 10;
	}
	else
	{
		goalMoveTimer -= frame_time;
	}

	switch (og)
	{
	case UPGOAL:
		GoalPlane_.LocalMove(0.0f, goalPosition * abs(marker_distance.y()), 0.05f);

		break;
	case RIGHTGOAL:
		GoalPlane_.LocalMove(goalPosition * abs(marker_distance.x()), 0.0f, 0.07f);

		break;
	case DOWNGOAL:
		GoalPlane_.LocalMove(0.0f, -goalPosition * abs(marker_distance.y()), 0.07f);

		break;
	case LEFTGOAL:
		GoalPlane_.LocalMove(-goalPosition * abs(marker_distance.x()), 0.0f, 0.07f);

		break;
	}
}

void ARApp::VectorCalculations()
{

	float x1, x2, y1, y2;
	
	x1 = touch_start_.x;
	x2 = touch_end_.x;
	y1 = touch_start_.y;
	y2 = touch_end_.y;

	//Pythagoras
	touchDistance = (sqrt(pow(x2 - x1, 2) + pow(y2 - y1, 2)) / 250);

	Ball_.velocityX = (x2 - x1) / 100000;
	Ball_.velocityY = (y2 - y1) / -100000;

}

void ARApp::gravity(float frame_time)
{
	if (touchDistance >= 0)
	{
		touchDistance -= frame_time;
	}
}

void ARApp::Collisions()
{
	//Collision between ball and the Goal
	if (CollisionAABB(Ball_, GoalPlane_) == true)
	{
		Ball_.tempx = 0;
		Ball_.tempy = 0;
		touchDistance = 0;
		goalsScored++;
	}

	//checks to see if the ball is still on the playing area
	if (CollisionAABB(Ball_, arenaPlane_) != true)
	{
		Ball_.tempx = 0;
		Ball_.tempy = 0;
		touchDistance = 0;
	}

	//Collisions between the ball and each plane of the goal block
	for (int i = 0; i <= 3; i++)
	{
		for (int p = 0; p <= 2; p++)
		{
			if (CollisionAABB(Ball_, Goal_[i].goal_walls_[p]) == true)
			{
				CollisionSide(Ball_, Goal_[i].goal_walls_[p]);
			}
		}
	}

	//Tells the ball which direction it can travel in
	Ball_.CollisionDirection();
}

void ARApp::GoalSetUp()
{
	//Setting up Goals
	for (int i = 0; i <= 3; i++)
	{
		Goal_[i].goal_meshes_[0] = CreateGoalBlock1Mesh();
		Goal_[i].goal_meshes_[1] = CreateGoalBlock2Mesh();
		Goal_[i].goal_meshes_[2] = CreateGoalBlock3Mesh();
		Goal_[i].goal_walls_[0].set_mesh(Goal_[i].goal_meshes_[0]);
		Goal_[i].goal_walls_[1].set_mesh(Goal_[i].goal_meshes_[1]);
		Goal_[i].goal_walls_[2].set_mesh(Goal_[i].goal_meshes_[2]);
	}

	Goal_[0].Rotation(0.0f, 0.0f, 0.0f);
	Goal_[1].Rotation(0.0f, 0.0f, 4.71f);
	Goal_[2].Rotation(0.0f, 0.0f, 3.14f);
	Goal_[3].Rotation(0.0f, 0.0f, 1.57f);

	GoalPlane_.gameobject_mesh_ = CreatePlaneMesh();
	GoalPlane_.set_mesh(GoalPlane_.gameobject_mesh_);
}



