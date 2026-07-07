#include <vector>
#include <string>

class CgiHandler {
public:
    CgiHandler(const std::string& scriptPath, const HttpRequest& req);
    
    bool start();             
    int  getInputFd();   
    int  getOutputFd();  
    bool isFinished(); 
    std::string getOutput();
private:
    pid_t pid;
    int inputPipe[2]; 
    int outputPipe[2]; 
    std::vector<std::string> buildEnv(const HttpRequest& req);
};