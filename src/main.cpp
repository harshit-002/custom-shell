#include <iostream>
#include <string>
#include <sstream>
#include <filesystem>
#include <unistd.h>    // For read()
#include <termios.h>   // For termios
#include <sys/ioctl.h> // For ioctl
#include <vector>
#include <cpr/cpr.h>
#include "json.hpp"

using namespace std;
using json = nlohmann::json;
namespace fs = std::filesystem;

std::vector<std::string> history = {
    "echo",
    "type",
    "mkdir",
    "exit",
    "help",
    "ai-chat",
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
         __         ____     _  __
   _____/ /_  ___  / / /    | |/ /
  / ___/ __ \/ _ \/ / /     |   / 
 (__  ) / / /  __/ / /     /   |  
/____/_/ /_/\___/_/_/     /_/|_|   )"
            << std::endl;
}

void display_help()
{
  std::cout << "\n\033[1;32mWelcome to shellX!\033[0m\n";
  std::cout << "Tailored Command Line with AI Integration!\n";
  std::cout << "\n\033[1;34mHere are some basic commands you can use:\033[0m\n\n";
  std::cout << "  \033[1;36mai-chat\033[0m         : Start chat with AI\n";
  std::cout << "  \033[1;36mcd [directory]\033[0m  : Change the current directory to [directory]\n";
  std::cout << "  \033[1;36mpwd\033[0m             : Print the current working directory\n";
  std::cout << "  \033[1;36mecho [message]\033[0m  : Display [message] on the terminal\n";
  std::cout << "  \033[1;36mtype [command]\033[0m  : Display the type of [command]\n";
  std::cout << "  \033[1;36mexit\033[0m            : Exit the shell\n";
  std::cout << "  \033[1;36mhelp\033[0m            : Show this help message\n";
  std::cout << "  \033[1;36mls\033[0m              : List directory contents\n";
  std::cout << "  \033[1;36mmkdir [dir]\033[0m     : Create a directory named [dir]\n";
  std::cout << "  \033[1;36mrm [file/dir]\033[0m   : Remove file or directory\n";
  std::cout << "  \033[1;36mtouch [file]\033[0m    : Create a file named [file]\n";
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
  if (command == "help")
    return validCommands::help;

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

static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
  ((std::string *)userp)->append((char *)contents, size * nmemb);
  return size * nmemb;
}

void getGeminiApiResponse(const std::string &prompt)
{
  string modifiedPrompt = "Answer this query in 50 words at max and just give plain text no markdown: " + prompt;
  CURL *curl;
  CURLcode res;
  std::string readBuffer;

  // Initialize CURL
  curl_global_init(CURL_GLOBAL_DEFAULT);
  curl = curl_easy_init();

  if (curl)
  {
    std::string apiKey = getenv("apiKey");
    std::string url = "https://generativelanguage.googleapis.com/v1beta/models/gemini-1.5-flash-latest:generateContent?key=" + apiKey;

    // Setup the URL and HTTP headers
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);

    // Setup the headers (replace YOUR_API_KEY with your actual key)
    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    // Prepare JSON data (the prompt to send to the API)
    json request_body;
    request_body["contents"] = {{{"parts", {{"text", modifiedPrompt}}}}};

    // Convert JSON data to string and set it as POST data
    std::string json_data = request_body.dump();
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_data.c_str());

    // Set up a callback function to receive the response
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

    // Perform the request
    res = curl_easy_perform(curl);

    // Check for errors
    if (res != CURLE_OK)
    {
      fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
    }

    // Cleanup
    curl_easy_cleanup(curl);
    curl_slist_free_all(headers);
  }

  curl_global_cleanup();
  json responseJson = json::parse(readBuffer);
  if (responseJson.contains("candidates") && !responseJson["candidates"].empty())
  {
    const auto &parts = responseJson["candidates"][0]["content"]["parts"];
    for (const auto &part : parts)
    {
      std::string text = part["text"];
      std::cout << "\033[32m>> AI Response: " << text << std::endl;
    }
  }
  else
  {
    std::cout << "No valid response found." << std::endl;
  }
}

std::string getSuggestion(std::string input)
{
  for (auto word : history)
  {
    if (word.size() >= input.size())
    {
      if (word.substr(0, input.size()) == input)
      {
        return word;
      }
    }
  }
  return "";
}

string getInputWithSuggestion()
{
  char ch;
  std::string last_suggestion;
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
  return input;
}

int main()
{
  struct termios original;
  tcgetattr(STDIN_FILENO, &original); // Get the terminal attributes

  enableRawMode(original); // Enable raw mode
  displayWelcomeMessage();
  bool exit = false;
  string input;
  bool ai_chat_mode = false;

  while (!exit)
  {
    std::cout << std::unitbuf;
    std::cerr << std::unitbuf;

    while (true)
    {
      if (ai_chat_mode)
      {
        std::cout << "\033[32m>>> AI chat active || enter 'exit-chat' command to exit AI chat mode " << std::endl;
        std::cout << "\033[32m>> \033[0m";
        std::getline(std::cin, input);
      }
      else
      {
        cout << "$ ";
        input = getInputWithSuggestion();
      }
      if (input == "ai-chat")
      {
        cout << "Entering AI chat mode" << endl;
        ai_chat_mode = true;
        disableRawMode(original);
        continue;
      }
      if (ai_chat_mode)
      {
        if (input == "exit-chat")
        {
          ai_chat_mode = false;
          std::cout << "Exiting AI chat mode.\n";
          enableRawMode(original);
        }
        else
        {
          getGeminiApiResponse(input);
        }
        continue;
      }

      switch (isValid(input))
      {
      case help:
        display_help();
        break;
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
}
