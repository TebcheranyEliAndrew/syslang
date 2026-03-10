![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)
![OS: Windows](https://img.shields.io/badge/OS-Windows-blue)
![OS: macOS](https://img.shields.io/badge/OS-macOS-lightgrey)
![OS: Linux](https://img.shields.io/badge/OS-Linux-green)

# SysLang - The System Information Language

A minimal, embeddable scripting language for getting hardware information across Windows, macOS, and Linux. Written in clean C99.

---

## 📁 File Structure

```
syslang/
├── syslang.h      # Data structures and API
├── syslang.c      # Lexer, parser, interpreter, hardware
├── main.c         # REPL and file loader
├── Makefile       # Build automation
└── README.md      # This file
```

---

## ⚡ Quick Start

### Prerequisites
- GCC (or any C99 compiler)
- Make (optional)

### Build & Run

```bash
# Compile
gcc -std=c99 -Wall -Wextra -O2 -o syslang main.c syslang.c

# Start interactive REPL
./syslang

# Run a script file
./syslang test.sl
```

### Windows Users

```bash
# Same command (with MinGW or MSYS2)
gcc -std=c99 -Wall -Wextra -O2 -o syslang.exe main.c syslang.c

# Run
syslang.exe
```

---

## 💻 Language Syntax

### Example Script (info.sl)
```
# This is a comment
print "=== SYSTEM INFO ==="
cpu = get_cpu_info()
ram = get_ram_total() / 1024 / 1024
print "CPU: " + cpu
print "RAM: " + ram + " MB"
print "OS: " + get_os_name()
```

### Built-in Functions

| Function | Returns |
|----------|---------|
| `get_cpu_info()` | CPU model string |
| `get_ram_total()` | Total RAM in bytes |
| `get_ram_used()` | Used RAM in bytes |
| `get_os_name()` | "Windows", "macOS", or "Linux" |
| `print(x)` | Prints any value |

### Features
- String concatenation with `+`
- Variables: `name = value`
- Print statements: `print expression`
- Comments starting with `#`

---

## 🔧 Build Options

### Using Make

```bash
make        # Build
make run    # Build and run REPL
make clean  # Remove compiled files
```

### Manual Compilation

```bash
# Release build
gcc -std=c99 -Wall -Wextra -O2 -o syslang main.c syslang.c

# Debug build
gcc -std=c99 -Wall -Wextra -g -o syslang_debug main.c syslang.c

# With address sanitizer
gcc -std=c99 -Wall -Wextra -g -fsanitize=address -o syslang_sanitize main.c syslang.c
```

---

## 🚀 Usage Examples

### Interactive Mode (REPL)

```bash
$ ./syslang
SysLang v0.1 - Type 'exit' to quit
> print "Hello World"
Hello World
> cpu = get_cpu_info()
> print "My CPU: " + cpu
My CPU: Intel(R) Core(TM) i5-2557M CPU @ 1.70GHz
> exit
```

### Run a Script File

Create `test.sl`:
```
print "System Report"
print "=============="
print "CPU: " + get_cpu_info()
print "RAM: " + (get_ram_total() / 1024 / 1024) + " MB"
```

Run it:
```bash
./syslang test.sl
```

### One-liner with Pipe

```bash
echo 'print "CPU: " + get_cpu_info()' | ./syslang
```

---

## 🌐 Cross-Platform Support

| OS | Status | Hardware Access Method |
|----|--------|------------------------|
| Windows | ✅ Full | Windows Registry, GlobalMemoryStatusEx |
| macOS | ✅ Full | sysctl, host_statistics |
| Linux | ✅ Full | /proc/cpuinfo, sysinfo() |
| Other | ⚠️ Fallback | Returns "Unknown" |

The same script runs on all platforms without changes.

---

## 📦 File Details

### syslang.h (80 lines)
- Defines token types (PRINT, IDENTIFIER, STRING, etc.)
- Defines AST node types and structures
- Declares all public functions
- Contains interpreter state struct

### syslang.c (350 lines)
- **Lexer**: Converts source text to tokens
- **Parser**: Builds Abstract Syntax Tree from tokens
- **Interpreter**: Executes the AST
- **Hardware**: OS-specific system info functions

### main.c (50 lines)
- REPL for interactive use
- File loading and execution
- Memory cleanup

### Makefile (10 lines)
- Build automation
- Clean and run targets

---

## 🔍 Code Components

### 1. Lexer - `lex()` in syslang.c
The lexer reads source code character by character and outputs tokens:
- `"print"` → `TOKEN_PRINT`
- `"cpu"` → `TOKEN_IDENTIFIER` with value "cpu"
- `"Hello"` → `TOKEN_STRING` with value "Hello"
- `+` → `TOKEN_PLUS`

### 2. Parser - `parse()` and helpers in syslang.c
The parser builds an Abstract Syntax Tree:
```
print "CPU: " + get_cpu_info()
        ↓
    [NODE_PRINT]
         ↓
    [NODE_BINOP (+)]
      /           \
[NODE_STRING]  [NODE_FUNC_CALL]
"CPU: "        "get_cpu_info"
```

### 3. Interpreter - `interpret()` in syslang.c
The interpreter walks the tree and executes:
- Prints strings
- Calls hardware functions
- Manages variables
- Concatenates with `+`

### 4. Hardware Layer - OS-specific implementations
- **Windows**: Registry and Windows API
- **macOS**: sysctl and mach calls
- **Linux**: /proc filesystem and sysinfo

---

## 🧪 Testing

Create `test.sl`:
```
# Test script
print "Testing SysLang..."
print "=================="

os = get_os_name()
print "OS detected: " + os

cpu = get_cpu_info()
print "CPU: " + cpu

ram_mb = get_ram_total() / 1024 / 1024
print "RAM: " + ram_mb + " MB"

print ""
print "All tests passed!"
```

Run:
```bash
./syslang test.sl
```

Expected output (varies by system):
```
Testing SysLang...
==================
OS detected: Windows
CPU: Intel(R) Core(TM) i7-10750H CPU @ 2.60GHz
RAM: 16384 MB

All tests passed!
```

---


## 📜 License

MIT License

The design is inspired by [Beryl](https://github.com/KarlAndr1/beryl) by KarlAndr1, but all code is original and written from scratch.

---

## 🙏 Acknowledgments

- **KarlAndr1** for Beryl
- **Robert Nystrom** for "Crafting Interpreters"
- **MSYS2 and MinGW** for Windows tooling

---

## 🐛 Troubleshooting

| Problem | Solution |
|---------|----------|
| `'gcc' is not recognized` | Install GCC (MinGW on Windows, Xcode on Mac, build-essential on Linux) |
| `undefined reference to 'get_cpu_info'` | Make sure you're linking `syslang.c` |
| Script shows "Unknown" | OS implementation may be missing |
| Memory leaks | Use `-fsanitize=address` during development |

---


*All comments and documentation in this codebase were generated with the assistance of AI. The code structure, logic, and functionality were designed by human .*



