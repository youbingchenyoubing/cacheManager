#include"similarlinkedhashmap.h"

/* Cache工厂类*/
// 创建cache管理实例
 CacheManager * CacheFactory::createCacheManager(ConfigureManager * configureInstance)
{

	return CacheManager::getInstance(configureInstance);
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
/*CacheManager::GC CacheManager::gc;*/
IOManager* CacheManager::ioInstance = CacheFactory::createIOManager();
 CacheManager * CacheManager:: getInstance(ConfigureManager * configureInstance)
{

  static CacheManager cacheInstance(configureInstance);

	return &cacheInstance;
}

CacheManager::CacheManager(ConfigureManager * configureInstance)
{

	 mapManager.clear();
	 linkListHead = linkListTail = NULL;
   //ioInstance = CacheFactory::createIOManager();
   configureContent = configureInstance-> readConfigure();
   guardNode = new  LinkListNode(configureContent->blockSize);

   if(!guardNode)
   { 

    throw std::exception();

   }


}
/*
CacheManager::~CacheManager()
{
  ~Manager();
}*/
void * CacheManager::searchBlock(char * fileName,int index,unsigned int & g_Info)
{

    #ifdef __CACHE_DEBUG
      cout<<"index="<<index<<endl;
      #endif
   string file = fileName;
   m_maplocker.rdlock();
   /*   
   if(mapManager.empty()) 
   {
      m_maplocker.unlock();
      return getIoBlock();
   }*/
    /*
    map<string, map<unsigned int,LinkListNode *> > ::iterator it33 = mapManager.begin();
    for(;it33!=mapManager.end();++it33)
    {
         cout<<"it->first="<<it33->first<<endl;
        map<unsigned int,LinkListNode *> temp = it33->second;
         map<unsigned int,LinkListNode *>::iterator it44 = temp.begin();
         for(;it44!=temp.end();++it44)
          cout<<"it44->index="<<it44->first<<endl;
    }*/
   map<string, map<unsigned int,LinkListNode *> > ::iterator it = mapManager.find(file);
   if(it == mapManager.end())
   {
       if( curBlockNum < configureContent->maxBlockNum)

       {
       m_maplocker.unlock();
       return  getIoBlock(fileName,index,g_Info); 
     }
     else 
     {
        m_maplocker.unlock();
        return getIoBlock2(fileName,index,g_Info);
     }
   }
   else
   {

      map<unsigned int,LinkListNode *>::iterator it_index = it->second.find(index);

      if(it_index == it->second.end())
      {

        
         if (curBlockNum < configureContent->maxBlockNum)
         {
              m_maplocker.unlock();
              /*
              #ifdef __CACHE_DEBUG
              cout<<"更新成功"<<endl;
              #endif*/
              return getIoBlock(fileName,index,g_Info);
         }

         else 
         {
             m_maplocker.unlock();
             return getIoBlock2(fileName,index,g_Info);
         }

      } // end  if(it_index == it->second.end())
      else  //命中cache
      { 
         #ifdef __CACHE_DEBUG
         cout<<"cache 命中了"<<endl;
         cout<<"命中数据是"<<it_index->second->data<<endl;
         #endif
         if(linkListDisplaceNode(it_index->second)) 
           #ifdef __CACHE_DEBUG
          cout<<"运行LRU 算法成功"<<endl;
          #endif
         m_maplocker.unlock();
         return it_index->second->data;
      }
   } //end else{}

}
void * CacheManager::getIoBlock(char * fileName,int index,unsigned int & g_Info)
{
    #ifdef __CACHE_DEBUG
    cout<<"开启IO读取模式"<<endl;
    #endif
    string file = fileName;
   
    LinkListNode * temp = new LinkListNode(fileName,index,configureContent->blockSize);
   
    if(!(ioInstance->AIORead(fileName,index,configureContent,g_Info,temp,false)))
    {
          //mutex_Lock.unlock();
         
          if(temp) delete temp;
          //curBlockNum--;
          return nullptr;
    }
   
    m_maplocker.rdlock();
    if(mapManager.find(file)!=mapManager.end() && mapManager[file].find(index)!=mapManager[file].end())
    {
         m_maplocker.unlock();
         //m_maplocker.unlock();
         if(temp) delete temp;
         #ifdef __CACHE_DEBUG
         cout<<"假命中"<<endl;
         #endif

         return (mapManager[file][index])->data;
    }
    m_maplocker.unlock();
    m_maplocker.wrlock();
    linkListInsertHead(temp);
    addMap(file,index);
    if(curBlockNum<configureContent->maxBlockNum)
    {
       curBlockNum++;
    }
    else
    {
      deleteLinklistTail();
    }
    //mutex_Lock.unlock();
     m_maplocker.unlock();
    return linkListHead->data;
    //m_maplocker.unlock();


}
void * CacheManager::getIoBlock2(char * fileName,int index,unsigned int & g_Info)
{

   #ifdef __CACHE_DEBUG
   cout<<"缓存空间已满,现在是LRU算法"<<endl;
   #endif
   string file = fileName;

    if(!(ioInstance->AIORead(fileName,index,configureContent,g_Info,guardNode,true)))
    {
          //mutex_Lock.unlock();
          return nullptr;
    }
    
    m_maplocker.rdlock();
    if(mapManager.find(file)!=mapManager.end() && mapManager[file].find(index)!=mapManager[file].end())
    {   
         mutex_Lock.unlock(); //一定要释放
         m_maplocker.unlock();
        #ifdef __CACHE_DEBUG
          cout<<"假命中"<<endl;
         #endif
         return (mapManager[file][index])->data;
    }
    m_maplocker.unlock();
    m_maplocker.wrlock();
    deleteMap();
    addMap(file,index);
    m_maplocker.unlock();
    return linkListHead->data;
    
}
bool CacheManager::deleteMap()
{
     linkListInsertHead(guardNode);


     guardNode = linkListTail;
     
     linkListTail = guardNode->pre;
     string name = guardNode->name;
     int index = guardNode->offset;
     mutex_Lock.unlock();
     linkListTail->post = NULL;
      


     /*在map中删除guardNode中fileName和index*/
    deleteMap2(name,index);
    return true;
    
}
bool CacheManager::deleteMap2(const  string &fileName,int index)
{
    /*在map中删除LinkListTail中fileName和index*/
     mapManager[fileName][index] = nullptr;
     map<unsigned int,LinkListNode *> *mapSecond = &(mapManager[fileName]);
     mapSecond->erase(index);

     if(mapSecond->empty())
     {
        mapManager.erase(fileName);
     }
     return true;
}
bool CacheManager::addMap(string const & fileName,int index)
{

    map<string, map<unsigned int,LinkListNode *> > ::iterator it = mapManager.find(fileName);

    if(it == mapManager.end())
    {
       map<unsigned int,LinkListNode *> temp;
       temp[index] = linkListHead;

       mapManager[fileName] = temp;
      
    }
    else
    {

        (it->second)[index] = linkListHead;
        

    }
   #ifdef __CACHE_DEBUG
    cout<<"addMap成功 "<<endl;
    #endif

}
bool CacheManager::linkListInsertHead(LinkListNode *nodeInfo)
{
	if (nodeInfo == NULL)
      {
             #ifdef __CACHE_DEBUG
           	cout<<"ERROR: 链表插入时候，传进来竟然是空节点，不能忍"<<endl;
            #endif
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
               #ifdef __CACHE_DEBUG
            	cout<<"EORROR: 在类CacheManager中的函数linklistInsertHead,链表头指针 和 尾部 指针 中有一个指向空指针，这个是明显有问题,请注意"<<endl;
              #endif
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
        #ifdef __CACHE_DEBUG
		    cout<<"ERROR: 链表替换时候，传进来竟然是空节点，不能忍"<<endl;
        #endif
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
bool CacheManager::deleteLinklistTail()
{
   LinkListNode * temp = linkListTail;
   if(linkListTail!=NULL&&linkListTail!=linkListHead)
   {
       linkListTail = linkListTail->pre;
       linkListTail->post = NULL;
       deleteMap2(temp->name,temp->offset);
       delete temp;
       temp = NULL;
       return true;
   }
   else
   {

        #ifdef __CACHE_DEBUG
        cout<<"ERROR: 只剩下一个链表，明显出错 ，除非块就是一个"<<endl;
        #endif
        return false;
   }

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

		configureFile->filePath = "/service/";

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


bool IOManager::AIORead(char *fileName,int index,shared_ptr <configureInfo> configureContent,unsigned  int & g_Info,LinkListNode * guardNode,bool flag)
{
   int fd,ret = -1;

   char *buf;

   aio_context_t ctx;
  
   struct io_event event;

  

   struct iocb cb;

   struct iocb *cblist[] = {&cb};
   char file[50];
   
   strcpy(file,(configureContent->filePath).c_str());
   strcat(file,fileName);
    #ifdef __CACHE_DEBUG
   cout<<"开启Native aio 读取文件:"<<file<<endl;
   #endif
   fd = open(file,O_RDONLY|O_DIRECT);

   if(fd == -1)
   {
        g_Info = 700;
        #ifdef __CACHE_DEBUG
        cout<<"打开文件错误"<<endl;
        #endif
        return false;
   }

   buf = (char *)aligned_alloc(512,configureContent->blockSize); // aligned_alloc() 申请存放读取内容的 buffer，起始地址需要和磁盘逻辑块大小对齐（一般是 512 字节）。
   
   if(!buf)
   {

      close(fd);

      g_Info = 800;
      #ifdef __CACHE_DEBUG
      cout<<"服务器空间申请失败"<<endl;
      #endif

      return false;
   }
  
   memset(&ctx,0,sizeof(ctx));

   ret = io_setup(NR_EVENT,&ctx);


   if (ret!=0)
   {

      free(buf);

      close(fd);
      #ifdef __CACHE_DEBUG
      cout<<"io_setup failed:"<<errno<<"\n";
      #endif

      g_Info = 900;

      return false;
   }
   
   memset(&cb,0,sizeof(cb));

   cb.aio_data = (__u64)buf;

   cb.aio_fildes = fd;

   cb.aio_lio_opcode = IOCB_CMD_PREAD;

   cb.aio_buf = (__u64)buf;

   cb.aio_offset = (__s64)index*(configureContent->blockSize);

   cb.aio_nbytes = configureContent->blockSize;



  ret = io_submit(ctx,1,cblist);

  if(ret != 1)
  {
    io_destroy(ctx);
    free(buf);
    close(fd);
    g_Info = 900;
     #ifdef __CACHE_DEBUG
     cout<<"io_submit error:"<<errno<<endl;
     #endif

     return  false;
  }
 

  ret = io_getevents(ctx,1,1,&event,NULL);

  if(ret!=1)
  {
        
      io_destroy(ctx);
      free(buf);
      close(fd);
      #ifdef __CACHE_DEBUG
      cout<<"io_getevents error:"<<errno<<endl;
      #endif

       g_Info = 900;

       return false;
  }

  if(event.res <= 0)
  {
        #ifdef __CACHE_DEBUG
       cout<<"io error:"<<event.res<<endl;
       #endif
       io_destroy(ctx);
       free(buf);
       close(fd);
       g_Info = 401;

       return false;
  }

  else {
    //write(1, (const void*)(event.data), event.res);
    if(flag) //当使用哨兵的时候要用
    {
     mutex_Lock.lock();
     guardNode->name  =  fileName;
     guardNode->offset = index;
    }
    memcpy(guardNode->data,(const void*)(event.data),event.res);
    guardNode->length = event.res;
    io_destroy(ctx);
    free(buf);
    close(fd);
    #ifdef __CACHE_DEBUG
    if(event.res< configureContent->blockSize)
        cout<<"WARNING!!!读出了"<<event.res<<"个字节，但是块的大小是："<<configureContent->blockSize<<"字节"<<endl;
    #endif
    return true;
  }
}