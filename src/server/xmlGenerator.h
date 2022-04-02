#include <vector>
#include <string>
#include "tinyxml2/tinyxml2.h"

using namespace std;
using namespace tinyxml2;
class xmlGenerator{
public:
    vector<string>type; // <create> <transaction>
    vector<string>create_child;
    vector<string>transactions_child;
    xmlGenerator():type({"create","transactions"}),create_child({"account","symbol"}),transactions_child({"order","query","cancel"}){}
    string generateXml(){
        string buf;
        XMLDocument doc;
        XMLDeclaration* declaraiton= doc.NewDeclaration();
        doc.InsertFirstChild(declaraiton);

        srand((unsigned int) time(NULL));
        int random_type = rand() % 2;
        
        if(random_type == 0){
            XMLElement* root=doc.NewElement("create");
            doc.InsertEndChild(root);

            int outer_loop = rand() % 5;
            for(int i = 0;i < outer_loop; i++){
                int random_create_child = rand() % 2;
                if(random_create_child==0){
                    XMLElement * element = doc.NewElement("account");
                    int account_id = rand() % 100;
                    int balance = rand() % 200000;
                    element->SetAttribute("id",account_id);
                    element->SetAttribute("balance",balance);
                    root->InsertEndChild(element);
                }else{
                    XMLElement * element = doc.NewElement("symbol");
                    int str_len = rand() % 3;
                    string sym = strRand(str_len);
                    element->SetAttribute("sym",sym.c_str());

                    int inner_loop = rand() % 5;
                    for(int j = 0;j < inner_loop; ++j){
                        XMLElement * subelement = doc.NewElement("account");
                        int account_id = rand() % 100;
                        int NUM = rand() % 1000;
                        subelement->SetAttribute("id",account_id);
                        subelement->SetText(NUM);
                        element->InsertEndChild(subelement);
                    }
                    root->InsertEndChild(element);
                }
            }
        }else{
            XMLElement* root=doc.NewElement("transactions");
            int account_id = rand() % 100;
            root->SetAttribute("id",account_id);
            doc.InsertEndChild(root);

            int outer_loop = rand() % 5 + 1;
            for(int i = 0;i < outer_loop; i++){
                int random_transactions_child = rand() % 3;
                if(random_transactions_child == 0){
                    //order
                    XMLElement * element = doc.NewElement("order");
                    int str_len = rand() % 3;
                    string sym = strRand(str_len);
                    int amount = rand() % 2000 - 1000; // may < 0
                    int limit = rand() % 500;
                    element->SetAttribute("sym",sym.c_str());
                    element->SetAttribute("amount",amount);
                    element->SetAttribute("limit",limit);
                    root->InsertEndChild(element);
                }else if(random_transactions_child == 1){
                    //query
                    XMLElement * element = doc.NewElement("query");
                    int transaction_id = rand() % 100;
                    element->SetAttribute("id",transaction_id);
                    root->InsertEndChild(element);
                }else{
                    //cancel
                    XMLElement * element = doc.NewElement("cancel");
                    int transaction_id = rand() % 100;
                    element->SetAttribute("id",transaction_id);
                    root->InsertEndChild(element);
                }
            }
        }

        XMLPrinter printer;
        doc.Print( &printer );//将Print打印到Xmlprint类中 即保存在内存中
        buf = printer.CStr();//转换成const char*类型
        return buf;
    }
    
    //reference:https://blog.csdn.net/lfod1997/article/details/107749492
    string strRand(int length){  
        char tmp;							
        string buffer;						
        
        srand((unsigned int) time(NULL));
        
        for (int i = 0; i < length; i++) {
            tmp = rand() % 36;	
            if (tmp < 10) {			
                tmp += '0';
            } else {				
                tmp -= 10;
                tmp += 'A';
            }
            buffer += tmp;
        }
        return buffer;
    }
};


