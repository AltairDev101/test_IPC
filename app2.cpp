#include <thread>
#include <stdexcept>
#include <sys/stat.h>
#include <unistd.h>
#include <iostream>
#include <fstream>
#include <string>
#include <fcntl.h>
#include <mutex>
#include <chrono>
#include <queue>
#include <ctime>
#include <algorithm>
#include <condition_variable>
#include <cstring>

using namespace std;

#define pipe1 "/tmp/fifo1"
#define pipe2 "/tmp/fifo2"
#define LOG_FILE "/tmp/log_app2.log" 

mutex log_mutex; 
mutex queue_mutex;
std::condition_variable cv;

struct MessageInfo {
    string original_msg;   
    chrono::system_clock::time_point receive_time;  
    chrono::high_resolution_clock::duration process_duration_ticks; 

    string get_receive_time_str() const {
        auto now = chrono::system_clock::to_time_t(receive_time);
        tm* local_time = localtime(&now);

        char time_buffer[100];
        strftime(time_buffer, sizeof(time_buffer), "%Y-%m-%d %H:%M:%S", local_time);

        return string(time_buffer);
    }

    string get_process_time_str() const {
        return to_string(process_duration_ticks.count());
    }
};

class NamePipe{
public:
	NamePipe()
	{
		if((mkfifo(pipe1, 0666)==-1) && (errno != EEXIST)){			
			throw std::runtime_error("Failed to create fifo1: " + std::string(strerror(errno)));
		}
		if((mkfifo(pipe2, 0666)==-1) && (errno != EEXIST)){			
			throw std::runtime_error("Failed to create fifo2: " + std::string(strerror(errno)));
		}
	}

	void send_string(const string &str){
		int fd = open(pipe2, O_CREAT | O_WRONLY);
		if((fd == -1) && (errno != EEXIST)){			
			throw::runtime_error("Failed to open pipe2 on write " + std::string(strerror(errno)));
		}
		write(fd, str.c_str(), str.size());
		close(fd);
	}

	string receive_string(void){
		char buffer[128];
		cout << "Listnening app1..." << endl;
		int fd = open(pipe1, O_RDONLY);				
		if((fd == -1) && (errno != EEXIST)){
			throw::runtime_error("Failed to open pipe1 on read: " + std::string(strerror(errno)));	
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

class Logger {
public:
    static void log(const MessageInfo& msg_info) {        
        lock_guard<mutex> lock(log_mutex);
        ofstream log(LOG_FILE, ios::app);
        log << "[" << msg_info.get_receive_time_str() << "] "
            << "Original: " << msg_info.original_msg
            << " | Processing time (ticks): " << msg_info.get_process_time_str() << endl;
    }
};

class MessageHandler{
public:
	MessageHandler(NamePipe &p) : pipe(p) {
		work = true;
	}
	void run(){
		thread receive_thread(&MessageHandler::receive_message, this);
		thread process_thread(&MessageHandler::process_message, this);		

		receive_thread.join();
		process_thread.join();		
	}
private:
	NamePipe &pipe;
	queue<MessageInfo> message_queue;
	bool work;

	void receive_message(){
		while(1){
			try{
				cout<<"Enter receiving..."<<endl;
				string msg = pipe.receive_string();
				if(msg.empty()){
					cout << "Received empty message. Exiting application." <<endl;
					work = false;
					break;
				}

				MessageInfo msg_info;
				msg_info.original_msg = msg;
				msg_info.receive_time = chrono::system_clock::now();

				{
					lock_guard<mutex> lock(queue_mutex);
					message_queue.push(msg_info);
				}

				cv.notify_one();

			}catch(const exception &e){
				cerr << "Error receiving message: " << e.what() << endl;
				break;
			}
		}
	}

	void process_message(){
		while(work){
			MessageInfo msg_info;							
			{
				unique_lock<mutex> lock(queue_mutex);
				cv.wait(lock, [this] { return (!message_queue.empty() || !work); });
				
				msg_info = message_queue.front();
				message_queue.pop();
			}

			auto start_time = chrono::high_resolution_clock::now();
			string rev = msg_info.original_msg;
			reverse(rev.begin(), rev.end());
			msg_info.process_duration_ticks = chrono::duration_cast<chrono::high_resolution_clock::duration>(chrono::high_resolution_clock::now() - start_time); 			
			Logger::log(msg_info);
			try{
				pipe.send_string(rev);				
			}catch(const exception &e){
				cerr << "Error sending message: " << e.what() << endl;
                break;
			}
		}
	}
};

int main()
{
	try{
		NamePipe pipe;
		MessageHandler handler(pipe);
		handler.run();
	}catch(const exception &e){
		cerr << "Error: " << e.what() << endl;
	}
	return 0;
}
