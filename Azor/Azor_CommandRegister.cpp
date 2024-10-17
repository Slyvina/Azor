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
// Version: 24.10.17
// End License
#include <Slyvina.hpp>

#include "Azor_CommandRegister.hpp"
#include "Azor_Config.hpp"
#include <map>
#include <SlyvString.hpp>
#include <SlyvQCol.hpp>
#include <SlyvDir.hpp>
#include <SlyvSilly.hpp>

using namespace Slyvina::Units;

namespace Slyvina {
	namespace Azor {
		static std::map<String, Azor_Command> _Reg{};

#pragma region "Base Commands"
		static void cmd_cls(std::vector<String>) { cls(); }
		static void cmd_fuck(std::vector<String>) { QCol->LMagenta("What kind of talk is that?\n"); }
		static void cmd_paratest(carg a) {
			QCol->Reset();
			for (size_t i = 0; i < a.size(); i++) printf("%9d - %s\n", (int)i + 1, a[i].c_str());
		}
		static void cmd_dir(carg) {
			auto d{ FileList(ProjectPath()) };
			auto l{ NewStringMap() };
			for (auto fa : *d) {
				auto
					a{ StripExt(fa) },
					e{ Upper(ExtractExt(fa)) };
				if (e == "PRJ" && (*l)[a] == "") (*l)[a] = "Devlog";
				else if (e == "AZOR") (*l)[a] = "  Azor";
			}
			auto cnt{ 0 };
			for (auto f : *l) {
				QCol->LCyan(TrSPrintF("%9d: ", ++cnt));
				QCol->LGreen(f.second+": ");
				QCol->LMagenta(f.first + "\n");
			}
		}

		static void cmd_help(carg a) {
			QCol->Yellow("Azor supports the next commands\n");
			for (auto cm : _Reg) {
				if (cm.second != cmd_fuck) {
					QCol->Red("= ");
					QCol->LGreen(cm.first + "\n");
				}
			}
		}
#pragma endregion


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
			RegCommand("dir", cmd_dir);
			RegCommand("Help", cmd_help);
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
