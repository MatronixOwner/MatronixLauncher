!define MATRONIX_NAME "MatronixLauncher"
!define MATRONIX_VERSION "1.0.0"
!define MATRONIX_PUBLISHER "MatronixLauncher"
!define MATRONIX_ICON "assets/icon.ico"
!define MATRONIX_EXE "MatronixLauncher.exe"
!define MATRONIX_SETUP "MatronixLauncher-Setup.exe"

Name "${MATRONIX_NAME} ${MATRONIX_VERSION}"
OutFile "${MATRONIX_SETUP}"
InstallDir "$PROGRAMFILES64\\${MATRONIX_NAME}"
RequestExecutionLevel admin
Icon "${MATRONIX_ICON}"

Page directory
Page instfiles

Section "Install"
  SetOutPath "$INSTDIR"
  File "${MATRONIX_EXE}"
  # Записываем деинсталлятор
  WriteUninstaller "$INSTDIR\\Uninstall.exe"
  
  # Ярлык на рабочем столе
  CreateShortCut "$DESKTOP\\${MATRONIX_NAME}.lnk" "$INSTDIR\\${MATRONIX_NAME}.exe" "" "${MATRONIX_ICON}" 0
  
  # Ярлык в меню Пуск
  CreateShortCut "$SMPROGRAMS\\${MATRONIX_NAME}.lnk" "$INSTDIR\\${MATRONIX_NAME}.exe" "" "${MATRONIX_ICON}" 0
  
  # Реестр
  WriteRegStr HKLM "Software\\${MATRONIX_NAME}" "" "$INSTDIR"
  WriteRegStr HKLM "Software\\${MATRONIX_NAME}" "Version" "${MATRONIX_VERSION}"
SectionEnd

Section "Uninstall"
  Delete "$INSTDIR\\${MATRONIX_NAME}.exe"
  Delete "$INSTDIR\\Uninstall.exe"
  Delete "$DESKTOP\\${MATRONIX_NAME}.lnk"
  Delete "$SMPROGRAMS\\${MATRONIX_NAME}.lnk"
  RMDir "$INSTDIR"
  DeleteRegKey HKLM "Software\\${MATRONIX_NAME}"
SectionEnd
