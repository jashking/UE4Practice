@ECHO OFF

SET EnginePath=E:\Epic Games\UE_4.24\Engine

CALL "%EnginePath%\Build\BatchFiles\Build.bat" -ProjectFiles -Project="%~dp0MyProject.uproject" -game -progress

PAUSE