# include "parsehash.h"

namespace cachemanager
{
   bool  parseHash(string rule,string & fileName,const string & hashName){

   string::size_type pos;

   rule+=":";

   unsigned int  size = rule.size();
   unsigned int begin = 0;
   unsigned int end = 0;
   unsigned  int length = hashName.length();
   for(unsigned int i = 0;i<size;i++)
   {
      pos = rule.find(":",i);

      if(pos<size)
      {
         std::string s = rule.substr(i,pos-i);
          end = atoi(s.c_str());
         if((begin+end)>=length)
            return false;
         fileName+=hashName.substr(begin,end);
         if(pos!=size-1)
         {

            if(!makeDir(fileName)){
               #ifdef __CACHE_DEBUG
               cout<<"ERROR!!!创建目录失败"<<endl;
               #endif
               return false;
             }
            fileName+="/";
            // 这个是目录

         }
         begin+=end;
         i = pos;
      }
   }
   return true;
}
//创建目录
bool makeDir(string & path)
{

   const char * paths = path.c_str();
   if(NULL==opendir(paths)) //目录不存在
   {
      //创建目录
     int ret  = mkdir(paths,S_IRWXU|S_IRGRP|S_IWGRP);

     if(ret != 0)
     {
       return false;
     }
     else  return true;
   }
  return true;
}
} // end  namespace