; ============================================================================
; Nico v2.1.0 - Script de InnoSetup para Windows
; Instalador profesional con MSYS2 opcional para desarrolladores
; ============================================================================
#define MyAppName "Nico"
#define MyAppVersion "2.1.0"
#define MyAppPublisher "Diego Alejandro Majluff"
#define MyAppURL "https://github.com/DiegoMajluff/nico"
#define MyAppExeName "nico-ide.exe"

[Setup]
AppId={{A1B2C3D4-E5F6-7890-ABCD-EF1234567890}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
AppPublisher={#MyAppPublisher}
AppPublisherURL={#MyAppURL}
DefaultDirName={autopf}\{#MyAppName}
DefaultGroupName={#MyAppName}
AllowNoIcons=yes
LicenseFile=dist\LICENSE
OutputDir=installer
OutputBaseFilename=Nico-{#MyAppVersion}-Setup
Compression=lzma2/max
SolidCompression=yes
WizardStyle=modern
PrivilegesRequired=admin
ArchitecturesInstallIn64BitMode=x64compatible
UninstallDisplayIcon={app}\nico-ide.ico
VersionInfoVersion={#MyAppVersion}
VersionInfoCompany={#MyAppPublisher}
VersionInfoDescription=Instalador de Nico - Lenguaje de Scripting en Español
MinVersion=10.0
SetupIconFile=nico-ide.ico

[Languages]
Name: "spanish"; MessagesFile: "compiler:Languages\Spanish.isl"

[Tasks]
Name: "desktopicon"; Description: "Crear acceso directo en el &Escritorio"; GroupDescription: "Accesos directos:"
Name: "quicklaunchicon"; Description: "Crear acceso directo en la barra de &Inicio Rápido"; GroupDescription: "Accesos directos:"; Flags: unchecked
Name: "addtopath"; Description: "Agregar Nico al &PATH del sistema (recomendado)"; GroupDescription: "Opciones del sistema:"
Name: "installmsys2"; Description: "Instalar &MSYS2 para compilar/extender Nico (requiere internet, ~400MB)"; GroupDescription: "Para desarrolladores:"; Flags: unchecked

[Files]
; === BINARIOS PRECOMPILADOS ===
Source: "dist\bin\nico.exe"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "dist\bin\nico-ide.exe"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "dist\bin\*.dll"; DestDir: "{app}\bin"; Flags: ignoreversion recursesubdirs
; === ÍCONO ===
Source: "nico-ide.ico"; DestDir: "{app}"; Flags: ignoreversion

; === CÓDIGO FUENTE DEL INTÉRPRETE ===
Source: "dist\src\*"; DestDir: "{app}\src"; Flags: ignoreversion recursesubdirs createallsubdirs

; === CÓDIGO FUENTE DEL IDE ===
Source: "dist\nico-ide\*"; DestDir: "{app}\nico-ide"; Flags: ignoreversion recursesubdirs createallsubdirs

; === SCRIPTS DE COMPILACIÓN ===
Source: "dist\compile.sh"; DestDir: "{app}"; Flags: ignoreversion
Source: "dist\compile_windows.bat"; DestDir: "{app}"; Flags: ignoreversion

; === EJEMPLOS ===
Source: "dist\ejemplos\*"; DestDir: "{app}\ejemplos"; Flags: ignoreversion recursesubdirs createallsubdirs

; === DOCUMENTACIÓN ===
Source: "dist\docs\*"; DestDir: "{app}\docs"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "dist\README.md"; DestDir: "{app}"; Flags: ignoreversion
Source: "dist\LICENSE"; DestDir: "{app}"; Flags: ignoreversion
Source: "dist\CHANGELOG.md"; DestDir: "{app}"; Flags: ignoreversion

; === SCRIPT DE INSTALACIÓN DE MSYS2 ===
Source: "scripts\install_msys2.ps1"; DestDir: "{app}\scripts"; Flags: ignoreversion

[Icons]
Name: "{group}\{#MyAppName} IDE"; Filename: "{app}\bin\{#MyAppExeName}"
Name: "{group}\Documentación de {#MyAppName}"; Filename: "{app}\docs\MANUAL.md"
Name: "{group}\Ejemplos de {#MyAppName}"; Filename: "{app}\ejemplos"
Name: "{group}\{cm:UninstallProgram,{#MyAppName}}"; Filename: "{uninstallexe}"
Name: "{autodesktop}\{#MyAppName} IDE"; Filename: "{app}\bin\{#MyAppExeName}"; Tasks: desktopicon

[Run]
Filename: "{app}\bin\{#MyAppExeName}"; Description: "Iniciar {#MyAppName} IDE ahora"; Flags: nowait postinstall skipifsilent unchecked

[Code]
// ============================================================================
// VARIABLES GLOBALES
// ============================================================================
var
  PaginaBienvenida: TOutputMsgMemoWizardPage;

// ============================================================================
// PÁGINA DE BIENVENIDA CON INFORMACIÓN
// ============================================================================
procedure InitializeWizard();
var
  InfoTexto: String;
begin
  InfoTexto :=
    'Nico es un lenguaje de scripting educativo en español con:' + #13#10 +
    '  - Motor logico estilo Prolog (unificacion, backtracking, corte)' + #13#10 +
    '  - Soporte completo para listas, matrices y matrices 3D' + #13#10 +
    '  - Control de GPIO y PWM para Raspberry Pi' + #13#10 +
    '  - Servidor HTTP con panel de administracion web' + #13#10 +
    '  - Base de datos SQLite integrada' + #13#10 +
    '  - IDE integrado con resaltado de sintaxis' + #13#10 + #13#10 +
    'ESTE INSTALADOR INCLUYE:' + #13#10 +
    '  - Interprete Nico (nico.exe) - precompilado y listo para usar' + #13#10 +
    '  - IDE Nico (nico-ide.exe) - entorno de desarrollo integrado' + #13#10 +
    '  - Codigo fuente completo (interprete + IDE)' + #13#10 +
    '  - Ejemplos y documentacion' + #13#10 +
    '  - Scripts de compilacion para Windows y Linux' + #13#10 + #13#10 +
    'OPCIONAL: INSTALAR MSYS2 (para desarrolladores)' + #13#10 +
    'Si desea compilar Nico desde el codigo fuente o extenderlo,' + #13#10 +
    'puede marcar la opcion "Instalar MSYS2" en la pagina de tareas.' + #13#10 +
    'Esto requiere conexion a internet y descargara aproximadamente 400MB.' + #13#10 + #13#10 +
    'Si solo quiere usar Nico, NO necesita marcar esta opcion.' + #13#10 + #13#10 +
    'Presione Siguiente para continuar con la instalacion.';

  // El texto se pasa DIRECTAMENTE al crear la pagina (no hay metodo Add)
  PaginaBienvenida := CreateOutputMsgMemoPage(wpWelcome,
    'Bienvenido al instalador de Nico v' + '{#MyAppVersion}',
    'Informacion importante sobre la instalacion',
    'Lea la siguiente informacion antes de continuar:',
    InfoTexto);
end;

// ============================================================================
// EJECUTAR DESPUÉS DE LA INSTALACIÓN
// ============================================================================
procedure CurStepChanged(CurStep: TSetupStep);
var
  PathActual: String;
  PathNico: String;
begin
  if CurStep = ssPostInstall then
  begin
    if IsTaskSelected('addtopath') then
    begin
      PathNico := ExpandConstant('{app}\bin');
      
      if RegQueryStringValue(HKEY_LOCAL_MACHINE,
         'SYSTEM\CurrentControlSet\Control\Session Manager\Environment',
         'Path', PathActual) then
      begin
        if Pos(PathNico, PathActual) = 0 then
        begin
          PathActual := PathActual + ';' + PathNico;
          RegWriteStringValue(HKEY_LOCAL_MACHINE,
             'SYSTEM\CurrentControlSet\Control\Session Manager\Environment',
             'Path', PathActual);
          Log('[PATH] Agregado: ' + PathNico);
        end
        else
        begin
          Log('[PATH] Ya existe: ' + PathNico);
        end;
      end;
    end;
  end;
end;

// ============================================================================
// DESINSTALACIÓN: LIMPIAR EL PATH
// ============================================================================
procedure CurUninstallStepChanged(CurUninstallStep: TUninstallStep);
var
  PathActual: String;
  PathNico: String;
  NuevaPos: Integer;
begin
  if CurUninstallStep = usUninstall then
  begin
    PathNico := ExpandConstant('{app}\bin');
    
    if RegQueryStringValue(HKEY_LOCAL_MACHINE,
       'SYSTEM\CurrentControlSet\Control\Session Manager\Environment',
       'Path', PathActual) then
    begin
      NuevaPos := Pos(PathNico, PathActual);
      if NuevaPos > 0 then
      begin
        Delete(PathActual, NuevaPos, Length(PathNico));
        StringChange(PathActual, ';;', ';');
        
        if Length(PathActual) > 0 then
        begin
          if Copy(PathActual, 1, 1) = ';' then
            Delete(PathActual, 1, 1);
          if Copy(PathActual, Length(PathActual), 1) = ';' then
            Delete(PathActual, Length(PathActual), 1);
        end;
        
        RegWriteStringValue(HKEY_LOCAL_MACHINE,
           'SYSTEM\CurrentControlSet\Control\Session Manager\Environment',
           'Path', PathActual);
        Log('[PATH] Eliminado: ' + PathNico);
      end;
    end;
  end;
end;

// ============================================================================
// VERIFICAR REQUISITOS ANTES DE INSTALAR
// ============================================================================
function InitializeSetup(): Boolean;
var
  Version: TWindowsVersion;
begin
  Result := True;
  
  GetWindowsVersionEx(Version);
  
  // Verificar versión de Windows (mínimo Windows 10)
  // Windows 10 = Major 10, Minor 0
  if (Version.Major < 10) or ((Version.Major = 10) and (Version.Minor < 0)) then
  begin
    MsgBox('Nico requiere Windows 10 o superior.' + #13#10 + #13#10 +
           'Su sistema no cumple con los requisitos mínimos.',
           mbError, MB_OK);
    Result := False;
  end;
end;