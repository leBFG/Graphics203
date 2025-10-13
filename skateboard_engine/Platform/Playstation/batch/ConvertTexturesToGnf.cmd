echo on
:: For each file in the directory, convert each texture format to a .gnf file
:: 
:: /r  - User specified root directory
:: %%a - Expands to the current file within the directory
:: %1  - First command line argument (input dir)
:: %2  - Second command line argument (output dir)
:: %3  - Third command line argument (image2gnf dir)
:: *.  - Masks all files with the specified extension
::
:: controlling the srgb crap -s "Rxl_Gxl_Bxl_All"

for /r %1assets\textures\ %%a in (*.png,*.jpeg,*.jpg,*.tga,*.bmp,*.dds) do (
    %3image2gnf.exe -g 1 -i %%a -f autoCompress -o %2assets\textures\%%~na.gnf
)
::
:: This script is executed upon rebuilding the GameApp project.

