@echo on
setlocal

:: Check if the correct number of arguments are passed (3 arguments: input folder, output folder, and texconv.exe)
:: if "%~3"=="" (
::    echo Usage: convert_images.bat [input_folder] [output_folder] [texconv_exe]
::    echo Example: convert_images.bat C:\path\to\your\images C:\path\to\output\dds C:\path\to\texconv.exe
::    exit /b
:: )

:: Get the input folder, output folder, and texconv path from the arguments
set "input_folder=%~1"
set "output_folder=%~2"
set "texconv=%~3"

:: Check if the input folder exists
if not exist "%input_folder%" (
    echo Input folder does not exist: %input_folder%
    exit /b
)

:: Create the output folder if it doesn't exist
if not exist "%output_folder%" mkdir "%output_folder%"

:: Loop through all image files in the input folder and subdirectories (supports common formats like .png, .jpg, .bmp, .tga)
for /r "%input_folder%" %%F in (*.png *.jpg *.bmp *.tga) do (

    :: Convert the image and save it in the corresponding output folder
    echo Processing file: %%F
    "%texconv%" "%%F" -f R32G32B32A32_FLOAT -m 0 -y -o "%output_folder%" 

    :: Ensure that the output file is written as the correct format, for example, DDS (check texconv options)
    echo Processed: %%F
)

echo Conversion complete!
endlocal