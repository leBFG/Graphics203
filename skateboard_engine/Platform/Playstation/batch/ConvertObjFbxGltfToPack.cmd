echo on
:: For each file in the directory, convert each model format to a .pack file
:: 
:: /r  - User specified root directory
:: %%a - Expands to the current file within the directory
:: %1  - First command line argument (input dir)
:: %2  - Second command line argument (output dir)
:: %3  - Third command line argument (Pack converter dir)
:: *.  - Masks all files with the specified extension
::
 
for /r %1assets\models\ %%a in (*.obj,*.fbx,*.gltf,*.glb) do (
%3pack_converter.exe --root %2assets\models\ %%a
)
::
:: This script is executed upon rebuilding the GameApp project.