; installers/windows/installer.iss
[Setup]
AppName=PawMage
AppVersion=1.0.0
AppPublisher=justdev-chris
DefaultDirName={autopf}\PawMage
DefaultGroupName=PawMage
Compression=lzma2
SolidCompression=yes
OutputDir=Output
OutputBaseFilename=PawMage-Setup
WizardStyle=modern
SetupIconFile=..\..\images\ico\PawMageIcon.ico
UninstallDisplayIcon={app}\PawMageViewerIcon.ico
ArchitecturesInstallIn64BitMode=x64
PrivilegesRequired=admin

[Types]
Name: "full"; Description: "Full installation"
Name: "custom"; Description: "Custom installation"; Flags: iscustom

[Components]
Name: "core"; Description: "PawMage Core Library (required)"; Types: full custom; Flags: fixed
Name: "convert"; Description: "CLI Converter (pawm-convert)"; Types: full custom
Name: "viewer"; Description: "GUI Viewer (pawm-view)"; Types: full custom

[Files]
; Core (libpawm)
Source: "..\..\libpawm\libpawm.dll"; DestDir: "{app}"; Components: core
Source: "..\..\libpawm\libpawm.a"; DestDir: "{app}"; Components: core
Source: "..\..\libpawm\include\pawm.h"; DestDir: "{app}\include"; Components: core

; CLI Converter
Source: "..\..\pawm-convert\pawm-convert.exe"; DestDir: "{app}"; Components: convert

; GUI Viewer
Source: "..\..\pawm-view\dist\pawm-view.exe"; DestDir: "{app}"; Components: viewer
Source: "..\..\images\ico\PawMageIcon.ico"; DestDir: "{app}"; Components: viewer
Source: "..\..\images\ico\PawMageConverterIcon.ico"; DestDir: "{app}"; Components: convert
Source: "..\..\images\ico\PawMageViewerIcon.ico"; DestDir: "{app}"; Components: viewer

[Icons]
Name: "{group}\PawMage CLI Converter"; Filename: "{app}\pawm-convert.exe"; Components: convert
Name: "{group}\PawMage GUI Viewer"; Filename: "{app}\pawm-view.exe"; Components: viewer
Name: "{group}\Uninstall PawMage"; Filename: "{uninstallexe}"

[Registry]
; .pawm file association
Root: HKCR; Subkey: ".pawm"; ValueType: string; ValueName: ""; ValueData: "PawMage.Image"; Flags: uninsdeletevalue
Root: HKCR; Subkey: "PawMage.Image"; ValueType: string; ValueName: ""; ValueData: "PawMage Image File"; Flags: uninsdeletekey
Root: HKCR; Subkey: "PawMage.Image\DefaultIcon"; ValueType: string; ValueName: ""; ValueData: "{app}\PawMageViewerIcon.ico,0"
Root: HKCR; Subkey: "PawMage.Image\shell\open\command"; ValueType: string; ValueName: ""; ValueData: """{app}\pawm-view.exe"" ""%1"""

[Run]
Filename: "{app}\pawm-convert.exe"; Description: "Test CLI Converter"; Components: convert; Flags: postinstall nowait skipifsilent
Filename: "{app}\pawm-view.exe"; Description: "Launch PawMage GUI Viewer"; Components: viewer; Flags: postinstall nowait skipifsilent