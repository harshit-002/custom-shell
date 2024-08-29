#include <iostream>
#include <string>
#include <sstream>
#include <filesystem>

using namespace std;
namespace fs = std::filesystem;

void displayWelcomeMessage() {
    std::cout << R"(
________  ________  ________  ___  ___          ___       ___  _________  _______      
|\   __  \|\   __  \|\   ____\|\  \|\  \        |\  \     |\  \|\___   ___\\  ___ \     
\ \  \|\ /\ \  \|\  \ \  \___|\ \  \\\  \       \ \  \    \ \  \|___ \  \_\ \   __/|    
 \ \   __  \ \   __  \ \_____  \ \   __  \       \ \  \    \ \  \   \ \  \ \ \  \_|/__  
  \ \  \|\  \ \  \ \  \|____|\  \ \  \ \  \       \ \  \____\ \  \   \ \  \ \ \  \_|\ \ 
   \ \_______\ \__\ \__\____\_\  \ \__\ \__\       \ \_______\ \__\   \ \__\ \ \_______\
    \|_______|\|__|\|__|\_________\|__|\|__|        \|_______|\|__|    \|__|  \|_______|
                       \|_________|                                                     )" << std::endl;
}

enum validCommands { 
  echo,
  cd,
  exit0,
  type,
  pwd,
  invalid
};
validCommands isValid(string command){
  command = command.substr(0,command.find(" "));

  if(command == "echo") return validCommands::echo;
  if(command == "cd") return validCommands::cd;
  if(command == "exit") return validCommands::exit0;
  if(command == "type") return validCommands::type;
  if(command == "pwd") return validCommands::pwd;

  return invalid;
}

std::string valid[5] = {"echo", "cd", "exit0", "type", "pwd"};

std::string get_path(const std::string command){
  //PATH="/usr/bin:/usr/local/bin" ./your_program.sh
  std::string path_env = std::getenv("PATH");
  std::stringstream ss(path_env);
  std::string path;

  while(getline(ss,path,':')){        //Read each directory in PATH
    std::string abs_path = path+"/"+command;
    if(std::filesystem::exists(abs_path)){
      return abs_path;
    }
  }
  return "";    // Return empty string if command not found in PATH
}

void chdir(std::string dir){
  fs::path newDirectory(dir);               // convert std::string to fs::path
  if(fs::is_directory(newDirectory)){   
    fs::current_path(newDirectory);        // set current path to dir
  }
  else cout<<"cd: "<<dir<<": No such file or directory\n";
}

int main()
{
  displayWelcomeMessage();
  bool exit = false;
  while(!exit){
    std::cout << std::unitbuf;
    std::cerr << std::unitbuf;
    std::cout << "$ ";
    std::string input;
    std::getline(std::cin, input);

    switch(isValid(input)){
      case cd:
        {
          string dirStr = input.substr(input.find(" ")+1);  
          if(dirStr=="~"){
            const char* homeDir_cStr = std::getenv("HOME");   // gives home directory
            std::string homeDir(homeDir_cStr);
            chdir(homeDir);
          }
          else chdir(dirStr);
          break;
        }
      case echo:
        input.erase(0,input.find(" ")+1);
        std::cout<<input<<"\n";
        break;
      case exit0:
        exit = true;
        break;
      case pwd:
        std::cout<<fs::current_path().string()<<endl;
        break;
      case type:
        input.erase(0,input.find(" ")+1);
        if(isValid(input) != invalid){
          std::cout<<input<<" is a shell builtin\n";
        }
        else{
          std::string path = get_path(input);
          if(path.empty()){
            std::cout<<input<<": not found\n";
          }
          else{
            std::cout<<input<<" is "<<path<<std::endl;
          }
        }
        break;
      default:
        // find program and execute using system
        string command = input.substr(0,input.find(" "));
        string path = get_path(command);
        if(path.empty()){
          std::cout<<input<<": command not found\n";
        }
        else{
          string command_with_full_path = path + input.substr(command.length());  // full = path+(space)+arguments

          const char *command_ptr = command_with_full_path.c_str();
          system(command_ptr);
        }
        break;
    }
  }
}
