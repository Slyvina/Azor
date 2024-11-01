import platform
from datetime import date
from os import system
from os import environ

Today = date.today().strftime("%b %d %y")

print("\x1b[32mAzor\x1b[33m\tBuilder script\x1b[0m\n(c) Jeroen P. Broks\n")
def Doing(a,b): print("\x1b[93m%s: \x1b[96m%s\x1b[0m"%(a,b))
Doing("Platform",platform.system())
print("\n\n")

# Base Functions
Files = []
Libs = []

def Add(cat,files):
    for f in files: 
        Doing(cat,f)
        Files.append(f)

def AddLib(cat,files):
    for f in files: 
        Doing("Lib "+cat,f)
        Libs.append(f)

Add("Unit",[		
    "../../Libs/Units/Source/SlyvAsk.cpp",
    "../../Libs/Units/Source/SlyvBank.cpp",
    "../../Libs/Units/Source/SlyvDir.cpp",
    "../../Libs/Units/Source/SlyvDirry.cpp",
    "../../Libs/Units/Source/SlyvMD5.cpp",
    "../../Libs/Units/Source/SlyvOpenURL.cpp",
    "../../Libs/Units/Source/SlyvQCol.cpp",
    "../../Libs/Units/Source/SlyvRoman.cpp",
    "../../Libs/Units/Source/SlyvSTOI.cpp",
    "../../Libs/Units/Source/SlyvStream.cpp",
    "../../Libs/Units/Source/SlyvString.cpp",
    "../../Libs/Units/Source/SlyvTime.cpp",
    "../../Libs/Units/Source/SlyvVolumes.cpp"])
Add("Azor",[    
    "Azor/Azor.cpp",
    "Azor/Azor_CommandRegister.cpp",
    "Azor/Azor_Config.cpp",
    "Azor/Azor_Project.cpp"])

Program("exe/%s/azor"%platform.system().lower(),Files,CPPPATH="../../Libs/Units/Headers")