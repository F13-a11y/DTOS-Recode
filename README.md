[README.md](https://github.com/user-attachments/files/26480429/README.md)
# DTOS Recode

DTOS Recode is a small Windows terminal/command-shell application — a rewritten ("recode") version of the original DTOS project.

## Overview
This project implements a compact command-line shell for Windows providing utilities such as a command prompt, file open dialogs, TTS (text-to-speech), console color control, launching screensavers, and a simple XOR-based encrypt/decrypt feature.

## Why "Recode"?
The original DTOS (Double Tap Operating Shell) source code was lost. This repository is a fresh reimplementation (a "recode") of the original project. The goals for this rewrite are:
- Recover the original functionality after the original source was lost.
- Improve code clarity and modularity so the project is easier to maintain and extend.
- Update platform-specific behaviors and use more modern C++ practices (for example enabling ANSI escapes on Windows consoles).

This release aims to preserve the original behavior while producing clearer, safer, and more maintainable code.

## Building
Open the included Visual C++ solution in Visual Studio (tested with Visual Studio Community 2026) and build the solution. The project already includes required WinAPI headers and code.

Quick steps:
1. Open `DTOS Recode.sln` in Visual Studio.
2. Build -> Build Solution.
3. Run the produced executable from the Debug/Release output folder.

## Contributing
- Fork the repository, create a branch for your changes, and submit a pull request.
- Please open an issue to discuss larger changes before implementing them.

## License
See the repository's license file or contact the project owner for licensing details.

## Note
Sorry main source file's commits is turkish,i trying to push new english commits
---
Project owner: https://github.com/F13-a11y
