//This was the gameobject class for the ball in the PS Vita AR project

#include "gameobject.h"
#include <assets\obj_loader.h>

Gameobject::Gameobject()
{
	position_ = gef::Vector4(0, 0, 0, 0);
	velocity_ = gef::Vector2(0, 0);
	rotation_.SetIdentity();

	local_transform_.SetIdentity();
	velocityX = 0;
	velocityY = 0;
	tempx = 0;
	tempy = 0;
}

Gameobject::~Gameobject()
{

}

void Gameobject::BuildTransform(gef::Matrix44 marker_transform)
{
	//take the main transform and multiply it by the scale, rotation, local transform and finally the marker to build the object
	gef::Matrix44 gameobject_transform;
	gameobject_transform.SetIdentity();

	set_transform(gameobject_transform * scale_transform_ * rotation_ * local_transform_ * marker_transform);
}

void Gameobject::Scale(float x, float y, float z)
{
	//Set the scale
	scale_ = gef::Vector4(x, y, z);
	scale_transform_.Scale(gef::Vector4(scale_.x(), scale_.y(), scale_.z(), 1.0f));
}

void Gameobject::LocalMove(float x, float y, float z)
{
	//set the local transform
	local_ = gef::Vector4(x, y, z);
	local_transform_.SetTranslation(gef::Vector4(local_.x(), local_.y(), local_.z()));
}

void Gameobject::Rotation(float rotX, float rotY, float rotZ)
{
	//set the rotation
	rotation_.RotationX(rotX);
	rotation_.RotationY(rotY);
	rotation_.RotationZ(rotZ);
}

void Gameobject::CollisionDirection()
{
	//if move bool is false and the object is moving in that direction then stop it.
	if (moveLeft == false && velocityX < 0)
	{
		velocityX = 0;
	}

	if (moveRight == false && velocityX > 0)
	{
		velocityX = 0;
	}

	if (moveUp == false && velocityY > 0)
	{
		velocityY = 0;
	}

	if (moveDown == false && velocityY < 0)
	{
		velocityY = 0;
	}

}

void Gameobject::TouchMove(float distance)
{
	//Move the object by the difference in the x and y touch positions and then multiply by the distance between the two positions
	tempx = tempx += (velocityX * distance);
	tempy = tempy += (velocityY * distance);
}
