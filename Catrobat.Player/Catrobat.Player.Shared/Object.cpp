#include "pch.h"
#include "Object.h"
#include "ProjectDaemon.h"
#include "PlayerException.h"
#include "OutOfBoundsException.h"
#include "StartScript.h"
#include "BroadcastScript.h"

#include <exception>
#include <math.h>

using namespace D2D1;
using namespace std;
using namespace ProjectStructure;

Object::Object(Catrobat_Player::NativeComponent::IObject^ object) :
	m_name(Helper::StdString(object->Name)),
	m_look(nullptr),
	m_opacity(1.f),
	m_rotation(0.f),
	m_translation(Point2F()),
	m_objectScale(SizeF(1.f, 1.f)),
	m_transformation(Matrix3x2F()),
	m_ratio(SizeF()),
	m_logicalSize(SizeF()),
	m_renderTargetSize(SizeF())
{
	for each (Catrobat_Player::NativeComponent::ILook^ look in object->Looks)
	{
		m_lookList.push_back(std::shared_ptr<Look>(make_shared<Look>(look)));
	}

	for each (Catrobat_Player::NativeComponent::IScript^ script in object->Scripts)
	{
		auto startScript = dynamic_cast<Catrobat_Player::NativeComponent::IStartScript^>(script);
		if (startScript)
		{
			m_scripts.push_back(std::shared_ptr<Script>(make_shared<StartScript>(startScript, this)));
			continue;
		}
		auto whenScript = dynamic_cast<Catrobat_Player::NativeComponent::IWhenScript^>(script);
		if (whenScript)
		{
			m_scripts.push_back(std::shared_ptr<Script>(make_shared<WhenScript>(whenScript, this)));
			continue;
		}
		auto broadcastScript = dynamic_cast<Catrobat_Player::NativeComponent::IBroadcastScript^>(script);
		if (broadcastScript)
		{
			m_scripts.push_back(std::shared_ptr<Script>(make_shared<BroadcastScript>(broadcastScript, this)));
			continue;
		}
	}

	for each (Catrobat_Player::NativeComponent::IUserVariable^ userVariable in object->UserVariables)
	{
		m_variableList.insert(std::pair<std::string, std::shared_ptr<UserVariable>>(Helper::StdString(userVariable->Name), make_shared<UserVariable>(userVariable)));
	}

	// TODO: SoundInfos
}

Object::~Object()
{
}

#pragma region TRANSFORMATION
void Object::SetTranslation(float x, float y)
{
	m_translation = Point2F(x, y);
	RecalculateTransformation();
}

void Object::GetTranslation(float &x, float &y)
{
	x = m_translation.x;
	y = m_translation.y;
}

void Object::SetTransparency(float transparency)
{
	if ((1.0f - transparency) > 0)
	{
		if ((1.0f - transparency) < 1.0f)
		{
			m_opacity = 1.0f - transparency;
		}
		else
		{
			m_opacity = 1.0f;
		}
	}
	else
	{
		m_opacity = 0.0f;
	}
}

float Object::GetTransparency()
{
	return 1.0f - m_opacity;
}

void Object::SetRotation(float rotation)
{
	m_rotation = rotation;
	RecalculateTransformation();
}

float Object::GetRotation()
{
	return m_rotation;
}

void Object::SetScale(float x, float y)
{
	if (x < 0.f)
	{
		x = 0.f;
	}
	if (y < 0.f)
	{
		y = 0.f;
	}

	m_objectScale = SizeF(x, y);
	RecalculateTransformation();
}

void Object::GetScale(float &x, float &y)
{
	x = m_objectScale.width;
	y = m_objectScale.height;
}
#pragma endregion

#pragma region GENERAL
void Object::SetLook(int index)
{
	if (m_lookList.size() > index)
	{
		list<shared_ptr<Look>>::iterator it = m_lookList.begin();
		advance(it, index);
		m_look = *it;
		RecalculateTransformation();
	}
}

bool Object::IsObjectHit(D2D1_POINT_2F position)
{
	if (m_look == NULL)
	{
		return false;
	}

	Matrix3x2F translation = CalculateTranslationMatrix();
	D2D1_POINT_2F origin;
	origin.x = (float) m_look->GetWidth() / 2;
	origin.y = (float) m_look->GetHeight() / 2;

	Matrix3x2F positionInBitMap = translation;
	positionInBitMap._31 = (position.x - positionInBitMap._31) / m_ratio.width;
	positionInBitMap._32 = m_look->GetHeight() - (position.y - positionInBitMap._32) / m_ratio.height;

	positionInBitMap = positionInBitMap *
		Matrix3x2F::Rotation(360 - m_rotation, origin) *
		Matrix3x2F::Scale(m_objectScale, origin);

	position.x = positionInBitMap._31;
	position.y = positionInBitMap._32;

	return m_look->GetPixelAlphaValue(position) != 0;
}
#pragma endregion

#pragma region RENDERING
void Object::LoadTextures(const std::shared_ptr<DX::DeviceResources>& deviceResources)
{
	SetTranslation(0.f, 0.f);

	for (auto const& look : m_lookList)
	{
		look->LoadTexture(deviceResources);
	}
}

void Object::SetupWindowSizeDependentResources(const std::shared_ptr<DX::DeviceResources>& deviceResources)
{
	auto deviceContext = deviceResources->GetD2DDeviceContext();
	m_logicalSize = deviceContext->GetSize();

	int screen_width = ProjectDaemon::Instance()->GetProject()->GetHeader()->GetScreenWidth();
	int screen_height = ProjectDaemon::Instance()->GetProject()->GetHeader()->GetScreenHeight();

	m_ratio.width = m_logicalSize.width / ProjectDaemon::Instance()->GetProject()->GetHeader()->GetScreenWidth();
	m_ratio.height = m_logicalSize.height / ProjectDaemon::Instance()->GetProject()->GetHeader()->GetScreenHeight() * (-1);
	RecalculateTransformation();
}

void Object::StartUp()
{
	SetLook(0);

	for (auto const& script : m_scripts)
	{
		if (script->GetType() == Script::TypeOfScript::StartScript)
		{
			script->Execute();
		}
	}
}

void Object::Draw(const std::shared_ptr<DX::DeviceResources>& deviceResources)
{
	if (m_look == NULL)
	{
		return;
	}

	auto deviceContext = deviceResources->GetD2DDeviceContext();
	deviceContext->SetTransform(m_transformation);
	deviceContext->DrawBitmap(m_look->GetBitMap().Get(),
		RectF(0.f, 0.f, m_renderTargetSize.width, m_renderTargetSize.height), m_opacity);

}
#pragma endregion

#pragma region GETTERS
shared_ptr<UserVariable> Object::GetVariable(std::string name)
{
	map<string, shared_ptr<UserVariable> >::iterator searchItem = m_variableList.find(name);

	if (searchItem != m_variableList.end())
	{
		return searchItem->second;
	}

	return NULL;
}

int Object::GetIndexOfCurrentLook()
{
	list<shared_ptr<Look>>::iterator it = m_lookList.begin();
	if (it == m_lookList.end())
	{
		throw new PlayerException("LookList empty! No current look available.");
	}
	while ((*it) != m_look)
	{
		it++;
	}
	return distance(m_lookList.begin(), it);
}

shared_ptr<Script> Object::GetScript(int index)
{
	if (m_scripts.size() > index)
	{
		list<shared_ptr<Script>>::iterator it = m_scripts.begin();
		advance(it, index);
		return *it;
	}
	return nullptr;
}
#pragma endregion

#pragma region INTERNAL
void Object::RecalculateTransformation()
{
	if (m_look == NULL)
	{
		return;
	}
	m_renderTargetSize = m_look->GetBitMap()->GetSize();
	m_renderTargetSize.width *= m_ratio.width;
	m_renderTargetSize.height *= m_ratio.height;

	Matrix3x2F translation = CalculateTranslationMatrix();
	D2D1_POINT_2F origin;
	origin.x = translation._31 + m_renderTargetSize.width / 2;
	origin.y = translation._32 + m_renderTargetSize.height / 2;

	m_transformation = translation *
		Matrix3x2F::Rotation(m_rotation, origin) *
		Matrix3x2F::Scale(m_objectScale, origin);
}

Matrix3x2F Object::CalculateTranslationMatrix()
{
	m_renderTargetSize = m_look->GetBitMap()->GetSize();
	m_renderTargetSize.width *= m_ratio.width;
	m_renderTargetSize.height *= m_ratio.height;
	Matrix3x2F renderTarget = Matrix3x2F::Identity();
	renderTarget = Matrix3x2F::Translation(m_logicalSize.width / 2 - m_renderTargetSize.width / 2,
		m_logicalSize.height / 2 - m_renderTargetSize.height / 2);

	return Matrix3x2F::Translation(m_translation.x * m_ratio.width, m_translation.y * m_ratio.height) * renderTarget;
}
#pragma endregion