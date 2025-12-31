# Custom Shell with AI Command Integration

## Overview
This project is a **custom Unix-like shell written in C++**.  
It supports standard Linux command execution using `fork`, `execvp`, and `waitpid`, basic built-in commands, and an experimental **AI-powered command (`ai`)** that allows users to execute Linux commands using natural language prompts.

The goal of this project is to explore **system programming concepts**, shell design, and how **AI-assisted interfaces** can improve command-line usability.

---

## Features

### Core Shell Functionality
- Interactive prompt displaying the current working directory
- Execution of external Linux commands via `execvp`
- Built-in commands:
  - `cd` (supports `~` and relative paths)
  - `exit`
- Quoted argument handling (`" "` and `' '`)
- Error handling for invalid commands
- POSIX-compliant process management

### AI Command (`ai`)
- Converts natural language prompts into Linux commands
- Executes the generated command directly in the shell
- Powered by the **Google Gemini API**
- Example:
  ```bash
  ai "list all files larger than 1MB in the current directory"
  ```
### Example Usage
```bash
/home/user$ ls -l
/home/user$ cd ~/Documents
/home/user/Documents$ ai show disk usage of this folder
/home/user/Documents$ exit
```
### Limitations
- No pipe (|) support yet
- No I/O redirection (>, <)
- No job control (&, fg, bg)
- AI command accuracy depends on prompt clarity


