[Setup]
AppName=Embedded Debug Toolbox
AppVersion=1.0.0
AppPublisher=Embedded Debug Toolbox
DefaultDirName={autopf}\Embedded Debug Toolbox
DefaultGroupName=Embedded Debug Toolbox
OutputBaseFilename=EmbeddedDebugToolbox_Setup
Compression=lzma
SolidCompression=yes
ArchitecturesAllowed=x64
ArchitecturesInstallIn64BitMode=x64

[Languages]
Name: "Chinese"; MessagesFile: "compiler:Languages\Chinese.isl"

[Files]
Source: "dist\*"; DestDir: "{app}"; Flags: recursesubdirs ignoreversion

[Icons]
Name: "{group}\Embedded Debug Toolbox"; Filename: "{app}\EmbeddedDebugToolbox.exe"
Name: "{autodesktop}\Embedded Debug Toolbox"; Filename: "{app}\EmbeddedDebugToolbox.exe"; Tasks: desktopicon

[Tasks]
Name: "desktopicon"; Description: "Create a &desktop icon"; GroupDescription: "Additional icons:"
