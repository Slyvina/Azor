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
// Version: 24.10.31
// End License

#pragma once
#include <memory>
#include <SlyvGINIE.hpp>
#include <SlyvAsk.hpp>
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
		struct Azor_Color {
			Byte R{ 0 }, G{ 0 }, B{ 0 }; // RGB
			int H{ 0 }; Byte S{ 100 }, V{ 100 }; //HSV
		};

		class _Azor_Project {
		private:
			String pname{ "" };
			Units::GINIE RawConfig{ nullptr };
		public:
			int AutoPush{ 10 };
			std::map<int, Azor_Index> Indexes{};
			static Azor_Project Use(std::string _pname);
			static Azor_Project Current();
			~_Azor_Project();
			inline _Azor_Project() {}
			_Azor_Project(std::string _pname);
			inline String ProjectName() { return pname; }
			inline String ProjectFileName() const { return ProjectPath() + "/" + pname + ".azor"; }
			inline String ProjectIndexFile() { return ProjectPath() + "/" + pname + ".index"; }
			inline String ProjectContentFile() { return ProjectPath() + "/" + pname + ".content"; }
			inline String GitHub() { return RawConfig->Value("GITHUB", "REPOSITORY"); }
			inline void GitHub(String NV) { RawConfig->Value("GITHUB", "REPOSITORY", NV); }
			std::vector<std::string>* Tags();
			std::vector<std::string>* Prefixes();
			Azor_CDPrefix Prefix(std::string tag);
			void Prefix(std::string tag, Azor_CDPrefix pref, bool noautosave = false);
			String Name();
			void SaveIndexes();
			Azor_Index& Index(int idx);
			String ParsePure(String pure);
			bool HasTag(String Tag);
			void NewTag(String Tag);
			int HighIndex();
			int CountRecords();
			void List(int start, int end);
			void List(int entry) { List(entry, entry); }
			void List() { List(0, HighIndex()); }
			Azor_Entry Entry(int i);
			Azor_Color FColMin();
			Azor_Color BColMin();
			Azor_Color FColMax();
			Azor_Color BColMax();
			String OutDir() { return Units::Ask(RawConfig, "Target", Platform(), "Export output directorty for " + Platform() + ":"); }
			void OutDir(String s) { RawConfig->Value("Target", Platform(), s); }
			String Template() { return Units::Ask(RawConfig, "Template", Platform(), "HTML Template for export on platform " + Platform() + ":"); }
			void Template(String s) { RawConfig->Value("Template", Platform(), s); }
			void Take(int t) { RawConfig->Value("Count", "Takes", t); }
			int Take() { return RawConfig->IntValue("Count", "Takes"); }
			void AddEntry(String Tag, String Content, bool forcenewtag = false);
			void Generate(bool andpush = false);
			void Push();
			void SaveRaw();
			int ExportTableWidth() { return RawConfig->NewValue("Export", "TableWidth", 1200); }
			void ExportTableWidth(int value) { RawConfig->Value("Export", "TableWidth", value); }
			int ExportContentWidth() { return RawConfig->NewValue("Export", "ContentWidth", 800); }
			void ExportContentWidth(int value) { RawConfig->Value("Export", "ContentWidth", value); }
			String AltIcon(String tag) { return RawConfig->Value("Tag:"+tag, "Icon"); }
			void AltIcon(String tag,String v) { RawConfig->Value("Tag:"+tag, "Icon", v); }
			String IconFloat() { return RawConfig->NewValue("Export", "IconFloatPosition", "right"); }
			void IconFloat(String v) { RawConfig->Value("Export", "IconFloatPosition", v); }
			int IconHeight() { return RawConfig->NewValue("Export", "IconHeight", 50); }
			void IconHeight(int v) { RawConfig->Value("Export", "IconHeight", v); }
			void Unlink(int victim);
			bool HasMacro(String Macro, String Platform = "Always");
			std::vector<String> Macro(String _Macro, String _Platform = "Always");

			
		};

		class _Azor_Entry {
		private:
			std::map<String, String> core{};
			bool modified{ false };
			_Azor_Project* parent{ nullptr };
			int index{ 0 };
		public:
			~_Azor_Entry();
			void GetMe(Units::InFile stream);
			void GetMe();
			void Pure(std::string a);
			String Pure();
			String Text();
			inline int ID() { return index; }
			void Tag(std::string nt, bool createifnotexistent = false);
			String Tag();
			String Date() { return core["DATE"]; }
			String Time() { return core["TIME"]; }
			String AltIcon() { return parent->AltIcon(Tag()); }
			String Icon() { return parent->AltIcon(Tag()); }
			//void Icon(String v) { core["ICON"] = v; }
			_Azor_Entry(){}
			_Azor_Entry(_Azor_Project* p);
			_Azor_Entry(_Azor_Project* p, int _idx);
			_Azor_Entry(_Azor_Project* p, String _Tag, String _Pure);
			void UpdateMe(Units::OutFile stream);
			void UpdateMe();
		};


		String Using();
		String Prompt();
		void ProjectCommands();

	}
}
