#include <iostream>
#include <string>
#include <sstream>
#include <filesystem>
#include <unistd.h>    // For read()
#include <termios.h>   // For termios
#include <sys/ioctl.h> // For ioctl
#include <vector>

using namespace std;
namespace fs = std::filesystem;

std::vector<std::string> history = {
    "echo",
    "type",
    "mkdir",
    "exit0",
};

// Set the terminal in raw mode
void enableRawMode(struct termios &original)
{
  struct termios raw = original;
  raw.c_lflag &= ~(ECHO | ICANON); // Turn off echo and canonical mode
  tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

// Restore the terminal to its original state
void disableRawMode(struct termios &original)
{
  tcsetattr(STDIN_FILENO, TCSAFLUSH, &original);
}

void displayWelcomeMessage()
{
  std::cout << R"(
________  ________  ________  ___  ___          ___       ___  _________  _______      
|\   __  \|\   __  \|\   ____\|\  \|\  \        |\  \     |\  \|\___   ___\\  ___ \     
\ \  \|\ /\ \  \|\  \ \  \___|\ \  \\\  \       \ \  \    \ \  \|___ \  \_\ \   __/|    
 \ \   __  \ \   __  \ \_____  \ \   __  \       \ \  \    \ \  \   \ \  \ \ \  \_|/__  
  \ \  \|\  \ \  \ \  \|____|\  \ \  \ \  \       \ \  \____\ \  \   \ \  \ \ \  \_|\ \ 
   \ \_______\ \__\ \__\____\_\  \ \__\ \__\       \ \_______\ \__\   \ \__\ \ \_______\
    \|_______|\|__|\|__|\_________\|__|\|__|        \|_______|\|__|    \|__|  \|_______|
                       \|_________|                                                     )"
            << std::endl;
}

enum validCommands
{
  echo,
  cd,
  exit0,
  type,
  pwd,
  invalid,
  help
};
validCommands isValid(string command)
{
  command = command.substr(0, command.find(" "));

  if (command == "echo")
    return validCommands::echo;
  if (command == "cd")
    return validCommands::cd;
  if (command == "exit")
    return validCommands::exit0;
  if (command == "type")
    return validCommands::type;
  if (command == "pwd")
    return validCommands::pwd;

  return invalid;
}

std::string valid[5] = {"echo", "cd", "exit0", "type", "pwd"};

std::string get_path(const std::string command)
{
  // PATH="/usr/bin:/usr/local/bin" ./your_program.sh
  std::string path_env = std::getenv("PATH");
  std::stringstream ss(path_env);
  std::string path;

  while (getline(ss, path, ':'))
  { // Read each directory in PATH
    std::string abs_path = path + "/" + command;
    if (std::filesystem::exists(abs_path))
    {
      return abs_path;
    }
  }
  return ""; // Return empty string if command not found in PATH
}

void chdir(std::string dir)
{
  fs::path newDirectory(dir); // convert std::string to fs::path
  if (fs::is_directory(newDirectory))
  {
    fs::current_path(newDirectory); // set current path to dir
  }
  else
    cout << "cd: " << dir << ": No such file or directory\n";
}

std::string getSuggestion(std::string input)
{
  for (auto it : history)
  {
    if (it.size() >= input.size())
    {
      if (it.substr(0, input.size()) == input)
      {
        return it;
      }
    }
  }
  return "";
}

int main()
{
  struct termios original;
  tcgetattr(STDIN_FILENO, &original); // Get the terminal attributes

  enableRawMode(original); // Enable raw mode
  // displayWelcomeMessage();
  bool exit = false;
  char ch;
  std::string last_suggestion;

  while (!exit)
  {
    std::cout << std::unitbuf;
    std::cerr << std::unitbuf;
    std::cout << "$ ";
    std::string input;

    while (true)
    {
      ch = getchar();
      if (ch == '\n')
      {
        if (!last_suggestion.empty())
        {
          for (size_t i = 0; i < last_suggestion.size() - input.size(); ++i)
            std::cout << " ";
          // Move cursor back to original position
          for (size_t i = 0; i < last_suggestion.size() - input.size(); ++i)
            std::cout << "\b";
        }
        cout << endl;
        break;
      }
      else if (ch == '\t')
      {
        if (!input.empty() && !last_suggestion.empty())
        {
          cout << last_suggestion.substr(input.size());
          input = last_suggestion;
          last_suggestion = "";
        }
      }
      else if (ch == 127) // Backspace pressed
      {
        if (!last_suggestion.empty())
        {
          for (size_t i = 0; i < last_suggestion.size() - input.size(); ++i)
            std::cout << " ";
          // Move cursor back to original position
          for (size_t i = 0; i < last_suggestion.size() - input.size(); ++i)
            std::cout << "\b";
        }
        if (!input.empty())
        {
          input.pop_back();
          std::cout << "\b \b"; // Erase character from terminal
        }
      }
      else
      {
        input += ch;
        cout << ch;

        std::string suggestion = getSuggestion(input);
        if (!last_suggestion.empty() && last_suggestion != suggestion)
        {
          // Move cursor forward to cover the old suggestion
          for (size_t i = 0; i < last_suggestion.size() - input.size(); ++i)
            std::cout << " ";

          // Move cursor back to original position
          for (size_t i = 0; i < last_suggestion.size() - input.size(); ++i)
            std::cout << "\b";
        }

        if (!suggestion.empty() && suggestion != input)
        {
          std::string to_display = suggestion.substr(input.size());
          // Print the new suggestion in grey
          std::cout << "\033[38;5;242m" << to_display << "\033[0m";

          // Move the cursor back to where the user input ends
          for (size_t i = 0; i < to_display.length(); ++i)
            std::cout << "\b";
          last_suggestion = suggestion;
        }
        else
          last_suggestion.clear(); // Clear the last suggestion if no valid suggestion
      }
    }
    // std::getline(std::cin, input);

    switch (isValid(input))
    {
    case cd:
    {
      string dirStr = input.substr(input.find(" ") + 1);
      if (dirStr == "~")
      {
        const char *homeDir_cStr = std::getenv("HOME"); // gives home directory
        std::string homeDir(homeDir_cStr);
        chdir(homeDir);
      }
      else
        chdir(dirStr);
      break;
    }
    case echo:
      input.erase(0, input.find(" ") + 1);
      std::cout << input << "\n";
      break;
    case exit0:
      exit = true;
      break;
    case pwd:
      std::cout << fs::current_path().string() << endl;
      break;
    case type:
      input.erase(0, input.find(" ") + 1);
      if (isValid(input) != invalid)
      {
        std::cout << input << " is a shell builtin\n";
      }
      else
      {
        std::string path = get_path(input);
        if (path.empty())
        {
          std::cout << input << ": not found\n";
        }
        else
        {
          std::cout << input << " is " << path << std::endl;
        }
      }
      break;
    default:
      // find program and execute using system
      string command = input.substr(0, input.find(" "));
      string path = get_path(command);
      if (path.empty())
      {
        std::cout << input << ": command not found\n";
      }
      else
      {
        string command_with_full_path = path + input.substr(command.length()); // full = path+(space)+arguments

        const char *command_ptr = command_with_full_path.c_str();
        system(command_ptr);
      }
      break;
    }
  }
  disableRawMode(original);
  return 0;
}
