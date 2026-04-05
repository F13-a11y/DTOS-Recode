EXEC: DTOS automation (.dtos) files

Purpose
-------
This document explains the simple automation / batch system provided by DTOS Recode. Use `.dtos` plain-text files to run a sequence of built-in or shell commands via the `execute` command in the terminal.

How to run
----------
1. In the DTOS Recode program, run the `execute` command.
2. An Open File dialog filtered to `.dtos` files will open. Pick the `.dtos` file and confirm.
3. The shell will read the file line-by-line and execute supported commands. Output and any errors are printed to the terminal.

Format and syntax
-----------------
- Files must be plain text (UTF-8). A UTF-8 BOM will be removed if present.
- Blank lines are ignored.
- Comments: lines starting with `#` or `;` are ignored.
- Each executable line must start with the literal prefix `run ` (lowercase or uppercase is accepted; the prefix check is case-insensitive).
- Basic line syntax:
  run <command>[, loop <N>]
  - `<command>` is the command string to execute. Use double quotes around multi-word arguments, e.g. `run talk "hello world" 3`.
  - Optional `, loop <N>` suffix repeats the command N times (N must be an integer >= 1). If omitted, the command runs once.
  - You can also use a standalone `sleep <ms>` command to pause the script for the specified milliseconds.
  - If a `run` line contains an `execute` command (which would open another `.dtos` script), the engine will warn you and require confirmation before running. Chaining automation files is considered potentially dangerous.

Command handling
----------------
- Built-in commands executed inside the application (handled directly by the automation engine):
  - `talk "text" [speed]` — uses the built-in TTS. `speed` is optional (1..5).
  - `echo arg1 arg2 ...` — prints arguments to the terminal.
  - `sleep <ms>` — pause the script for <ms> milliseconds (e.g. `run sleep 500` pauses for 0.5s).
- Any other command is executed via `cmd /C <command>` (Windows fallback). This allows running system commands such as `ping`, `dir`, etc.

Parsing notes
-------------
- Arguments are split using the application's `splitArgs` logic which supports quoted arguments. Use double quotes for multi-word arguments.
- Loop parsing expects the suffix after a comma to begin with the word `loop` followed by the number (spaces allowed). Example: `, loop 5`.
- The engine trims surrounding whitespace for command and loop parts.

Examples
--------
Example 1 — speak a phrase three times and then print "Done":

run talk "Hello, world" , loop 3
run echo "All done"

Example 2 — run a system ping command once (falls back to cmd):

run ping 127.0.0.1

Limitations and extension
-------------------------
- The current implementation supports only the basic parsing rules above. Escaped quotes inside quoted strings and very complex command lines may not be handled robustly.
- You can extend the application to support more internal commands (e.g. `start`, `colorset`, `scrsvr`) by adding handlers in `main.cpp` where automation commands are dispatched.

Safety
------
- Automation files run commands on the host machine. Be careful with `.dtos` files from untrusted sources.

Saved example
-------------
A sample `.dtos` file is included in the repository at `examples/greet_loop.dtos`.


