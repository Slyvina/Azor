// License:
// 
// Azor
// Global Config
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

#include <SlyvGINIE.hpp>
#include <SlyvTime.hpp>
#include <SlyvDirry.hpp>
#include <SlyvQCol.hpp>
#include "Azor_Config.hpp"
#include <SlyvAsk.hpp>

using namespace Slyvina::Units;

#define AzorConfigFile Dirry("$AppSupport$/Azor.GlobalConfig.ini")

namespace Slyvina {
	namespace Azor {
		static GINIE MainConfig{ nullptr };

		String ConfigCreation() {
			if (!MainConfig) {
				if (!FileExists(AzorConfigFile)) {
					QCol->Doing("Creating", AzorConfigFile);
					SaveString(AzorConfigFile, "# Nothing to see here\n# No nothing at all!");
				} else {
					QCol->Doing("Loading", AzorConfigFile);
				}
				MainConfig = LoadGINIE(AzorConfigFile, AzorConfigFile, "Azor global config file.\n(c) Jeroen P. Broks");
			}
			return MainConfig->NewValue("Azor", "Creation", CurrentDate() + "; " + CurrentTime());
		}

		String ProjectPath() {
			return Ask(MainConfig, "Project", "Directory", "Full Directory of where the projects should be stored: ");
		}

		String ConverterProgram() {
			return Ask(MainConfig, "Convert", "Program", "Executable of the converter program please: ");
		}

		bool HasMacro(String Macro, String Platform) {
			return MainConfig->HasList("MACRO::"+Macro,Platform);
		}

		std::vector<String>* Macro(String _Macro, String _Platform) {
			return HasMacro(_Macro, _Platform) ? MainConfig->List("MACRO::"+_Macro, _Platform) : nullptr;
		}

	}
}
