# include "writeDisk.h"
# include <cstddef>
namespace cachemanager{
DiskManager* DiskManager::getInstance(const size_t &ds)
{
	static DiskManager m_instance(ds);

	return &m_instance;
}

bool DiskManager::writeOneBuff(const string & hashName,const char buff[],const unsigned int &offset,unsigned int &length)
{
   /*磁盘块单位为MB为单位*/
  
    unsigned int beginBlock = offset/diskSize;
   //cout<<beginBlock<<endl;
  
   unsigned int newoffset =  offset%(diskSize);

   unsigned int endBlock = (offset+length)/diskSize;

   int filelength = hashName.length();

   char *str  = new char[filelength+10];
   //char str2[10]; 
   //strcpy(str,hashName.c_str());
    unsigned int len1;
   /*
   if(beginBlock == endBlock)
   {
   	 snprintf(str,filelength+10,"%s.t%zd",hashName.c_str(),beginBlock);
     int fd = open(str, O_RDWR|O_CREAT|O_WRONLY,0666);
     lseek(fd, diskSize- 1, SEEK_SET);
     lseek(fd,newoffset,SEEK_SET);
     if(write(fd,buff,length)==length)
     {
     	close(fd);
        freeNew(str);
     	return true;
     }
        
     else 
     {

        freeNew(str);
     		return false;
     }*/
    
   	//unsigned  int len2 = length -len1;
   	for(unsigned int i=beginBlock;i<=endBlock;++i)
   	{


    len1 = (diskSize-newoffset<length ? diskSize-newoffset:length);
    memset(str,'\0',filelength+10);
   	snprintf(str,filelength+10,"%s.t%u",hashName.c_str(),i);
    // 多进程写文件这边要 加锁吗？这个等以后再考虑
    if(access(str,F_OK)==-1)
    {
        /*这个文件不存在*/
        FILE *tempfp  = fopen(str,"a");
        if(tempfp == NULL)
          {
              #ifdef __CACHE_DEBUG
             cout<<"ERROR!!!创建位图文件失败"<<endl;
             #endif
             return false;
          }
          fclose(tempfp);
          //isFirst = true;
        
    }
    FILE *fp = fopen(str, "rb+"); //二进制写文件
    if(fp==NULL)
    {
      //因为上面建文件的时候，没有加锁，对于多线程很可能会有问题，不过这个小概率事件。
      #ifdef __CACHE_DEBUG
      cout<<"errno="<<errno<<endl;
      #endif
      freeNew(str);
      return false;
    }
    fseek(fp,diskSize- 1, SEEK_SET);
    fseek(fp,newoffset,SEEK_SET);

    if(fwrite(buff,sizeof(char),len1,fp)==len1)
     {
     	fclose(fp);
     	newoffset = 0;
      length -=len1;
     }
    else
    {
    	  fclose(fp);
        freeNew(str);
        return false;
    }
   } //end  for 
   freeNew(str);
   if(length == 0)
   {
      return true;
   }
   else {
   	     #ifdef __CACHE_DEBUG
          cout<<"ERROR!!!计算出错了"<<endl;
          #endif
          return false;
   }

  
}


void DiskManager::freeNew(char * str)
{
   if(str)
   {
   	  delete [] str;
   	  str = NULL;
   }

}
}