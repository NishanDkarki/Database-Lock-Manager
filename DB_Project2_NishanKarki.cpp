#include<bits/stdc++.h>
using namespace std;

int data;
string data_string_memory;
string data_string_buffer;
string command;

int tr_counter;
map<int, int>given_tr_id;
map<int, int>input_tr_id;
vector<int>transaction_list;

int get_tr_id(int tr_id, bool is_given_tr_id) {
    if(is_given_tr_id == true) {
        return given_tr_id[tr_id];
    } else {
        return input_tr_id[tr_id];
    }
}

void data_to_data_string() {
    string data_string = "";
    for(int i = 0; i < 32; i++) {
        if(data % 2 == 1) {
            data_string += '1';
        } else {
            data_string += '0';
        }
        data /= 2;
    }
    data_string_memory = data_string;
    data_string_buffer = data_string;
}

int convert_to_num(string data_string) {
    int num = 0;
    for(int i = 0; i < 32; i++) {
        if(data_string[i] == '1') {
            num += (1 << i);
        }
    }
    return num;
}

class Lock_Type {
  public:
    int Empty = 0;
    int Shared = 1;
    int Exclusive = 2;
};

Lock_Type lock_type;

class Transaction_Category {
  public:
    int Read = 0;
    int Write = 1;
};

Transaction_Category transaction_category;

class Transaction_History {
  public:
    int Waiting = 0;
    int Granted = 1;
    int Done = 2;
    int Undone = 3;
};

Transaction_History transaction_history;

class Transaction_Builder {
  public:
    int tr_id;
    int data_id;
    int tr_cat;
    int tr_status;
};

class Data_Transaction_Builder {
  public:
    int type_of_lock = lock_type.Empty;
    vector<Transaction_Builder>granted_transactions;
    vector<Transaction_Builder>waiting_transactions;
};

Data_Transaction_Builder data_transaction[32];
vector<int>graph_edges[10000];
bool check_visited[10000];

bool run_dfs(int tr_id) {
    check_visited[tr_id] = true;
    bool cycle = false;
    for(int i = 0; i < graph_edges[tr_id].size(); i++) {
        if(check_visited[graph_edges[tr_id][i]] == true) {
            cycle = true;
            break;
        }
        cycle = run_dfs(graph_edges[tr_id][i]);
        if(cycle == true) {
            break;
        }
    }
    check_visited[tr_id] = false;
    return cycle;
}

bool graph_cycle_checker() {
    for(int i = 0; i < transaction_list.size(); i++) {
        check_visited[i] = false;
    }
    for(int i = 1; i < transaction_list.size(); i++) {
        if(transaction_list[i] == 0) {
            if(check_visited[i] == false) {
                if(run_dfs(i) == true) {
                    return true;
                }
            }
        }
    }
    return false;
}

void build_graph_with_edge(int tr_id, int data_id, int tr_cat) {
    set<int>tr_wait_list;
    set<int> :: iterator set_iterator;

    for(int i = 0; i < graph_edges[tr_id].size(); i++) {
        tr_wait_list.insert(graph_edges[tr_id][i]);
    }

    vector<Transaction_Builder>previous_transactions;
    for(int i = 0; i < data_transaction[data_id].granted_transactions.size(); i++) {
        previous_transactions.push_back(data_transaction[data_id].granted_transactions[i]);
    }
    for(int i = 0; i < data_transaction[data_id].waiting_transactions.size(); i++) {
        previous_transactions.push_back(data_transaction[data_id].waiting_transactions[i]);
    }

    if(tr_cat == transaction_category.Read) {
        for(int i = previous_transactions.size() - 1; i >= 0; i--) {
            if(previous_transactions[i].tr_cat == transaction_category.Write) {
                if(previous_transactions[i].tr_id != tr_id) {
                    tr_wait_list.insert(previous_transactions[i].tr_id);
                    break;
                }
            }
        }
        graph_edges[tr_id].clear();
        for(set_iterator = tr_wait_list.begin(); set_iterator != tr_wait_list.end(); set_iterator++) {
            graph_edges[tr_id].push_back(*set_iterator);
        }
        return;
    } else {
        bool found_any_tr = false;
        for(int i = previous_transactions.size() - 1; i >= 0; i--) {
            if(previous_transactions[i].tr_cat == transaction_category.Write) {
                if(found_any_tr) {
                    break;
                }
                if(previous_transactions[i].tr_id != tr_id) {
                    found_any_tr = true;
                    tr_wait_list.insert(previous_transactions[i].tr_id);
                    break;
                }
            } else {
                if(previous_transactions[i].tr_id != tr_id) {
                    found_any_tr = true;
                    tr_wait_list.insert(previous_transactions[i].tr_id);
                }
            }
        }
        graph_edges[tr_id].clear();
        for(set_iterator = tr_wait_list.begin(); set_iterator != tr_wait_list.end(); set_iterator++) {
            graph_edges[tr_id].push_back(*set_iterator);
        }
        return;
    }
}

bool transaction_checker(int tr_id, int data_id, int tr_cat) {
    if(data_transaction[data_id].type_of_lock == lock_type.Empty) {
        return true;
    }
    if(tr_cat == transaction_category.Write) {
        for(int i = 0; i < data_transaction[data_id].granted_transactions.size(); i++) {
            int granted_tr_id = data_transaction[data_id].granted_transactions[i].tr_id;
            if(granted_tr_id != tr_id) {
                return false;
            }
        }
        return true;
    }
    if(tr_cat == transaction_category.Read) {
        for(int i = 0; i < data_transaction[data_id].granted_transactions.size(); i++) {
            int granted_tr_id = data_transaction[data_id].granted_transactions[i].tr_id;
            if(granted_tr_id == tr_id) {
                return true;
            }
        }
        if(data_transaction[data_id].type_of_lock == lock_type.Exclusive) {
            return false;
        }
        if(data_transaction[data_id].waiting_transactions.size() == 0) {
            return true;
        }
        return false;
    }
}

void data_transformer(int tr_id, int data_id, int tr_cat) {
    if(tr_cat == transaction_category.Write) {
        if(data_string_buffer[data_id] == '0') {
            data_string_buffer[data_id] = '1';
        } else {
            data_string_buffer[data_id] = '0';
        }
    }
}

void rollback(int tr_id);

void grant_transaction(int tr_id, int data_id, int tr_cat) {
    data_transformer(tr_id, data_id, tr_cat);

    Transaction_Builder transaction_builder;
    transaction_builder.data_id = data_id;
    transaction_builder.tr_id = tr_id;
    transaction_builder.tr_status = transaction_history.Granted;
    transaction_builder.tr_cat = tr_cat;

    data_transaction[data_id].granted_transactions.push_back(transaction_builder);

    if(data_transaction[data_id].type_of_lock == lock_type.Exclusive) {
        return;
    }

    if(tr_cat == transaction_category.Read) {
        data_transaction[data_id].type_of_lock = lock_type.Shared;
        return;
    }
    data_transaction[data_id].type_of_lock = lock_type.Exclusive;
    return;
}

void wait_transaction(int tr_id, int data_id, int tr_cat) {
    Transaction_Builder transaction_builder;
    transaction_builder.data_id = data_id;
    transaction_builder.tr_id = tr_id;
    transaction_builder.tr_status = transaction_history.Waiting;
    transaction_builder.tr_cat = tr_cat;

    data_transaction[data_id].waiting_transactions.push_back(transaction_builder);

    build_graph_with_edge(tr_id, data_id, tr_cat);

    if(graph_cycle_checker() == true) {
        rollback(tr_id);
    }

    return;
}

void rollback(int tr_id) {
    transaction_list[tr_id] = 1;
    data_string_buffer = data_string_memory;
    for(int i = 1; i < transaction_list.size(); i++) {
        graph_edges[i].clear();
    }
    for(int data_id = 0; data_id < 32; data_id++) {
        vector<Transaction_Builder>previous_transactions;
        for(int i = 0; i < data_transaction[data_id].granted_transactions.size(); i++) {
            Transaction_Builder tr = data_transaction[data_id].granted_transactions[i];
            if(tr.tr_id != tr_id) {
                previous_transactions.push_back(data_transaction[data_id].granted_transactions[i]);
            }
        }
        for(int i = 0; i < data_transaction[data_id].waiting_transactions.size(); i++) {
            Transaction_Builder tr = data_transaction[data_id].waiting_transactions[i];
            if(tr.tr_id != tr_id) {
                previous_transactions.push_back(data_transaction[data_id].waiting_transactions[i]);
            }
        }
        data_transaction[data_id].type_of_lock = lock_type.Empty;
        data_transaction[data_id].granted_transactions.clear();
        data_transaction[data_id].waiting_transactions.clear();
        for(int i = 0; i < previous_transactions.size(); i++) {
            Transaction_Builder tr = previous_transactions[i];
            if(transaction_checker(tr.tr_id, tr.data_id, tr.tr_cat)) {
                grant_transaction(tr.tr_id, tr.data_id, tr.tr_cat);
            } else {
                wait_transaction(tr.tr_id, tr.data_id, tr.tr_cat);
            }
        }
    }
}

void commit(int tr_id) {
    transaction_list[tr_id] = 1;
    data_string_buffer = data_string_memory;
    for(int i = 1; i < transaction_list.size(); i++) {
        graph_edges[i].clear();
    }
    for(int data_id = 0; data_id < 32; data_id++) {
        for(int i = 0; i < data_transaction[data_id].granted_transactions.size(); i++) {
            Transaction_Builder tr = data_transaction[data_id].granted_transactions[i];
            if(tr.tr_id == tr_id) {
                data_transformer(tr.tr_id, tr.data_id, tr.tr_cat);
            }
        }
    }
    data_string_memory = data_string_buffer;
    for(int data_id = 0; data_id < 32; data_id++) {
        vector<Transaction_Builder>previous_transactions;
        for(int i = 0; i < data_transaction[data_id].granted_transactions.size(); i++) {
            Transaction_Builder tr = data_transaction[data_id].granted_transactions[i];
            if(tr.tr_id != tr_id) {
                previous_transactions.push_back(data_transaction[data_id].granted_transactions[i]);
            }
        }
        for(int i = 0; i < data_transaction[data_id].waiting_transactions.size(); i++) {
            Transaction_Builder tr = data_transaction[data_id].waiting_transactions[i];
            if(tr.tr_id != tr_id) {
                previous_transactions.push_back(data_transaction[data_id].waiting_transactions[i]);
            }
        }
        data_transaction[data_id].type_of_lock = lock_type.Empty;
        data_transaction[data_id].granted_transactions.clear();
        data_transaction[data_id].waiting_transactions.clear();
        for(int i = 0; i < previous_transactions.size(); i++) {
            Transaction_Builder tr = previous_transactions[i];
            if(transaction_checker(tr.tr_id, tr.data_id, tr.tr_cat)) {
                grant_transaction(tr.tr_id, tr.data_id, tr.tr_cat);
            } else {
                wait_transaction(tr.tr_id, tr.data_id, tr.tr_cat);
            }
        }
    }
}

void start() {
    int tr_id;
    scanf("%d", &tr_id);
    tr_counter++;
    given_tr_id[tr_id] = tr_counter;
    input_tr_id[tr_counter] = tr_id;
    if(transaction_list.size() == 0) {
        transaction_list.push_back(0);
    }
    transaction_list.push_back(0);
}

void read() {
    int data_id, tr_id;
    scanf("%d %d", &data_id, &tr_id);
    tr_id = get_tr_id(tr_id, true);
    if(transaction_list[tr_id] == 1) {
        return;
    }
    if(transaction_checker(tr_id, data_id, transaction_category.Read)) {
        grant_transaction(tr_id, data_id, transaction_category.Read);
    } else {
        wait_transaction(tr_id, data_id, transaction_category.Read);
    }
}

void write() {
    int data_id, tr_id;
    scanf("%d %d", &data_id, &tr_id);
    tr_id = get_tr_id(tr_id, true);
    if(transaction_list[tr_id] == 1) {
        return;
    }
    if(transaction_checker(tr_id, data_id, transaction_category.Write)) {
        grant_transaction(tr_id, data_id, transaction_category.Write);
    } else {
        wait_transaction(tr_id, data_id, transaction_category.Write);
    }
}

void commit() {
    int tr_id;
    scanf("%d", &tr_id);
    tr_id = get_tr_id(tr_id, true);
    commit(tr_id);
}

void rollback() {
	int tr_id;
	scanf("%d", &tr_id);
	tr_id = get_tr_id(tr_id, true);
	rollback(tr_id);
}

int main() {
    freopen("input_transactions.txt", "r", stdin);
    cin >> data;
    data_to_data_string();

    while(cin >> command) {
        if(command == "EOF") {
            break;
        }
        if(command == "Start") {
            start();
        }
        if(command == "Read") {
            read();
        }
        if(command == "Write") {
            write();
        }
        if(command == "Commit") {
            commit();
        }
		if (command == "Rollback") {
			rollback();
		}
    }



    cout << "Data Lock Table: " << endl << endl;
    for(int data_id = 0; data_id < 32; data_id++) {
        if(data_transaction[data_id].type_of_lock != lock_type.Empty) {
            cout << "Data Index = " << data_id << endl;
            cout << "\tLock Type = ";
            if(data_transaction[data_id].type_of_lock == lock_type.Shared) {
                cout << "Shared";
            } else {
                cout << "Exclusive";
            }
            cout << endl;
            cout << "\tGranted List:  ";
            for(int j = 0; j < data_transaction[data_id].granted_transactions.size(); j++) {
                cout << get_tr_id(data_transaction[data_id].granted_transactions[j].tr_id, false) << "  ";
            }
            cout << endl;
            cout << "\tWaiting List:  ";
            for(int j = 0; j < data_transaction[data_id].waiting_transactions.size(); j++) {
                cout << get_tr_id(data_transaction[data_id].waiting_transactions[j].tr_id, false) << "  ";
            }
            cout << endl << endl;
        }
    }
    cout << endl;

    cout << "Transaction Wait Table:" << endl<< endl;
    for(int i = 1; i < transaction_list.size(); i++) {
        if(transaction_list[i] == 0) {
            cout << "Transaction - " << get_tr_id(i, false) << ", Waiting for : ";
            for(int j = 0; j < graph_edges[i].size(); j++) {
                cout << get_tr_id(graph_edges[i][j], false) << "  ";
            }
            cout << endl;
        }
    }
    cout << endl << endl;

    cout << "Memory Integer = " << convert_to_num(data_string_memory) << endl;
    cout << "Buffer Integer = " << convert_to_num(data_string_buffer) << endl << endl << endl;

    return 0;
}
