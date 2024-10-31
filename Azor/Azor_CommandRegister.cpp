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
// Version: 24.10.31
// End License
#include <Slyvina.hpp>

#include "Azor_CommandRegister.hpp"
#include "Azor_Config.hpp"
#include "Azor_Project.hpp"
#include <map>
#include <SlyvString.hpp>
#include <SlyvQCol.hpp>
#include <SlyvDir.hpp>
#include <SlyvSilly.hpp>
#include <SlyvStream.hpp>

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

		static void cmd_convert(carg a) {
			auto converter{ ConverterProgram() };
			for (auto prj : a) system(String(converter + " \"" + prj + "\"").c_str());
		}
		void cmd_Shell(carg c) {
			auto CS{ c[0] };
			auto Cmd{ CS };
			QCol->Reset();
			for(size_t i=1;i<c.size();i++){	Cmd += " \"" + c[i] + "\"";	}
			system(Cmd.c_str());
		}
		void cmd_pwd(carg c) { QCol->Reset(); std::cout << CurrentDir() << "\n"; }
		void cmd_doing(carg c) { 
			if (c.size() < 2) { QCol->Error("Doing got invalid input!"); return; }
			QCol->Doing(c[0], c[1], c.size() > 2 ? c[2] : "\n");
		}
		void cmd_cd(carg c) {
			if (c.size() != 1) { QCol->Error("Invalid number of arguments"); return; }
			ChangeDir(c[0]);
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
			RegCommand("convert", cmd_convert);
			RegCommand("shell", cmd_Shell);
			RegCommand("pwd", cmd_pwd);
			RegCommand("doing", cmd_doing);
			RegCommand("cd", cmd_cd);
		}
		void ExecuteMacro(std::vector<String> Lines, std::vector<String>* args) {
			//QCol->Doing("Macro", TrSPrintF("%d line(s)", Lines.size()));
			for (size_t ln=0; ln < Lines.size(); ln++) {
				String fcmd{ Trim(Lines[ln]) }, cmd{ "" };
				std::vector<String> Para{};
				auto fparaf{ IndexOf(fcmd,' ') };
				if (fparaf <= 0) cmd = Upper(fcmd);
				else {
					cmd = Upper(fcmd.substr(0, fparaf));
					Para.clear();
					Para.push_back("");
					auto escape{ false }, instring{ false };
					for (auto p = fparaf; p < fcmd.size(); p++) {
						auto& CP{ Para[Para.size() - 1] };
						auto ch{ fcmd[p] };
						if (escape) {
							switch (ch) {
							case 'n': CP += "\n"; break;
							case 'r': CP += "\r"; break;
							case 'b': CP += "\b"; break;
							case 't': CP += "\t"; break;
							default:
								CP += ch;
							}
							escape = false;
						} else {
#ifdef PARADEBUG
							QCol->Doing("Param", Para.size());
							QCol->Doing("-->", CP + "/" + ch);
#endif
							switch (ch) {
							case ' ':
							case '\t':
								if (instring) CP += ch;
								else if (CP.size()) {
									Para.push_back("");
									continue;
								}
								break;
							case '"':
								instring = !instring;
								break;
							case '\\':
								escape = true;
								break;
							default:
								CP += ch;
								break;
							}
						}
					}
				}
				for (size_t ai = 0; ai < args->size(); ai++) {
					cmd = StReplace(cmd, TrSPrintF("$%d", ai), (*args)[ai]);
					for (size_t li = 0; li < Para.size(); li++) {

						Para[li] = StReplace(Para[li], TrSPrintF("$%d", ai), (*args)[ai]);
						
					}
				}
				for (size_t li = 0; li < Para.size(); li++) Para[li] = StReplace(Para[li], "$bs", "\\");
				if (cmd == "ARGDEMAND") {
					auto want{ args->size() >= 1 ? ToInt((*args)[0]) : 1 };
					if (args->size() < want) {
						QCol->Error(TrSPrintF(String("Macro requires %d " + String(want == 1 ? "argument" : "arguments")).c_str(), want));
						return;
					}
				}
				else if (cmd == "BYE") { QCol->Error(TrSPrintF("Macro line #%d: BYE not allowed in Macro!", ln + 1)); }
				else if (cmd == "" || Prefixed(cmd, "#")) {} // do nothing on whitelines or comments
				else { 
					Execute(cmd, Para); 
				}
			}
		}

		void Execute(std::string cmd, std::vector<std::string> args) {
			cmd = Trim(cmd);
			Trans2Upper(cmd);
			cmd = StReplace(cmd, "$qt", "\\");
			for (size_t i = 0; i < args.size(); i++) args[i] = StReplace(args[i], "$qt", "\\");
			if (!cmd.size()) return;
			if (!_Reg.count(cmd)) { 
				if(_Azor_Project::Current()) {
					auto C{ _Azor_Project::Current() };
					if (C->HasMacro(cmd)) {
						ExecuteMacro(C->Macro(cmd), &args);
						return;
					}
					if (C->HasMacro(cmd, Platform())) {
						ExecuteMacro(C->Macro(cmd,Platform()), &args);
						return;
					}
					if (HasMacro(cmd)) { ExecuteMacro(*Macro(cmd), &args); return; }
					if (HasMacro(cmd,Platform())) { ExecuteMacro(*Macro(cmd,Platform()), &args); return; }
				}
				QCol->Error("Command " + cmd + " not understood!"); return; 
			};
			if (!_Reg[cmd]) { QCol->Error("Command " + cmd + " is a null pointer! This is a bug in Azor! Please report!"); return; }
			_Reg[cmd](args);
		}
		
	}
}
