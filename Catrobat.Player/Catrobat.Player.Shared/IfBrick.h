#pragma once

#include "ContainerBrick.h"
#include "Object.h"
#include "IIfBrick.h"

#include <list>

namespace ProjectStructure
{
	enum IfBranchType
	{
		If,
		Else
	};

	class IfBrick :
		public ContainerBrick
	{
	public:
		IfBrick(Catrobat_Player::NativeComponent::IIfBrick^ brick, Script* parent);
		~IfBrick();

		void Execute();
		std::list<std::unique_ptr<Brick>> *ListPointer() override;

		void ElseMode();
	private:
		bool m_elseMode;
		std::list<std::unique_ptr<Brick>> m_alternateBrickList;
		std::shared_ptr<FormulaTree> m_condition;
	};
}