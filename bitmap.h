// 这是一个管理磁盘文件块的位图类

#ifndef BITMAP_H
#define BITMAP_H
//# include "locker.h"
//static const unsigned  int MAX_PIECE = 1000000; //最多的块，这个是为了方便位图的初始化
namespace cachemanager
{
class  BitMap{
public:
	BitMap(const char *hashName,const unsigned int & MAX_PIECE1);
	int getByteValue(unsigned int index);
	int setByteValue(unsigned int index,unsigned
		char v);
	bool all_zero();
	bool all_set();
	bool restoreBitMap(const char *hashName);
	~BitMap();
private:
	char * bitfiled; //位图
	unsigned int bit_size; //多少个bit
	const unsigned int MAX_PIECE;

};
}
#endif