#ifndef _QUERY_FUNCS_H
#define _QUERY_FUNCS_H

#include <fstream>
#include <iostream>
#include <pqxx/pqxx>
#include <string>
#include <sstream>
#include <vector>
#include <exception>

using namespace std;
using namespace pqxx;

class Database {
public:
    connection *C;
    //int count;
    //int transaction_num;

public:
    Database() {
        C = NULL;
        //count = 0;
    }
    void init_db();
    void create_tables();
    void drop_tables();
    void connect_db();
    void disconnect();

    void create_account(int account_id, int balance, string &error_msg); //
    void create_position(int account_id, string sym, double amount, string &error_msg); //
    int create_order(int account_id, string sym, double amount, double price, string &error_msg); //

    void deduct_seller_shares(int account_id, string sym, double amount, string &e);
    void deduct_buyer_cost(int account_id, double price, double amount, string &e);


    bool cancel_order(int account_id, int trans_id, string &cancel_s, string &cancel_t,\
    vector<string> &exec_shares, vector<string> &exec_price, vector<string> &exec_time, string &e); // 
    void match_order(int account_id, int trans_id, string sym, double amount, double price);

    void match_sell_order(int account_id, int trans_id, string sym, double amount, double price);
    void update_seller_balance(int account_id, double price, double amount);
    void update_buyer_balance(int account_id, double price, double exec_price, double amount);
    void update_order_status(int trans_id, int account_id, double price);
    void insert_new_order(int trans_id, int account_id, string sym, double amount, double price, double price2);

    
    void match_buy_order(int account_id, int trans_id, string sym, double amount, double price);
    void query(int account_id, int trans_id, string &open_s, string &cancel_s, string &cancel_t, vector<string> &exec_shares,\
    vector<string> &exec_price, vector<string> &exec_time, string &e); //

    void add_from_account();
    void add_from_position();
    void add_from_orders();
};


#endif
