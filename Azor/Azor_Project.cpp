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
// Version: 24.11.01
// End License

#include "Azor_Project.hpp"
#include "Azor_CommandRegister.hpp"
#include "Azor_Config.hpp"
#include <SlyvQCol.hpp>
#include <SlyvTime.hpp>
#include <SlyvRandom.hpp>
#include <SlyvRoman.hpp>

using namespace std;

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
					if (tag != 0) { QCol->Error(TrSPrintF("Unknown index command tag #%d (position %x)!",tag,bix->Position()-1));  goto closure; }					cnt++;

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
		_Azor_Project::~_Azor_Project() { QCol->Doing("Closing", pname); RawConfig->Value("Azor", "Closed", CurrentDate() + "; " + CurrentTime()); QCol->Reset(); }
		Azor_Project _Azor_Project::Use(std::string _pname) {
			if (_PrjReg.count(Upper(_pname))) { _Using = _pname; return _PrjReg[Upper(_pname)]; }
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
			Chat("Getting prefix data for " << ft);
			Azor_CDPrefix ret{ RawConfig->Value(ft,"Prefix"),(uint32)RawConfig->IntValue(ft,"Reset"),(uint32)RawConfig->IntValue(ft,"CD") };
			Chat("- Returning");
			return ret;
		}

		void _Azor_Project::Prefix(std::string tag, Azor_CDPrefix pref, bool noautosave) {
			auto TAS{ RawConfig->AutoSave };
			if (noautosave) RawConfig->AutoSave = "";
			auto ft{ "CDP:" + tag };
			Chat("Setting prefix data for ", ft);
			RawConfig->Value(ft, "Prefix", pref.Prefix);
			RawConfig->Value(ft, "CD", to_string(pref.CD));
			RawConfig->Value(ft, "Reset", to_string(pref.Reset));
			RawConfig->AutoSave = TAS;
			RawConfig->AutoSave = TAS;
		}

		String Slyvina::Azor::_Azor_Project::Name() { return pname; }

		void _Azor_Project::SaveIndexes() {
			auto bix{ WriteFile(ProjectIndexFile()) };
			for (auto idxv : Indexes) {
				auto& idx{ idxv.second };
				if (idx.valid) {
					bix->Write('\0');
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

		void _Azor_Project::NewTag(String atag) {
			auto tag{ Trim(Upper(atag)) };
			auto CurrentProject{ this };
			if (tag == "") { QCol->Error("No Tag!"); return; } else if (CurrentProject == nullptr) { QCol->Error("No project!"); } else if (IndexOf(tag, ' ') >= 0) { QCol->Error("Invalid tag!"); } else if (CurrentProject->HasTag(tag)) { QCol->Error("Ttag " + tag + " already exists!"); } else {
				auto
					FMin = FColMin(),
					FMax = FColMax(),
					BMin = BColMin(),
					BMax = BColMax();
				auto
					FR = Rand.Get(FMin.R, FMax.R),
					FG = Rand.Get(FMin.G, FMax.G),
					FB = Rand.Get(FMin.B, FMax.B),
					BR = (int)(FR / 20),
					BG = (int)(FG / 20),
					BB = (int)(FB / 20);
				if (BMax.R > 0) BR = Rand.Get(BMin.R, BMax.R);
				if (BMax.G > 0) BG = Rand.Get(BMin.G, BMax.G);
				if (BMax.B > 0) BB = Rand.Get(BMin.B, BMax.B);
				RawConfig->Add("LISTS", "TAGS", tag, true, true); //CurrentProject.Data.Add("TAGS", tag);
				RawConfig->Value("Tag:" + tag, "Head", TrSPrintF("background-color:rgb(0,0,0); color:rgb(%d,%d,%d);", FR, FG, FB));
				RawConfig->Value("Tag:" + tag, "Content", TrSPrintF("background-color:rgb(%d,%d,%d); color:rgb(%d,%d,%d);", BR, BG, BB, FR, FG, FB));
				QCol->Doing("Added tag", tag);
				AddEntry("SITE","Added tag "+tag,true);
				//GUI.UpdateTags();
			}
		}

		int _Azor_Project::HighIndex() {
			auto ret{ 0 };
			for (auto& r : Indexes) ret = std::max(ret, r.first);
			return ret;
		}

		int _Azor_Project::CountRecords() {
			auto ret{ 0 };
			for (auto& r : Indexes) if (r.second.valid) ret++;
			return ret;
		}

		void _Azor_Project::List(int start, int end) {
			static auto tagtab{ 0 };
			String olddate{ "" };
			if (start > end) {
				QCol->Error("Invalid input for list!");
				return;
			}
			for (auto& ti : Indexes) {
				if (ti.first >= start && ti.first <= end) {
					auto ent{ Entry(ti.first) };
					if (!ent) {
						QCol->Error(TrSPrintF("Null pointer received for entry #%d", ti.first));
						continue;
					}
					if (olddate != ent->Date()) {
						QCol->LGreen(ent->Date()+"\n");
						olddate = ent->Date();
					}
					QCol->Cyan(TrSPrintF("%9d ", ti.first));
					tagtab = std::max(tagtab, (int)ent->Tag().size());
					QCol->LMagenta(TabStr(ent->Tag(), tagtab + 1));
					QCol->Yellow(ent->Pure() + "\n");
				}
			}
		}

		Azor_Entry _Azor_Project::Entry(int i) {
			if (!Indexes.count(i)) { QCol->Error("Unidentified entry index"); return nullptr; }
			return std::make_shared<_Azor_Entry>(this, i);
		}

		Azor_Color _Azor_Project::FColMin() {
			Azor_Color ret{ (Byte)RawConfig->NewValue("FColMin","R",127),(Byte)RawConfig->NewValue("FColMin","G",127),(Byte)RawConfig->NewValue("FColMin","B",127),RawConfig->NewValue("FColMin","H",0),(Byte)RawConfig->NewValue("FColMin","S",100),(Byte)RawConfig->NewValue("FColMin","V",100) };
			return ret;
		}

		Azor_Color _Azor_Project::BColMin() {
			Azor_Color ret{ (Byte)RawConfig->NewValue("BColMin","R",0),(Byte)RawConfig->NewValue("BColMin","G",0),(Byte)RawConfig->NewValue("BColMin","B",0),RawConfig->NewValue("BColMin","H",0),(Byte)RawConfig->NewValue("BColMin","S",100),(Byte)RawConfig->NewValue("BColMin","V",0) };
			return ret;
		}
		Azor_Color _Azor_Project::FColMax() {
			Azor_Color ret{ (Byte)RawConfig->NewValue("FColMax","R",255),(Byte)RawConfig->NewValue("FColMax","G",255),(Byte)RawConfig->NewValue("FColMax","B",255),RawConfig->NewValue("FColMax","H",0),(Byte)RawConfig->NewValue("FColMax","S",100),(Byte)RawConfig->NewValue("FColMax","V",100) };
			return ret;
		}

		Azor_Color _Azor_Project::BColMax() {
			Azor_Color ret{ (Byte)RawConfig->NewValue("BColMax","R",0),(Byte)RawConfig->NewValue("BColMax","G",0),(Byte)RawConfig->NewValue("BColMax","B",0),RawConfig->NewValue("BColMax","H",0),(Byte)RawConfig->NewValue("BColMax","S",100),(Byte)RawConfig->NewValue("BColMax","V",50) };
			return ret;
		}

		void _Azor_Project::AddEntry(String Tag, String Content, bool forcenewtag) {
			if (forcenewtag && (!HasTag(Tag))) NewTag(Tag);
			else if (!HasTag(Tag)) { QCol->Error("Tag \"" + Tag + "\" doesn't exist!"); return; }			
			auto ent{ std::make_shared<_Azor_Entry>(this) };
			auto ActContent{ Content };
			auto CDPA{ Prefixes() };
			for (auto& CDP : *CDPA) {
				auto CD{ Prefix(CDP) };
				CD.CD--;
				if (CD.CD <= 0) {
					CD.Prefix = StReplace(CD.Prefix,"\\\"", "\"");
					ActContent = CD.Prefix + ActContent;
					CD.CD += CD.Reset;
				}
				Prefix(CDP, CD, true);
			}		
			ent->Pure(ActContent);
			ent->Tag(Tag);
			ent->UpdateMe();
			List(ent->ID());
			AutoPush--;
			if (AutoPush > 1) { QCol->Yellow("Azor requires "); QCol->Cyan(std::to_string(AutoPush)); QCol->Yellow(" more entries before the autopush.\n"); }
			else if (AutoPush == 1) { QCol->Yellow("Azor requires "); QCol->Cyan("1"); QCol->Yellow(" more entry before the autopush.\n"); }
			else {
				Generate();
				Push();
				AutoPush += 10;
			}
		}

		void _Azor_Project::Generate(bool andpush) {
			//QCol->Error("Generating pages has not yet been implemented!");
			String html_template;
			auto cp{ this }; if (cp == nullptr) { QCol->Error("GEN: No project!"); return; }
			/* C#
			try {
				System.IO.Directory.CreateDirectory(OutDir);
			} catch (Exception e) {
				GUI.WriteLn($"GEN: {e.Message}");
				return;
			}
			//*/
			// C++
			MakeDir(OutDir());
			int pages = cp->CountRecords() / 200;
			int page = 1;
			int pcountdown = 200;
			bool justnewpaged = false;
			String olddate{ "____OLD____" };
			String iconext[4]  {"png", "gif", "svg", "jpg"};
			if (cp->CountRecords() % 200 > 0) pages++;
			try {
				html_template = FLoadString(Trim(cp->Template()));
			} catch (std::runtime_error e) {
				QCol->Error("html_template "+cp->Template() + " could not be properly loaded!");
				QCol->LMagenta(String(e.what()) + "\n");
				//GUI.WriteLn($"GEN:html_template {cp.html_template} could not be properly loaded!");
				return;
			}
			QCol->Yellow("Exporting...");
			String pageline = "";
			for (int p = 1; p <= pages; p++) {
				if (page == p) pageline += TrSPrintF("<big><big>%d</big></big> ", p); else pageline += "<a href='" + cp->ProjectName() + TrSPrintF("_DevLog_Page_%d.html'>%d</a> ", p, p);
			}
			pageline = Trim(pageline);
			String content{ TrSPrintF("<table style=\"width:%d\">\n",ExportTableWidth()) };
			content += "<tr><td colspan=3 align=center>" + pageline + "</td></tr>\n";
			for (int i = HighIndex(); i > 0; i--) {
				//if (i % 6 == 0) { GUI.Write("."); Console.Write($".{i}."); }
				justnewpaged = false;
				if (!Indexes.count(i)) continue;
				auto rec = Entry(i); //var rec = new dvEntry(cp, i, true);
				if (rec) {
					if (rec->Date() != olddate) content += "<tr><td align=center colspan=3 style='font-size:30pt;'>- = " + rec->Date() + " = -</td></tr>\n"; olddate = rec->Date();
					string headstyle = RawConfig->Value("Tag:" + rec->Tag(), "HEAD"); //cp.Data.C($"HEAD.{rec.Tag.ToUpper()}");
					string contentstyle =  RawConfig->Value("Tag:" + rec->Tag(), "CONTENT"); //cp.Data.C($"INHD.{rec.Tag.ToUpper()}");
					content += TrSPrintF("<tr valign=top><td align=left><a id='dvRec_%d'></a>", rec->ID()) + rec->Time() + "</td><td style=\"" + headstyle + "\">" + rec->Tag() + TrSPrintF("</td><td style = 'width: %d; ", ExportContentWidth()) + contentstyle + TrSPrintF("'><div style = \"width: %d; overflow-x:auto;\">", ExportContentWidth());
					auto alticon = rec->AltIcon();
					if (alticon == "") {
						String
							icon{ OutDir() + "/Icons/" + Lower(rec->Tag()) },
							neticon{ "Icons/" + Lower(rec->Tag()) };
						icon = StReplace(icon, "#", "hashtag");
						neticon = StReplace(neticon, "#", "%23");
						for(String pfmt : iconext) {
							String iconfile{ icon + "." + pfmt };
							iconfile = StReplace(iconfile,"#", "%23");
							if (FileExists(iconfile)) { content += "<img style='float:" + IconFloat() + TrSPrintF("; height: %d' src='", IconHeight()) + neticon + "." + pfmt + "' alt = '" + rec->Tag() + "' > "; break; }
						}
					} else {
						//content.Append($"<img style='float:{cp.GetDataDefault("EXPORT.ICONFLOATPOSITION", "Right")}; height:{cp.GetDataDefaultInt("EXPORT.ICONHEIGHT", 50)};'  src='{alticon}' alt='{rec.Tag}'>"); 
						content += "<img style='float:" + IconFloat() + TrSPrintF(";' height='%d' src='", IconHeight()) + alticon + "' alt='" + rec->Tag() + "' />";
					}
					content += rec->Text() + "</div></td></tr>\n";
					pcountdown--;
					if (pcountdown <= 0) {
						pcountdown = 200;
						justnewpaged = true;
						content += "<tr><td colspan=3 align=center>" + pageline + "</td></tr>\n</table>\n\n";
						SaveString(OutDir() + "/" + ProjectName() + TrSPrintF("_DevLog_Page_%d.html", page), StReplace(html_template, "@CONTENT@", content));
						page++;
						pageline = "";
						for (int p = 1; p <= pages; p++) {
							if (page == p) pageline += TrSPrintF("<big><big>%d</big></big> ", p); else pageline += "<a href='" + cp->ProjectName() + TrSPrintF("_DevLog_Page_%d.html'>%d</a> ", p, p);
						}
						content = TrSPrintF("<table style=\"width:%d\">\n",ExportTableWidth());
						content+="<tr><td colspan=3 align=center>"+pageline+"</td></tr>\n";
						QCol->Pink(".");
					}
				}
			}
			if (!justnewpaged) {
				content += "<tr><td colspan=3 align=center>" + pageline + "</td></tr>\n";
				SaveString(OutDir() + "/" + ProjectName() + TrSPrintF("_DevLog_Page_%d.html", page), StReplace(html_template, "@CONTENT@", content));
			}
			QCol->Cyan(" Done\n");
			//Console.WriteLine(" GENERATED");		
		}

		void _Azor_Project::Push() {
			//QCol->Error("Pushing has not yet been implemented");
			auto PWD{ CurrentDir() };
			ChangeDir(OutDir());
			auto Cnt{ RawConfig->IntValue("Count","Pushes") };
			RawConfig->Value("Count", "Pushes", ++Cnt);
			QCol->Doing("Collecting", "Data"); QCol->Reset(); system("git add *");
			QCol->Doing("Committing", "Data"); QCol->Reset(); system(string("git commit -m \"Azor push "+CurrentDate()+"; "+CurrentTime()+TrSPrintF(" Push %d\"",Cnt)).c_str());
			QCol->Doing("Pushing", "Data"); QCol->Reset(); system("git push");
			ChangeDir(PWD);
		}

		void _Azor_Project::SaveRaw() {
			QCol->Doing("Saving",ProjectFileName());
			RawConfig->SaveSource(ProjectFileName(), "Force saved by user on " + CurrentDate() + "; " + CurrentTime());
		}

		void _Azor_Project::Unlink(int victim) {
			if (!Indexes.count(victim))
				QCol->Error(TrSPrintF("I can't unlink non-existent entry #%d", victim));
			else
				Indexes.erase(victim);
		}

		bool _Azor_Project::HasMacro(String Macro, String Platform) { return RawConfig->HasList("MACRO::" + Macro, Platform); }

		std::vector<String> _Azor_Project::Macro(String _Macro, String _Platform) {
			if (HasMacro(_Macro, _Platform)) {
				auto ret{ *RawConfig->List("MACRO::" + _Macro, _Platform) };
				return ret;
			} 
			return std::vector<String>();
		}

		_Azor_Entry::~_Azor_Entry() {
			if (modified) {
				//QCol->Warn("WARNING! Entry has been modified, but there's no update routine here (yet)");
				UpdateMe();
			}
		}

		void _Azor_Entry::GetMe(InFile stream) {
			if (parent->Indexes.count(index)) {
				auto Idx{ &(parent->Indexes[index]) };
				core.clear();
				stream->Position(Idx->offset);
				while (stream->Position() < Idx->offset + Idx->size) {
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
			Trans2Upper(nt);
			if (!parent->HasTag(nt)) {
				if (createifnotexistent) parent->NewTag(nt); else { QCol->Error("Tag " + nt + " not existent!"); return; }
			}
			core["TAG"] = nt;
			modified = true;
		}

		String _Azor_Entry::Tag() { return core["TAG"]; }

		

		_Azor_Entry::_Azor_Entry(_Azor_Project* p) {
			parent = p;
			index = p->HighIndex() + 1;
			core["DATE"] = CurrentDate();
			core["TIME"] = CurrentTime();
			core["TAG"] = "SITE"; 
			//if (!p->HasTag("SITE")) p->NewTag("SITE");
			core["PURE"] = "? Nothing";
			core["TEXT"] = "? Nothing";
			modified = false;
		}

		_Azor_Entry::_Azor_Entry(_Azor_Project* p, int _idx) {
			parent = p;
			index = _idx;
			GetMe();
			if (!core.count("DATE")) core["DATE"] = CurrentDate();
			if (!core.count("TIME")) core["TIME"] = CurrentTime();
			modified = false;
		}

		_Azor_Entry::_Azor_Entry(_Azor_Project* p, String _Tag, String _Pure) {
			parent = p;
			index = p->HighIndex() + 1;
			modified = true;
			Tag(_Tag, true);
			Pure(_Pure);
		}

		void _Azor_Entry::UpdateMe(OutFile stream) {
			auto _index{ &parent->Indexes[index] };
			_index->id = index;
			_index->offset = stream->Size(); //stream->Position();
			_index->valid = true;
			for (auto cent : core) {
				stream->Write('\0');
				stream->Write(cent.first);
				stream->Write(cent.second);
			}
			_index->size = (uint64)stream->Position() - _index->offset;
			QCol->Doing("Saved", TrSPrintF("Entry #%d", _index->id));
		}

		void _Azor_Entry::UpdateMe() {
			auto BT{ AppendFile(parent->ProjectContentFile()) };
			UpdateMe(BT);
			parent->SaveIndexes();
			BT->Close();
			modified = false;
		}



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

		static void pcmd_list(carg f) {
			NoProject;
			auto c{ _Azor_Project::Current() };
			if (f.size() == 0) { c->List(c->HighIndex() - 25, c->HighIndex()); return; }
			for (auto cnd : f) {
				if (Prefixed(cnd, "-")) {
					c->List(0, ToInt(cnd.substr(1))); 
					return;
				}
				if (Suffixed(cnd,"-")) {
					c->List(0, ToInt(Left(cnd,cnd.size()-1)));
					return;
				}
				auto p{ IndexOf(cnd,'-') };
				if (p < 0) c->List(ToInt(cnd)); else c->List(ToInt(cnd.substr(0, p)), ToInt(cnd.substr(p + 1)));
			}
		}

		static void pcmd_addtag(carg f) {
			NoProject;
			auto c{ _Azor_Project::Current() };
			if (f.size() == 0) { QCol->Error("No tags"); return; }
			for (auto tag : f) c->NewTag(tag);
		}

		static void pcmd_add(carg f) {
			NoProject;
			auto c{ _Azor_Project::Current() };
			if (f.size() < 2) { QCol->Error("Not enough imput for adding"); return; }
			auto nt{ f[0] };
			String ns{ "" };
			for (int i = 1; i < f.size(); ++i) {
				if (ns.size()) ns += " ";
				ns += f[i];
			}
			c->AddEntry(nt, ns);
		}
		static void pcmd_ntadd(carg f) {
			NoProject;
			auto c{ _Azor_Project::Current() };
			if (f.size() < 2) { QCol->Error("Not enough imput for adding"); return; }
			auto nt{ f[0] };
			String ns{ "" };
			for (int i = 1; i < f.size(); ++i) {
				if (ns.size()) ns += " ";
				ns += f[i];
			}
			c->AddEntry(nt, ns, true);
		}

		static void pcmd_indexes(carg f) {
			NoProject;
			auto c{ _Azor_Project::Current() };
			QCol->Reset();
			for (auto& i : c->Indexes)
				//printf("Index %d; offset %d, size %s\n", i.first, (int)i.second.offset, (int)i.second.size);
				std::cout << "Index: " << i.first << "; offset: " << i.second.offset << "; size: " << i.second.size << "\n";
		}
		
		static void pcmd_save(carg f) {
			NoProject;
			_Azor_Project::Current()->SaveRaw();
		}

		static bool isnum(String s) {
			for (int i = 0; i < s.size(); i++) {
				if (s[i] < '0' || s[i]>'9') return false;
			}
			return s.size() > 0;
		}
		static void pcmd_take(carg f) {
			NoProject;
			auto c{ _Azor_Project::Current() };
			String Tag{ "TEST" };
			for (auto a : f) {
				if (c->HasTag(a)) Tag = a;
				else if (isnum(a)) c->Take(ToInt(a));
				else { QCol->Error("Parameter " + a + " has not been understood"); return; }
			}
			c->Take(c->Take() + 1);
			c->AddEntry(Tag, "Take " + ToRoman(c->Take()));
		}
		static void pcmd_gen(carg) {
			NoProject;
			_Azor_Project::Current()->Generate();
		}
		static void pcmd_del(carg victims) {
			NoProject;
			for (auto v : victims) {
				auto vi{ ToInt(v) };
				QCol->Doing("Unlinking", vi);
				_Azor_Project::Current()->Unlink(vi);
			}
		}
		static void pcmd_push(carg) {
			NoProject;
			_Azor_Project::Current()->Push();
		}
		static void pcmd_modify(carg args) {
			NoProject;
			//var args = str.Split(' '); // Not needed!
			//if (CurrentProject == null) { Annoy("No Project!"); return; }
			auto CurrentProject{ _Azor_Project::Current() };
			if (args.size() < 3) { QCol->Error("Modify syntax error!"); return; }
			auto num{ ToInt(args[0]) };
			auto e{ CurrentProject->Entry(num) }; if (!e) return; //var e = new dvEntry(CurrentProject, num, true);
			//if (e == null) { Annoy("Entry couldn't be accessed!"); return; }
			auto tag{ Upper(args[1]) };
			if (!CurrentProject->HasTag(tag)) { QCol->Error("There's no tag: "+tag); return; }
			String sb{ "" };//var sb = new System.Text.StringBuilder();
			e->Tag(tag);
			for (int i = 2; i < args.size(); ++i) sb+=args[i]+" ";
			e->Pure(Trim(sb));
			//GUI.UpdateEntries(CurrentProject.HighestRecordNumber - 200, CurrentProject.HighestRecordNumber);
		}

		void ProjectCommands() {
			RegCommand("Use", pcmd_use);
			RegCommand("Tags", pcmd_tags);
			RegCommand("Prefixes", pcmd_prefixes);
			RegCommand("List", pcmd_list);
			RegCommand("AddTag", pcmd_addtag);
			RegCommand("NewTag", pcmd_addtag);
			RegCommand("Add", pcmd_add);
			RegCommand("ntAdd", pcmd_ntadd);
			RegCommand("Indexes", pcmd_indexes);
			RegCommand("Save", pcmd_save);
			RegCommand("take", pcmd_take);
			RegCommand("gen", pcmd_gen);
			RegCommand("unlink", pcmd_del);
			RegCommand("delete", pcmd_del);
			RegCommand("del", pcmd_del);
			RegCommand("erase", pcmd_del);
			RegCommand("Push", pcmd_push);
			RegCommand("Modify", pcmd_modify);
		}
#pragma endregion
}
}
