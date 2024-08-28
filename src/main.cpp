#include <iostream>
#include <string>
#include <sstream>
#include <filesystem>

using namespace std;

enum validCommands { 
  echo,
  cd,
  exit0,
  type,
  invalid
};
validCommands isValid(string command){
  command = command.substr(0,command.find(" "));

  if(command == "echo") return validCommands::echo;
  if(command == "cd") return validCommands::cd;
  if(command == "exit") return validCommands::exit0;
  if(command == "type") return validCommands::type;

  return invalid;
}

std::string valid[4] = {"echo", "cd", "exit0"};

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

int main()
{
  // Flush after every std::cout / std:cerr
  bool exit = false;
  while(!exit){
    std::cout << std::unitbuf;
    std::cerr << std::unitbuf;
    std::cout << "$ ";
    std::string input;
    std::getline(std::cin, input);

    switch(isValid(input)){
      case cd:
        break;
      case echo:
        input.erase(0,input.find(" ")+1);
        std::cout<<input<<"\n";
        break;
      case exit0:
        exit = true;
        break;
      case type:
        input.erase(0,input.find(" ")+1);
        if(isValid(input) != invalid){
          std::cout<<input<<" is a shell builtin\n";
        }
        else{
          std::string path = get_path(input);
          if(path.empty()){
            std::cout<<input<<" not found\n";
          }
          else{
            std::cout<<input<<" is "<<path<<std::endl;
          }
        }
        break;
      default:
        std::cout<<input<<": command not found\n";
        break;
    }
  }

}
