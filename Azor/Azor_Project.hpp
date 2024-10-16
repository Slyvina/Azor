// License:
// 
// Azor
// Project (header)
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

#pragma once
#include <memory>
#include <SlyvGINIE.hpp>
#include "Azor_Config.hpp"


namespace Slyvina {
	namespace Azor {
		class _Azor_Project; typedef std::shared_ptr<_Azor_Project> Azor_Project; 
		class _Azor_Entry; typedef std::shared_ptr<_Azor_Entry> Azor_Entry;
		struct Azor_CDPrefix { std::string Prefix{ "" }; uint32 Reset{ 0 }, CD{ 0 }; };

		struct Azor_Index {
			int32 id{ 0 };
			uint64 size{ 0 };
			uint64 offset{ 0 };
			bool valid{ true };
		};

		class _Azor_Project {
		private:
			String pname{ "" };
			Units::GINIE RawConfig{ nullptr };
		public:
			std::map<int, Azor_Index> Indexes{};
			static Azor_Project Use(std::string _pname);
			static Azor_Project Current();
			~_Azor_Project();
			inline _Azor_Project() {}
			_Azor_Project(std::string _pname);
			inline String ProjectFileName() const { return ProjectPath() + "/" + pname + ".azor"; }
			inline String ProjectIndexFile() { return ProjectPath() + "/" + pname + ".index"; }
			inline String ProjectContentFile() { return ProjectPath() + "/" + pname + ".content"; }
			inline String GitHub() { return RawConfig->Value("GITHUB", "REPOSITORY"); }
			inline void GitHub(String NV) { RawConfig->Value("GITHUB", "REPOSITORY", NV); }
			std::vector<std::string>* Tags();
			std::vector<std::string>* Prefixes();
			Azor_CDPrefix Prefix(std::string tag);
			String Name();
			void SaveIndexes();
			Azor_Index& Index(int idx);
			String ParsePure(String pure);
			bool HasTag(String Tag);
			void NewTag(String Tag);
			int HighIndex();
		};

		class _Azor_Entry {
		private:
			std::map<String, String> core{};
			bool modified{ false };
			_Azor_Project* parent{ nullptr };
			int index{ 0 };
		public:
			void GetMe(InFile stream);
			void GetMe();
			void Pure(std::string a);
			String Pure();
			String Text();
			inline int ID() { return index; }
			void Tag(std::string nt, bool createifnotexistent = false);
			String Tag();
		};


		String Using();
		String Prompt();
		void ProjectCommands();

	}
}
