#include <iostream>
#include <mutex>
#include <thread>
#include <vector>

/* useless function. used to easily find address of breakpoint. */
static int debug_me() { return 0; }

/* class to represent the bank account. */
class account {
  public:
    account() : _access(), _balance(0) { }
    
    account(int b) : _access(), _balance(b) { }
    
    /* atomically returns the balance of the account. */
    int balance() { 
        int b;
        _access.lock();
        b = _balance;
        _access.unlock();
        return b; 
    }
    
    /* atomically decrease the balance of the account. */
    int decrease(int d) {
        _access.lock();
        _balance -= d;
        _access.unlock();
        return _balance;
    }
    
    /* execute charge on an account. contains atomicity violation.  */
    static void charge_executor(account* acc, int amt) {
        if (acc->balance() < amt) {
            std::cout << "alert, account cannot afford charge for " << amt << "!" << std::endl;
            return;
        }
        
        // create an easy to spot debug point. 
        volatile int i = debug_me();
        
        acc->decrease(amt);
    }
    
  private:
    std::mutex _access;
    int _balance;
};

int main(int argc, char** argv) {
    account acc(70);
    std::vector<std::thread> charges;
    
    // basic args check.
    if (argc < 2) {
        std::clog << argv[0] << ": error, must provide a list of charges to be applied." << std::endl;
        return -1;
    }
    
    // spawn threads to execute charges.
    for (int i = 1; i < argc; ++i) {
        int amt;
        
        // convert charges to integers and catch potential exceptions.
        try {
            amt = std::stoi(argv[i]);
        } catch (std::invalid_argument& e) {
            std::clog << argv[0] << ": unknown charge, " << argv[i] << std::endl;
            continue;
        } catch (std::out_of_range& e) {
            std::clog << argv[0] << ": bad charge, " << argv[i] << std::endl;
            continue;
        }
        
        // create charge executor.
        charges.push_back(std::thread(account::charge_executor, &acc, amt));
    }
    
    // join threads.
    for (auto it = charges.begin(); it != charges.end(); ++it) {
        it->join();
    }
    
    // print ending balance.
    std::cout << "ending balance is: " << acc.balance() << std::endl;
    
    return 0;
}



