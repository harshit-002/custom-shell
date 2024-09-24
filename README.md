# shellX

**shellX** is a custom command-line shell designed for a tailored user experience with **AI** integration. It supports basic shell commands and features an AI chat functionality for user assistance directly in the command line interface.

## Features

- **AI Chat Integration**: Chat with a Google AI model directly from the shell, allowing users to get real-time assistance.
- **Command Autocomplete**: Suggests command completions based on user input, enhancing the command-line experience.
- **Basic Shell Commands**: Supports standard commands like `cd`, `pwd`, `echo`, `ls`, `mkdir`, `rm`, and more.

## Installation & Usage

1. Clone the repository:
   ```bash
   git clone https://github.com/harshit-002/custom-shell.git
   cd custom-shell
   ```
2. Clone cpr library inside custom-shell folder

3. Run the build script:
   ```bash
   ./build.sh
   ```

### Basic Commands

- `cd [directory]`: Change the current directory to `[directory]`
- `ai-chat`: Start a chat with AI
- `pwd`: Print the current working directory
- `echo [message]`: Display `[message]` on the terminal
- `type [command]`: Display the type of `[command]`
- `exit`: Exit the shell
- `help`: Show this help message
- `ls`: List directory contents
- `mkdir [dir]`: Create a directory named `[dir]`
- `rm [file/dir]`: Remove file or directory
- `touch [file]`: Create a file named `[file]`

## Technologies Used

- C++
- CPR for API requests
- Google AI Integration
- CMake for build management

## Contributing

Contributions are welcome! Please feel free to open issues or submit pull requests.
