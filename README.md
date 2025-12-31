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
### How the AI Command Works (High-Level)
1. User enters an ai prompt
2. The prompt is sent to the Gemini API via HTTPS
3. Gemini returns a Linux command as text
4. The shell parses and executes the command using existing execution logic
This design keeps AI strictly as a suggestion layer, not a privileged executor.

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

### Disclaimer
The AI command executes real system commands.
Use it **only in a safe environment**. The author is not responsible for unintended command execution.

### License
This project is for educational purposes.
