#ifndef AppVersion
  #define AppVersion "0.0.0"
#endif

#ifndef AppFiles
  #define AppFiles "..\..\dist\windows\RickSheets"
#endif

#ifndef OutputDirectory
  #define OutputDirectory "..\..\dist\windows"
#endif

[Setup]
AppId={{CE33799A-CF70-48D0-A742-BB31B28AB629}
AppName=RickSheets
AppVersion={#AppVersion}
AppVerName=RickSheets {#AppVersion}
AppPublisher=Erik Heidenreich
AppPublisherURL=https://github.com/erixx-hub/RickSheets
AppSupportURL=https://github.com/erixx-hub/RickSheets/issues
AppUpdatesURL=https://github.com/erixx-hub/RickSheets/releases
DefaultDirName={autopf}\RickSheets
DefaultGroupName=RickSheets
DisableProgramGroupPage=yes
LicenseFile=..\..\LICENSE
OutputDir={#OutputDirectory}
OutputBaseFilename=RickSheets-{#AppVersion}-Setup-x64
SetupIconFile=ricksheets.ico
UninstallDisplayIcon={app}\ricksheets.exe
Compression=lzma2/max
SolidCompression=yes
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible
PrivilegesRequired=admin
WizardStyle=modern

[Languages]
Name: "german"; MessagesFile: "compiler:Languages\German.isl"
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked

[Files]
Source: "{#AppFiles}\*"; DestDir: "{app}"; Flags: ignoreversion recursesubdirs createallsubdirs

[Icons]
Name: "{group}\RickSheets"; Filename: "{app}\ricksheets.exe"
Name: "{autodesktop}\RickSheets"; Filename: "{app}\ricksheets.exe"; Tasks: desktopicon

[Registry]
Root: HKA; Subkey: "Software\Classes\.ricksheet"; ValueType: string; ValueName: ""; ValueData: "RickSheets.ChordSheet"; Flags: uninsdeletevalue
Root: HKA; Subkey: "Software\Classes\RickSheets.ChordSheet"; ValueType: string; ValueName: ""; ValueData: "RickSheets Chordsheet"; Flags: uninsdeletekey
Root: HKA; Subkey: "Software\Classes\RickSheets.ChordSheet\DefaultIcon"; ValueType: string; ValueName: ""; ValueData: """{app}\ricksheets.exe"",0"
Root: HKA; Subkey: "Software\Classes\RickSheets.ChordSheet\shell\open\command"; ValueType: string; ValueName: ""; ValueData: """{app}\ricksheets.exe"" ""%1"""

[Run]
Filename: "{app}\ricksheets.exe"; Description: "{cm:LaunchProgram,RickSheets}"; Flags: nowait postinstall skipifsilent
