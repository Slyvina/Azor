// License:
// 
// Azor
// Devlog Management
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
// Version: 24.10.15 II
// End License

#include <SlyvQCol.hpp>
#include <SlyvConInput.hpp>
#include "Azor_Config.hpp"
#include <SlyvString.hpp>
#include "Azor_CommandRegister.hpp"
#include "Azor_Project.hpp"

#undef PARADEBUG

using namespace Slyvina;
using namespace Slyvina::Units;
using namespace Slyvina::Azor;

int main(int cargs, char** args) {
	QCol->LMagenta("Azor\n\n");
	QCol->Doing("Coded by", "Jeroen P. Broks");
	QCol->Doing("License", "General Public License 3");
	QCol->Doing("Config", ConfigCreation());
	BaseCommands();
	ProjectCommands();
	do {
		QCol->Yellow(Prompt());
		QCol->Cyan("");
		String fcmd{ Trim(ReadLine()) }, cmd{ "" };
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
						QCol->Doing("-->", CP+"/"+ch);
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
		if (cmd == "BYE") {
			QCol->Doing("Quitting", "Azor");
			QCol->Reset();
			return 0;
		} else Execute(cmd, Para);
	} while (true);
}
