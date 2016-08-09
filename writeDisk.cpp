# include "writeDisk.h"
# include <cstddef>
namespace cachemanager{
DiskManager* DiskManager::getInstance(const size_t &ds)
{
	static DiskManager m_instance(ds);

	return &m_instance;
}

bool DiskManager::writeOneBuff(const string & hashName,const char buff[],const unsigned int &offset,int &length)
{
   /*磁盘块单位为MB为单位*/
 
   unsigned int beginBlock = offset/diskSize;

   unsigned long int newoffset =  offset%(diskSize);

   unsigned int endBlock = (offset+length)/diskSize;

   int filelength = hashName.length();

   char *str  = new char[filelength+10];
   //char str2[10]; 
   //strcpy(str,hashName.c_str());
   
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
     }
    }
   else if(endBlock ==(beginBlock+1))
   {
   	  int len1 = diskSize-newoffset;
   	//unsigned  int len2 = length;
   	//unsigned  int len2 = length -len1;
   	for(int i=beginBlock;i<=endBlock;++i)
   	{
    memset(str,'\0',filelength+10);
   	snprintf(str,filelength+10,"%s.t%zd",hashName.c_str(),i);
    int fd = open(str, O_RDWR|O_CREAT|O_WRONLY,0666);
    lseek(fd, diskSize- 1, SEEK_SET);
    lseek(fd,newoffset,SEEK_SET);

    if(write(fd,buff,len1)==len1)
     {
     	close(fd);
     	newoffset = 0;
     	len1 = length-len1;
     	length -=len1;
     }
    else
    {
    	 close(fd);
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
} // end else if(endBlock ==(beginBlock+1))
   else
   {
   	      #ifdef __CACHE_DEBUG
          cout<<"ERROR!!!写文件出现了错误"<<endl;
          #endif
         freeNew(str);
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