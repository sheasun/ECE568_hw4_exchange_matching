#include "parser.h"


Mytype::Mytype():type(""),is_error(false),error_msg(""){}

void transaction_mode(XMLElement* node,vector<Mytype>&res){
    for(XMLElement* current = node->FirstChildElement();current!=nullptr;current = current->NextSiblingElement()){
        XMLElement*temp = current;
        if(temp->Value()!=nullptr){
            Mytype curr_object;
            curr_object.account_id = node->Attribute("id");
            string curr_type = temp->Value();
            if(curr_type == "order"){
                curr_object.type = "order";
                curr_object.order_symbol = temp->Attribute("sym");
                curr_object.order_amount = temp->Attribute("amount");
                curr_object.order_limit = temp->Attribute("limit");
            }else if(curr_type == "query"){
                curr_object.type = "query";
                curr_object.transaction_id = temp->Attribute("id");
            }else if(curr_type == "cancel"){
                curr_object.type = "cancel";
                curr_object.transaction_id = temp->Attribute("id");
            }else{
                //xml error: <transactions> should only have order/query/cancel
                curr_object.type = "error";
            }
            res.push_back(curr_object);
        }
    }
}


void symbol_helper(XMLElement*node,Mytype & curr_object){
    for(XMLElement* current = node->FirstChildElement();current!=nullptr;current = current->NextSiblingElement()){
        XMLElement*temp = current;
        if(temp->Value()!=nullptr){
            string curr_name = temp->Value();
            //check curr_name == "account"
            if(curr_name!="account"){
                curr_object.type="error";
                return;
            }
            curr_object.symbol_id.push_back(temp->Attribute("id"));
            curr_object.symbol_shares.push_back(temp->GetText());
        }
    }
}
void create_mode(XMLNode* node,vector<Mytype>&res){
    for(XMLElement* current = node->FirstChildElement();current!=nullptr;current = current->NextSiblingElement()){
        XMLElement* temp = current;
        if(temp->Value()!=nullptr){
            Mytype curr_object;
            string curr_type = temp->Value();
            if(curr_type == "account"){
                curr_object.type = "account";
                curr_object.account_id = temp->Attribute("id");
                curr_object.account_balance = temp->Attribute("balance");
            }else if(curr_type == "symbol"){
                curr_object.type = "symbol";
                curr_object.symbol = temp->Attribute("sym");
                symbol_helper(temp,curr_object);
            }else{
                //xml error: <create> should only have account/symbol
                curr_object.type = "error";
            }
            res.push_back(curr_object);
        }
    }
}

//return -1 是xml格式的问题，整个request都是error
int myXmlParser(string xml,vector<Mytype>&res){
    XMLDocument doc;
    XMLError error = doc.Parse(xml.c_str());
    if(error!=XMLError::XML_SUCCESS){
        cout<<"error: cannot parse xml"<<endl;
        return -1;
    }
    
    XMLElement* root = doc.RootElement();
    if(root == nullptr){
        cout<<"error: xml does not have root (hint: add create or transactions)"<<endl;
        return -1;
    }
    
    string root_name = root->Name();
    if(root_name == "create"){
        create_mode(root,res);
    }else if(root_name == "transactions"){
        transaction_mode(root,res);
    }else{
        cout<<"error: request should only contain <create>/<transactions>"<<endl;
        return -1;
    }
    return 0;
}

 
void ResToXml(vector<Mytype>&res, string&response){
    string buf;
    XMLDocument doc;
    XMLDeclaration* declaraiton= doc.NewDeclaration();
    doc.InsertFirstChild(declaraiton);

    XMLElement* root=doc.NewElement("results");
    doc.InsertEndChild(root);
    

    for(int i = 0;i < res.size();++i){
        //cout << res[i].type;
        /*
        if(res[i].is_error == true){
            XMLElement * element = doc.NewElement("error");
            if(res[i].type == "account"){element}
            else if(res[i].type == "symbol"){}
            root->InsertEndChild(element);
            continue;
        }*/
        if(res[i].type=="account"){
            XMLElement * element = res[i].is_error ? doc.NewElement("error"): doc.NewElement("created");
            element->SetAttribute("id",res[i].account_id.c_str());
            if(res[i].error_msg!="")element->SetText(res[i].error_msg.c_str());
            root->InsertEndChild(element);
           //cout<<" id = "<<res[i].account_id<<" balance = "<<res[i].account_balance;
        }
        else if (res[i].type == "symbol"){
            //cout<<" = "<< res[i].symbol<<endl;
            for(int j = 0; j < res[i].symbol_id.size();++j){
                XMLElement * element = res[i].error_arr[j]==true? doc.NewElement("error"):doc.NewElement("created");
                element->SetAttribute("sym",res[i].symbol.c_str());
                element->SetAttribute("id",res[i].symbol_id[j].c_str());
                //element->SetText();//!!!!!!!!!
                if(res[i].error_arr[j])element->SetText(res[i].error_arr_msg[j].c_str());
                root->InsertEndChild(element);
                //cout<<"id = "<<res[i].symbol_id[j] <<" ";
                //cout<<"shares = "<<res[i].symbol_shares[j]<<endl;
            }
        }else if (res[i].type == "order"){
            XMLElement * element = res[i].is_error ? doc.NewElement("error"): doc.NewElement("opened");
            element->SetAttribute("sym",res[i].order_symbol.c_str());
            element->SetAttribute("amount",res[i].order_amount.c_str());
            element->SetAttribute("limit",res[i].order_limit.c_str());
            if(res[i].is_error){
                element->SetText(res[i].error_msg.c_str());
            }else{
                element->SetAttribute("id",res[i].transaction_id.c_str());
            }
            root->InsertEndChild(element);
            //cout<< " symbol = "<<res[i].order_symbol;
            //cout<< " amount = "<<res[i].order_amount;
            //cout<< " limit = "<<res[i].order_limit<<endl;
        }else if (res[i].type == "query"){
            if(res[i].is_error){
                XMLElement * element = doc.NewElement("error");
                element->SetText(res[i].error_msg.c_str());
                root->InsertEndChild(element);
            }else{
                XMLElement * element = doc.NewElement("status");
                element->SetAttribute("id",res[i].transaction_id.c_str());
                if(res[i].open_shares!=""){
                    XMLElement * subelement = doc.NewElement("open");
                    subelement->SetAttribute("shares",res[i].open_shares.c_str());
                    element->InsertEndChild(subelement);
                }
                if(res[i].cancel_shares!=""){
                    XMLElement * subelement = doc.NewElement("canceled");
                    subelement->SetAttribute("shares",res[i].cancel_shares.c_str());
                    subelement->SetAttribute("time",res[i].cancel_time.c_str());
                    element->InsertEndChild(subelement);
                }
                for(int j = 0; j < res[i].execute_shares.size(); ++j){
                    XMLElement * subelement = doc.NewElement("executed");
                    subelement->SetAttribute("shares",res[i].execute_shares[j].c_str());
                    subelement->SetAttribute("price",res[i].execute_price[j].c_str());
                    subelement->SetAttribute("time",res[i].execute_time[j].c_str());
                    element->InsertEndChild(subelement);
                }
                root->InsertEndChild(element);
            }
            //cout<<" transaction id = "<<res[i].transaction_id <<endl;
        }else if (res[i].type == "cancel"){
            if(res[i].is_error){
                XMLElement * element = doc.NewElement("error");
                element->SetText(res[i].error_msg.c_str());
                root->InsertEndChild(element);
            }else{
                XMLElement * element = doc.NewElement("status");
                element->SetAttribute("id",res[i].transaction_id.c_str());
                /*
                if(res[i].open_shares!=""){
                    XMLElement * subelement = doc.NewElement("executed");
                    subelement->SetAttribute("shares",res[i].open_shares.c_str());
                    element->InsertEndChild(subelement);
                }
                */
                if(res[i].cancel_shares!=""){
                    XMLElement * subelement = doc.NewElement("canceled");
                    subelement->SetAttribute("shares",res[i].cancel_shares.c_str());
                    subelement->SetAttribute("time",res[i].cancel_time.c_str());
                    element->InsertEndChild(subelement);
                }
                for(int j = 0; j < res[i].execute_shares.size(); ++j){
                    XMLElement * subelement = doc.NewElement("executed");
                    subelement->SetAttribute("shares",res[i].execute_shares[j].c_str());
                    subelement->SetAttribute("price",res[i].execute_price[j].c_str());
                    subelement->SetAttribute("time",res[i].execute_time[j].c_str());
                    element->InsertEndChild(subelement);
                }
                root->InsertEndChild(element);
            }
            //cout<<" transaction id = "<<res[i].transaction_id <<endl;
        }else if(res[i].type == "error"){
            XMLElement * element = doc.NewElement("error");
            element->SetText("bad XML format");
            root->InsertEndChild(element);
            //cout<<" error "<<endl;
        }
        //cout << endl;
    }

    //如果想要转换成string类型的字符串流以下是操作过程
    XMLPrinter printer;
    doc.Print( &printer );//将Print打印到Xmlprint类中 即保存在内存中
    buf = printer.CStr();//转换成const char*类型
    //cout<<buf<<endl;//buf即为创建后的XML 字符串。
    response = buf;

}