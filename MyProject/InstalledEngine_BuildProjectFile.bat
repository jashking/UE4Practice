@ECHO OFF

SET EnginePath=E:\Epic Games\UE_4.24\Engine

REM 重命名 InstalledBuild.txt
IF EXIST "%EnginePath%\Build\InstalledBuild.txt" (
    REN "%EnginePath%\Build\InstalledBuild.txt" InstalledBuild1.txt
)

CALL "%EnginePath%\Build\BatchFiles\Build.bat" MyBlankProgram Win64 Development -Project="%~dp0MyProject.uproject" -WaitMutex

REM 重命名 InstalledBuild.txt
IF EXIST "%EnginePath%\Build\InstalledBuild1.txt" (
    REN "%EnginePath%\Build\InstalledBuild1.txt" InstalledBuild.txt
)

PAUSE