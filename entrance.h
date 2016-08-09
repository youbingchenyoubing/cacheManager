#ifndef ENTRANCE_H
#define ENTRANCE_H
# include "parsehash.h"
# include "writeDisk.h"
# include "thread.h"
# include "bitmap.h"

# include "similarlinkedhashmap.h" //管理资源
//# include "process.h" //进程池
//# include "thread.h"
//# include "locker.h"
# include "socket.h"
 //线程池
# include "parsexml.h"
# include "protocol.h"
# include "epoll_thread.h"
# include <string.h>
# include <string>
# define  MAX_FD 65536
# define  MAX_EVENT_NUMBER 10000
using namespace cachemanager;
struct ClientRequest
{
	char * fileName;  //文件名

	unsigned int offset; // 偏移量

	unsigned int len; // 请求长度

	//unsigned int beginOffset; // 请求数据第一块开始的偏移量，不等于上面的偏移量 ,这个是要根据，cache配置进行计算的

	//unsigned int endOffset; // 请求数据最后一块开始的偏移量。这个是要根据，cache配置进行计算的

	//void  *data;    //  如果是读事件就是为空

	char *type;   //表明用户是读、写事件

  //char * flag;   //如果是为真的，即使缓存没有也要立即从磁盘上读IO
	//char  *ip; // 客户端的ip地址

	//unsigned short int port; //客户端的端口号

	//bool done; //用来标示是否处理完这个事件
	ClientRequest()
	{
   
	}

};
// 配置文件类
static ConfigureManager * configureInstance = NULL;
//static UserManager * userInstance = NULL;
static CacheManager * cacheInstance = NULL;
static DiskManager * diskInstance  = NULL;
static shared_ptr<configureInfo> configure = NULL;
//static MemoryManager * memoryInstance = NULL;
//static IOManager * ioInstance = NULL;
/*用于处理客户的请求类*/
 class CacheConn
  {
  public:
  	CacheConn(){clientData == NULL;}
  	~CacheConn(){
      /*
      if(clientData)
      {
        delete clientData;
        clientData = NULL;
      }*/
    }
    //void init(int fd,)
    void close_conn();
    void init(int sockfd,const sockaddr_in & client_addr);
    void process();
    bool read();
    static int m_epollfd;
    static int m_user_count;//统计用户数量
   private:
   	  static const int BUFFER_SIZE = 1024;  //缓存区的大小
      static const int W_DATA = 1024;   //存储客户写数据的大小

      char m_write[W_DATA];  // 写文件块
   	  sockaddr_in m_address;
      int m_sockfd;
   	  char m_buf[BUFFER_SIZE];
   	  int m_read_idx; //标志读缓存区中已经读入客户数据的最后一个字节的下一个位置
      int m_write_idx;
   	  shared_ptr<ClientRequest> clientData;
      //ClientRequest *clientData;
   private:
       void init(); //初始化其他变量
       void printftest();//打印解析后文件，仅仅测试使用
       bool ParseRequest();
       void ResponesClient(unsigned int code, void *data = NULL);
       void writeDisk(); //处理客户端的写事件
       void readCache(); //处理客户端的读事件
       bool checkRequest(); //检查客户端请求是否合理
       
       void writevClient(void * data,const unsigned int &blockSize,const unsigned int &offset,const unsigned int  &len,const int & index);

       struct iovec m_iv[2]; //成功请求的时候，将采用writev来执行操作。
       
};
// 入口类
class Entrance{
public:
	Entrance();
	void parseParameters(int argc, char *const *argv); //解析这个程序运行的参数
    bool initProgram(); // 初始化 程序的必要的东西

    
 private:
 	bool openTCP_Process(shared_ptr <configureInfo> configurefile); //服务器开启TCP服务（进程池）
  void openTCP_Thread(shared_ptr <configureInfo> configurefile);
 	char *fileName; //存储配置文件名称
  //nt setNonBlocking(int sockfd);
    //queue<userInfo * > postUserInfo; //用来收集监听到的事件
   void setup_sig_pipe(int epollfd); // 初始化信号管道
    // queue<userInfo *> sendPostData; 优化的时候再考虑这个问题 

};

#endif
