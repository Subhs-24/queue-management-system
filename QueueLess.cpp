#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <iomanip>
#include <ctime>
using namespace std;
struct Service {
    int id;
    string name;
    double price;
};
struct Shop {
    int id;
    string name;
    vector<Service> services;
};
struct Order {
    int token;
    string customer;
    int shopId;
    string shopName;
    string item;
    int quantity;
    double total;
    string status;
    time_t createdAt;
    int prepSeconds;
};
class QueueLess {
private:
    const string RESET = "\033[0m";
    const string CYAN = "\033[36m";
    const string GREEN = "\033[32m";
    const string YELLOW = "\033[33m";
    const string RED = "\033[31m";
    const string BOLD = "\033[1m";

    void enableConsoleColors() {
        // ANSI colors are supported by modern VS Code and Windows terminals.
    }

    void boxTop(const string &title) {
        cout << RED << BOLD
             << "\n+==================================================+\n"
             << "| " << left << setw(48) << title << " |\n"
             << "+==================================================+\n" << RESET;
    }

    void boxBottom() {
        cout << RED << "+==================================================+\n" << RESET;
    }

    void menuOption(const string &number, const string &label) {
        cout << RED << "| " << RESET << YELLOW << "[" << number << "]"
             << RESET << " " << left << setw(45) << label
             << RED << "|\n" << RESET;
    }

    void coloredMessage(const string &message, const string &color) {
        cout << color << message << RESET;
    }
    vector<Shop> shops;
    vector<Order> orders;
    int nextShopId = 1, nextServiceId = 1, nextToken = 1001;
    const string adminUser = "admin";
    const string adminPass = "admin123";
    const int preparationSeconds = 30;
    void saveShops() {
        ofstream out("shops.txt");
        for (const auto &s : shops)
            out << s.id << '|' << s.name << '\n';
    }
    void saveServices() {
        ofstream out("services.txt");
        for (const auto &s : shops)
            for (const auto &v : s.services)
                out << s.id << '|' << v.id << '|' << v.name << '|'
                    << v.price << '\n';
    }
    void saveOrders() {
        ofstream out("orders.txt");
        for (const auto &o : orders)
            out << o.token << '|' << o.customer << '|' << o.shopId << '|'
                << o.shopName << '|' << o.item << '|' << o.quantity << '|'
                << o.total << '|' << o.status << '|' << o.createdAt
                << '|' << o.prepSeconds << '\n';
    }
    void loadData() {
        ifstream in("shops.txt");
        string line;
        while (getline(in, line)) {
            stringstream ss(line);
            string id, name;
            if (getline(ss, id, '|') && getline(ss, name)) {
                Shop s{stoi(id), name, {}};
                shops.push_back(s);
                if (s.id >= nextShopId) nextShopId = s.id + 1;
            }
        }
        in.close();
        ifstream sin("services.txt");
        while (getline(sin, line)) {
            stringstream ss(line);
            string shopId, serviceId, name, price;
            if (getline(ss, shopId, '|') && getline(ss, serviceId, '|') &&
                getline(ss, name, '|') && getline(ss, price)) {
                for (auto &s : shops) {
                    if (s.id == stoi(shopId)) {
                        s.services.push_back({stoi(serviceId), name, stod(price)});
                        if (stoi(serviceId) >= nextServiceId)
                            nextServiceId = stoi(serviceId) + 1;
                    }
                }
            }
        }
        sin.close();
        ifstream oin("orders.txt");
        while (getline(oin, line)) {
            stringstream ss(line);
            string token, customer, shopId, shopName, item, qty, total, status;
            if (getline(ss, token, '|') && getline(ss, customer, '|') &&
                getline(ss, shopId, '|') && getline(ss, shopName, '|') &&
                getline(ss, item, '|') && getline(ss, qty, '|') &&
                getline(ss, total, '|') && getline(ss, status, '|')) {
                string timeField, durationField;
                time_t createdAt = time(nullptr);
                int prepSeconds = preparationSeconds;

                if (getline(ss, timeField, '|')) {
                    time_t savedTime = static_cast<time_t>(stoll(timeField));
                    if (getline(ss, durationField)) {
                        createdAt = savedTime;
                        prepSeconds = stoi(durationField);
                    } else {
                        // Older file format stored the ready-at timestamp.
                        prepSeconds = 5;
                        createdAt = savedTime - prepSeconds;
                        if (status == "Preparing" && time(nullptr) >= savedTime)
                            status = "Ready";
                    }
                }

                orders.push_back({stoi(token), customer, stoi(shopId), shopName,
                                  item, stoi(qty), stod(total), status,
                                  createdAt, prepSeconds});
                if (stoi(token) >= nextToken) nextToken = stoi(token) + 1;
            }
        }
        oin.close();
        if (shops.empty()) addDefaultShops();
    }
    void addDefaultShops() {
        Shop stationery{nextShopId++, "Stationery", {}};
        stationery.services.push_back({nextServiceId++, "Printout", 5});
        stationery.services.push_back({nextServiceId++, "Practical File", 40});
        stationery.services.push_back({nextServiceId++, "Notebook", 35});
        Shop cafe{nextShopId++, "Campus Cafe", {}};
        cafe.services.push_back({nextServiceId++, "Coffee", 30});
        cafe.services.push_back({nextServiceId++, "Sandwich", 50});
        cafe.services.push_back({nextServiceId++, "Maggie", 40});
        Shop pharmacy{nextShopId++, "Health Store", {}};
        pharmacy.services.push_back({nextServiceId++, "Bandage", 20});
        pharmacy.services.push_back({nextServiceId++, "Face Mask", 10});
        Shop photocopy{nextShopId++, "Photocopy Centre", {}};
        photocopy.services.push_back({nextServiceId++, "Black & White Copy", 2});
        photocopy.services.push_back({nextServiceId++, "Colour Copy", 10});
        shops.push_back(stationery);
        shops.push_back(cafe);
        shops.push_back(pharmacy);
        shops.push_back(photocopy);
        saveShops();
        saveServices();
    }
    Shop* findShop(int id) {
        for (auto &s : shops) if (s.id == id) return &s;
        return nullptr;
    }
    void showShops() {
        boxTop("AVAILABLE SHOPS");
        if (shops.empty()) coloredMessage("No shops available.\\n", RED);
        for (const auto &s : shops)
            cout << GREEN << "[" << s.id << "] " << RESET << s.name << '\n';
        boxBottom();
    }
    void showServices(const Shop &s) {
        boxTop("SERVICES | " + s.name);
        for (const auto &v : s.services)
            cout << GREEN << "[" << v.id << "] " << RESET << v.name
                 << " - Rs. " << fixed << setprecision(2) << v.price << '\n';
        boxBottom();
    }
    void updateOrders() {
        bool changed = false;
        time_t now = time(nullptr);
        for (auto &o : orders) {
            double elapsed = difftime(now, o.createdAt);
            if (o.status == "Preparing" && elapsed >= o.prepSeconds) {
                o.status = "Ready";
                changed = true;
                cout << GREEN << "\\n[QueueLess] Token #" << o.token
                     << " is now READY!\\n" << RESET;
            }
        }
        if (changed) saveOrders();
    }

    void placeOrder(const string &customer) {
        if (shops.empty()) {
            cout << "No shops are currently available.\n";
            return;
        }
        showShops();
        int sid;
        cout << "Enter shop ID: ";
        cin >> sid;
        Shop *shop = findShop(sid);
        if (!shop) {
            cout << "Invalid shop ID.\n";
            return;
        }
        if (shop->services.empty()) {
            cout << "This shop has no services yet.\n";
            return;
        }
        showServices(*shop);
        int vid, qty;
        cout << "Enter service ID: ";
        cin >> vid;
        Service *service = nullptr;
        for (auto &v : shop->services)
            if (v.id == vid) service = &v;
        if (!service) {
            cout << "Invalid service ID.\n";
            return;
        }
        cout << "Enter quantity: ";
        cin >> qty;
        if (qty < 1 || qty > 100) {
            cout << "Quantity must be between 1 and 100.\n";
            return;
        }
        Order o{nextToken++, customer, shop->id, shop->name, service->name,
                qty, service->price * qty, "Preparing", time(nullptr),
                preparationSeconds};
        orders.push_back(o);
        saveOrders();
        cout << "\nOrder placed! Your token is " << o.token
             << "\nShop: " << o.shopName << "\nTotal: Rs. "
             << fixed << setprecision(2) << o.total
             << "\nStatus: Preparing... (ready in about 5 seconds)"
             << "\nYou can browse shops or place another order now.\n";
    }
    void customerMenu() {
        string name;
        cout << "Enter your name: ";
        cin >> ws;
        getline(cin, name);
        if (name.empty()) {
            cout << "Name cannot be empty.\n";
            return;
        }
        int choice;
        do {
            updateOrders();
            boxTop("QUEUELESS | CUSTOMER: " + name);
            menuOption("1", "Browse shops");
            menuOption("2", "Place an order");
            menuOption("3", "Track / collect order");
            menuOption("0", "Back to main menu");
            boxBottom();
            cout << YELLOW << "Choose an option: " << RESET;
            cin >> choice;
            if (choice == 1) showShops();
            else if (choice == 2) placeOrder(name);
            else if (choice == 3) trackOrder();
        } while (choice != 0);
    }
    void trackOrder() {
        updateOrders();
        int token;
        cout << "Enter token number: ";
        cin >> token;
        for (auto &o : orders) {
            if (o.token == token) {
                cout << "\nToken: " << o.token << "\nCustomer: " << o.customer
                     << "\nShop: " << o.shopName << "\nItem: " << o.item
                     << "\nQuantity: " << o.quantity << "\nTotal: Rs. "
                     << fixed << setprecision(2) << o.total
                     << "\nStatus: " << o.status << '\n';
                if (o.status == "Ready") {
                    char collect;
                    cout << "Collect this order now? (y/n): ";
                    cin >> collect;
                    if (collect == 'y' || collect == 'Y') {
                        o.status = "Collected";
                        saveOrders();
                        cout << "Order marked as collected. Enjoy!\n";
                    }
                } else if (o.status == "Preparing") {
                    cout << "Your order is still being prepared.\n";
                } else {
                    cout << "This order has already been collected.\n";
                }
                return;
            }
        }
        cout << "Token not found.\n";
    }
    bool adminLogin() {
        string u, p;
        cout << "Admin username: ";
        cin >> u;
        cout << "Admin password: ";
        cin >> p;
        if (u == adminUser && p == adminPass) return true;
        cout << "Incorrect login.\n";
        return false;
    }
    void addShop() {
        Shop s;
        s.id = nextShopId++;
        cout << "Enter shop name (no | character): ";
        cin >> ws;
        getline(cin, s.name);
        if (s.name.empty() || s.name.find('|') != string::npos) {
            cout << "Invalid shop name.\n";
            return;
        }
        shops.push_back(s);
        saveShops();
        cout << "Shop added with ID " << s.id << ".\n";
    }
    void removeShop() {
        showShops();
        int id;
        cout << "Enter shop ID to remove: ";
        cin >> id;
        for (auto it = shops.begin(); it != shops.end(); ++it) {
            if (it->id == id) {
                string removed = it->name;
                shops.erase(it);
                saveShops();
                saveServices();
                cout << removed << " removed.\n";
                return;
            }
        }
        cout << "Shop not found.\n";
    }
    void addService() {
        showShops();
        int id;
        cout << "Enter shop ID: ";
        cin >> id;
        Shop *s = findShop(id);
        if (!s) {
            cout << "Shop not found.\n";
            return;
        }
        Service v;
        v.id = nextServiceId++;
        cout << "Service/product name (no | character): ";
        cin >> ws;
        getline(cin, v.name);
        if (v.name.empty() || v.name.find('|') != string::npos) {
            cout << "Invalid name.\n";
            return;
        }
        cout << "Price: ";
        cin >> v.price;
        if (v.price < 0) {
            cout << "Price cannot be negative.\n";
            return;
        }
        s->services.push_back(v);
        saveServices();
        cout << "Service added.\n";
    }
    void removeService() {
        showShops();
        int id;
        cout << "Enter shop ID: ";
        cin >> id;
        Shop *s = findShop(id);
        if (!s) {
            cout << "Shop not found.\n";
            return;
        }
        showServices(*s);
        int vid;
        cout << "Enter service ID to remove: ";
        cin >> vid;
        for (auto it = s->services.begin(); it != s->services.end(); ++it) {
            if (it->id == vid) {
                s->services.erase(it);
                saveServices();
                cout << "Service removed.\n";
                return;
            }
        }
        cout << "Service not found.\n";
    }
    void showAllOrders() {
        updateOrders();
        cout << "\n--- All Orders ---\n";
        if (orders.empty()) cout << "No orders yet.\n";
        for (const auto &o : orders)
            cout << "Token " << o.token << " | " << o.customer << " | "
                 << o.shopName << " | " << o.item << " x" << o.quantity
                 << " | Rs. " << o.total << " | " << o.status << '\n';
    }
    void adminMenu() {
        int choice;
        do {
            updateOrders();
            boxTop("QUEUELESS | ADMIN PANEL");
            menuOption("1", "View shops");
            menuOption("2", "Add shop");
            menuOption("3", "Remove shop");
            menuOption("4", "Add service / product");
            menuOption("5", "Remove service / product");
            menuOption("6", "View all orders");
            menuOption("0", "Logout");
            boxBottom();
            cout << YELLOW << "Choose an option: " << RESET;
            cin >> choice;
            switch (choice) {
                case 1: showShops(); break;
                case 2: addShop(); break;
                case 3: removeShop(); break;
                case 4: addService(); break;
                case 5: removeService(); break;
                case 6: showAllOrders(); break;
                case 0: cout << "Admin logged out.\n"; break;
                default: cout << "Invalid choice.\n";
            }
        } while (choice != 0);
    }
public:
    QueueLess() { enableConsoleColors(); loadData(); }
    void run() {
        int choice;
        do {
            boxTop("QUEUELESS | SMART CAMPUS QUEUE");
            menuOption("1", "Customer portal");
            menuOption("2", "Admin panel");
            menuOption("0", "Exit QueueLess");
            boxBottom();
            cout << YELLOW << "Choose an option: " << RESET;
            cin >> choice;
            switch (choice) {
                case 1: customerMenu(); break;
                case 2:
                    if (adminLogin()) adminMenu();
                    break;
                case 0: cout << "Thank you for using QueueLess!\n"; break;
                default: cout << "Invalid choice.\n";
            }
        } while (choice != 0);
    }
};
int main() {
    QueueLess app;
    app.run();
    return 0;
}