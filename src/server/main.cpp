#include "server.h"


int main() {
    Database db;
    db.init_db();

    //db.add_from_account();//testing
    //db.add_from_position();

    db.disconnect();
    /*
    db.add_from_account();
    db.add_from_position();
    db.add_from_orders();
    db.match_order(0, 0, "abc", -50, 1);
    db.match_order(0, 2, "abc", -20, 1);
    string o, cancel_s, cancel_t;
    vector<string> exec_shares, exec_price, exec_time;
    db.query(1, 0, o, cancel_s, cancel_t, exec_shares, exec_price, exec_time);
    db.disconnect();
    */

    //db.add_from_orders();
    
    Server s;
    s.run();
    


    return 0;
    
}
