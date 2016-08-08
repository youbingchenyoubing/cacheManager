# include "entrance.h"

int CacheConn::m_epollfd = -1;
int CacheConn::m_user_count = 0;
void CacheConn::init(int sockfd,const sockaddr_in & client_addr)
{
   //m_epollfd = epollfd;
   
   m_sockfd = sockfd;

   m_address = client_addr;
  /*
   memset(m_buf,'\0',BUFFER_SIZE);
   memset(m_write,'\0',W_DATA);
   m_read_idx = 0;
   m_write_idx = 0;*/
   /*以下两行是为了避免TIME_WAIT状态为了调试，实际应用将其去掉*/
   int reuse =  1;
   setsockopt(m_sockfd,SOL_SOCKET,SO_REUSEADDR,&reuse,sizeof(reuse));
   addfd(m_epollfd,sockfd,true);

   m_user_count++;
   init();
}
void CacheConn::init()
{
   memset(m_buf,'\0',BUFFER_SIZE);
   memset(m_write,'\0',W_DATA);
   m_read_idx = 0;
   m_write_idx = 0;

}
void  CacheConn::process()
{
     if (strcmp(clientData->type,"read")==0)
     {

          if(access(clientData->fileName,F_OK)==-1)
          {
          
           ResponesClient(NOFILE,NULL);

          }
         else
        
          readCache();

     }

     else 
    {

    }

    init();

}

//接收用户的请求
bool CacheConn::read()
{

   int idx = 0;
   //int indxW = 0; //收集用户的Content
   int ret =  -1; 
   // bool writeflag  = false;  // 标志解析协议是否正确
   while(true)
   {
      idx = m_read_idx;
      //indxW = m_read_idx;
      ret = recv(m_sockfd,m_buf+idx,BUFFER_SIZE-1-idx,0);
      // 操作错误，关闭客户端连接，但是如果是暂时无数据可读，则退出循环
      if( ret < 0)
      {
        if (errno == EAGAIN||errno == EWOULDBLOCK)
        {
          ResponesClient(ERRORDATA,NULL);
          break; 
        }
        return false;
      }// end if(ret < 0)
      // 客户端关闭连接
      else if( ret == 0)
      {
         return false;
      }// end else if(ret == 0)
     else
     {
        m_read_idx+=ret;
        cout<<"user send content is "<<m_buf<<endl;

      for(; idx < m_read_idx;++idx)
      {
         if((idx>=1)&&(m_buf[idx-1] == '\r')&&(m_buf[idx]=='\n'))
            break;
      }
      if (idx == m_read_idx)
          continue;
       m_buf[idx-1]='\0';
      if(ParseRequest()) //解析报文
       {
          if (strcmp(clientData->type,"read")==0)
            {
               cout<<"客户是发送过来读事件"<<endl;
                return  true;
            }
          else
          {

            cout<<"客户端发送过来的是写事件"<<endl;
            int i = 0;
            for(++idx;idx<m_read_idx;++idx,++i)
                 m_write[i]  = m_buf[idx];

            ret = recv(m_sockfd,m_write+i,clientData->len-i,0);

            if (ret <=0)
            {

              ResponesClient(ERRORBODY,NULL);
              return false;
            } //end if(ret<=0)
            else  //真正写事件
            {
               return true;
            }
                
          }// end else
       }//end  if(ParseRequest())
      else 
      {
         cout<<"客户端了发送错误数据"<<endl;
         ResponesClient(ERRORHEADER,NULL);
         return false;
      }
     }//end else
   }// end while(true)
   
}
//处理客户端读的请求
void CacheConn::readCache()
{

   unsigned int blockSize = configureInstance->readConfigure()->blockSize;
   unsigned int beginBlock =  (clientData->offset)/blockSize;
   unsigned int endBlock = (clientData->offset+clientData->len)/blockSize;
   if((clientData->offset+clientData->len)%blockSize)
         endBlock+=1;
   void *  data = nullptr;
   for( unsigned  int i=beginBlock;i<=endBlock;++i)
   {
       //if(configureInstance->)
      data = cacheInstance->searchBlock(clientData->fileName,i,configureInstance);
   } 
   
}
// 判读客户请求是否合理
bool CacheConn::checkRequest()
{
  //这里写检查代码，还没实现 
  return true;
}
//解析应用层的头部协议
bool CacheConn::ParseRequest()
{
   clientData = shared_ptr<ClientRequest>(new  ClientRequest);
  //clientData = new ClientRequest();
   char * buf = m_buf;
   int i = 0;
   bool isbegin = true;
   while(*buf!='\0')
   {

     if(*buf == '/')
     {
         *buf = '\0';
         isbegin = true;
         ++i;
     }
     else if(isbegin)
     {
        switch(i)
        {
          case 0: clientData->type = buf;break;
          case 1: clientData->fileName = buf;break;
          case 2: clientData->offset = atoi(buf);break;
          case 3: clientData->len = atoi(buf); break;
          case 4: clientData->flag = buf;break;
          default:break;
        }
        isbegin = false;
     }//end else if
     buf++;
   }//end while
   //printftest();
   if(strcmp(clientData->type,"read")==0 || strcmp(clientData->type,"write")==0)
         return true;

   else  return false;
   
}
void CacheConn::printftest()
{
  cout<<"打印信息用户发送过来的信息"<<endl;
  if(clientData)
  {
     cout<<"clientData->type="<<clientData->type<<endl;

      cout<<"clientData->fileName="<<clientData->fileName<<endl;
  }
}
//响应客户的请求 (没有发送成功数据的)
void CacheConn::ResponesClient(unsigned int code,void *data)
{
  if(m_sockfd !=-1)
  {

    char codeTrans[4];
    cout<<code<<endl;
    char  sendData[10];
    memset(sendData,'\0',10);
    snprintf(codeTrans,sizeof(codeTrans),"%u",code);

    strcat(sendData,codeTrans);
    strcat(sendData,"\r\n");
    send(m_sockfd,sendData,strlen(sendData),0);

    return ;
  }
}
//主动关闭连接
void CacheConn::close_conn()
{
  if((m_sockfd != -1))
  {
    removefd(m_epollfd,m_sockfd);
    close(m_sockfd);
    m_sockfd = -1;
    m_user_count --;
  }

}
Entrance::Entrance()
{
	fileName = "config.xml";

    configureInstance = CacheFactory::createConfigureManager();

    //userInstance = CacheFactory::createUserManger();

    cacheInstance =  CacheFactory::createCacheManager();

    //memoryInstance  = CacheFactory::createMemoryManager();

    //ioInstance = CacheFactory::createIOManager();
}
// 解析参数
void Entrance::parseParameters(int argc, char *const *argv)
{

    cout<<"开始解析程序命令"<<endl;
      char * parse;


      int i;

      for ( i = 1; i< argc ; ++i)
      {
      	 parse = argv[i];

      	 if (*parse++ != '-')
      	 {
      	 	std::cout<<"invaild option:"<<argv[i]<<"\n";
      	 }
      

      while (*parse)
      {
      	switch(*parse++)
      	{
           case  'c':
            if (argv[++i])
            {
            	fileName = argv[i];

            	break;
            }
            std::cout<<"invaild parse,when parse \"c\"\n"; return;
            
      	} // end switch
      }// end while 
     } // end for 
}

//初始化这个程序

bool Entrance::initProgram()
{
     cout<<"开始初始化整个程序"<<endl;
     shared_ptr <configureInfo> configurefile = configureInstance->readConfigure();
     parsexml::parseXML(fileName,configurefile);

     cout<<"配置信息如下"<<endl;
     configureInstance->printfConfigure();

     openTCP_Thread(configurefile);
     
}
// 真正入口函数

bool Entrance::openTCP_Process(shared_ptr <configureInfo> configurefile)
{
   int listenfd = socket(PF_INET,SOCK_STREAM,0);

   assert(listenfd >=0);

   int ret = 0;

   struct sockaddr_in address;

   bzero(&address,sizeof(address));

   address.sin_family = AF_INET;
  
   char * ip = configurefile->ip;

   assert(ip!=NULL);

   inet_pton(AF_INET,ip,&address.sin_addr);

   address.sin_port = htons (configurefile->port);

   ret = bind(listenfd,(struct  sockaddr*)&address,sizeof(address));

   assert(ret!=-1);


   ret = listen(listenfd,5);

  /*ProcessManager< CacheConn >* pool = ProcessManager < CacheConn >::getInstance(listenfd,configurefile->worker);

   if(pool)
   {
      pool->run();
      delete pool;

   }*/
   close(listenfd);

   return true;

}// end 


void Entrance::openTCP_Thread(shared_ptr <configureInfo> configurefile)
{
  /*忽略SIGPIPE信号*/
  addsig(SIGPIPE,SIG_IGN);
  /*创建线程池*/
  
   ThreadManager < CacheConn> * pool = ThreadManager <CacheConn>::getInstance(configurefile->worker,10000);

   if(!pool)
      return;
  /*为客户连接分配一个CaConn*/
  CacheConn * users = new CacheConn[MAX_FD];
  assert(users);

  int user_count = 0;

  int listenfd = socket(PF_INET,SOCK_STREAM,0);

   assert(listenfd >=0);
   struct linger tmp = {1,0};
   setsockopt(listenfd,SOL_SOCKET,SO_LINGER,&tmp,sizeof(tmp));
   int ret = 0;

   struct sockaddr_in address;

   bzero(&address,sizeof(address));

   address.sin_family = AF_INET;
  
   char * ip = configurefile->ip;

   assert(ip!=NULL);

   inet_pton(AF_INET,ip,&address.sin_addr);

   address.sin_port = htons (configurefile->port);

   ret = bind(listenfd,(struct  sockaddr*)&address,sizeof(address));

   assert(ret!=-1);

   ret = listen(listenfd,5);
   epoll_event events[MAX_EVENT_NUMBER];

   int epollfd = epoll_create(5);

   assert(epollfd!=-1);

   addfd(epollfd,listenfd,false);

   CacheConn::m_epollfd = epollfd;

   while(true)
   {
     int number = epoll_wait(epollfd,events,MAX_EVENT_NUMBER,-1);

     if((number<0)&&(errno!=EINTR))
     {
        cout<<"epoll failure\n";
        break;
     }
     for (int i = 0;i<number;++i)
     {
         int sockfd = events[i].data.fd;

         if(sockfd == listenfd)
         {
            struct sockaddr_in client_address;

            socklen_t client_addrlength = sizeof(client_address);

            int connfd = accept(listenfd,(struct sockaddr *)&client_address,&client_addrlength);

            if( connfd < 0)
            {

              cout<<"curr error when accept connect erron  is  "<<errno<<endl;

              continue;
            }
            if (CacheConn::m_user_count >=MAX_FD)
            {
              cout<<"server is overload "<<endl;
              char * info = "500\r\n";
              send(connfd,info,strlen(info),0);
              continue;
            }
            users[connfd].init(connfd,client_address);
         } //end if(sockfd == listenfd)
         else if(events[i].events &(EPOLLRDHUP|EPOLLHUP|EPOLLERR))
         {
             //关闭 连接
             users[sockfd].close_conn();
         } // end  else if
         else if(events[i].events & EPOLLIN)
         {

             cout<<"hello,world"<<endl;
          
            if(users[sockfd].read())
            {
                 pool->append(users+sockfd);    
            }
            else
            {
               
                //char * info ="600\r\n";
                //send(sockfd,info,strlen(info),0);
                users[sockfd].close_conn();
            }

         }
        else if(events[i].events &  EPOLLOUT)
        {
          /*根据写的结果，决定是否关闭连接*/
          //if (!users[sockfd].write())
          //{
            //users[sockfd].close_conn();
          //}
        }
        else
        {
           
        }
     }// end for (int i = 0;i<number;++i)
   }// end  while(true)

   close(epollfd);
   close(listenfd);
   if(users)
    delete [] users;
   users = nullptr;
   if(pool)
   delete pool;
   pool = nullptr;
   return ;
  
}

