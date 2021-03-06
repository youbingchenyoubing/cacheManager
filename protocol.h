#ifndef PROTOCOL_H
#define PROTOCOL_H
#define ERRORHEADER 100 //客户发送过来的头部是错误的
#define ERRORBODY   101 //客户发送过来的内容是错误的
#define CACHEMISS   102  //某一部分块缓存缺失
#define OKR         200 //完全读成功
#define OKW         300  // 写成功
#define NOFILE      400   //请求文件不存在
#define NOBLOCK     401  // 请求越界
#define MAXCLIENT   500  // 请求超过上限
#define ERRORDATA   600  //接收错误
#define ERRORREAD   700  //读取文件错误
#define MALLOCERROR 800  // 动态空间分配错误
#define AIOERROR    900  //异步读错误
#define INFOCLIENT  103 // 用户告知用户开辟多少空间来接收数据
#endif