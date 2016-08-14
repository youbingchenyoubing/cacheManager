# ifndef PARSEHASH_H
# define PARSEHASH_H
# include <string>
# include <string.h>
# include <sys/stat.h>
# include <sys/types.h>
# include <dirent.h>
#ifdef  __CACHE_DEBUG
# include <iostream>
#endif
using namespace std;
namespace cachemanager
{
bool  parseHash( string rule, string& fileName, const string& hashName );
bool  makeDir( string& path );
}
#endif