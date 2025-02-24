#include <stdexcept>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h> 
#include <unistd.h>
#include <iostream>
#include <string>
#include <cstring>

using namespace std;

#define pipe1 "/tmp/fifo1"
#define pipe2 "/tmp/fifo2"

class NamePipe{
public:
    NamePipe() {        
        if ((mkfifo(pipe1, 0666) == -1) && (errno != EEXIST)) {
            throw std::runtime_error("Failed to create fifo1: " + std::string(strerror(errno)));
        }
        
        if ((mkfifo(pipe2, 0666) == -1) && (errno != EEXIST)) {
            throw std::runtime_error("Failed to create fifo2: " + std::string(strerror(errno)));
        }
    }

	void send_string(const string &str){
		int fd = open(pipe1, O_CREAT | O_WRONLY);
		if((fd == -1) && (errno != EEXIST)){			
			throw::runtime_error("Failed to open pipe1 on write " + std::string(strerror(errno)));
		}
		write(fd, str.c_str(), str.size());
		close(fd);
	}

	string receive_string(void){
		char buffer[128];
		int fd = open(pipe2, O_RDONLY);
		if((fd == -1) && (errno != EEXIST)){
			throw::runtime_error("Failed to open pipe2 on read: " + std::string(strerror(errno)));	
		}
		memset(buffer,0,sizeof(buffer));		
		int nbytes = read(fd, buffer, sizeof(buffer));
		if(nbytes>0){
			cout<< "receiving: " << string(buffer) << endl;
		}

		close(fd);
		return string(buffer);
	}

	~NamePipe()
	{
		unlink(pipe1);
		unlink(pipe2);
	}
};

class WriterString
{
public:
	WriterString(NamePipe &p) : pipe(p) {}
	void run(){
		while (true) {
			try{
				string input;
				cout << "Enter a string or empty to exit: ";
				getline(cin, input);

				if (input.empty()) {
				   cout << "Exiting..." << endl;
				   break;
				}	        
				pipe.send_string(input);

				pipe.receive_string();
			}catch(const exception &e){
				cerr << "Error: " << e.what() << endl;
			}
    	}
	}
private:
	NamePipe pipe;
};

int main()
{
	try
	{		
		NamePipe pipe;
		WriterString writer(pipe);
		writer.run();
	}catch(const exception &e){
		cerr << "Error: " << e.what() << endl;
	}
	return 0;
}	

