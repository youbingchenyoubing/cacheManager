#ifndef WRITEDISK_H
#define WRITEDISK_H
# include <iostream>
# include <memory.h>
# include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
# include <string>
#include <unistd.h>
using namespace std;
//static ConfigureManager * configureInstance;
namespace cachemanager
{
class DiskManager
{
public:
    bool writeOneBuff( const string& hashName, const char buff[], const unsigned int& offset, unsigned int& length );
    static DiskManager* getInstance( const size_t& ds );
private:
    /*begin 和end 通过计算得到的跨越块的情况*/
    //unsigned int begin;
    //unsigned int end;
    const  unsigned int   diskSize;
    DiskManager( const size_t& ds ): diskSize( ds * 1024 * 1024 )
    {
        //cout<<"ds="<<ds<<endl;
    }
    void freeNew( char* str );
    //static DiskManager * m_Instance;
    //这个是至关重要的

};
}
#endif