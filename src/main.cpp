#include <iostream>
#include <string>
#include <map>
using namespace std;
int main()
{
  // Flush after every std::cout / std:cerr
  std::cout << std::unitbuf;
  std::cerr << std::unitbuf;

  std:: map<string,string> inBuiltCommands = {{"echo","echo is a shell builtin"},
      {"exit","exit is a shell builtin"},{"cat","cat is /bin/cat"},{"type","type is a shell builtin"}};

  while(true){
    std::cout << "$ ";
    std::string command;
    std::getline(std::cin, command);

    if(command=="exit 0"){
      break;
    }

    if(command.size()>4){
      std::string commandCat = command.substr(0,4);
      std::string commandText = command.substr(5);

      if(commandCat=="echo"){
        std::cout<<commandText<<endl;
      }

      if(commandCat=="type"){
        if(inBuiltCommands.find(commandText)!=inBuiltCommands.end())
          std::cout<<inBuiltCommands[commandText]<<endl;
        else std::cout<<commandText<<": not found"<<endl;
      }
    }
    else
    std::cout << command << ": command not found" << std::endl;

  }

}
