[Setup]
AppName=Cellyz Phone FX
AppVersion=1.0
AppPublisher=Cellyz Audio
AppPublisherURL=https://github.com/kdnify/cellyz-plugin
AppSupportURL=https://github.com/kdnify/cellyz-plugin
AppUpdatesURL=https://github.com/kdnify/cellyz-plugin
DefaultDirName={autopf}\Cellyz
DefaultGroupName=Cellyz Phone FX
AllowNoIcons=yes
LicenseFile=
InfoBeforeFile=
InfoAfterFile=
OutputDir=.
OutputBaseFilename=Cellyz_Phone_FX_v1.0_Setup
SetupIconFile=
Compression=lzma
SolidCompression=yes
WizardStyle=modern
WizardSizePercent=120
DisableProgramGroupPage=yes
DisableReadyPage=no
DisableFinishedPage=no
ShowLanguageDialog=no
UninstallDisplayIcon={app}\Cellyz.ico

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Files]
; VST3 Plugin - Install to common VST3 directory
Source: "..\artifacts\Cellyz.vst3\*"; DestDir: "{commoncf}\VST3\Cellyz.vst3"; Flags: ignoreversion recursesubdirs createallsubdirs

[Registry]
; Register VST3 plugin
Root: HKLM; Subkey: "SOFTWARE\VST3"; Flags: uninsdeletekeyifempty
Root: HKLM; Subkey: "SOFTWARE\VST3\Cellyz"; Flags: uninsdeletekey
Root: HKLM; Subkey: "SOFTWARE\VST3\Cellyz"; ValueType: string; ValueName: "Path"; ValueData: "{commoncf}\VST3\Cellyz.vst3"

[Icons]
Name: "{group}\Uninstall Cellyz Phone FX"; Filename: "{uninstallexe}"

[Code]
procedure CurStepChanged(CurStep: TSetupStep);
begin
  if CurStep = ssPostInstall then
  begin
    // Nothing special needed for VST3 registration
  end;
end;

function InitializeSetup(): Boolean;
begin
  Result := True;
end;

[Messages]
WelcomeLabel1=Welcome to the Cellyz Phone FX Setup Wizard
WelcomeLabel2=This will install Cellyz Phone FX v1.0 on your computer.%n%nCellyz Phone FX is a professional VST3 plugin featuring authentic phone simulations:%n%n• Nokia Classic Ring Tone Effect%n• iPhone Voice Processing%n• Sony Ericsson Vintage Phone Sound%n%nThe plugin will be installed to your VST3 directory for use in all compatible DAWs.%n%nIt is recommended that you close all other applications before continuing.
ClickFinish=Click Finish to complete the Cellyz Phone FX installation. 