#include <iostream>
#include <fstream>
#include <sstream>
#include <map>
#include <unordered_map>
#include <vector>
#include <queue>
#include <set>
#include <limits>
#include <algorithm>
#include <cctype>
#include <iomanip>
#include <random>
#include <chrono>
#include <thread>
#include <ctime>
#include <cmath>

using namespace std;

const float INF = numeric_limits<float>::infinity();
const string DATA_PATH = "delhi_location_edges.csv";
const string DRIVERS_FILE = "registered_drivers.csv";
const string RIDE_HISTORY = "ride_history.csv";
const string USER_ACCOUNTS = "user_accounts.csv";
const string DRIVER_RATINGS = "driver_ratings.csv";

const float PI = 3.14159265358979323846;

string normalize(const string& s) {
    string res;
    for (char c : s) {
        if (!isspace(c)) res += tolower(c);
    }
    return res;
}

class RatingSystem {
private:
    map<string, pair<float, int>> driverRatings;
public:
    void loadRatings() {
        ifstream fin(DRIVER_RATINGS);
        if (!fin.is_open()) return;
        string line;
        while (getline(fin, line)) {
            stringstream ss(line);
            string phone, ratingStr, countStr;
            getline(ss, phone, ',');
            getline(ss, ratingStr, ',');
            getline(ss, countStr, ',');
            try {
                float rating = stof(ratingStr);
                int count = stoi(countStr);
                driverRatings[phone] = {rating, count};
            } catch (...) { continue; }
        }
        fin.close();
    }
    void addRating(const string& driverPhone, float rating) {
        if (rating < 1 || rating > 5) return;
        auto& [total, count] = driverRatings[driverPhone];
        total += rating;
        count++;
    }
    float getAverageRating(const string& driverPhone) {
        if (!driverRatings.count(driverPhone)) return 0.0f;
        auto [total, count] = driverRatings[driverPhone];
        return count > 0 ? total / count : 0.0f;
    }
    void saveRatings() {
        ofstream fout(DRIVER_RATINGS);
        if (!fout.is_open()) {
            cerr << "Error: Could not open " << DRIVER_RATINGS << " for writing.\n";
            return;
        }
        for (const auto& [phone, ratingData] : driverRatings) {
            fout << phone << "," << ratingData.first << "," << ratingData.second << "\n";
        }
        fout.close();
    }
    void displayDriverRating(const string& driverPhone) {
        float rating = getAverageRating(driverPhone);
        cout << "Driver Rating: ";
        if (rating == 0) cout << "No ratings yet\n";
        else cout << fixed << setprecision(1) << rating << "/5 (" << string(static_cast<int>(round(rating)), '*') << ")\n";
    }
};

class UserAccount {
private:
    string username, phone, email, paymentMethod;
    float walletBalance;
public:
    UserAccount(const string& uname, const string& ph, const string& em, const string& pm, float bal)
        : username(uname), phone(ph), email(em), paymentMethod(pm), walletBalance(bal) {}
    void displayProfile() const {
        cout << "\n=== User Profile ===\nName: " << username << "\nPhone: " << phone 
             << "\nEmail: " << (email.empty() ? "Not provided" : email)
             << "\nPayment Method: " << paymentMethod << "\nWallet Balance: RS " 
             << fixed << setprecision(2) << walletBalance << "\n";
    }
    bool makePayment(float amount) {
        if (walletBalance >= amount) { walletBalance -= amount; return true; }
        return false;
    }
    void addToWallet(float amount) { walletBalance += amount; }
    float getBalance() const { return walletBalance; }
    string getPaymentMethod() const { return paymentMethod; }
    void updatePaymentMethod(const string& newMethod) { paymentMethod = newMethod; }
    void saveToFile(ofstream& fout) const {
        fout << username << "," << phone << "," << email << ","
             << paymentMethod << "," << walletBalance << "\n";
    }
};

class UserManager {
private:
    unordered_map<string, UserAccount> users;
public:
    UserManager() { loadUsers(); }
    void loadUsers() {
        ifstream fin(USER_ACCOUNTS);
        if (!fin.is_open()) return;
        string line;
        while (getline(fin, line)) {
            stringstream ss(line);
            string username, phone, email, paymentMethod, balanceStr;
            getline(ss, username, ',');
            getline(ss, phone, ',');
            getline(ss, email, ',');
            getline(ss, paymentMethod, ',');
            getline(ss, balanceStr, ',');
            try {
                users.emplace(phone, UserAccount(username, phone, email, paymentMethod, stof(balanceStr)));
            } catch (...) { continue; }
        }
        fin.close();
    }
    bool userExists(const string& phone) const { return users.count(phone) > 0; }
    UserAccount* getUser(const string& phone) { return users.count(phone) ? &users.at(phone) : nullptr; }
    void createUser(const string& username, const string& phone) {
        if (userExists(phone)) { cout << "User already exists!\n"; return; }
        string email, paymentMethod;
        cout << "Enter email (optional): "; getline(cin, email);
        while (true) {
            cout << "Enter payment method (Cash/Wallet): "; getline(cin, paymentMethod);
            if (normalize(paymentMethod) == "cash" || normalize(paymentMethod) == "wallet") break;
            cout << "Invalid payment method! Please choose Cash or Wallet.\n";
        }
        paymentMethod = normalize(paymentMethod) == "cash" ? "Cash" : "Wallet";
        users.emplace(phone, UserAccount(username, phone, email, paymentMethod, 0.0f));
        saveUsers();
        cout << "Account created successfully!\n";
    }
    void saveUsers() {
        ofstream fout(USER_ACCOUNTS);
        if (!fout.is_open()) {
            cerr << "Error: Could not open " << USER_ACCOUNTS << " for writing.\n";
            return;
        }
        for (const auto& [phone, account] : users) account.saveToFile(fout);
        fout.close();
    }
    size_t getUserCount() const { return users.size(); }
};

class Driver {
public:
    string name, phone, location, vehicleType;
    bool available;
    Driver(const string& n, const string& p, const string& loc, const string& vt, bool avail = true)
        : name(n), phone(p), location(loc), vehicleType(vt), available(avail) {}
    void displayInfo() const {
        cout << "\n=== Driver Information ===\nName: " << name << "\nPhone: " << phone
             << "\nLocation: " << location << "\nVehicle: " << vehicleType 
             << "\nStatus: " << (available ? "Available" : "On Ride") << "\n";
    }
    void toggleAvailability() { available = !available; }
    void saveToFile(ofstream& fout) const {
        fout << name << "," << phone << "," << location << ","
             << vehicleType << "," << available << "\n";
    }
};

class DriverManager {
private:
    vector<Driver> drivers;
public:
    DriverManager() { loadDrivers(); }
    void loadDrivers() {
        drivers.clear();
        ifstream fin(DRIVERS_FILE);
        if (!fin.is_open() || fin.peek() == ifstream::traits_type::eof()) {
            cout << "No driver file found or empty. Initializing default drivers...\n";
            drivers = {
                Driver("Ramesh", "9990010001", "Connaught Place", "2-wheeler", true),
                Driver("Neha", "9990051112", "Lajpat Nagar", "2-wheeler", true),
                Driver("Anil", "9991012223", "RK Puram", "4-seater", true),
                Driver("Sita", "9992023334", "Saket", "7-seater", true),
                Driver("Aman", "9993034445", "Jasola", "4-seater", true),
                Driver("Priya", "9994045556", "Tilak Nagar", "2-wheeler", true),
                Driver("Deepak", "9995056667", "Dwarka", "4-seater", true),
                Driver("Komal", "9996067778", "Janakpuri", "2-wheeler", true),
                Driver("Manoj", "9997078889", "Karol Bagh", "7-seater", true),
                Driver("Tina", "9998089990", "Green Park", "2-wheeler", true),
                Driver("Rahul", "9999099999", "Saket", "4-seater", true),
                Driver("Vikram", "9999098888", "Saket", "7-seater", true),
                Driver("Sunita", "9999097777", "Lajpat Nagar", "7-seater", true)
            };
            saveDrivers();
            cout << "Initialized " << drivers.size() << " default drivers.\n";
            return;
        }
        string line;
        int lineNum = 0;
        cout << "Loading drivers from " << DRIVERS_FILE << ":\n";
        while (getline(fin, line)) {
            lineNum++;
            stringstream ss(line);
            string name, phone, location, vehicleType, availStr;
            getline(ss, name, ',');
            getline(ss, phone, ',');
            getline(ss, location, ',');
            getline(ss, vehicleType, ',');
            getline(ss, availStr, ',');
            if (name.empty() || phone.empty() || location.empty() || vehicleType.empty()) {
                cout << "Warning: Skipping invalid line " << lineNum << ": " << line << "\n";
                continue;
            }
            bool available = (availStr == "1" || availStr == "true");
            drivers.emplace_back(name, phone, location, vehicleType, available);
            cout << "Loaded driver: " << name << ", " << phone << ", " << vehicleType << "\n";
        }
        fin.close();
        bool hasTwoWheeler = false, hasSevenSeaterSaket = false;
        for (const auto& driver : drivers) {
            if (driver.vehicleType == "2-wheeler" && driver.available) hasTwoWheeler = true;
            if (driver.vehicleType == "7-seater" && normalize(driver.location) == "saket" && driver.available) hasSevenSeaterSaket = true;
        }
        if (!hasTwoWheeler) {
            drivers.emplace_back("Ramesh", "9990010001", "Connaught Place", "2-wheeler", true);
            cout << "Added 2-wheeler driver.\n";
        }
        if (!hasSevenSeaterSaket) {
            drivers.emplace_back("Vikram", "9999098888", "Saket", "7-seater", true);
            cout << "Added 7-seater driver at Saket.\n";
        }
        saveDrivers();
        cout << "Loaded " << drivers.size() << " drivers.\n";
    }
    void registerNewDriver() {
        string name, phone, location, vehicle;
        cout << "Enter your name: "; getline(cin, name);
        while (true) {
            cout << "Phone number (10 digits): "; getline(cin, phone);
            if (phone.length() == 10 && all_of(phone.begin(), phone.end(), ::isdigit)) break;
            cout << "Invalid number! Try again.\n";
        }
        cout << "Enter your current location: "; getline(cin, location);
        cout << "Select vehicle (1 = 2-wheeler, 2 = 4-seater, 3 = 7-seater): ";
        int choice; cin >> choice; cin.ignore();
        if (choice == 1) vehicle = "2-wheeler";
        else if (choice == 2) vehicle = "4-seater";
        else if (choice == 3) vehicle = "7-seater";
        else { cout << "Invalid choice. Registration failed.\n"; return; }
        drivers.emplace_back(name, phone, location, vehicle);
        saveDrivers();
        cout << "Driver registered successfully!\n";
    }
    void saveDrivers() {
        ofstream fout(DRIVERS_FILE);
        if (!fout.is_open()) {
            cerr << "Error: Could not open " << DRIVERS_FILE << " for writing.\n";
            return;
        }
        int count = 0;
        for (const auto& driver : drivers) {
            if (!driver.name.empty() && !driver.phone.empty()) {
                driver.saveToFile(fout);
                count++;
            }
        }
        fout.close();
        cout << "Saved " << count << " drivers to " << DRIVERS_FILE << ".\n";
    }
    vector<Driver> getAllDrivers() const { return drivers; }
    void toggleDriverAvailability(const string& phone) {
        for (auto& driver : drivers) {
            if (driver.phone == phone) {
                driver.toggleAvailability();
                saveDrivers();
                cout << "Availability updated to: " << (driver.available ? "Available" : "On Ride") << "\n";
                return;
            }
        }
        cout << "Driver not found!\n";
    }
    size_t getDriverCount() const { return drivers.size(); }
};

class Graph {
private:
    map<string, vector<pair<string, float>>> adj;
    map<string, pair<float, float>> locationCoords;
public:
    void addEdge(const string& from, const string& to, float distance) {
        string a = normalize(from), b = normalize(to);
        adj[a].emplace_back(b, distance);
        adj[b].emplace_back(a, distance);
    }
    void addLocation(const string& location, float lat, float lon) {
        locationCoords[normalize(location)] = {lat, lon};
    }
    float haversineDistance(const string& loc1, const string& loc2) {
        string l1 = normalize(loc1), l2 = normalize(loc2);
        if (!locationCoords.count(l1) || !locationCoords.count(l2)) return INF;
        auto [lat1, lon1] = locationCoords[l1];
        auto [lat2, lon2] = locationCoords[l2];
        lat1 *= PI / 180.0; lon1 *= PI / 180.0;
        lat2 *= PI / 180.0; lon2 *= PI / 180.0;
        float dlat = lat2 - lat1, dlon = lon2 - lon1;
        float a = pow(sin(dlat / 2), 2) + cos(lat1) * cos(lat2) * pow(sin(dlon / 2), 2);
        return 6371 * 2 * atan2(sqrt(a), sqrt(1 - a));
    }
    pair<float, vector<string>> shortestPath(const string& src, const string& dest) {
        string s = normalize(src), d = normalize(dest);
        if (!adj.count(s) || !adj.count(d)) return {INF, {}};
        map<string, float> dist;
        map<string, string> parent;
        set<string> visited;
        for (const auto& [node, _] : adj) dist[node] = INF;
        dist[s] = 0.0;
        priority_queue<pair<float, string>, vector<pair<float, string>>, greater<>> pq;
        pq.push({0.0, s});
        while (!pq.empty()) {
            auto [currDist, u] = pq.top(); pq.pop();
            if (visited.count(u)) continue;
            visited.insert(u);
            for (auto [v, weight] : adj[u]) {
                if (dist[v] > dist[u] + weight) {
                    dist[v] = dist[u] + weight;
                    parent[v] = u;
                    pq.push({dist[v], v});
                }
            }
        }
        if (dist[d] == INF) return {INF, {}};
        vector<string> path;
        for (string at = d; at != s; at = parent[at]) path.push_back(at);
        path.push_back(s);
        reverse(path.begin(), path.end());
        return {dist[d], path};
    }
    bool locationExists(const string& location) const { return adj.count(normalize(location)) > 0; }
};

class RideHistory {
public:
    struct RideRecord {
        string bookingID, user, userPhone, source, destination, vehicleType;
        float fare;
        string driverName, driverPhone, timestamp;
        float rating;
    };
    vector<RideRecord> records;
    void loadHistory() {
        ifstream fin(RIDE_HISTORY);
        if (!fin.is_open()) return;
        string line; getline(fin, line);
        while (getline(fin, line)) {
            stringstream ss(line);
            RideRecord record;
            string fareStr, ratingStr;
            getline(ss, record.bookingID, ',');
            getline(ss, record.user, ',');
            getline(ss, record.userPhone, ',');
            getline(ss, record.source, ',');
            getline(ss, record.destination, ',');
            getline(ss, record.vehicleType, ',');
            getline(ss, fareStr, ',');
            getline(ss, record.driverName, ',');
            getline(ss, record.driverPhone, ',');
            getline(ss, record.timestamp, ',');
            getline(ss, ratingStr, ',');
            try {
                record.fare = stof(fareStr);
                record.rating = ratingStr.empty() ? 0.0f : stof(ratingStr);
                records.push_back(record);
            } catch (...) { continue; }
        }
        fin.close();
    }
    void displayUserHistory(const string& userPhone) {
        cout << "\n=== Your Ride History ===\n";
        bool found = false;
        for (const auto& record : records) {
            if (record.userPhone == userPhone) {
                found = true;
                cout << "Booking ID: " << record.bookingID << "\nFrom: " << record.source 
                     << " To: " << record.destination << "\nVehicle: " << record.vehicleType
                     << "\nDriver: " << record.driverName << " (" << record.driverPhone << ")\n"
                     << "Fare: RS " << fixed << setprecision(2) << record.fare << "\nDate: " 
                     << record.timestamp << "\nRating: " << (record.rating > 0 ? to_string(record.rating)+"/5" : "Not rated") << "\n------------------------\n";
            }
        }
        if (!found) cout << "No ride history found.\n";
    }
    void addRating(const string& bookingID, float rating, RatingSystem& ratingSystem) {
        for (auto& record : records) {
            if (record.bookingID == bookingID && record.rating == 0.0f) {
                record.rating = rating;
                ratingSystem.addRating(record.driverPhone, rating);
                saveHistory();
                cout << "Rating submitted successfully!\n";
                return;
            }
        }
        cout << "Invalid booking ID or already rated.\n";
    }
    void saveHistory() {
        ofstream fout(RIDE_HISTORY);
        if (!fout.is_open()) {
            cerr << "Error: Could not open " << RIDE_HISTORY << " for writing.\n";
            return;
        }
        fout << "BookingID,User,UserPhone,Source,Destination,VehicleType,Fare,Driver,DriverPhone,Timestamp,Rating\n";
        for (const auto& record : records) {
            fout << record.bookingID << "," << record.user << "," << record.userPhone << ","
                 << record.source << "," << record.destination << "," << record.vehicleType << ","
                 << record.fare << "," << record.driverName << "," << record.driverPhone << ","
                 << record.timestamp << "," << (record.rating > 0 ? to_string(record.rating) : "") << "\n";
        }
        fout.close();
    }
    void logRide(const string& bookingID, const string& user, const string& phone,
                 const string& src, const string& dest, const string& vehicle,
                 float fare, const Driver& driver) {
        time_t now = time(0);
        string dt = ctime(&now); dt.pop_back();
        records.push_back({bookingID, user, phone, src, dest, vehicle, fare, 
                          driver.name, driver.phone, dt, 0.0f});
        saveHistory();
    }
    size_t getRideCount() const { return records.size(); }
};

class RideBookingSystem {
private:
    Graph cityMap;
    DriverManager driverManager;
    UserManager userManager;
    RideHistory rideHistory;
    RatingSystem ratingSystem;
public:
    RideBookingSystem() {
        loadCityMap();
        ratingSystem.loadRatings();
        rideHistory.loadHistory();
    }
    void loadCityMap() {
        ifstream file(DATA_PATH);
        if (!file.is_open()) {
            cerr << "Warning: Could not open " << DATA_PATH << ". Using default edges.\n";
            cityMap.addEdge("Saket", "INA", 7.54);
            cityMap.addEdge("Saket", "Lajpat Nagar", 5.2);
            cityMap.addEdge("Lajpat Nagar", "INA", 4.0);
            cityMap.addEdge("Saket", "Connaught Place", 10.5);
            cityMap.addEdge("Saket", "RK Puram", 4.8);
            cityMap.addEdge("Saket", "Jasola", 6.0);
        } else {
            string line; getline(file, line);
            while (getline(file, line)) {
                stringstream ss(line);
                string from, to, distStr;
                getline(ss, from, ','); getline(ss, to, ','); getline(ss, distStr, ',');
                try { 
                    cityMap.addEdge(from, to, stof(distStr)); 
                    cout << "Added edge: " << from << " -> " << to << " (" << distStr << " km)\n";
                } catch (...) { continue; }
            }
            file.close();
        }
        cityMap.addLocation("Connaught Place", 28.6333, 77.2167);
        cityMap.addLocation("Lajpat Nagar", 28.5675, 77.2431);
        cityMap.addLocation("RK Puram", 28.5611, 77.1747);
        cityMap.addLocation("Saket", 28.5244, 77.2069);
        cityMap.addLocation("Jasola", 28.5422, 77.2847);
        cityMap.addLocation("INA", 28.5833, 77.2167);
        if (cityMap.locationExists("Saket")) {
            cout << "Saket is in the graph.\n";
        } else {
            cout << "Error: Saket not found in graph!\n";
            cityMap.addEdge("Saket", "INA", 7.54);
        }
    }
    string generateOTP() {
        random_device rd; mt19937 gen(rd());
        return to_string(uniform_int_distribution<>(1000, 9999)(gen));
    }
    string generateBookingID() {
        random_device rd; mt19937 gen(rd());
        return "UB" + to_string(uniform_int_distribution<>(10000, 99999)(gen));
    }
    void bookRide() {
        cout << "\n=== Book a Ride ===\n";
        string username, phone;
        cout << "Enter your name: "; getline(cin, username);
        while (true) {
            cout << "Phone number (10 digits): "; getline(cin, phone);
            if (phone.length() == 10 && all_of(phone.begin(), phone.end(), ::isdigit)) break;
            cout << "Invalid number! Try again.\n";
        }
        if (!userManager.userExists(phone)) {
            cout << "\nCreating new account...\n";
            userManager.createUser(username, phone);
        }
        UserAccount* user = userManager.getUser(phone);
        if (!user) { cout << "Error accessing account.\n"; return; }
        user->displayProfile();
        string source, destination;
        cout << "\nEnter your current location: "; getline(cin, source);
        cout << "Enter your destination: "; getline(cin, destination);
        if (!cityMap.locationExists(source) || !cityMap.locationExists(destination)) {
            cout << "Location not found in database.\n"; return; }
        cout << "\nSelect vehicle type:\n1. 2-wheeler (RS 10/km)\n2. 4-seater car (RS 15/km)\n3. 7-seater SUV (RS 20/km)\nEnter choice: ";
        int vehicleChoice; cin >> vehicleChoice; cin.ignore();
        string vehicle = vehicleChoice == 1 ? "2-wheeler" : vehicleChoice == 2 ? "4-seater" : vehicleChoice == 3 ? "7-seater" : "";
        if (vehicle.empty()) { cout << "Invalid choice. Booking cancelled.\n"; return; }
        auto [distance, path] = cityMap.shortestPath(source, destination);
        if (path.empty()) { cout << "No route found.\n"; return; }
        float baseRate = vehicle == "2-wheeler" ? 10.0 : vehicle == "4-seater" ? 15.0 : 20.0;
        float fare = baseRate * distance;
        time_t now = time(0); tm* ltm = localtime(&now);
        if ((ltm->tm_hour >= 17 && ltm->tm_hour <= 20) || (ltm->tm_hour >= 8 && ltm->tm_hour <= 10)) {
            fare *= 1.25f; cout << "\nNote: Surge pricing (1.25x) applied due to peak hours.\n";
        }
        cout << fixed << setprecision(2);
        cout << "\n=== Ride Details ===\nDistance: " << distance << " km\nEstimated Fare: RS " << fare << "\nRoute: ";
        for (size_t i = 0; i < min(path.size(), size_t(5)); i++) {
            cout << path[i] << (i < min(path.size(), size_t(5)) - 1 ? " -> " : "");
        }
        if (path.size() > 5) cout << " -> ... -> " << path.back();
        cout << "\n";
        vector<Driver> allDrivers = driverManager.getAllDrivers();
        vector<pair<Driver, int>> availableDrivers;
        for (const auto& driver : allDrivers) {
            if (driver.vehicleType == vehicle && driver.available) {
                auto [driverDist, _] = cityMap.shortestPath(driver.location, source);
                if (driverDist != INF) {
                    int eta = max(1, static_cast<int>(driverDist * 3));
                    availableDrivers.emplace_back(driver, eta);
                }
            }
        }
        if (availableDrivers.empty()) {
            cout << "No drivers available. Initializing default drivers...\n";
            driverManager.loadDrivers();
            allDrivers = driverManager.getAllDrivers();
            for (const auto& driver : allDrivers) {
                if (driver.vehicleType == vehicle && driver.available) {
                    auto [driverDist, _] = cityMap.shortestPath(driver.location, source);
                    if (driverDist != INF) {
                        int eta = max(1, static_cast<int>(driverDist * 3));
                        availableDrivers.emplace_back(driver, eta);
                    }
                }
            }
            if (availableDrivers.empty()) {
                cout << "No drivers available for " << vehicle << ". Would you like to try another vehicle type? (y/n): ";
                char retry; cin >> retry; cin.ignore();
                if (tolower(retry) == 'y') {
                    cout <<("\nSelect vehicle type:\n1. 2-wheeler (RS 10/km)\nstartxref2. 4-seater car (RS 15/km)\n3. 7-seater SUV (RS 20/km)\nEnter choice: ");
                    int newChoice; cin >> newChoice; cin.ignore();
                    vehicle = newChoice == 1 ? "2-wheeler" : newChoice == 2 ? "4-seater" : newChoice == 3 ? "7-seater" : "";
                    baseRate = vehicle == "2-wheeler" ? 10.0 : vehicle == "4-seater" ? 15.0 : 20.0;
                    fare = baseRate * distance;
                    if ((ltm->tm_hour >= 17 && ltm->tm_hour <= 20) || (ltm->tm_hour >= 8 && ltm->tm_hour <= 10)) {
                        fare *= 1.25f;
                    }
                    if (!vehicle.empty()) {
                        for (const auto& driver : allDrivers) {
                            if (driver.vehicleType == vehicle && driver.available) {
                                auto [driverDist, _] = cityMap.shortestPath(driver.location, source);
                                if (driverDist != INF) {
                                    int eta = max(1, static_cast<int>(driverDist * 3));
                                    availableDrivers.emplace_back(driver, eta);
                                }
                            }
                        }
                    }
                }
                if (availableDrivers.empty()) {
                    cout << "Still no drivers available. Please try again later.\n";
                    return;
                }
            }
        }
        sort(availableDrivers.begin(), availableDrivers.end(), [](auto& a, auto& b) { return a.second < b.second; });
        cout << "\nAvailable Drivers:\n";
        for (size_t i = 0; i < min(size_t(3), availableDrivers.size()); i++) {
            auto [driver, eta] = availableDrivers[i];
            cout << i+1 << ". " << driver.name << " (" << driver.vehicleType << ") - ETA: " << eta << " mins - ";
            ratingSystem.displayDriverRating(driver.phone);
        }
        cout << "\nConfirm booking? (y/n): ";
        char confirm; cin >> confirm; cin.ignore();
        if (tolower(confirm) != 'y') { cout << "Booking cancelled.\n"; return; }
        auto [driver, eta] = availableDrivers[0];
        for (auto& d : driverManager.getAllDrivers()) {
            if (d.phone == driver.phone) { d.available = false; break; }
        }
        driverManager.saveDrivers();
        cout << "\n=== Payment ===\nAmount: RS " << fare << "\nPayment method: " << user->getPaymentMethod() << "\n";
        bool paymentSuccess = false;
        if (user->getPaymentMethod() == "Wallet") {
            if (user->getBalance() >= fare) {
                paymentSuccess = user->makePayment(fare);
                if (paymentSuccess) cout << "Payment successful from wallet.\n";
            } else {
                cout << "Insufficient wallet balance (RS " << user->getBalance() << "). Please pay RS " << fare << " in cash.\n";
                paymentSuccess = true; // Assume cash payment is successful
            }
        } else {
            cout << "Please pay RS " << fare << " in cash.\n";
            paymentSuccess = true; // Cash payment assumed successful
        }
        if (!paymentSuccess) {
            cout << "Payment failed. Booking cancelled.\n";
            for (auto& d : driverManager.getAllDrivers()) {
                if (d.phone == driver.phone) { d.available = true; break; }
            }
            driverManager.saveDrivers();
            return;
        }
        string otp = generateOTP(), bookingID = generateBookingID();
        cout << "\n=== Booking Confirmed ===\nBooking ID: " << bookingID 
             << "\nDriver: " << driver.name << " (" << driver.phone << ")\nVehicle: " 
             << driver.vehicleType << "\nETA: " << eta << " minutes\nOTP: " << otp << "\n";
        string inputOTP;
        cout << "Driver, enter OTP: "; cin >> inputOTP; cin.ignore();
        if (inputOTP != otp) {
            cout << "Invalid OTP. Ride cancelled.\n";
            if (user->getPaymentMethod() == "Wallet" && paymentSuccess) {
                user->addToWallet(fare); cout << "Amount refunded to wallet.\n";
            }
            for (auto& d : driverManager.getAllDrivers()) {
                if (d.phone == driver.phone) { d.available = true; break; }
            }
            driverManager.saveDrivers();
            return;
        }
        cout << "\nDriver is on the way...\n";
        for (int i = eta; i > 0; --i) {
            if (i % 5 == 0 || i <= 3) cout << "Arriving in " << i << " minute(s)...\n";
            this_thread::sleep_for(chrono::milliseconds(100));
        }
        cout << "\nYou have reached your destination!\nTotal Fare: RS " << fare << "\n";
        rideHistory.logRide(bookingID, username, phone, source, destination, vehicle, fare, driver);
        for (auto& d : driverManager.getAllDrivers()) {
            if (d.phone == driver.phone) { d.available = true; break; }
        }
        driverManager.saveDrivers();
        cout << "\n=== Rate Your Ride ===\nEnter rating (1-5 stars): ";
        float rating; cin >> rating; cin.ignore();
        if (rating >= 1 && rating <= 5) {
            rideHistory.addRating(bookingID, rating, ratingSystem);
            ratingSystem.saveRatings();
        } else cout << "Invalid rating. Rate later from history.\n";
        cout << "\nThank you for using our service!\n";
    }
    void manageUserAccount() {
        cout << "\n=== Account Management ===\n";
        string phone;
        cout << "Enter your phone number: "; getline(cin, phone);
        UserAccount* user = userManager.getUser(phone);
        if (!user) { cout << "Account not found.\n"; return; }
        user->displayProfile();
        cout << "\n1. Add money to wallet\n2. Change payment method\n3. Back\nEnter choice: ";
        int choice; cin >> choice; cin.ignore();
        if (choice == 1) {
            cout << "Enter amount to add: RS ";
            float amount; cin >> amount; cin.ignore();
            if (amount > 0) {
                user->addToWallet(amount);
                userManager.saveUsers();
                cout << "Wallet updated. New balance: RS " << user->getBalance() << "\n";
            } else cout << "Invalid amount.\n";
        } else if (choice == 2) {
            string method;
            while (true) {
                cout << "Enter new payment method (Cash/Wallet): "; getline(cin, method);
                if (normalize(method) == "cash" || normalize(method) == "wallet") break;
                cout << "Invalid payment method! Please choose Cash or Wallet.\n";
            }
            method = normalize(method) == "cash" ? "Cash" : "Wallet";
            user->updatePaymentMethod(method);
            userManager.saveUsers();
            cout << "Payment method updated.\n";
        }
    }
    void viewRideHistory() {
        cout << "\n=== Ride History ===\n";
        string phone;
        cout << "Enter your phone number: "; getline(cin, phone);
        rideHistory.displayUserHistory(phone);
    }
    void driverMenu() {
        cout << "\n=== Driver Menu ===\n1. Register\n2. Toggle Availability\n3. Back\nEnter choice: ";
        int choice; cin >> choice; cin.ignore();
        if (choice == 1) driverManager.registerNewDriver();
        else if (choice == 2) {
            string phone;
            cout << "Enter your phone number: "; getline(cin, phone);
            driverManager.toggleDriverAvailability(phone);
        }
    }
    void adminMenu() {
        cout << "\n=== Admin Menu ===\n";
        cout << "Total Users: " << userManager.getUserCount() << "\n";
        cout << "Total Drivers: " << driverManager.getDriverCount() << "\n";
        cout << "Total Rides: " << rideHistory.getRideCount() << "\n";
    }
};

int main() {
    RideBookingSystem rbs;
    while (true) {
        cout << "\n=== Ride Booking System ===\n1. Book a Ride\n2. Manage Account\n3. View Ride History\n4. Driver Menu\n5. Admin Menu\n6. Exit\nEnter choice: ";
        int choice; cin >> choice; cin.ignore();
        switch (choice) {
            case 1: rbs.bookRide(); break;
            case 2: rbs.manageUserAccount(); break;
            case 3: rbs.viewRideHistory(); break;
            case 4: rbs.driverMenu(); break;
            case 5: rbs.adminMenu(); break;
            case 6: cout << "Thank you. Goodbye!\n"; return 0;
            default: cout << "Invalid choice.\n";
        }
    }
}