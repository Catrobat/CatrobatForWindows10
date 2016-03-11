#pragma once

#include "Script.h"
#include "IStartScript.h"

namespace ProjectStructure
{
	class StartScript :
		public Script
	{
	public:
		StartScript(Catrobat_Player::NativeComponent::IStartScript^ script, Object* parent);
		~StartScript();
	};
}