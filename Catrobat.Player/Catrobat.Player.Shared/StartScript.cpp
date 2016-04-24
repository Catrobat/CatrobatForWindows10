#include "pch.h"
#include "StartScript.h"

#include <windows.system.threading.h>
#include <windows.foundation.h>
#include <ppltasks.h>

using namespace Windows::System::Threading;
using namespace Windows::Foundation;
using namespace std;
using namespace ProjectStructure;

StartScript::StartScript(Catrobat_Player::NativeComponent::IStartScript^ script, Object* parent) :
	Script(TypeOfScript::StartScript, parent, script)
{
}

StartScript::~StartScript()
{
}
