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
// Version: 24.10.15 I
// End License

#include <SlyvQCol.hpp>
#include "Azor_Config.hpp"
using namespace Slyvina::Azor;

int main(int cargs, char** args) {
	QCol->LMagenta("Azor\n\n");
	QCol->Doing("Coded by", "Jeroen P. Broks");
	QCol->Doing("License", "General Public License 3");
	QCol->Doing("Config", ConfigCreation());
}
