; installers/windows/installer.iss
[Setup]
AppName=PawMage
AppVersion=1.0
DefaultDirName={pf}\PawMage
DefaultGroupName=PawMage
Compression=lzma2
SolidCompression=yes
OutputDir=output
OutputBaseFilename=PawMage-Setup
WizardStyle=modern
UninstallDisplayIcon={app}\PawMageViewerIcon.ico
SetupIconFile=..\..\images\ico\PawMageIcon.ico

[Types]
Name: "custom"; Description: "Select components"; Flags: iscustom

[Components]
Name: "libpawm"; Description: "PawMage Core Library (required)"; Types: custom; Flags: fixed
Name: "convert"; Description: "PawMage CLI Converter (command-line tool)"; Types: custom
Name: "viewer"; Description: "PawMage GUI Viewer (desktop app)"; Types: custom

[Files]
Source: "..\..\libpawm\libpawm.dll"; DestDir: "{app}"; Components: libpawm
Source: "..\..\libpawm\libpawm.a"; DestDir: "{app}"; Components: libpawm
Source: "..\..\libpawm\include\pawm.h"; DestDir: "{app}\include"; Components: libpawm
Source: "..\..\pawm-convert\pawm-convert.exe"; DestDir: "{app}"; Components: convert
Source: "..\..\pawm-view\dist\pawm-view.exe"; DestDir: "{app}"; Components: viewer
Source: "..\..\images\ico\PawMageIcon.ico"; DestDir: "{app}"; Components: viewer
Source: "..\..\images\ico\PawMageConverterIcon.ico"; DestDir: "{app}"; Components: convert
Source: "..\..\images\ico\PawMageViewerIcon.ico"; DestDir: "{app}"; Components: viewer

[Icons]
Name: "{group}\PawMage CLI Converter"; Filename: "{app}\pawm-convert.exe"; Components: convert; IconFilename: "{app}\PawMageConverterIcon.ico"
Name: "{group}\PawMage GUI Viewer"; Filename: "{app}\pawm-view.exe"; Components: viewer; IconFilename: "{app}\PawMageViewerIcon.ico"
Name: "{group}\Uninstall PawMage"; Filename: "{uninstallexe}"

[Registry]
; File association for .pawm
Root: HKA; Subkey: "Software\Classes\.pawm"; ValueType: string; ValueName: ""; ValueData: "PawMage.Image"; Flags: uninsdeletevalue
Root: HKA; Subkey: "Software\Classes\PawMage.Image"; ValueType: string; ValueName: ""; ValueData: "PawMage Image"; Flags: uninsdeletekey
Root: HKA; Subkey: "Software\Classes\PawMage.Image\DefaultIcon"; ValueType: string; ValueName: ""; ValueData: "{app}\PawMageViewerIcon.ico,0"
Root: HKA; Subkey: "Software\Classes\PawMage.Image\shell\open\command"; ValueType: string; ValueName: ""; ValueData: """{app}\pawm-view.exe"" ""%1"""

[Run]
Filename: "{app}\pawm-convert.exe"; Description: "Test PawMage CLI Converter"; Components: convert; Flags: postinstall nowait
Filename: "{app}\pawm-view.exe"; Description: "Launch PawMage GUI Viewer"; Components: viewer; Flags: postinstall nowait
