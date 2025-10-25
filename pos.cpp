#include <iostream>
#include <string>
#include <memory>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <vector>
#include <regex>
#include <map>
#include <windows.h>
#include <unistd.h>
#include "nlohmann/json.hpp"

#ifdef __has_include
#       if __has_include(<curl/curl.h>)
#        include <curl/curl.h>
#        define HAVE_CURL 1
#   endif
#endif


using namespace std;
using json = nlohmann::json;



struct PaymentDetails{
    int paymentId;
    string paymentMethod;
    double amount;
    string transactionTime;
    string status; // Pending , completed or failed
    string authorizationCode;
};

class PaymentProcessor{
    private:
        static int nextPaymentId;
        PaymentDetails*currentPayment;//raw pointer to current payment
        vector<PaymentDetails> transactionHistory;

    public:
        //constructor
        PaymentProcessor();
        ~PaymentProcessor();
        
        //payment methods
        bool processMobilePayment(double amount, const string& mobileNumber);
        double processCashPayment(double amount, double tendered);
        bool processCardPayment(double amount, string cardNumber, string expiry, string cvv, string cardType);
        

        //utility functions
        void saveTransaction();
        void viewTransactionHistory();
        void displayPaymentReceipt();
        double calculateChange(double amount, double tendered);
        string generateAuthorizationCode();
        string transactionTime();
        bool validateCard(string cardNumber, string expiry, string cvv);



};


int PaymentProcessor::nextPaymentId = 1001;

PaymentProcessor::PaymentProcessor(){
    int nextPaymentId =1001;
    currentPayment=nullptr;
}
PaymentProcessor::~PaymentProcessor(){
    // clean up  dynamic memory
    if (currentPayment != nullptr){
        delete currentPayment;
        currentPayment= nullptr;
    }
}
string PaymentProcessor:: transactionTime(){
    time_t now = time(nullptr);
    tm local;
#ifdef _WIN32
     localtime_s(&local, &now);
#else
     localtime_r(&now, &local);
#endif 
     stringstream ss;
     ss << put_time(&local, "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

//store transactions
void PaymentProcessor::saveTransaction(){
    if(currentPayment != nullptr){
        transactionHistory.push_back(*currentPayment);
    }

    
}

//view the transactions
void PaymentProcessor::viewTransactionHistory(){
    cout <<"\n========== TRANSACTION HISTORY========\n";
    
    if (transactionHistory.empty()){
        cout << "No Transactions yet.\n";
        return;
    }
    for (const auto& t: transactionHistory){
        cout <<"Payment ID: " <<t.paymentId << endl;
        cout <<"Method: "<< t.paymentMethod<< endl;
        cout << "Amount: Kes " << fixed << setprecision(2) << t.amount<< endl;
        cout << "Status: " << t.status << endl;
        cout << "Time: " << t.transactionTime << endl;
        cout << "---------------------------------\n";
    }

}

double PaymentProcessor::calculateChange(double amount, double tendered){
    double change = amount - tendered;
    if (change < 0){
        cout << "Insufficient cash. Customer still owes: " << -change << endl;

    }else {
        cout << "Change to return: "<< change << endl;
    }
    return change;
}

string PaymentProcessor::generateAuthorizationCode() {
    stringstream code;
    code << "MP" << rand() % 900000 + 100000; // e.g. MP736492
    return code.str();
}
bool PaymentProcessor::validateCard(string cardNumber, string expiry, string cvv){
    
    //erase spaces
    cardNumber.erase(remove(cardNumber.begin(),cardNumber.end(),' '),cardNumber.end());

    //check if its digits only
    if (!regex_match(cardNumber, regex("^[0-9]{13,19}$"))){
        cout << "❌ Invalid card number format."<< endl;
        return false;
    }

    int sum =0;
    bool alternate = false;
    for (int i=cardNumber.length()-1; i>=0;i--){
        int n = cardNumber[i]-'0';
        if (alternate){
            n*=2;
            if (n>9) n-=9;
        }
         sum  +=n;
        alternate = !alternate;
    }
    if (sum % 10 !=0){
        cout << "❌ Card number not Valid" << endl;
        return false;
    }
    //validate expiry
    if (!regex_match(expiry, regex("^(0[1-9]|1[0-2])/[0-9]{2}$"))){
        cout << "❌Invalid expiry format."<< endl;
        return false;

    }
    //validate cvv
    if (!regex_match(cvv, regex("^[0-9]{3}"))){
        cout << "❌Invalid CVV" << endl;
        return false;
    }

    return true;


}

double PaymentProcessor::processCashPayment(double amount, double tendered){

    double change = calculateChange(amount, tendered);
    currentPayment = new PaymentDetails {
        nextPaymentId++,
        "Cash",
        tendered,
        transactionTime(),
        (change>0) ? "Success":"Failed",
        "N/A"
    };
    cout << fixed << setprecision(2);
    displayPaymentReceipt();
    saveTransaction();
    delete currentPayment;
    return change;
}

bool PaymentProcessor::processMobilePayment(double amount, const string& mobileNumber){
    cout << "\n 📱 Intiating M-pesa payment..." << endl;
    cout << "Sending kes" << amount << " to " << mobileNumber <<endl;
    
    //simulate delay

    cout <<"Processing";
    for (int i= 0;i <= 3; ++i){
        cout << ".";
        cout.flush();
#ifdef _WIN32
        Sleep(500);
#else
        usleep(50000);
#endif
    }
    cout << endl;

#ifdef HAVE_CURL
        
    CURL *curl;
    CURLcode res;
    curl = curl_easy_init();
    if(!curl){
        cerr << "Failed to intialize cURL" << endl;
        return false;
    }
    struct curl_slist *headers = nullptr;
    headers = curl_slist_append(headers, "Authorization: Basic U3o2bWl5YTU2UlVEeEtQeXJzRzR5aUtHVGdkcXh6Q25wd3BaNkx0ckJCTVBtMEZ1OnQ3TTN3UmUxU1l4eExEWG1DZVdQeFlBZlJkR0dQU0pDcVZPdlVHZlVuN1hEOVVxd2xZckpicGQxNW90YmRncEE=");
    
    string responseString;
    string url = "https://sandbox.safaricom.co.ke/oauth/v1/generate?grant_type=client_credentials";
    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "GET");
    curl_easy_setopt(curl, CURLOPT_URL, "https://sandbox.safaricom.co.ke/oauth/v1/generate?grant_type=client_credentials");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);

    //JSON data
    json requestData = {
        {"amount", amount},
        {"currency", "KES"}, 
        {"phoneNumber", mobileNumber},
        {"description", "POS Payment"}
    };

    //intialize cURL
    curl = curl_easy_init();
    if(curl){
        struct curl_slist*headers = nullptr;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        headers = curl_slist_append(headers, "Authorization: <Your apikey>");

        //setting up the request
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        string jsonData = requestData.dump();
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, jsonData.c_str());

        //perform the request
        res = curl_easy_perform(curl);

        //check error
        if (res != CURLE_OK){
            cerr << "cURL ERROR: " << curl_easy_strerror(res) << endl;
            curl_easy_cleanup(curl);
            return false;

        }
        //clean up
    curl_easy_cleanup(curl);
#endif

    currentPayment = new PaymentDetails{
        nextPaymentId++,
        "M-pesa",
        amount,
        transactionTime(),
        "Completed",
        generateAuthorizationCode()
    };

    displayPaymentReceipt();
    saveTransaction();
    delete currentPayment;
    return true;


}
bool PaymentProcessor::processCardPayment(double amount, string cardNumber, string expiry, string cvv, string cardType){
    if (!validateCard(cardNumber,expiry,cvv)){
        cout << "❌Card Validation Failed.Payment Canceled."<<endl;
        return false;

    }
    cout << "\n💳Processing Card Payment...."<< endl;
    Sleep(1000);
    bool success = true;
    currentPayment = new PaymentDetails{
        nextPaymentId++,
        "Card",
        amount,
        transactionTime(),
        success ? "Completed" : "Failed",
        success  ? generateAuthorizationCode() : "N/A"
    };
    displayPaymentReceipt();
    saveTransaction();
    delete currentPayment;
    return success;
}     
// ---------------- RECEIPT DISPLAY ----------------
void PaymentProcessor::displayPaymentReceipt() {
    cout << "\n🧾 --------- PAYMENT RECEIPT ---------" << endl;
    cout << "Payment ID: " << currentPayment->paymentId << endl;
    cout << "Method: " << currentPayment->paymentMethod << endl;
    cout << "Amount: KES " << fixed << setprecision(2) << currentPayment->amount << endl;
    cout << "Status: " << currentPayment->status << endl;
    cout << "Authorization: " << currentPayment->authorizationCode << endl;
    cout << "Time: " << currentPayment->transactionTime << endl;
    cout << "-------------------------------------\n" << endl;
}


int main() {
    PaymentProcessor p;
    double amount = 550.0;
    double tendered = 300.0;
    p.processCashPayment(amount, tendered);

    PaymentProcessor p2;
    p2.processCardPayment(1200, "4539 4512 0398 7356", "08/27", "123", "VISA");

    PaymentProcessor p3;
    p3.processMobilePayment(200.0, "254727951049");

    // ✅ Show all transactions done in this session
    p3.viewTransactionHistory();

    return 0;
}




