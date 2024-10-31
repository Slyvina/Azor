// License:
// 
// Azor
// Devlog to Azor converter
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
using System.Linq.Expressions;
using System.Net.Quic;
using TrickyUnits;

class MainProgramForAzorFromDevLog {
	static string GlobConfigFile => Dirry.C("$AppSupport$/Azor.GlobalConfig.ini");
	private static GINIE _gc = null;
	static GINIE GlobConfig {
		get {            
			if (_gc == null) {
				if (!File.Exists(GlobConfigFile)) throw new Exception($"Global config file {GlobConfigFile} doesn't exist!");
				QCol.Doing("Loading", GlobConfigFile);
				_gc = GINIE.FromFile(GlobConfigFile);
				if (_gc == null) throw new Exception($"Loading global config {GlobConfigFile} failed!");
			}
			return _gc;
		}
	}
	static string ProjectPath {
		get {
			if (GlobConfig["Project", "Directory"] == "") throw new Exception("Project directory is not known");
			return GlobConfig["Project", "Directory"];
		}
	}


	readonly string
		Project,
		OudFile,
		NieuwFile;
	readonly TGINI Oud = null;
	readonly GINIE Nieuw = null;

	MainProgramForAzorFromDevLog(int i, int mi, string prj) {
		Project = prj;
		QCol.Green($"{i}/{mi}\t");
		QCol.Doing("Converting", prj);
		OudFile = $"{ProjectPath}/{prj}.prj";
		NieuwFile = $"{ProjectPath}/{prj}.azor";
		if (!File.Exists(OudFile)) { QCol.QuickError($"Can't find {OudFile}"); return; }
		if (File.Exists(NieuwFile)) {
			QCol.Red("WARNING!! ");
			QCol.Yellow($"{NieuwFile} already exists! Overwrite it ? ");
			do {
				var k = Console.ReadKey(true);
				switch (k.Key) {
					case ConsoleKey.Y:
						QCol.Green("Yes\n");
						goto YesSir;
					case ConsoleKey.N:
						QCol.Red("No!");
						return;
				}
			} while (true);
		YesSir:;
		}
		QCol.Doing("Loading", OudFile);
		Oud = GINI.ReadFromFile(OudFile);
		Nieuw = GINIE.FromSource("# Nothing\n#At all");
		foreach (var v in Oud.Vars()) {
			var p = v.IndexOf('.');
			if (p < 0) {
				switch (v) {
					case "TAKES":
						Nieuw["Count", "Takes"] = Oud[v];
						break;
					case "GITHUBREPOSITORY":
						Nieuw["Github", "Repository"] = Oud[v];
						break;
					default:
						QCol.QuickError($"Devlog variable '{v}' ({Oud[v]}) unrecognized! Ignoring");
						break;
				}
			} else {
				string
					vcl = v.Substring(0, p),
					vfd = v.Substring(p + 1),
					vvl = Oud[v];
				switch (vcl) {
					case "EXPORT":
					case "BCOLMAX":
					case "BCOLMIN":
					case "FCOLMAX":
					case "FCOLMIN":
						Nieuw[vcl, vfd] = vvl;
						break;
					case "HEAD":
					case "ICON":
						if(!Nieuw.List("Lists","Tags").Contains(vfd)) Nieuw.List("Lists", "Tags").Add(vfd);
						Nieuw[$"TAG:{vfd}",vcl] = vvl;
						break;
					case "INHD":
						if (!Nieuw.List("Lists", "Tags").Contains(vfd)) Nieuw.List("Lists", "Tags").Add(vfd);
						Nieuw[$"TAG:{vfd}", "Content"] = vvl;
						break;
					case "TARGET":
					case "TEMPLATE":
						switch (vfd) {
							case "WIN":
							case "WINDOWS":
								Nieuw[vcl, "Windows"] = vvl;
								break;
							case "MAC":
								Nieuw[vcl, "Mac"] = vvl;
								break;
							case "LIN":
							case "LINUX":
								Nieuw[vcl, "Linux"] = vvl;
								break;
							default:
								QCol.QuickError($"Unknown {vcl.ToLower()} platform {vfd}");
								break;
						}
						break;
					case "VAR":
						Nieuw["VAR", vfd] = vvl;
						break;
					default:
						QCol.QuickError($"Class {vcl} unknown, so cannot parse field {vfd} ({vvl})");
						break;
				}
			}
		}
		var tag = "";
		foreach(var pref in Oud.List("CDPREFIX")) if (pref!="" & (!pref.Trim().StartsWith("#"))) {
			var p = pref.IndexOf(":"); if(p<=0) { QCol.QuickError($"Countdown prefix syntax error: {pref}"); continue; }
			var fld = pref.Substring(0, p);
			var val = pref.Substring(p + 1);			
			if (tag=="" && fld!="NEW") { QCol.QuickError($"Countdown field not tagged {pref}"); continue; }
			switch (fld) {
				case "NEW":
					tag = val;
					QCol.Doing("-> CD", tag);
					Nieuw.ListAdd("Lists", "CDPREFIXES", tag);
					break;
				case "CD":
				case "PREFIX":
				case "RESET":
					Nieuw[$"CDP:{tag}",fld]= val;
					break;
				default:
					QCol.QuickError($"Countdown field known {fld} ({val})");
					break;
			}
		}
		QCol.Doing("Saving", NieuwFile);
		Nieuw.SaveSource(NieuwFile);
	}


	static void Main(string[] args) {
		try {
			for (int i = 0; i < args.Length; i++) {
				new MainProgramForAzorFromDevLog(i + 1, args.Length, args[i]);

			}
		} catch (Exception ex) {
			QCol.QuickError(ex.Message);
#if DEBUG
			QCol.Magenta($"{ex.StackTrace}\n");
#endif
		} finally {
			QCol.Cyan("\n\nOk");
			Console.ResetColor();
		}
	}
}

