#ifndef SIMILARLINKEDHASHMAP_H

#define  SIMILARLINKEDHASHMAP_H
#include <iostream>
#include <string>
//   不使用Nginx
# include <errno.h>
 #include <sys/types.h>
 #include <sys/stat.h>
 #include <fcntl.h> 
#include <stdio.h>
#include <memory>
#include <map>
#include <queue>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>
#include "locker.h"
#include "aio.h"
#define NR_EVENT 1024
//#include "parsexml.h"
/*一个双向链表，两个Map数据结构*/
static locker mutex_Lock;
using namespace  std;
//  双向链表的数据 结构
struct LinkListNode
{
	LinkListNode *pre;  // 指向前面节点
	LinkListNode *post; // 指向后面节点

	//char * name;
	string name; 
	void * data;
	int length;

	unsigned int offset;
	LinkListNode(string fileName,int index,size_t size)
	{
        name = fileName;
        offset = index;
        pre = NULL; 
        post = NULL;

        data = malloc(size);
        //memset(data,'\0',size);

        if(!data)
        {
        	perror("malloc");
            exit(1);
        }
	}
    LinkListNode(size_t size)
	{
        name = "";
        pre = NULL;
        post = NULL;

        data = malloc(size);

        if(!data)
        {
        	perror("malloc");
            exit(1);
        }
	}
	~LinkListNode()
	{

		pre = NULL;
		post = NULL;
		if(data)
		{
			free(data);

			data = NULL;
		}
	}
};
// 用来存储用户传过来的信息
struct userInfo
{
	char *fileName;  //文件名

	unsigned int offset; // 偏移量

	unsigned int len; // 请求长度

	unsigned int beginOffset; // 请求数据第一块开始的偏移量，不等于上面的偏移量 ,这个是要根据，cache配置进行计算的

	unsigned int endOffset; // 请求数据最后一块开始的偏移量。这个是要根据，cache配置进行计算的

	void  *data;    //  如果是读事件就是为空

	char  type;   //表明用户是读、写事件，还有去磁盘读数据

	char  *ip; // 客户端的ip地址

	unsigned short int port; //客户端的端口号

	bool done; //用来标示是否处理完这个事件

};
// 储存配置信息
struct configureInfo
{
	size_t  blockSize; // 内存块的大小，在读取配置文件时候，进行初始化

	unsigned int  maxBlockNum;// 最大申请规定大小的内存块

	
    size_t  diskSize; //磁盘分块大小
	string filePath; //提供服务的路径，文件都会在 这个路径进行查找

	unsigned short int port; // 默认的端口是1234
	string ip; // 默认IP是127.0.0.1

	unsigned int worker; //监听和处理的进程个数

	string levels; //制定hash文件名的规则


	unsigned int maxPiece; //最大文件分块块数，不是以上面磁盘分块为基础，而是以服务器接收的buffer为基数
    
     configureInfo()
     {

     }
};

// 定义Manager是为了以后代码可扩展
/*
class Manager
{
  public:
  	Manager();
	virtual void * readSomething();
	virtual bool writeSomething();
	virtual ~Manager(){};
};
*/
// 配置文件的管理器
class ConfigureManager
{
public:
	void  writeSomething(char *fileName); // 重定义，生成配置信息
	shared_ptr <configureInfo> readConfigure();  //得到这个cache的配置信息
    void printfConfigure();  // 测试接口
	static  ConfigureManager * getInstance();
	~ConfigureManager(){};
private:
	shared_ptr<configureInfo> configureFile; // 解析配置文件
	ConfigureManager();

	 // 替换成内存池操作
	
};
/*
// 用户管理器  没用
class UserManager
{
 public:
	userInfo * readSomething(); //从队列中取出事件处理的
	bool writeSomething(userInfo *userdata);  // 添加待处理事件到队列中去
	void caculateInfo(userInfo *userdata,configureInfo *configureFile); // 计算 userInfo中的 beginOffset成员和endOffset成员
	static UserManager * getInstance();
	~UserManager(){};
 private:
 	UserManager();
	 //static shared_ptr <UserManger> userInstance(new UserManger); // 替换成内存池操作
	 queue<userInfo * > postUserInfo;   //用队列存储用户请求事件
	 //queue<userInfo *>

};*/
// io管理器
class IOManager
{

  public:
      static IOManager * getInstance();
      ~IOManager(){};
      bool AIORead(const char * fileName,int index,shared_ptr <configureInfo> configureInfo,unsigned  int & g_Info,LinkListNode * guardNode,bool flag);
  private:
  	   IOManager();
  	   //void IOMmap();
       
};

// cache manager 管理器
class CacheManager
{

  public:
	//void * readSomething(userInfo * uInfo);  // 重定义，向外面提供的读接口
	//bool writeSomething(userInfo * uInfo); // 重定义，向外面提供的写 接口
	static CacheManager * getInstance(ConfigureManager * configureInstance);  //获得唯一的对象实例
	~CacheManager(){
	    deleteAll();
	}
	void deleteAll()
	{
       	mapManager.clear();
		if(guardNode!=NULL)
	      delete guardNode;
		guardNode = NULL;
		LinkListNode * node  = linkListHead;
		while(node!=NULL)
		{
			LinkListNode  *temp = node->post;

			delete node;

			node = temp;
		}
		linkListHead = linkListTail = NULL;
		//linkListHead = linkListTail = NULL;
	}
	void * searchBlock(const char *fileName,int block,unsigned int &g_Info); //查询数据块是否在内存,查询到直接发送过去 ，并改变队列的位置
  private:
	CacheManager(ConfigureManager * configureInstance); //构造函数，初始化参数

	void  caculateRead(userInfo * uInfo); // 计算某一块要发送多少数据过去

	
   
    bool deleteLinklistTail();
    bool deleteMap2(const  string &fileName,int index); //删除尾部指针的东西

    bool addMap(const string &fileName,int index);
    bool deleteMap();
	bool linkListInsertHead(LinkListNode *nodeInfo); // 将特定的节点插入到链表的首部

	bool linkListDisplaceNode(LinkListNode *nodeInfo); //替换节点到首部 节点 ，命中的时候，才会调用这个函数
    void * getIoBlock(const char * fileName,int index,unsigned int & g_info); //说明cache块数还没达到系统设定的要求

	void * getIoBlock2(const char * fileName,int index,unsigned int &g_info); // 系统的cache块数已经达到最大要求，要采用LRU算法替换
    unsigned int curBlockNum; // 现在链表中的内存块数量，初始化应该设置为0。
   
	LinkListNode * linkListHead; // 链表头部指针，初始化设为NULL；


	LinkListNode * linkListTail; // 链表尾部 指针，初始化应该为NULL；

    //int curBlockNum;
	//map<char  *,unsigned int> missFileName; // 用来表示不命中文件名，它表示就是缺失的次数
	//map<char *,map<unsigned   int,unsigned int>> missCache; //记录 具体 某一块缺失的次数
	//map<char *, map<unsigned int,LinkListNode *> > mapManager;  //这个结构是核心，为了快速定位到链表中的某个节点
     map<string,map<unsigned int,LinkListNode*>> mapManager;
     rwlocker m_maplocker;

     static IOManager * ioInstance;

    
	 //static shared_ptr <CacheManager>  cacheInstance(new CacheManager); //为了实现单例模式
    shared_ptr <configureInfo > configureContent;
    LinkListNode * guardNode;

 };
class LogManager
{
	public:

};
/*
// 内存 池 的实现 （利用Nginx实现）
class MemoryManager
{
   public:
	static MemoryManager *  getInstance();
	~MemoryManager(){};
	//ngx_pool_t * getClassPool();
	//ngx_pool_t * getCachePool();
	void buildCachePool(configureInfo *configureFile);
	//void printfPool(); // 查看内存池的内容
	private:
	 MemoryManager();
	 //ngx_pool_t * classPool;
	 //ngx_pool_t * cachePool;
	// static shared_ptr <MemoryManager> memoryInstance(new MemoryManager; // 唯一用new实现
  
};
*/




class CacheFactory
{
public:
	 static CacheManager * createCacheManager(ConfigureManager * configureInstance);
	 static ConfigureManager * createConfigureManager();
	 //static UserManager * createUserManger();
	 //static MemoryManager * createMemoryManager();
	 static IOManager * createIOManager();
	 //static LogManager * createLogManager();
	// template  <typename T>
	 //static ProcessManager <T>* createProcessManager(int listenfd, unsigned int processNumber);
};
#endif