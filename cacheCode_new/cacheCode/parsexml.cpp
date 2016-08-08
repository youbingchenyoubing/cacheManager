# include "parsexml.h"
# include <stdlib.h>
# include <limits.h>
//# include <stdio.h>
#include <string>  
#include <iostream>  
#include <fstream>  
#include "rapidxml/rapidxml.hpp"  
#include "rapidxml/rapidxml_utils.hpp"  
#include "rapidxml/rapidxml_print.hpp"  
  
  
using namespace rapidxml;  
using namespace std;  
// 真正解析了xml文件,将解析的参数赋值给
void parsexml::parseXML(const char * name,shared_ptr <configureInfo> configurefile)
{
   if (configurefile == NULL) exit(1);
   char absPath [PATH_MAX];
   getAbasPath(name,absPath);

   file<> fdoc(absPath);
   
   xml_document<>doc;
   
   doc.parse<0>(fdoc.data());

   cout<<absPath<<endl;

   xml_node <>*root = doc.first_node();

   xml_node<> *block_node = root->first_node();
   xml_attribute<>* node_attribute = NULL;
   if(!block_node)
   {
   	   cout<<"ERROR: no name  block in  configure file\n";
   	   exit(1);
   }
   else
   {
   	while(block_node)
   	  {
   	  	//cout<<block_node->name()<<","<<block_node->value()<<endl;
   	  	node_attribute = block_node->first_attribute("type");
   	  	const char * attributeValue = node_attribute->value();
   	  	switch(atoi(attributeValue))
   	  	{
   	  		case 1: configurefile->blockSize = atoi(block_node->value()); break;
   	  		case 2: configurefile->maxBlockNum = atoi(block_node->value()); break;
   	  		case 3: configurefile->filePath  = block_node->value();break;
   	  		case 4: configurefile->ip = block_node->value();break;
   	  		case 5: configurefile->port  = atoi(block_node->value());break;
   	  		case 6: configurefile->worker = atoi(block_node->value()); break; 
   	  		default: break;
   	  	}
   	    block_node = block_node->next_sibling();
       }
   }
}
// 测试解析xml文件
parsexml::parsexml(const char *name)
{
  char absPath [PATH_MAX];
   getAbasPath(name,absPath);

   file<> fdoc(absPath);
   
   xml_document<>doc;
   
   doc.parse<0>(fdoc.data());

   cout<<absPath<<endl;

   xml_node <>*root = doc.first_node();

   xml_node<> *block_node = root->first_node();

    xml_attribute<>* node_attribute = NULL; 
   if(!block_node)
   {
   	   cout<<"ERROR: no name  block in  configure file\n";
   	   exit(1);
   }
   else
   {
   	while( block_node)
   	  {
   	  	node_attribute = block_node->first_attribute("type");
   	  	const char * attributeValue = node_attribute->value();
   	  	//cout<<block_node->name()<<","<<block_node->value()<<attributeValue<<endl;
   	  	switch(atoi(attributeValue))
   	  	{
   	  		case 1: cout<<block_node->name()<<","<<block_node->value()<<attributeValue<<endl; break;
   	  		case 2: cout<<block_node->name()<<","<<block_node->value()<<attributeValue<<endl; break;
   	  		case 3: cout<<block_node->name()<<","<<block_node->value()<<attributeValue<<endl; break;
   	  		default: break;
   	  	}
   	    block_node = block_node->next_sibling();
   	    //block_node = block_node->next_sibling();
     	
   }
   
 }
  
}
void parsexml::getAbasPath(const char * name,char absPath[])
{
   

   if (realpath(name,absPath))
   {
   	  cout<<"found the file named "<<name<<" in "<<absPath<<endl;
   	  //name = absPath;
      return ;
   }

   else
   {
    cout<<"ERROR: the configure file  named  "<<name<<" no exits,please for checking\n";

     exit(1);
   }
}