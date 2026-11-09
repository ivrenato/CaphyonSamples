# MSI Preview Handler Demo

Sample project that shows how to install a Windows preview handler with an MSI package built in Advanced Installer.

The MSI installs a small WPF app that opens `.xyz` files, together with a native COM preview handler that shows the content of `.xyz` files in the File Explorer Preview Pane.

## Contents

- `src/ViewerApp` – WPF app (.NET 10) that opens `.xyz` plain text files. Published as a single self-contained x64 executable.
- `src/XyzPreviewHandler` – native C++ COM DLL (x64) that implements the preview handler.
- `installer/Setup Project.aip` – Advanced Installer project (per-machine, 64-bit package) that builds the MSI.
- `installer/XyzPreviewHandler-Registry.reg` – preview handler registry entries (AppID, `.xyz` link, preview handler list) to import on the Registry page.
- `test_file/file.xyz` – sample file to test the preview.

## How to use

1. Open `src/MsiPreviewHandlerDemo.sln` in Visual Studio. Build `XyzPreviewHandler` (Release, x64) and publish `ViewerApp` with the included publish profile.
2. Open `installer/Setup Project.aip` in Advanced Installer and build the MSI. The project expects the files at `src\x64\Release\XyzPreviewHandler.dll` and `src\ViewerApp\bin\x64\Release\net10.0-windows\win-x64\publish\ViewerApp.exe`. If you build them to a different folder, update the paths on the Files and Folders page.
3. Install the MSI.
4. In File Explorer, turn on the Preview Pane and select `test_file/file.xyz`.
