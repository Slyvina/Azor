#include <Slyvina.hpp>

#include "Azor_CommandRegister.hpp"
#include <map>
#include <SlyvString.hpp>
#include <SlyvQCol.hpp>

using namespace Slyvina::Units;

namespace Slyvina {
	namespace Azor {

		static std::map<String, Azor_Command> _Reg{};

		void RegCommand(std::string CS, Azor_Command AC) {
			Trans2Upper(CS);
			if (_Reg.count(CS))
				QCol->Error("Dupe command register: " + CS);
			else
				_Reg[CS] = AC;
		}
	}
}