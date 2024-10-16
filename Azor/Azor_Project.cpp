// License:
// 
// Azor
// Project
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
// Version: 24.10.16
// End License
#include "Azor_Project.hpp"
#include "Azor_CommandRegister.hpp"
#include "Azor_Config.hpp"
#include <SlyvQCol.hpp>
#include <SlyvTime.hpp>

#undef PRJ_DEBUG

using namespace Slyvina::Units;


#ifdef PRJ_DEBUG
#define Chat(abc) QCol->Red("DEBUG> "); QCol->Grey(""); std::cout <<abc<<std::endl;
#else
#define Chat(abc)
#endif

namespace Slyvina {
	namespace Azor {
		static String _Using{ "" };
		static std::map<String, Azor_Project> _PrjReg{};


		_Azor_Project::_Azor_Project(std::string _pname) {
			pname = _pname;
			if (FileExists(ProjectFileName())) {
				QCol->Doing("Using", pname);
				RawConfig = LoadGINIE(ProjectFileName(), ProjectFileName(), "Project: " + pname + "\nUsed by Azor");
				if (!RawConfig) {					
					QCol->Error(ProjectFileName() + " failed to load");
					return;
				}
				RawConfig->Value("Azor", "Checked", CurrentDate());
			} else {
				QCol->Doing("Creating", pname);
				RawConfig = ParseGINIE("# Nothing\n# to see\n#here!", ProjectFileName(), "Project: " + pname + "\nUsed by Azor");
			}
			if (FileExists(ProjectIndexFile())) {
				QCol->Doing("Loading", ProjectIndexFile());
				auto bix{ ReadFile(ProjectIndexFile()) };
				byte tag;
				Azor_Index Index;
				//GUI.WriteLn($"Loading indexes for project: {prjname}");
				uint64 cnt{ 0 };
				auto high{ 0 };
				while (!bix->EndOfFile()) {
					//if (bix.EOF) { Console.WriteLine($"ERROR! Record {want} not found"); goto closure; } // I hate the "goto" command but in this particular case it's the safest way to go! (No I do miss the "defer" keyword Go has!)
					tag = bix->ReadByte();
					if (tag != 0) { QCol->Error(TrSPrintF("Unknown index command tag #%d!",tag));  goto closure; }					cnt++;

					Index = Azor_Index();
					Index.id = bix->ReadInt(); high = std::max(Index.id, high);
					Index.size = bix->ReadLong();
					Index.offset = bix->ReadLong();
					if (Indexes.count(Index.id)) QCol->Warn(TrSPrintF("Duplicate index #%d. One of them will overwrite another", Index.id));
					Indexes[Index.id] = Index;
					//einde = offset + size;
				} //while (id != want);//(id < 0 || id < min || id > max);
				closure:
				QCol->Doing("Records", cnt);
				QCol->Doing("Highest", high);
			}
			_Using = pname;			

		}
		_Azor_Project::~_Azor_Project() { QCol->Doing("Closing", pname); QCol->Reset(); }
		Azor_Project _Azor_Project::Use(std::string _pname) {
			if (_PrjReg.count(Upper(_pname))) return _PrjReg[_pname];
			auto fname = ProjectPath() + "/" + _pname + ".azor";
			if (!FileExists(fname)) {
				QCol->Error("Azor project not found!");
				if (FileExists(StripExt(fname) + ".prj")) {
					QCol->LMagenta("An old devlog project with that name was found. Try to convert it!\n");
				}
				return nullptr;
			}
			//_Azor_Project* np{ new _Azor_Project(_pname) };
			//auto ret{ std::shared_ptr<_Azor_Project>(np) };
			auto ret{ std::make_shared<_Azor_Project>(_pname) };
			if (ret.get() == nullptr) { QCol->Error("Shared null for project!"); exit(500); }
			_PrjReg[Upper(_pname)] = ret;
			return ret;
		}

		Azor_Project _Azor_Project::Current() {
			if (!_Using.size()) return nullptr;
			if (!_PrjReg.count(Upper(_Using))) {
				QCol->Error("No project on: " + _Using);
				return nullptr;
			}
			return _PrjReg[Upper(_Using)];
		}

		std::vector<std::string>* _Azor_Project::Tags() {
			 auto r{ RawConfig->List("Lists", "Tags") }; 
			 return r; 
		}

		std::vector<std::string>* _Azor_Project::Prefixes() {
			auto r{ RawConfig->List("Lists","CDPREFIXES") };
			return r;
		}

		Azor_CDPrefix _Azor_Project::Prefix(std::string tag) {
			auto ft{ "CDP:" + tag };
			Chat("Getting prefix data for ", ft);
			Azor_CDPrefix ret{ RawConfig->Value(ft,"Prefix"),(uint32)RawConfig->IntValue(ft,"Reset"),(uint32)RawConfig->IntValue(ft,"CD") };
			Chat("- Returning");
			return ret;
		}

		String Slyvina::Azor::_Azor_Project::Name() { return pname; }

		void _Azor_Project::SaveIndexes() {
			auto bix{ WriteFile(ProjectIndexFile()) };
			for (auto idxv : Indexes) {
				auto& idx{ idxv.second };
				if (idx.valid) {
					bix->Write('0');
					bix->Write(idx.id);
					bix->Write(idx.size);
					bix->Write(idx.offset);
				}
			}
			bix->Close();
		}

		Azor_Index& _Azor_Project::Index(int idx) {
			if (Indexes.count(idx))
				return Indexes[idx];
			Indexes[idx].valid = false;
			return Indexes[idx];
		}

		String _Azor_Project::ParsePure(String pure) {
			auto words{ Split(pure,' ') };
			auto cp{ this }; // = dvProject.Get(GUI.CurrentProjectName);
			String content = "";
			if (cp == nullptr) { throw std::runtime_error("Internal error! Parse attempt from a null project");/* return ""; */}
			for(String word : *words) {
				auto hashtag = IndexOf(word,'#');
				auto newword = word;
				if (Prefixed(word, "##")) {
					newword = StReplace(word.substr(1),"##", "#");
				} else if (hashtag >= 0 && ToInt(word.substr(hashtag + 1)) > 0) {
					auto rsplit = Split(word,'#');
					//if (Regex.Match(word, "##").Success) { newword = word.Replace("##", "#"); } else if (rsplit.Length > 2) { newword = word.Replace("##", "#"); } else if (qstr.Left(word, 1) == "#") {
					if (StrContains(word,"##")) { newword = StReplace(word,"##", "#"); } else if (rsplit->size() > 2) { newword = StReplace(word,"##", "#"); } else if (Left(word, 1) == "#") {
							if (cp->GitHub() != "") {
								newword = "<a href='http://github.com/" + cp->GitHub() + "/issues/" + Right(word, word.size() - 1) + "'>" + word + "</a>";
						} else {
							newword = word;
							QCol->Warn("No github repository to refer to!");
						}
					} else {
						newword = "<a href='http://github.com/" + (*rsplit)[0] + "/issues/" + (*rsplit)[1] + "'>" + word + "</a>";
					}
				} else if (Left(word, 1) == "$") {
					auto p{ IndexOf(word,':') };
					//newword = cp.Data.C($"VAR.{word.Substring(1)}");
					if (p >= 1) {
						newword = RawConfig->Value(word.substr(0, p), word.substr(p + 1));
					} else newword = word;
				}
				if (content != "") content += " ";
				content += newword;
			}
			
			return content;		
		}

		bool _Azor_Project::HasTag(String Tag) {
			auto l{ RawConfig->List("LISTS","TAGS") };
			return VectorContains(*l, Upper(Tag));
		}

		void _Azor_Project::NewTag(String Tag) {
			QCol->Error("NewTag not yet implemented");
		}

		int _Azor_Project::HighIndex() {
			auto ret{ 0 };
			for (auto r : Indexes) ret = std::max(ret, r.first);
			return ret;
		}

		void _Azor_Entry::GetMe(InFile stream) {
			if (parent->Indexes.count(index)) {
				auto Idx{ &(parent->Indexes[index]) };
				core.clear();
				stream->Position(Idx->offset);
				while (stream->Position() <= Idx->offset + Idx->size) {
					auto tag{ stream->ReadByte() }; if (tag != 0) { QCol->Error(TrSPrintF("Unknown content tag: %d", tag)); return; }
					auto key{ stream->ReadString() }, value{ stream->ReadString() };
					core[key] = value;
				}
			}
		}

		void _Azor_Entry::GetMe() {
			auto rf{ ReadFile(parent->ProjectContentFile()) };
			GetMe(rf);
			rf->Close();
		}

		void _Azor_Entry::Pure(std::string a) {
			core["PURE"] = a;
			core["TEXT"] = parent->ParsePure(a);
			modified = true;
		}

		String _Azor_Entry::Pure() { return core["PURE"]; }
		String _Azor_Entry::Text() { return core["TEXT"]; }

		void _Azor_Entry::Tag(std::string nt, bool createifnotexistent) {
			if (!parent->HasTag(nt)) {
				if (createifnotexistent) parent->NewTag(nt); else { QCol->Error("Tag " + nt + " not existent!"); return; }
			}
			core["TAG"] = nt;
		}

		String _Azor_Entry::Tag() { return core["TAG"]; }



		String Using() {
			return _Using;
		}

		String Prompt() {
			if (!_Using.size()) return "? No Project ? >";
			return _Using + ">";
		}


#pragma region "Project commands"
#define NoProject if (_Using == "") { QCol->Error("No Project!"); return; }
		static void pcmd_use(carg a) {
			for (auto arg : a) _Azor_Project::Use(arg);
		}

		static void pcmd_tags(carg) {
			NoProject;
			auto c{ _Azor_Project::Current() };
			auto t{ c->Tags() };
			for (auto tag : *t) {
				QCol->Pink(tag + "\n");
			}
		}

		static void pcmd_prefixes(carg f) {
			NoProject;
			static size_t prnname{ 0 };
			auto c{ _Azor_Project::Current() }; Chat("Got Project");
			auto full{ false }; Chat("Full preset: false");
			auto prlist{ c->Prefixes() }; Chat("Prefix tags received");
			for (auto ch : f) full = full || ch[0] == 'F' || ch[0] == 'f'; Chat("Full = " << boolstring(full));
			for (auto prt : *prlist) {
				if (prt.size()) {
					Chat("Checking: "<< prt);
					prnname = std::max(prnname, prt.size()); Chat("Tab: " << prnname);
					auto pf{ c->Prefix(prt) };
					QCol->LGreen(TabStr(prt, prnname + 1));
					QCol->Pink(TrSPrintF("%9d/%9d ", pf.CD, pf.Reset));
					if (full) QCol->Yellow(pf.Prefix);
					QCol->Grey("\n");
				}
#ifdef PRJ_DEBUG
				else Chat("prt is an empty string");
#endif
			}
		}

		void ProjectCommands() {
			RegCommand("Use", pcmd_use);
			RegCommand("Tags", pcmd_tags);
			RegCommand("Prefixes", pcmd_prefixes);
		}
#pragma endregion
}
}
