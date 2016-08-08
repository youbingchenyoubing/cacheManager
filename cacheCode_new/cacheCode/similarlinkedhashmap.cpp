#include"similarlinkedhashmap.h"

/* Cache工厂类*/
// 创建cache管理实例
 CacheManager * CacheFactory::createCacheManager()
{

	return CacheManager::getInstance();
}

 // 创建配置文件管理实例
 ConfigureManager *  CacheFactory::createConfigureManager()
 {

 	 return ConfigureManager::getInstance();
 }
// 创建用户请求管理机制 
 UserManager * CacheFactory::createUserManger()
{

	return UserManager::getInstance();
}

 MemoryManager * CacheFactory::createMemoryManager()
{
	//还没实现 
   return  MemoryManager::getInstance();
}

 IOManager * CacheFactory::createIOManager()
{

  return  IOManager::getInstance();
}
/*
ProcessManager <T>* CacheFactory::createProcessManager(int listenfd, unsigned int processNumber)
{
 
  return  ProcessManager::getInstance(int listenfd,unsigned int processNumber);

}*/
/*结束工厂类的实现*/

/*实现内存池管理类*/
MemoryManager::MemoryManager()
{
	//classPool = NULL;
	//cachePool = NULL;
	//classPool = ngx_create_pool(4096,NULL);
}
/*
MemoryManager::~MemoryManager()
{
	
   ~Manager();
}*/
//  打印内存池的内容
//MemoryManager::printfPool()
//{
	
//}

 MemoryManager* MemoryManager:: getInstance()
{
  static MemoryManager memoryInstance;
	return & memoryInstance;
}

/*ngx_pool_t * MemoryManager::getClassPool()
{
	return classPool;
}*/
/*ngx_pool_t * MemoryManager::getCachePool()
{
	return cachePool;
}*/
/*void  MemoryManager::buildCachePool(configureInfo *configureFile)
{
	if(configureFile == NULL)
		 exit(1);

	cachePool =  ngx_create_cache_pool(((configureFile->maxBlockNum)/2)*(configureFile->blockSize),NULL); //这个目前暂时预定，到时候测试在修改
}*/
/*实现 cache管理类*/

 CacheManager * CacheManager:: getInstance()
{

  static CacheManager cacheInstance;

	return &cacheInstance;
}

CacheManager::CacheManager()
{

	 mapManager.clear();
	 linkListHead = linkListTail = NULL;
   ioInstance = CacheFactory::createIOManager();
}
/*
CacheManager::~CacheManager()
{
  ~Manager();
}*/
void * CacheManager::searchBlock(char * fileName,int index,ConfigureManager *configureInstance)
{

   m_maplocker.rdlock();
   map<char *, map<unsigned int,LinkListNode *> > ::iterator it = mapManager.find(fileName);
   if(it == mapManager.end())
   {
       m_maplocker.unlock();
       return  getIoBlock(); 
   }
   else
   {
      map<unsigned int,LinkListNode *>::iterator it_index = it->second.find(index);

      if(it_index == it->second.end())
      {

          m_maplocker.unlock(); 
          configureInfo = configureInstance-> readConfigure();
         if (curBlockNum < configureInfo->maxBlockNum)
         {
              return getIoBlock(fileName,index);
         }

         else 
         {
             return getIoBlock2(fileName,index);
         }

      } // end  if(it_index == it->second.end())
      else  //命中cache
      { 
         cout<<"cache 命中了"<<endl

         if(linkListInsertHead(it_index->second)) cout<<"运行LRU 算法成功"<<endl;
         m_maplocker.unlock();
         return it_index->second;
      }
   } //end else{}

}
void CacheManager::getIoBlock(char * fileName,int index)
{
   
    cout<<"开启IO读取模式"<<endl;
    m_maplocker.wrlock();
    LinkListNode * temp = new LinkListNode();
    curBlockNum++;  //当前内存中的块数加1
    m_maplocker.unlock();
   
    //io去读数据

    m_maplocker.wrlock();

    m_maplocker.unlock();


}
bool CacheManager::addListNode(char *fileName,int index,LinkListNode * temp)
{
    map<char *, map<unsigned int,LinkListNode *> > ::iterator it = mapManager.find(fileName);
    
    else 
    {

    }

}
bool CacheManager::linkListInsertHead(LinkListNode *nodeInfo)
{
	if (nodeInfo == NULL)
      {
           	cout<<"ERROR: 链表插入时候，传进来竟然是空节点，不能忍"<<endl;
           	return false;
      }
    if ( linkListHead == NULL && linkListTail == NULL) //链表是为 空的
    	{
    		linkListHead = linkListTail = nodeInfo;

    		linkListHead->pre = NULL;

    		linkListTail->post = NULL;

    	}

     else 
     {
     	if ( linkListHead == NULL || linkListTail == NULL)  // 正常情况这种情况是永远不会成立的，但是为了防止出现不可控的情况
            {
            	cout<<"EORROR: 在类CacheManager中的函数linklistInsertHead,链表头指针 和 尾部 指针 中有一个指向空指针，这个是明显有问题,请注意"<<endl;
            	return false;
            }  

          else
          {
          	linkListHead-> pre = nodeInfo;
          	nodeInfo->post = linkListHead;
            nodeInfo->pre = NULL;
            linkListHead = nodeInfo;
          }

          
      }
      return true;
}

bool CacheManager::linkListDisplaceNode(LinkListNode *nodeInfo)  
{
	if ( nodeInfo == NULL || linkListHead == NULL || linkListTail == NULL)
		{

		    cout<<"ERROR: 链表替换时候，传进来竟然是空节点，不能忍"<<endl;
			return false;
		}
	if (linkListHead == nodeInfo)  // 命中的节点就是首部节点，就不要替换了
	 	{
	 		return true;
	 	}

     else if(  nodeInfo == linkListTail) //这个运气也挺好嘛，尾部的数据被命中了。
     {
          linkListTail = nodeInfo->pre; // 改变 尾部指针的位置
          linkListTail-> post = NULL;
          nodeInfo->post = linkListHead; //将原来首部节点 放在nodeInfo后面

          linkListHead-> pre = nodeInfo; 

          nodeInfo-> pre = NULL;

          linkListHead = nodeInfo;

     }

     else  //要替换节点 处于一般情况
     {

     	 nodeInfo->pre->post = nodeInfo->post;

     	 nodeInfo->post->pre = nodeInfo->pre;

     	 // 这时断开nodeinfo原有的 连接

     	 nodeInfo->post = linkListHead;

     	 linkListHead -> pre = nodeInfo;

     	 nodeInfo->pre = NULL;

     	 linkListHead  = nodeInfo;

     }

     return  true;
}
// 结束cache管理类实现
// 开始实现配置 文件类的实现
/*
ConfigureManager::~ConfigureManager()
{
  ~Manager();
}*/
ConfigureManager::ConfigureManager(): configureFile(new configureInfo)
{
	//configureFile = static_cast<configureInfo *>(ngx_palloc(memory_Instance->getClassPool(),sizeof(configureFile)));
	if (configureFile)
	{
    //cout<<"test"<<endl;
		configureFile->blockSize = 0;

		configureFile->maxBlockNum = 0;

		configureFile->filePath = "";

		configureFile->port = 1234;

		configureFile->ip = "127.0.0.1";

		configureFile->worker = 1;
	}
  //printfConfigure();
}

void ConfigureManager::printfConfigure()
{
   if (configureFile)
   {
     cout<<"configureFile->blockSize="<<configureFile->blockSize<<endl;
     
     cout<<"configureFile->maxBlockNum="<<configureFile->maxBlockNum<<endl;

     cout<<"configureFile->filePath="<<configureFile->filePath<<endl;

     cout<<"configureFile->port="<<configureFile->port<<endl;

     cout<<"configureFile->ip="<<configureFile->ip<<endl;

     cout<<"configureFile->worker="<<configureFile->worker<<endl;
   }
}
// 
/*void ConfigureManager::writeSomething(char *fileName)
{
	parsexml parseConfig(fileName,configureFile);

}*/
shared_ptr <configureInfo>  ConfigureManager::readConfigure()
{
	return configureFile;
}


ConfigureManager* ConfigureManager::getInstance()
{
  static ConfigureManager confiureInstance;

  return &confiureInstance;
}
//结束配置类的实现
// 客户端事件管理 类实现
/*
UserManager::~UserManager()
{
    ~Manager();
}*/
UserManager::UserManager()
{
  if (!postUserInfo.empty())
  {
    cout<<"Warning,程序的事件不为空"<<endl;
    //postUserInfo.clear();
  }
}

userInfo * UserManager::readSomething()
{

     userInfo * data = NULL;
    if( !postUserInfo.empty())
    {
        data = postUserInfo.front();

        postUserInfo.pop();
    }

    return data;
}


bool UserManager::writeSomething(userInfo *userdata)
{
   if (userdata != NULL)
   {

       postUserInfo.push(userdata);

       return true;
   }

    return false;
}

void UserManager::caculateInfo(userInfo *userdata,configureInfo *configureFile)
{
  //待会实现
}

 UserManager* UserManager::getInstance()
{
    static UserManager userInstance;

    return &userInstance;
}
//结束用户管理类的配置

//开始实现IO类

IOManager::IOManager()
{

}


IOManager * IOManager::getInstance()
{
     static IOManager ioInstance;

     return & ioInstance;
}
