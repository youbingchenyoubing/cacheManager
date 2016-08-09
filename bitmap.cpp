# include "bitmap.h"
# include <iostream>
# include <memory.h>
# include <stdio.h>
#include <unistd.h>
//# include <fstream>
#include <sys/types.h>    
#include <sys/stat.h> 
#include <sys/file.h>
#include <fcntl.h>
# include <errno.h>   
//#include <fcntl.h>
using namespace std;
namespace cachemanager
{
	BitMap::BitMap(const char * hashName,const unsigned int & MAX_PIECE1):MAX_PIECE(MAX_PIECE1)
	{
      //MAX_PIECE = MAX_PIECE1;
	    bit_size = MAX_PIECE /8;
	    if(MAX_PIECE%8) bit_size++;
		  bitfiled = new char[bit_size];

	     if(!bitfiled){
         perror("malloc error");
       }
		/*
        fstream openhandler;
        openhandler.open(hashName,ios::in);
         */
        FILE * fp = fopen(hashName,"r");

        if( fp == NULL) //打开文件失败，说明开始是一个全新的下载
        {
             memset(bitfiled,0,bit_size);
        }
        else
        {

        	fseek(fp,0,SEEK_SET);
        	for(int i =0; i<bit_size;++i)
            {
            	bitfiled[i] = fgetc(fp);
            }

            fclose(fp);
        }
  
	}
    /*取某一个index的位图*/
	int BitMap::getByteValue(unsigned int index)
	{

	    unsigned int byte_index;
	    unsigned char inner_byte_index;

	    unsigned char byte_value;

	    if(index >= MAX_PIECE) 
	    {
	    	#ifndef  __CACHE_DEBUG
	    	cout<<"ERROR!!!,上传的文件过大，拒绝上传"<<endl;
	    	#endif
	    	return -1;
	    }

	    byte_index = index/8;
	    byte_value = bitfiled[byte_index];
	    inner_byte_index = index %8;

	    byte_value = byte_value >>(7-inner_byte_index);


	    if(byte_value%2 == 0) return 0;

	    else return 1;
	}

  
   int BitMap::setByteValue(unsigned int index,unsigned char v)
   {
      int byte_index;
      unsigned char inner_byte_index;


      if(index > MAX_PIECE)
      {
      		#ifndef  __CACHE_DEBUG
	    	cout<<"ERROR!!!,上传的文件过大，拒绝上传"<<endl;
	    	#endif
	    	return -1;
      }

      if((v!=0)&&(v!=1))
      {
      	    #ifndef  __CACHE_DEBUG
	    	cout<<"ERROR!!! v的值出错了!!!"<<endl;
	    	#endif
	    	return -1; 
      }

      byte_index = index/8;
      inner_byte_index = index%8;
      v= v<<(7-inner_byte_index);
      bitfiled[byte_index] = bitfiled[byte_index]|v;

      return 0;
   }

  bool BitMap::restoreBitMap(const char *hashName)
   {
      //int fd;

      if(bitfiled == NULL||hashName==NULL) return false;


      FILE *fp = fopen(hashName,"w+");
      if(fp==NULL)
      {
      	#ifdef __CACHE_DEBUG
      	cout<<"更新位图文件失败"<<endl;
      	#endif
      	return false;

      }
      if(flock(fileno(fp),LOCK_EX)<0) //建立互斥锁，以免更新位图的时候，有线程进行读取
      {
      	#ifdef __CACHE_DEBUG
      	cout<<"位图文件加锁失败"<<endl;
      	#endif
      	return false;
      }
      /*写之间先于文件异或一下，为了防止多线程写,出现脏写，这个过程有点像数据库的乐观锁*/
      fseek(fp,0,SEEK_SET);
      for(int i =0; i<bit_size;++i)
      {
              bitfiled[i] |= fgetc(fp);
      }
      fseek(fp,0,SEEK_SET);
      fwrite(bitfiled,sizeof(char ),bit_size,fp);
      if(flock(fileno(fp), LOCK_UN) == -1) //释放文件锁
      {
         perror("flock()");
      }
      fclose(fp);
      return true;
   }
   BitMap::~BitMap()
{
  if(bitfiled)
  {
    delete bitfiled;
    #ifdef __CACHE_DEBUG
    cout<<"释放位图空间"<<endl;
    #endif
  }
}
}

