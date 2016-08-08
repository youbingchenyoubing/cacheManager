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
   memset(m_iv,'NULL',sizeof(m_iv));
   m_read_idx = 0;
   m_write_idx = 0;

}
void  CacheConn::process()
{
     if (strcmp(clientData->type,"read")==0)
     {
          //shared_ptr <configureInfo> configureContent = configureInstance->readConfigure();
        
          readCache();

     }

     else 
    {
         //写事件
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
        #ifdef __CACHE_DEBUG
        cout<<"user send content is "<<m_buf<<endl;
        #endif
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
              #ifdef __CACHE_DEBUG
               cout<<"客户是发送过来读事件"<<endl;
              #endif
                return  true;
            }
          else
          {
            #ifdef __CACHE_DEBUG
            cout<<"客户端发送过来的是写事件"<<endl;
            #endif
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
        #ifdef __CACHE_DEBUG
         cout<<"客户端了发送错误数据"<<endl;
         #endif
         ResponesClient(ERRORHEADER,NULL);
         return false;
      }
     }//end else
   }// end while(true)
   
}
//处理客户端读的请求
void CacheConn::readCache()
{
   modfd(m_epollfd,m_sockfd,EPOLLOUT);
   unsigned int blockSize = configureInstance->readConfigure()->blockSize;
   unsigned int beginBlock =  (clientData->offset)/blockSize;
   unsigned int endBlock = (clientData->offset+clientData->len)/blockSize;
   if((clientData->len)>blockSize &&((clientData->offset+clientData->len)%blockSize))
         endBlock+=1;
   #ifdef __CACHE_DEBUG
         cout<<"beginBlock = "<<beginBlock<<",endBlock="<<endBlock<<", ";
   #endif
   void *  data = nullptr;
   unsigned int g_Info = 0;
   for( unsigned  int i=beginBlock;i<=endBlock;++i)
   {
       //if(configureInstance->)
      data = cacheInstance->searchBlock(clientData->fileName,i,g_Info);

      if(!data)
      {

        ResponesClient(g_Info,NULL);
        sleep(5);
        close_conn();
        return;
      }
      writevClient(data,blockSize,clientData->offset,clientData->len,i);
   } //end for
   modfd(m_epollfd,m_sockfd,EPOLLIN);
   return;
}
void CacheConn::writevClient(void * data,const unsigned int &blockSize,const unsigned int &offset,const unsigned int  &len,const int & index )
{
  size_t begin_offset = 0;
  size_t end_offset = 0;
  if( offset> index*blockSize)
  {
      begin_offset = offset-index*blockSize;
  }
  if((offset+len)<((index+1)*blockSize-1))
  {
       end_offset = ((index+1)*blockSize-1)-offset-len;
  }
  
  size_t send_count =blockSize-begin_offset-end_offset;

  if(send_count <=0)
  {
     #ifdef __CACHE_DEBUG
      cout<<"头部协议出错"<<endl;
      #endif
     return ;
  }

  
  char headInfo[20];
  memset(headInfo,'\0',20);
  int ret = snprintf(headInfo,20,"%d/%zd/%zd%s",CACHEMISS,index*blockSize,send_count,"\r\n");
  if(ret>20) 
  {
      #ifdef __CACHE_DEBUG
      cout<<"头部协议过长"<<endl;
      #endif
    return ;
  }
  m_iv[0].iov_base = headInfo;
  m_iv[0].iov_len  = strlen(headInfo);

  m_iv[1].iov_base = data+begin_offset;
  m_iv[1].iov_len = send_count;
  
  ret = writev(m_sockfd,m_iv,2);

  if(ret<=-1)
  {
      #ifdef __CACHE_DEBUG
      cout<<"WARNING!!!TCP写缓冲空间不足"<<endl;
      #endif
     
  }
  else if(ret==strlen(headInfo)+send_count)
  {
       #ifdef __CACHE_DEBUG
       cout<<"data="<<(data+begin_offset)<<endl;
       #endif
       return;
  }
  else
  {
      #ifdef __CACHE_DEBUG
      cout<<"发送数据不一致"<<endl;
     #endif

  }
 
  //return ;

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
  #ifdef __CACHE_DEBUG
  cout<<"打印信息用户发送过来的信息"<<endl;
 
  if(clientData)
  {
    //#ifdef __CACHE_DEBUG
     cout<<"clientData->type="<<clientData->type<<endl;

      cout<<"clientData->fileName="<<clientData->fileName<<endl;
  }
  #endif
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

    cacheInstance =  CacheFactory::createCacheManager(configureInstance);

    //memoryInstance  = CacheFactory::createMemoryManager();

    //ioInstance = CacheFactory::createIOManager();
}
// 解析参数
void Entrance::parseParameters(int argc, char *const *argv)
{

  #ifdef __CACHE_DEBUG

    cout<<"开始解析程序命令"<<endl;
  #endif
      char * parse;


      int i;

      for ( i = 1; i< argc ; ++i)
      {
      	 parse = argv[i];

      	 if (*parse++ != '-')
      	 {
            #ifdef __CACHE_DEBUG
      	 	std::cout<<"invaild option:"<<argv[i]<<"\n";
          #endif
          exit(1);
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
            #ifdef __CACHE_DEBUG
            std::cout<<"invaild parse,when parse \"c\"\n"; return;
            #endif
      	} // end switch
      }// end while 
     } // end for 
}

//初始化这个程序

bool Entrance::initProgram()
{    
       #ifdef __CACHE_DEBUG
     cout<<"开始初始化整个程序"<<endl;
     #endif
     shared_ptr <configureInfo> configurefile = configureInstance->readConfigure();
     parsexml::parseXML(fileName,configurefile);
      #ifdef __CACHE_DEBUG
     cout<<"配置信息如下"<<endl;
     #endif
     configureInstance->printfConfigure();

     openTCP_Thread(configurefile);
     
}
// 真正入口函数(进程池，不再使用)

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
void Entrance::setup_sig_pipe(int epollfd)
{
   /*忽略SIGPIPE信号*/
  int ret = socketpair(PF_UNIX,SOCK_STREAM,0,sig_pipefd);
  assert( ret != -1);
  setnonblocking( sig_pipefd[1]);
  
   addfd(epollfd,sig_pipefd[0],false);

   addsig(SIGCHLD,sig_handler);
   addsig(SIGTERM,sig_handler);
   
   addsig(SIGINT,sig_handler);

   addsig(SIGPIPE,SIG_IGN);
}
//初始化线程池
void Entrance::openTCP_Thread(shared_ptr <configureInfo> configurefile)
{
    /*创建线程池*/
  
   ThreadManager < CacheConn> * pool = ThreadManager <CacheConn>::getInstance(configurefile->worker,10000);

   if(!pool)
      return;
  /*为客户连接分配一个CaConn*/
  CacheConn * users = new CacheConn[MAX_FD];
  assert(users);

  //int user_count = 0;

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
   setup_sig_pipe(epollfd);  //初始化信号

   while(true)
   {
     int number = epoll_wait(epollfd,events,MAX_EVENT_NUMBER,-1);

     if((number<0)&&(errno!=EINTR))
     {
        #ifdef __CACHE_DEBUG
        cout<<"epoll failure\n";
        #endif
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
              #ifdef __CACHE_DEBUG
              cout<<"curr error when accept connect erron  is  "<<errno<<endl;
              #endif

              continue;
            }
            if (CacheConn::m_user_count >=MAX_FD)
            {
              #ifdef __CACHE_DEBUG
              cout<<"server is overload "<<endl;
              #endif
              char * info = "500\r\n";
              send(connfd,info,strlen(info),0);
              continue;
            }
            users[connfd].init(connfd,client_address);
         } //end if(sockfd == listenfd)
           //信号处理
         else if((sockfd == sig_pipefd[0])&&(events[i].events & EPOLLIN))
         {
              int sig;
              char signals[1024];

              ret  = recv(sig_pipefd[0],signals,sizeof(signals),0);
              if(recv <= 0)
              {
                 continue;
              }

              else 
              {
                 for( int i = 0;i<ret;++i)
                 {
                    switch(signals[i])
                    {
                       case SIGTERM:
                       case SIGHUP:
                       case SIGINT:
                       {
                         #ifdef __CACHE_DEBUG
                         cout<<"程序被kill掉"<<endl;
                         #endif
                         cacheInstance->deleteAll();

                         if(users)
                         {
                            delete []  users;
                             #ifdef __CACHE_DEBUG
                            cout<<"users 空间被释放"<<endl;
                             #endif
                         }
                          users = nullptr;
                          if(pool)
                          {
                            delete pool;
                            cout<<"pool 空间被释放"<<endl;
                          }
                          pool = nullptr;
                          close(epollfd);
                          close(listenfd);
                          #ifdef  __CACHE_DEBUG
                          cout<<"程序安全退出"<<endl;
                           #endif
                          return;
                       }
                    } // end switch
                 } //  end  for

              } //end else
         }//end   else if((sockfd == sig_pipefd[0])&&(events[i].events & EPOLLIN))
         else if(events[i].events &(EPOLLRDHUP|EPOLLHUP|EPOLLERR))
         {
             //关闭 连接
             users[sockfd].close_conn();
         } // end  else if
         else if(events[i].events & EPOLLIN)
         {

             //cout<<"hello,world"<<endl;
          
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

         }  //end else if(events[i].events & EPOLLIN)
         else if(events[i].events & EPOLLOUT)
         {
              //char * info = "000\r\n";
              //send(sockfd,info,strlen(info),0);
         }
       
        else
        {
           users[sockfd].close_conn();
        }
     }// end for (int i = 0;i<number;++i)
   }// end  while(true)

   close(epollfd);
   close(listenfd);
   cacheInstance->deleteAll();
   if(users)
    delete [] users;
   users = nullptr;
   if(pool)
   delete pool;
   pool = nullptr;
   #ifdef  __CACHE_DEBUG
   cout<<"程序安全退出"<<endl;
   #endif
   return ;
  
}

