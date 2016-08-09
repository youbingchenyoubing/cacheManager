# include "parsehash.h"

namespace cachemanager
{
   bool  parseHash(string rule,string & fileName,const string & hashName){

   string::size_type pos;

   rule+=":";

   int  size = rule.size();
   unsigned int begin = 0;
   unsigned int end = 0;
   unsigned  int length = hashName.length();
   for(int i = 0;i<size;i++)
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
            fileName+="/";
         begin+=end;
         i = pos;


      }
   }
   return true;
}
}