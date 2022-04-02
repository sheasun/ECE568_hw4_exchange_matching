#include "query_funcs.h"

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

void Database::connect_db(){
    try{
    //Establish a connection to the database
    //Parameters: database name, user name, user password
    C = new connection("host=db dbname=MATCH_ENGINE user=postgres password=passw0rd port=5432");
    if (C->is_open()) {
        cout << "Opened database successfully: " << C->dbname() << endl;
    } else {
        cout << "Can't open database" << endl;
        return;
    }
    } catch (const std::exception &e){
        cerr << e.what() << std::endl;
        return;
    }
}

void Database::init_db() 
{
    //Allocate & initialize a Postgres connection object
    //connection *C;
    connect_db();
    drop_tables();
    create_tables();
    return;
}

void Database::create_tables() {
    string filename = "create_tables.txt";
    ifstream file(filename.c_str());
    string query, line;
    if(file.is_open()) {
        while(getline(file, line)) {
            query.append(line);
        }
        file.close();
    }
    cout << "Create tables successfully!" << endl;
    work W(*C);
    W.exec(query);
    W.commit();
}


void Database::drop_tables() {
    string query2 = "DROP TABLE IF EXISTS ACCOUNT CASCADE;\n";
    string query1 = "DROP TABLE IF EXISTS POSITION CASCADE;\n";
    string query3 = "DROP TABLE IF EXISTS ORDERS CASCADE;\n";
    string query4 = "DROP TYPE IF EXISTS status;\n";
    string sql = query1 + query2 + query3 + query4;
    work W(*C);
    W.exec(sql);
    W.commit();
    cout << "Drop all tables successfully!" << endl;
}

void Database::disconnect() {

  C->disconnect();
}


void Database::create_account(int account_id, int balance, string &error_msg) {
    stringstream query;
    work W(*C);
    //nontransaction N(*C);
    query << "SELECT ACCOUNT_ID FROM ACCOUNT WHERE ACCOUNT_ID = " << account_id << ";";
    result R(W.exec(query.str()));
    auto iter = R.begin();
    if (iter != R.end()) {
        error_msg = "Account already exists";
        return;
    }
    else {
        stringstream query2;
        if(balance < 0) {
            cout << "balance cannot be less than 0!" << endl;
            return;
        }
        //work W(*C);
        query2 << "INSERT INTO ACCOUNT (ACCOUNT_ID, BALANCE) VALUES (" << W.quote(account_id) << ", ";
        query2 << balance << ");";
        W.exec(query2.str());
        W.commit();
        cout << "Create new account!" << endl;
    }
}


void Database::create_position(int account_id, string sym, double amount, string &error_msg) {
    stringstream query1;
    work W(*C);
    query1 << "SELECT ACCOUNT_ID FROM ACCOUNT WHERE ACCOUNT_ID = " << account_id << ";";
    result R(W.exec(query1.str()));
    auto iter = R.begin();
    if(iter == R.end()) {
        error_msg = "this account does not exist\n";
        cout << error_msg <<endl;
        return;
    }
    stringstream query2;
    query2 << "SELECT AMOUNT FROM POSITION WHERE ACCOUNT_ID = " << account_id << " AND SYMBOL = ";
    query2 << W.quote(sym) << ";";
    result R2(W.exec(query2.str()));
    auto iter2 = R2.begin();
    if(iter == R2.end()) {
        if (amount <= 0) {
            error_msg = "For new created position, its amount cannot be negative or zero";
            return;
        }
        stringstream q;
        q << "INSERT INTO POSITION (ACCOUNT_ID, SYMBOL, AMOUNT) VALUES (" << account_id << ", ";
        q << W.quote(sym) << ", " << amount << ");";
        cout << "Create new position!" << endl;
        W.exec(q.str());
        W.commit();
        return;
    } 
    stringstream query3;
    query3 << "UPDATE POSITION SET AMOUNT = AMOUNT +" << amount << " WHERE ACCOUNT_ID = ";
    query3 << account_id << " AND SYMBOL = " << W.quote(sym) << ";";
    //"INSERT INTO POSITION (ACCOUNT_ID, SYMBOL, AMOUNT) VALUES (" << W.quote(account_id) << ", ";
    //query3 << W.quote(sym) << ", " << amount << ");";
    W.exec(query3.str());
    W.commit();
    return;
}
int count = 0;
int Database::create_order(int account_id, string sym, double amount, double price, string &error_msg) {
    pthread_mutex_lock(&lock);
    int order_id = count;
    count++;
    pthread_mutex_unlock(&lock);
    try {
        if (amount < 0) {
            deduct_seller_shares(account_id, sym, amount, error_msg);
            if (error_msg != "") {
            return -1;
            }
        }
        else{
            deduct_buyer_cost(account_id, price, amount, error_msg);
            if (error_msg != "") {
            return -1;
            }
        }
        time_t curr_time = time(NULL);
        stringstream query;
        work W(*C);
        query << "INSERT INTO ORDERS (TRANS_ID, ACCOUNT_ID, SYMBOL, AMOUNT, PRICE, TIME) VALUES (" << order_id << ", " << W.quote(account_id);
        query << ", " << W.quote(sym) << ", " << amount << ", " << price << ", " << curr_time << ");";
        W.exec(query.str());
        W.commit();

    } catch(const std::exception &e) {
        error_msg = "cannot place order";
        cerr << e.what() << std::endl;
    }
    cout << order_id << endl;
    match_order(account_id, order_id, sym, amount, price);
    return order_id;
}



void Database::deduct_seller_shares(int account_id, string sym, double amount, string &e) {
    stringstream q;
    work W2(*C);
    q << "SELECT AMOUNT FROM POSITION WHERE ACCOUNT_ID = " << account_id << " AND SYMBOL = " << W2.quote(sym) <<";";
    result R2 = W2.exec(q.str());
    W2.commit();
    auto iter = R2.begin();
    if (iter == R2.end()) {
        e = "deduct seller shares failure: this account doesn't exist or this account does not have the symbol";
        return;
    }
    double position_amount = iter[0].as<double>();
    //cout<<"position_ammount = "<<position_amount<<endl;
    //cout<<"amount"<<amount<<endl;
    if (position_amount < -amount) {
        e = "the amount in the position is less than shares";
        return;
    }
    stringstream ss;
    work W(*C);
    
    ss << "UPDATE POSITION SET AMOUNT = AMOUNT + " << amount << " WHERE ACCOUNT_ID =" << account_id;
    ss << " AND SYMBOL = " << W.quote(sym) << ";";
    W.exec(ss.str());
    W.commit();
}

void Database::deduct_buyer_cost(int account_id, double price, double amount, string &e) {
    stringstream q;
    q << "SELECT BALANCE FROM ACCOUNT WHERE ACCOUNT_ID = " << account_id << ";";
    work W2(*C);
    result R2 = W2.exec(q.str());
    W2.commit();
    auto iter = R2.begin();
    if (iter == R2.end()) {
        e = "deduct buyer cost failure: the account does not exist";
        return;
    }
    double balance = iter[0].as<double>();
    if (balance < price * amount) {
        e = "your balance is less than the cost";
        return;
    }
    stringstream ss;
    work W(*C);
    ss << "UPDATE ACCOUNT SET BALANCE = BALANCE - " << price * amount << " WHERE ACCOUNT_ID = ";
    ss << account_id;
    W.exec(ss.str());
    W.commit();
}


bool Database::cancel_order(int account_id, int trans_id, string &cancel_s, string &cancel_time, vector<string> &exec_shares,\
vector<string> &exec_price, vector<string> &exec_time, string &error_msg){
    stringstream query;
    work W(*C);
    query << "SELECT * FROM ACCOUNT WHERE ACCOUNT_ID = " << account_id << ";";
    result R = W.exec(query.str());
    W.commit();
    if(R.size() == 0) {
        error_msg = "cancellation failure: this account does not exist \
        or the trans_id does not belong to you";
        return false;
    }
  stringstream q;
  q << "SELECT SYMBOL, AMOUNT, PRICE, ACCOUNT_ID FROM ORDERS WHERE TRANS_ID = " << trans_id;
  q << " AND ACCOUNT_ID = " << account_id << " AND STATUS='open';";
  work W2(*C);
  result R2 = W2.exec(q.str());
  W2.commit();
  if (R2.size() != 0) {
    result::const_iterator iter = R2.begin();
    string sym = iter[0].as<string>();
    double amount = iter[1].as<int>();
    double price = iter[2].as<double>();
            if (amount < 0) {
              string error_msg2;
              create_position(account_id, sym, amount, error_msg2);
            } else {
              update_buyer_balance(account_id, price, 0, amount);
            }
            stringstream ss;
            ss << "UPDATE ORDERS SET STATUS = 'canceled'";
            ss << " WHERE TRANS_ID = "<< trans_id << " AND ACCOUNT_ID = " << account_id << " AND STATUS='open';";
            work W3(*C);
            W3.exec(ss.str());
            W3.commit();
  }
    stringstream ss2;
    work W4(*C);
    ss2 << "SELECT STATUS, AMOUNT, PRICE, TIME FROM ORDERS WHERE ACCOUNT_ID = " <<  account_id << " AND TRANS_ID = " << trans_id << ";";
    result R4 = W4.exec(ss2.str());
    W4.commit();
    auto iter2 = R4.begin();
    if (iter2 == R4.end()) {
        error_msg = "fail to query: this account does not exist, or transaction id is invalid for this acccount";
        return false;
    }
    while(iter2 != R4.end()) {
        string status = iter2[0].as<string>();
        double amount = iter2[1].as<double>();
        double price = iter2[2].as<double>();
        int time = iter2[3].as<int>();
        if (status == "canceled") {
            stringstream q2, t;
            q2 << amount;
            cancel_s = q2.str(); // r.cancel_shares
            t << time;
            cancel_time = t.str(); // r.cancel_time
        }
        else if (status == "executed") {
            string q_shares = to_string(amount);
            exec_shares.push_back(q_shares);
            string q_price = to_string(price);
            exec_price.push_back(q_price);
            string q_time = to_string(time);
            exec_time.push_back(q_time);
        }
        ++iter2;
    }
  return true;
}

void Database::match_order(int account_id, int trans_id, string sym, double amount, double price) {
    if (amount < 0) {
        match_sell_order(account_id, trans_id, sym, amount, price);
    }
    else {
        match_buy_order(account_id, trans_id, sym, amount, price);
    }
}

void Database::match_sell_order(int account_id, int trans_id, string sym, double amount, double price) {
    stringstream ss;
    work W(*C);
    ss << "SELECT ACCOUNT_ID, TRANS_ID, SYMBOL, AMOUNT, PRICE FROM ORDERS WHERE " << "STATUS = 'open' AND ";
    ss << "SYMBOL = " << W.quote(sym) << " AND AMOUNT > 0 AND PRICE >= " << price;
    ss << " AND ACCOUNT_ID != " << account_id << " ORDER BY PRICE DESC, TIME ASC;";
    result R(W.exec(ss.str()));
    W.commit();
    auto iter = R.begin();
    while (amount < 0 && iter != R.end()) {
        int buyer_id = iter[0].as<int>();
        int buy_id = iter[1].as<int>();
        double buy_amount = iter[3].as<double>();
        double buy_price = iter[4].as<double>();
        double exec_price = price;
        if(buy_id < trans_id) {
            exec_price = buy_price;
        }
        double exec_amount = -amount;
        if (exec_amount == buy_amount) {
            update_order_status(trans_id, account_id, exec_price);
            update_order_status(buy_id, buyer_id, exec_price);
        }
        else if (exec_amount > buy_amount) {
            update_order_status(buy_id, buyer_id, exec_price);
            insert_new_order(trans_id, account_id, sym, buy_amount-exec_amount, exec_price, price);
            exec_amount = buy_amount;
        }
        else {
            update_order_status(trans_id, account_id, exec_price);
            insert_new_order(buy_id, buyer_id, sym, buy_amount-exec_amount, exec_price, buy_price);
        }
        //
        update_seller_balance(account_id, exec_price, exec_amount);
        update_buyer_balance(buyer_id, buy_price, exec_price, exec_amount);
        string msg;
        create_position(buyer_id, sym, exec_amount, msg);
        //
        amount = amount + buy_amount;
        ++iter;
    }
}


void Database::update_seller_balance(int account_id, double price, double amount) {
    stringstream ss;
    work W(*C);
    ss << "UPDATE ACCOUNT SET BALANCE = BALANCE + " << price * amount << " WHERE ACCOUNT_ID = ";
    ss << account_id << ";";
    W.exec(ss.str());
    W.commit();
}


void Database::update_buyer_balance(int account_id, double price, double exec_price, double amount) {
    stringstream ss;
    work W(*C);
    double diff = price - exec_price;
    ss << "UPDATE ACCOUNT SET BALANCE = BALANCE + " << diff * amount << " WHERE ACCOUNT_ID = ";
    ss << account_id << ";";
    W.exec(ss.str());
    W.commit();
}


void Database::update_order_status(int trans_id, int account_id, double price) {
    stringstream ss;
    work W(*C);
    ss << "UPDATE ORDERS SET STATUS = 'executed', PRICE = " << price;
    //ss << ", TIME = " << time(NULL);
    ss << " WHERE TRANS_ID = " << trans_id << " AND ACCOUNT_ID = " << account_id <<" AND STATUS = 'open';";
    W.exec(ss.str());
    W.commit();
}

void Database::insert_new_order(int trans_id, int account_id, string sym, double amount, double price, double price2) {
    work W2(*C);
    stringstream ss2;
    ss2 << "UPDATE ORDERS SET STATUS = 'executed', AMOUNT = AMOUNT - " << amount;
    ss2 << ", PRICE = " << price;
    ss2 << " WHERE STATUS = 'open' AND TRANS_ID = ";
    ss2 << trans_id << ";";
    W2.exec(ss2.str());
    W2.commit();
    stringstream ss;
    work W(*C);
    ss << "INSERT INTO ORDERS (TRANS_ID, ACCOUNT_ID, SYMBOL, AMOUNT, PRICE, TIME) VALUES (";
    ss << trans_id << ", " << account_id << ", " << W.quote(sym) << ", " << amount << ", ";
    ss << price2 << ", " << time(NULL) << ");";
    W.exec(ss.str());
    W.commit();
}


void Database::match_buy_order(int account_id, int trans_id, string sym, double amount, double price) {
    stringstream ss;
    work W(*C);
    ss << "SELECT ACCOUNT_ID, TRANS_ID, SYMBOL, AMOUNT, PRICE FROM ORDERS WHERE " << "STATUS = 'open' AND ";
    ss << "SYMBOL = " << W.quote(sym) << " AND AMOUNT < 0 AND PRICE <= " << price;
    ss << " AND ACCOUNT_ID != " << account_id << " ORDER BY PRICE DESC, TIME ASC;";
    result R(W.exec(ss.str()));
    W.commit();
    auto iter = R.begin();
    while (amount > 0 && iter != R.end()) {
        int seller_id = iter[0].as<int>();
        int sell_id = iter[1].as<int>();
        double sell_amount = iter[3].as<int>();
        double sell_price = iter[4].as<double>();
        double exec_price = price;
        if(sell_id < trans_id) {
            exec_price = sell_price;
        }
        double exec_amount = amount;
        if (amount == -sell_amount) {
            update_order_status(trans_id, account_id, exec_price);
            update_order_status(sell_id, seller_id, exec_price);
        }
        else if (amount > -sell_amount) {
            update_order_status(sell_id, seller_id, exec_price);
            insert_new_order(trans_id, account_id, sym, amount+sell_amount,  exec_price, price);
            exec_amount = -sell_amount;
        }
        else {
            update_order_status(trans_id, account_id, exec_price);
            insert_new_order(sell_id, seller_id, sym, amount+sell_amount, exec_price, sell_price);
        }
        //
        update_seller_balance(seller_id, exec_price, exec_amount);
        update_buyer_balance(account_id, price, exec_price, exec_amount);
        string msg;
        create_position(account_id, sym, exec_amount, msg);
        //
        amount = amount + sell_amount;
        ++iter;
    }
}

void Database::query(int account_id, int trans_id, string &open_shares, string &cancel_shares, string &cancel_time,\
vector<string> &exec_shares, vector<string> &exec_price, vector<string> &exec_time, string &e) {
    stringstream ss;
    work W(*C);
    ss << "SELECT STATUS, AMOUNT, PRICE, TIME FROM ORDERS WHERE ACCOUNT_ID = " <<  account_id << " AND TRANS_ID = " << trans_id << ";";
    result R(W.exec(ss.str()));
    string q;
    auto iter = R.begin();
    if (iter == R.end()) {
        e = "fail to query: this account does not exist, or transaction id is invalid for this acccount";
        return;
    }
    while(iter != R.end()) {
        string status = iter[0].as<string>();
        double amount = iter[1].as<double>();
        double price = iter[2].as<double>();
        int time = iter[3].as<int>();
        if (status == "open") {
            stringstream q;
            q << amount;
            open_shares = q.str(); // r.open_shares
        }
        else if (status == "canceled") {
            stringstream q, t;
            q << amount;
            cancel_shares = q.str(); // r.cancel_shares
            t << time;
            cancel_time = t.str(); // r.cancel_time
        }
        else if (status == "executed") {
            string q_shares = to_string(amount);
            exec_shares.push_back(q_shares);
            string q_price = to_string(price);
            exec_price.push_back(q_price);
            string q_time = to_string(time);
            exec_time.push_back(q_time);
        }
        ++iter;
    }
}

//testing
void Database::add_from_account() {
  string filename = "account.txt";
  ifstream file(filename.c_str());
  if(file.fail()) {
    cout << "fail to open account.txt" << endl;
    return;
  }
  string line;
  int account_id, balance;
  while(getline(file, line)) {
    stringstream query(line);
    query >> account_id >> balance;
    string msg;
    create_account(account_id, balance, msg);
  }
  file.close();
}

void Database::add_from_position() {
  string filename = "position.txt";
  ifstream file(filename.c_str());
  if(file.fail()) {
    cout << "fail to open position.txt" << endl;
    return;
  }
  string line;
  int account_id;
  string sym;
  double amount;
  while(getline(file, line)) {
    stringstream query(line);
    query >> account_id >> sym >> amount;
    string msg;
    create_position(account_id, sym, amount, msg);
  }
  file.close();
}

void Database::add_from_orders() {
  string filename = "orders.txt";
  ifstream file(filename.c_str());
  if(file.fail()) {
    cout << "fail to open orders.txt" << endl;
    return;
  }
  string line;
  int account_id;
  string sym;
  double amount, price;
  while(getline(file, line)) {
    stringstream query(line);
    query >> account_id >> sym >> amount >> price;
    string msg;
    create_order(account_id, sym, amount, price, msg);
  }
  file.close();
}
