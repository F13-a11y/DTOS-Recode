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

## Key features
- Built-in commands: `help`, `cls`, `cdview`, `rundll`, `del`, `colorset`, `start`, `talk`, `echo`, `about`.
- Screensaver management: `scrsvr` lists `.scr` files in the system directory and can run them; `scrsvr spec` opens a file picker filtered to `.scr` files for a custom selection.
- Simple XOR encryption/decryption: `xorenc "text" "key"` and `xordec "hex" "key"`.
- `dev` command opens the project author's GitHub URL in the default browser.

## Building
Open the included Visual C++ solution in Visual Studio (tested with Visual Studio Community 2026) and build the solution. The project already includes required WinAPI headers and code.

Quick steps:
1. Open `DTOS Recode.sln` in Visual Studio.
2. Build -> Build Solution.
3. Run the produced executable from the Debug/Release output folder.

## Usage examples
- `help` — show available commands.
- `scrsvr` — list `.scr` screensavers in the Windows system directory and choose one to run.
- `scrsvr spec` — open an Open File dialog filtered to `.scr` files and run the selected screensaver.
- `xorenc "hello" "key"` — XOR-encrypt `hello` with `key` and output the result as hex.
- `xordec "hexstring" "key"` — XOR-decrypt the provided hex string with `key` and print the plaintext.
- `dev` — open the author's GitHub page in the default browser.

## Contributing
- Fork the repository, create a branch for your changes, and submit a pull request.
- Please open an issue to discuss larger changes before implementing them.

## License
See the repository's license file or contact the project owner for licensing details.

---
Project owner: https://github.com/F13-a11y
