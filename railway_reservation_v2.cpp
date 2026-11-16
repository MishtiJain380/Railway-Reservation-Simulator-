// ============================================================
//  Railway Reservation Simulator  -  Version 2 (class-wise seats)
//
//  Every train now has separate seats, fares and waiting lists for:
//     1st AC (1A), 2nd AC (2A), 3rd AC (3A), Sleeper (SL), Non-Sleeper (NS)
//  A class can have 0 seats on a train (it is then shown as "no seats").
//
//  Concepts: classes, inheritance, polymorphism, operator overloading,
//            queue (one waiting list per class), file handling.
// ============================================================

#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <string>
#include <vector>
#include <queue>
#include <memory>
#include <limits>
#include <cstdlib>
#include <cctype>

using namespace std;

// ------------------------------------------------------------
//  Seat classes (single place that defines names, codes, fares)
// ------------------------------------------------------------
enum SeatClass { FIRST_AC = 0, SECOND_AC, THIRD_AC, SLEEPER, NON_SLEEPER, NUM_CLASSES };

const char*  CLASS_NAMES[NUM_CLASSES]  = {"1st AC", "2nd AC", "3rd AC", "Sleeper", "Non-Sleeper"};
const char*  CLASS_CODES[NUM_CLASSES]  = {"1A", "2A", "3A", "SL", "NS"};
// Fare = train base fare (the Sleeper fare) x factor
const double CLASS_FACTOR[NUM_CLASSES] = {4.0, 3.0, 2.0, 1.0, 0.5};

int classFromCode(const string& code) {
    for (int i = 0; i < NUM_CLASSES; i++)
        if (code == CLASS_CODES[i]) return i;
    return -1;
}

// ------------------------------------------------------------
//  Helpers
// ------------------------------------------------------------
vector<string> split(const string& line, char delim) {
    vector<string> parts;
    string item;
    string clean = line;
    // Windows files end lines with \r\n; remove the stray \r
    if (!clean.empty() && clean.back() == '\r') clean.pop_back();
    stringstream ss(clean);
    while (getline(ss, item, delim)) parts.push_back(item);
    return parts;
}

string readLine(const string& prompt) {
    string s;
    do {
        cout << prompt;
        getline(cin, s);
        if (!cin) exit(0);           // input closed
    } while (s.empty());
    // '|' is our file delimiter, so replace it
    for (char& c : s) if (c == '|') c = '/';
    return s;
}

int readInt(const string& prompt, int lo, int hi) {
    while (true) {
        cout << prompt;
        string s;
        getline(cin, s);
        if (!cin) exit(0);
        try {
            size_t pos;
            int v = stoi(s, &pos);
            if (pos == s.size() && v >= lo && v <= hi) return v;
        } catch (...) {}
        cout << "  Invalid input. Enter a number between " << lo << " and " << hi << ".\n";
    }
}

// ------------------------------------------------------------
//  Person -> Passenger   (inheritance)
// ------------------------------------------------------------
class Person {
protected:
    string name;
    int age;
    char gender;   // M / F / O
public:
    Person(const string& n = "", int a = 0, char g = 'O') : name(n), age(a), gender(g) {}
    virtual ~Person() {}
    string getName() const { return name; }
    int getAge() const { return age; }
    char getGender() const { return gender; }
};

class Passenger : public Person {
public:
    Passenger(const string& n = "", int a = 0, char g = 'O') : Person(n, a, g) {}
};

// ------------------------------------------------------------
//  Ticket (abstract) and one subclass per seat class (polymorphism)
// ------------------------------------------------------------
enum class Status { CONFIRMED, WAITING, CANCELLED };

string statusToString(Status s) {
    switch (s) {
        case Status::CONFIRMED: return "CONFIRMED";
        case Status::WAITING:   return "WAITING";
        default:                return "CANCELLED";
    }
}

Status stringToStatus(const string& s) {
    if (s == "CONFIRMED") return Status::CONFIRMED;
    if (s == "WAITING")   return Status::WAITING;
    return Status::CANCELLED;
}

class Ticket {
protected:
    int pnr;
    Passenger passenger;
    int trainNo;
    int seatNo;        // seat number inside its class; 0 when not allotted
    double baseFare;
    Status status;
public:
    Ticket(int p, const Passenger& ps, int t, double fare)
        : pnr(p), passenger(ps), trainNo(t), seatNo(0), baseFare(fare), status(Status::WAITING) {}
    virtual ~Ticket() {}

    virtual double calculateFare() const = 0;   // each class has its own fare rule
    virtual int getClassIndex() const = 0;      // which SeatClass this ticket belongs to

    string getClassName() const { return CLASS_NAMES[getClassIndex()]; }
    string getClassCode() const { return CLASS_CODES[getClassIndex()]; }

    int getPNR() const { return pnr; }
    int getTrainNo() const { return trainNo; }
    int getSeatNo() const { return seatNo; }
    Status getStatus() const { return status; }
    const Passenger& getPassenger() const { return passenger; }

    void setSeat(int s) { seatNo = s; }
    void setStatus(Status s) { status = s; }

    // Operator overloading: compare tickets by PNR
    bool operator==(const Ticket& other) const { return pnr == other.pnr; }

    // Operator overloading: print a ticket
    friend ostream& operator<<(ostream& os, const Ticket& t) {
        os << "\n  ------------------------------------\n"
           << "  PNR        : " << t.pnr << "\n"
           << "  Passenger  : " << t.passenger.getName()
           << " (" << t.passenger.getAge() << ", " << t.passenger.getGender() << ")\n"
           << "  Train No   : " << t.trainNo << "\n"
           << "  Class      : " << t.getClassName() << " (" << t.getClassCode() << ")\n"
           << "  Seat       : " << (t.seatNo ? to_string(t.seatNo) : string("-")) << "\n"
           << "  Fare       : Rs. " << fixed << setprecision(2) << t.calculateFare() << "\n"
           << "  Status     : " << statusToString(t.status) << "\n"
           << "  ------------------------------------\n";
        return os;
    }
};

class FirstACTicket : public Ticket {
public:
    using Ticket::Ticket;
    double calculateFare() const override { return baseFare * CLASS_FACTOR[FIRST_AC]; }
    int getClassIndex() const override { return FIRST_AC; }
};

class SecondACTicket : public Ticket {
public:
    using Ticket::Ticket;
    double calculateFare() const override { return baseFare * CLASS_FACTOR[SECOND_AC]; }
    int getClassIndex() const override { return SECOND_AC; }
};

class ThirdACTicket : public Ticket {
public:
    using Ticket::Ticket;
    double calculateFare() const override { return baseFare * CLASS_FACTOR[THIRD_AC]; }
    int getClassIndex() const override { return THIRD_AC; }
};

class SleeperTicket : public Ticket {
public:
    using Ticket::Ticket;
    double calculateFare() const override { return baseFare * CLASS_FACTOR[SLEEPER]; }
    int getClassIndex() const override { return SLEEPER; }
};

class NonSleeperTicket : public Ticket {
public:
    using Ticket::Ticket;
    double calculateFare() const override { return baseFare * CLASS_FACTOR[NON_SLEEPER]; }
    int getClassIndex() const override { return NON_SLEEPER; }
};

// Factory: builds the right ticket subclass for a seat class
unique_ptr<Ticket> makeTicket(int cls, int pnr, const Passenger& p, int trainNo, double fare) {
    switch (cls) {
        case FIRST_AC:  return unique_ptr<Ticket>(new FirstACTicket(pnr, p, trainNo, fare));
        case SECOND_AC: return unique_ptr<Ticket>(new SecondACTicket(pnr, p, trainNo, fare));
        case THIRD_AC:  return unique_ptr<Ticket>(new ThirdACTicket(pnr, p, trainNo, fare));
        case NON_SLEEPER: return unique_ptr<Ticket>(new NonSleeperTicket(pnr, p, trainNo, fare));
        default:        return unique_ptr<Ticket>(new SleeperTicket(pnr, p, trainNo, fare));
    }
}

// ------------------------------------------------------------
//  Train  (separate seat map + waiting queue for every class)
// ------------------------------------------------------------
class Train {
    struct Inventory {
        int total;
        vector<bool> taken;     // index 0 => seat 1
        queue<int> waiting;     // PNRs waiting for a seat in this class
    };

    int number;
    string name, source, destination;
    double baseFare;
    Inventory inv[NUM_CLASSES];
public:
    Train(int no, const string& n, const string& s, const string& d,
          const vector<int>& seats, double fare)
        : number(no), name(n), source(s), destination(d), baseFare(fare) {
        for (int c = 0; c < NUM_CLASSES; c++) {
            inv[c].total = (c < (int)seats.size() && seats[c] > 0) ? seats[c] : 0;
            inv[c].taken.assign(inv[c].total, false);
        }
    }

    int getNumber() const { return number; }
    string getName() const { return name; }
    string getSource() const { return source; }
    string getDestination() const { return destination; }
    double getBaseFare() const { return baseFare; }

    int totalSeats(int c) const { return inv[c].total; }
    int availableSeats(int c) const {
        int n = 0;
        for (bool t : inv[c].taken) if (!t) n++;
        return n;
    }
    int waitingCount(int c) const { return (int)inv[c].waiting.size(); }

    // Allot the lowest free seat of a class; returns 0 if that class is full
    int allotSeat(int c) {
        for (int i = 0; i < inv[c].total; i++)
            if (!inv[c].taken[i]) { inv[c].taken[i] = true; return i + 1; }
        return 0;
    }
    void occupySeat(int c, int seat) { if (seat >= 1 && seat <= inv[c].total) inv[c].taken[seat - 1] = true; }
    void freeSeat(int c, int seat)   { if (seat >= 1 && seat <= inv[c].total) inv[c].taken[seat - 1] = false; }

    // ---- queue operations (one queue per class) ----
    void addToWaiting(int c, int pnr) { inv[c].waiting.push(pnr); }
    bool hasWaiting(int c) const { return !inv[c].waiting.empty(); }
    int  popWaiting(int c) { int p = inv[c].waiting.front(); inv[c].waiting.pop(); return p; }

    void removeFromWaiting(int c, int pnr) {    // rebuild queue without this PNR
        queue<int> fresh;
        while (!inv[c].waiting.empty()) {
            int p = inv[c].waiting.front(); inv[c].waiting.pop();
            if (p != pnr) fresh.push(p);
        }
        inv[c].waiting = fresh;
    }

    vector<int> waitingSnapshot(int c) const {  // copy so the real queue is untouched
        vector<int> v;
        queue<int> copy = inv[c].waiting;
        while (!copy.empty()) { v.push_back(copy.front()); copy.pop(); }
        return v;
    }

    // Prints the class-wise availability table of this train
    void printAvailability() const {
        cout << "\n  [" << number << "] " << name << "  (" << source << " to " << destination << ")\n";
        cout << "  " << left << setw(20) << "Class" << setw(12) << "Fare (Rs)"
             << setw(8) << "Total" << setw(11) << "Available" << "Waiting\n";
        cout << "  " << string(59, '-') << "\n";
        for (int c = 0; c < NUM_CLASSES; c++) {
            string label = string(CLASS_NAMES[c]) + " (" + CLASS_CODES[c] + ")";
            cout << "  " << left << setw(20) << label;
            if (inv[c].total == 0) {
                cout << "no seats on this train\n";
                continue;
            }
            cout << fixed << setprecision(2) << setw(12) << baseFare * CLASS_FACTOR[c]
                 << setw(8) << inv[c].total << setw(11) << availableSeats(c)
                 << waitingCount(c) << "\n";
        }
        if (inv[NON_SLEEPER].total > 0)
            cout << "  Non-sleeper seats available: " << availableSeats(NON_SLEEPER)
                 << " of " << inv[NON_SLEEPER].total << "\n";
        else
            cout << "  Note: this train has no non-sleeper seats.\n";
    }

    // number|name|source|destination|1A|2A|3A|SL|NS|baseFare
    string toFileLine() const {
        string s = to_string(number) + "|" + name + "|" + source + "|" + destination;
        for (int c = 0; c < NUM_CLASSES; c++) s += "|" + to_string(inv[c].total);
        return s + "|" + to_string(baseFare);
    }
};

// ------------------------------------------------------------
//  ReservationSystem  (controller)
// ------------------------------------------------------------
class ReservationSystem {
    vector<Train> trains;
    vector<unique_ptr<Ticket>> tickets;
    int nextPNR;

    const string trainFile  = "trains.txt";
    const string ticketFile = "tickets.txt";

    Train* findTrain(int no) {
        for (auto& t : trains) if (t.getNumber() == no) return &t;
        return nullptr;
    }
    Ticket* findTicket(int pnr) {
        for (auto& t : tickets) if (t->getPNR() == pnr) return t.get();
        return nullptr;
    }

    // seats order: 1A, 2A, 3A, SL, NS
    void seedDefaultTrains() {
        trains.push_back(Train(12951, "Mumbai Rajdhani",  "Mumbai", "Delhi",     vector<int>{2, 3, 4, 0, 0}, 1200.0));
        trains.push_back(Train(12009, "Shatabdi Express", "Mumbai", "Ahmedabad", vector<int>{0, 2, 3, 0, 4},  600.0));
        trains.push_back(Train(11301, "Udyan Express",    "Mumbai", "Bengaluru", vector<int>{1, 2, 3, 4, 3},  900.0));
    }

public:
    ReservationSystem() : nextPNR(1001) {}

    // ---------------- File handling ----------------
    void loadData() {
        int skipped = 0;

        ifstream tf(trainFile);
        if (tf) {
            string line;
            while (getline(tf, line)) {
                if (line.empty() || line == "\r") continue;
                auto p = split(line, '|');
                if (p.size() != 10) { skipped++; continue; }   // old 6-field format or damaged line
                try {
                    vector<int> seats;
                    for (int c = 0; c < NUM_CLASSES; c++) seats.push_back(stoi(p[4 + c]));
                    trains.push_back(Train(stoi(p[0]), p[1], p[2], p[3], seats, stod(p[9])));
                } catch (...) { skipped++; }
            }
        }
        if (trains.empty()) seedDefaultTrains();

        ifstream kf(ticketFile);
        if (kf) {
            string line;
            while (getline(kf, line)) {
                if (line.empty() || line == "\r") continue;
                auto p = split(line, '|');
                // pnr|classCode|name|age|gender|trainNo|seat|status
                if (p.size() != 8) { skipped++; continue; }
                try {
                    int pnr = stoi(p[0]);
                    int cls = classFromCode(p[1]);              // old files used 1/2, so they are skipped
                    if (cls < 0) { skipped++; continue; }
                    Passenger ps(p[2], stoi(p[3]), p[4].empty() ? 'O' : p[4][0]);
                    int trainNo = stoi(p[5]);
                    Train* tr = findTrain(trainNo);
                    if (!tr || tr->totalSeats(cls) == 0) { skipped++; continue; }

                    auto t = makeTicket(cls, pnr, ps, trainNo, tr->getBaseFare());
                    t->setSeat(stoi(p[6]));
                    t->setStatus(stringToStatus(p[7]));

                    if (t->getStatus() == Status::CONFIRMED) tr->occupySeat(cls, t->getSeatNo());
                    else if (t->getStatus() == Status::WAITING) tr->addToWaiting(cls, pnr);

                    if (pnr >= nextPNR) nextPNR = pnr + 1;
                    tickets.push_back(move(t));
                } catch (...) { skipped++; }
            }
        }
        if (skipped > 0)
            cout << "\n  Note: " << skipped << " line(s) in the data files were skipped "
                 << "(old format or damaged).\n";
    }

    void saveData() {
        ofstream tf(trainFile);
        for (auto& t : trains) tf << t.toFileLine() << "\n";

        ofstream kf(ticketFile);
        for (auto& t : tickets) {
            kf << t->getPNR() << "|" << t->getClassCode() << "|"
               << t->getPassenger().getName() << "|" << t->getPassenger().getAge() << "|"
               << t->getPassenger().getGender() << "|" << t->getTrainNo() << "|"
               << t->getSeatNo() << "|" << statusToString(t->getStatus()) << "\n";
        }
        cout << "\n  Data saved to " << trainFile << " and " << ticketFile << ".\n";
    }

    // ---------------- Menu actions ----------------
    void showTrains() {
        for (auto& t : trains) t.printAvailability();
    }

    void bookTicket() {
        showTrains();
        int trainNo = readInt("\n  Enter train number: ", 1, 999999);
        Train* tr = findTrain(trainNo);
        if (!tr) { cout << "  Train not found.\n"; return; }

        cout << "\n  Classes:  ";
        for (int c = 0; c < NUM_CLASSES; c++)
            cout << (c + 1) << " = " << CLASS_NAMES[c] << (c + 1 < NUM_CLASSES ? ",  " : "\n");
        int cls = readInt("  Choose class (1-5): ", 1, NUM_CLASSES) - 1;
        if (tr->totalSeats(cls) == 0) {
            cout << "  Sorry, train " << trainNo << " has no " << CLASS_NAMES[cls] << " seats.\n";
            return;
        }

        string name = readLine("  Passenger name: ");
        int age = readInt("  Age: ", 1, 120);
        string g = readLine("  Gender (M/F/O): ");
        char gender = (char)toupper((unsigned char)g[0]);
        if (gender != 'M' && gender != 'F') gender = 'O';

        Passenger ps(name, age, gender);
        auto t = makeTicket(cls, nextPNR++, ps, trainNo, tr->getBaseFare());

        int seat = tr->allotSeat(cls);
        if (seat) {
            t->setSeat(seat);
            t->setStatus(Status::CONFIRMED);
            cout << "\n  Ticket CONFIRMED!  (" << tr->availableSeats(cls) << " "
                 << CLASS_NAMES[cls] << " seat(s) left)";
        } else {
            t->setStatus(Status::WAITING);
            tr->addToWaiting(cls, t->getPNR());
            cout << "\n  " << CLASS_NAMES[cls] << " is full. Added to its WAITING LIST (position "
                 << tr->waitingCount(cls) << ").";
        }
        cout << *t;                       // uses overloaded <<
        tickets.push_back(move(t));
    }

    void cancelTicket() {
        int pnr = readInt("\n  Enter PNR to cancel: ", 1, 99999999);
        Ticket* t = findTicket(pnr);
        if (!t) { cout << "  PNR not found.\n"; return; }
        if (t->getStatus() == Status::CANCELLED) { cout << "  Ticket already cancelled.\n"; return; }

        Train* tr = findTrain(t->getTrainNo());
        int cls = t->getClassIndex();

        if (t->getStatus() == Status::WAITING) {
            tr->removeFromWaiting(cls, pnr);
            t->setStatus(Status::CANCELLED);
            cout << "  Waiting-list ticket cancelled.\n";
            return;
        }

        // Confirmed ticket: free the seat, then promote the first waiting passenger of the SAME class
        tr->freeSeat(cls, t->getSeatNo());
        t->setSeat(0);
        t->setStatus(Status::CANCELLED);
        cout << "  Ticket " << pnr << " cancelled.\n";

        while (tr->hasWaiting(cls)) {
            int nextPnr = tr->popWaiting(cls);
            Ticket* w = findTicket(nextPnr);
            if (w && w->getStatus() == Status::WAITING) {
                int seat = tr->allotSeat(cls);
                w->setSeat(seat);
                w->setStatus(Status::CONFIRMED);
                cout << "  Waiting ticket PNR " << nextPnr << " (" << w->getPassenger().getName()
                     << ") promoted to CONFIRMED, " << CLASS_NAMES[cls] << " seat " << seat << ".\n";
                break;
            }
        }
    }

    void checkPNR() {
        int pnr = readInt("\n  Enter PNR: ", 1, 99999999);
        Ticket* t = findTicket(pnr);
        if (!t) { cout << "  PNR not found.\n"; return; }
        cout << *t;
    }

    void showWaitingList() {
        showTrains();
        int trainNo = readInt("\n  Enter train number: ", 1, 999999);
        Train* tr = findTrain(trainNo);
        if (!tr) { cout << "  Train not found.\n"; return; }

        bool any = false;
        for (int c = 0; c < NUM_CLASSES; c++) {
            auto list = tr->waitingSnapshot(c);
            if (list.empty()) continue;
            any = true;
            cout << "\n  Waiting list for " << tr->getName() << " - "
                 << CLASS_NAMES[c] << " (" << CLASS_CODES[c] << "):\n";
            int pos = 1;
            for (int p : list) {
                Ticket* t = findTicket(p);
                if (t) cout << "   " << pos++ << ". PNR " << p << " - "
                            << t->getPassenger().getName() << "\n";
            }
        }
        if (!any) cout << "  Waiting list is empty for every class.\n";
    }
};

// ------------------------------------------------------------
//  main
// ------------------------------------------------------------
int main() {
    ReservationSystem sys;
    sys.loadData();

    cout << "\n===== RAILWAY RESERVATION SIMULATOR =====\n";
    while (true) {
        cout << "\n  1. View trains and seat availability\n"
             << "  2. Book ticket\n"
             << "  3. Cancel ticket\n"
             << "  4. Check PNR status\n"
             << "  5. Show waiting list\n"
             << "  6. Save & Exit\n";
        int choice = readInt("  Choice: ", 1, 6);
        switch (choice) {
            case 1: sys.showTrains(); break;
            case 2: sys.bookTicket(); break;
            case 3: sys.cancelTicket(); break;
            case 4: sys.checkPNR(); break;
            case 5: sys.showWaitingList(); break;
            case 6: sys.saveData(); cout << "  Goodbye!\n"; return 0;
        }
    }
}
