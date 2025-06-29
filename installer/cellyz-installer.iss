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

[InstallDelete]
; Clean up any previous installation
Type: filesandordirs; Name: "{commoncf}\VST3\Cellyz.vst3"

[Files]
; VST3 Plugin - Install to common VST3 directory (automatically detected by DAWs)
Source: "artifacts\Cellyz.vst3\*"; DestDir: "{commoncf}\VST3\Cellyz.vst3"; Flags: ignoreversion recursesubdirs createallsubdirs

[Registry]
; Register VST3 plugin
Root: HKLM; Subkey: "SOFTWARE\VST3"; Flags: uninsdeletekeyifempty
Root: HKLM; Subkey: "SOFTWARE\VST3\Cellyz"; Flags: uninsdeletekey
Root: HKLM; Subkey: "SOFTWARE\VST3\Cellyz"; ValueType: string; ValueName: "Path"; ValueData: "{commoncf}\VST3\Cellyz.vst3"

[Icons]
Name: "{group}\Uninstall Cellyz Phone FX"; Filename: "{uninstallexe}"

[Code]
procedure CurStepChanged(CurStep: TSetupStep);
var
  VST3Path: String;
begin
  if CurStep = ssPostInstall then
  begin
    // Verify VST3 installation
    VST3Path := ExpandConstant('{commoncf}\VST3\Cellyz.vst3');
    if DirExists(VST3Path) then
    begin
      Log('VST3 plugin successfully installed to: ' + VST3Path);
    end else
    begin
      Log('Warning: VST3 directory not found at expected location');
    end;
  end;
end;

function InitializeSetup(): Boolean;
begin
  Result := True;
  // The welcome screen already explains everything clearly
end;

[Messages]
WelcomeLabel1=Welcome to the Cellyz Phone FX Setup Wizard
WelcomeLabel2=This will install Cellyz Phone FX v1.0 on your computer.%n%nCellyz Phone FX is a professional VST3 plugin featuring authentic phone simulations:%n%n• Nokia Classic Ring Tone Effect%n• iPhone Voice Processing%n• Sony Ericsson Vintage Phone Sound%n%n✅ AUTOMATIC INSTALLATION:%nThe plugin will be automatically installed to:%nC:\Program Files\Common Files\VST3\%n%nNo manual setup required! Your DAW will detect it automatically.%n%nIt is recommended that you close all DAWs before continuing.
ClickFinish=Cellyz Phone FX has been successfully installed!%n%n✅ Plugin location: C:\Program Files\Common Files\VST3\Cellyz.vst3%n%n🎵 NEXT STEPS:%n1. Open your DAW (Reaper, Ableton, FL Studio, etc.)%n2. Rescan for plugins (if needed)%n3. Look for "Cellyz Phone FX" in your VST3 effects%n4. Load some audio and test the phone simulations!%n%nEnjoy creating with Cellyz Phone FX! 