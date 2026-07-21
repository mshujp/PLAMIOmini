Import("env")

from pathlib import Path
from zipfile import ZIP_DEFLATED, ZipFile


project_dir = Path(env.subst("$PROJECT_DIR"))
output_path_sdk = project_dir / "docs" / "PLAMIOmini_SDK.zip"
output_path_project = project_dir / "docs" / "PLAMIOmini_PROJECT.zip"
input_files_sdk = (
    (project_dir / "src" / "PLAMIOmini.h", "PLAMIOmini.h"),
    (project_dir / "docs" / "PLAMIOmini_AI_GUIDELINES.md", "PLAMIOmini_AI_GUIDELINES.md"),

    (project_dir / "src" / "graphics" / "GraphicsILI9341.h", "graphics/GraphicsILI9341.h"),
    (project_dir / "src" / "graphics" / "GraphicsSSD1306.h", "graphics/GraphicsSSD1306.h"),

    (project_dir / "src" / "input" / "InputBase.h", "input/InputBase.h"),
    (project_dir / "src" / "input" / "InputGpioButtons.h", "input/InputGpioButtons.h"),
    (project_dir / "src" / "input" / "InputSnes.h", "input/InputSnes.h"),

    (project_dir / "src" / "audio" / "AudioI2S.h", "audio/AudioI2S.h"),
    (project_dir / "src" / "audio" / "AudioPWM.h", "audio/AudioPWM.h"),
    (project_dir / "src" / "audio" / "AudioStub.h", "audio/AudioStub.h"),

    (project_dir / "src" / "storage" / "StorageSD.h", "storage/StorageSD.h"),
    (project_dir / "src" / "storage" / "StorageEEPROM.h", "storage/StorageEEPROM.h"),
    (project_dir / "src" / "storage" / "StorageStub.h", "storage/StorageStub.h"),

    (project_dir / "examples" / "00B_Hardware_Setup" / "00B_Hardware_Setup.ino",
     "examples/00B_Hardware_Setup/00B_Hardware_Setup.ino"),
    (project_dir / "examples" / "01_Hello_PLAMIO" / "01_Hello_PLAMIO.ino",
     "examples/01_Hello_PLAMIO/01_Hello_PLAMIO.ino"),
    (project_dir / "examples" / "02_Input_Basics" / "02_Input_Basics.ino",
     "examples/02_Input_Basics/02_Input_Basics.ino"),
    (project_dir / "examples" / "03_Graphics_Basics" / "03_Graphics_Basics.ino",
     "examples/03_Graphics_Basics/03_Graphics_Basics.ino"),
    (project_dir / "examples" / "04_Audio_Basics" / "04_Audio_Basics.ino",
     "examples/04_Audio_Basics/04_Audio_Basics.ino"),
    (project_dir / "examples" / "05_Save_Data" / "05_Save_Data.ino",
     "examples/05_Save_Data/05_Save_Data.ino"),
)
output_path_sdk.parent.mkdir(parents=True, exist_ok=True)
with ZipFile(output_path_sdk, "w", compression=ZIP_DEFLATED) as archive:
    for source_path, archive_name in input_files_sdk:
        if not source_path.is_file():
            raise RuntimeError(f"SDK source file not found: {source_path}")
        archive.write(source_path, archive_name)

input_files_project = (
    (project_dir / "docs" / "PLAMIOmini_GAME_DESIGN_TEMPLATE.md", "PLAMIOmini_GAME_DESIGN_TEMPLATE.md"),
    (project_dir / "docs" / "PLAMIOmini_GAME_DESIGN_TEMPLATE_JP.md", "PLAMIOmini_GAME_DESIGN_TEMPLATE_JP.md"),
    (project_dir / "docs" / "PLAMIOmini_HARDWARE_CONFIG_TEMPLATE.md", "PLAMIOmini_HARDWARE_CONFIG_TEMPLATE.md"),
)
with ZipFile(output_path_project, "w", compression=ZIP_DEFLATED) as archive:
    for source_path, archive_name in input_files_project:
        if not source_path.is_file():
            raise RuntimeError(f"SDK source file not found: {source_path}")
        archive.write(source_path, archive_name)


print(f"PLAMIO mini SDK: {output_path_sdk}")
print(f"PLAMIO mini project files: {output_path_project}")
