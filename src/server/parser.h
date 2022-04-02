#include <iostream>
#include <string>
#include <vector>
#include "tinyxml2/tinyxml2.h"

using namespace tinyxml2;
using namespace std;

class Mytype{
public:
    //create: "account","symbol", transactions:"order","query","cancel"，error:"error"
    string type;
    
    //check digit?

    /* "account" */
    string account_id;
    string account_balance;

    /* "symbol" */
    string symbol;
    vector<string>symbol_id;
    vector<string>symbol_shares;
    vector<bool>error_arr;//for database error, not xml parser// whether true/false, if(database return wrong)push_back(true);else{push_back(false)}
    vector<string>error_arr_msg;// whether true/false, if(database return wrong)(push_back(msg))else{push_back(msg)}
    //databse return wrong can be indicated by msg !=""

    /* order */
    string order_symbol;
    string order_amount;
    string order_limit;

    string transaction_id;
    /* query */
    string open_shares;

    string cancel_shares;
    string cancel_time;

    /* cancel */
    vector<string> execute_shares;
    vector<string> execute_price;
    vector<string> execute_time;

    /* error */
    bool is_error;
    string error_msg;

public:
    Mytype();
};


void transaction_mode(XMLNode* node,vector<Mytype>&res);
void symbol_helper(XMLElement*node,Mytype & curr_object);
void create_mode(XMLNode* node,vector<Mytype>&res);
int myXmlParser(string xml,vector<Mytype>&res);
void ResToXml(vector<Mytype>&res, string&response);


/*
error case(database需要检查的);

error case in <create> -> <results><create> or <results><error id = account_id>Account Already exists</error><>
1. 重复create account是error，重复sym不是 (database?)

error case in <transaction>

(i)<order> -> <results><opened>  or  <results><error>
1.account_id 不合法 是error
2.balance 选择purchase不够支付是 error

(ii)<query> -> <status id = "TRANS_ID"><status>one or more children

1.response: open 和cancel互斥，所以只有<execute><execute><open> ,<execute><execute><execute><cancel>
2.invalid trans_id?

(iii)<cancel> -> <>
1.<execute><open>

*/
