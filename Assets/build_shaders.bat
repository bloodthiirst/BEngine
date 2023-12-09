@echo off
setlocal

echo Building shaders ...
set BuildShader=%VULKAN_SDK%\Bin\glslc.exe
echo Path for the GLSL Compiler : %BuildShader%
echo Current Path : %CurrentPath%

%BuildShader% -fshader-stage=frag Builtin.ObjectShader.frag.glsl  -o Builtin.ObjectShader.frag.spv
%BuildShader% -fshader-stage=vert Builtin.ObjectShader.vert.glsl  -o Builtin.ObjectShader.vert.spv
echo Done!
pause