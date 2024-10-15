// License:
// 
// Azor
// Command Register
// 
// 
// 
// 	(c) Jeroen P. Broks, 2024
// 
// 		This program is free software: you can redistribute it and/or modify
// 		it under the terms of the GNU General Public License as published by
// 		the Free Software Foundation, either version 3 of the License, or
// 		(at your option) any later version.
// 
// 		This program is distributed in the hope that it will be useful,
// 		but WITHOUT ANY WARRANTY; without even the implied warranty of
// 		MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// 		GNU General Public License for more details.
// 		You should have received a copy of the GNU General Public License
// 		along with this program.  If not, see <http://www.gnu.org/licenses/>.
// 
// 	Please note that some references to data like pictures or audio, do not automatically
// 	fall under this licenses. Mostly this is noted in the respective files.
// 
// Version: 24.10.15 I
// End License
#include <Slyvina.hpp>

#include "Azor_CommandRegister.hpp"
#include <map>
#include <SlyvString.hpp>
#include <SlyvQCol.hpp>
#include <SlyvSilly.hpp>

using namespace Slyvina::Units;

namespace Slyvina {
	namespace Azor {

#pragma region "Base Commands"
		static void cmd_cls(std::vector<String>) { cls(); }
		static void cmd_fuck(std::vector<String>) { QCol->LMagenta("What kind of talk is that?\n"); }
		static void cmd_paratest(carg a) {
			QCol->Reset();
			for (size_t i = 0; i < a.size(); i++) printf("%9d - %s\n", (int)i + 1, a[i].c_str());
		}
#pragma endregion

		static std::map<String, Azor_Command> _Reg{};

		void RegCommand(std::string CS, Azor_Command AC) {
			Trans2Upper(CS);
			if (_Reg.count(CS))
				QCol->Error("Dupe command register: " + CS);
			else
				_Reg[CS] = AC;
		}

		void BaseCommands() {
			RegCommand("cls", cmd_cls);
			RegCommand("fuck", cmd_fuck);
			RegCommand("shit", cmd_fuck);
			RegCommand("Paratest", cmd_paratest);
		}
		void Execute(std::string cmd, std::vector<std::string> args) {
			cmd = Trim(cmd);
			Trans2Upper(cmd);
			if (!cmd.size()) return;
			if (!_Reg.count(cmd)) { QCol->Error("Command " + cmd + " not understood!"); return; };
			if (!_Reg[cmd]) { QCol->Error("Command " + cmd + " is a null pointer! This is a bug in Azor! Please report!"); return; }
			_Reg[cmd](args);
		}
	}
}
