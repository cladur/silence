REM Create a release build of the project
mkdir silence_release

REM copy all .dll, .lib and .exe files (recursively) from build_release to silence_release
xcopy build_release\*.dll silence_release /E /I /Y
xcopy build_release\*.lib silence_release /E /I /Y
xcopy build_release\*.exe silence_release /E /I /Y

REM Copy specific folders from resources to silence_release
REM assets_export, fmod_banks, fonts, icons, prefabs, scenes, shaders
mkdir silence_release\resources
xcopy resources\assets_export silence_release\resources\assets_export /E /I /Y
xcopy resources\fmod_banks silence_release\resources\fmod_banks /E /I /Y
xcopy resources\fonts silence_release\resources\fonts /E /I /Y
xcopy resources\icons silence_release\resources\icons /E /I /Y
xcopy resources\prefabs silence_release\resources\prefabs /E /I /Y
xcopy resources\scenes silence_release\resources\scenes /E /I /Y
xcopy resources\shaders silence_release\resources\shaders /E /I /Y

REM Also copy that one libomp dll
xcopy resources\libomp140.x86_64.dll silence_release /E /I /Y