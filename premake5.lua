-- premake5.lua
workspace "raytracing-rt"
   architecture "x64"
   configurations { "Debug", "Release", "Dist" }
   startproject "raytracing-rt"

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"
include "Walnut/WalnutExternal.lua"

include "raytracing-rt"