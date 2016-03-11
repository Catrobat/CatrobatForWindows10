#include "pch.h"
#include "HideBrick.h"
#include "Script.h"
#include "Object.h"

using namespace ProjectStructure;

HideBrick::HideBrick(Script* parent) :
	Brick(TypeOfBrick::HideBrick, parent)
{
}

void HideBrick::Execute()
{
	m_parent->GetParent()->SetTransparency(1.0f);
}