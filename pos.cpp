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

    public:
        //constructor
        PaymentProcessor();
        ~PaymentProcessor();
        
        //payment methods
        bool processMobilePayment(double amount, const string& mobileNumber);
        double processCashPayment(double amount, double tendered);
        bool processCardPayment(double amount, string cardNumber, string expiry, string cvv, string cardType);
        

        //utility functions
        void displayPaymentReceipt();
        double calculateChange(double amount, double tendered);
        string generateAuthorizationCode();
        string transactionTime();
        bool validateCard(string cardNumber, string expiry, string cvv);



};


int PaymentProcessor::nextPaymentId = 1001;

PaymentProcessor::PaymentProcessor(){
    nextPaymentId = 1001;
    PaymentDetails*currentPayment=nullptr;
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

string PaymentProcessor::generateAuthorizationCode() {
    stringstream code;
    code << "MP" << rand() % 900000 + 100000; // e.g. MP736492
    return code.str();
}

double PaymentProcessor::processCashPayment(double amount, double tendered){
    
    double change = amount - tendered;
    currentPayment = new PaymentDetails {
        nextPaymentId++,
        "Cash",
        amount,
        transactionTime(),
        (change >= 0) ? "Completed" : "Pending",
        "N/A"
    };
    cout << fixed << setprecision(2);
    cout << "\nCash Payment" << endl;
    cout << "Enter amount received: KEs" << amount << endl;
    cout << "Tendered: KEs"<< tendered << endl;
    

    if (change < 0){
        cout << "Insufficient cash. Customer still owes: " << -change << endl;

    }else {
        cout << "Change to return: "<< change << endl;
    }
    displayPaymentReceipt();
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
        usleep(50000)
#endif
    }
    cout << endl;

    //simulating response(as from api)
    bool success = true;
    currentPayment = new PaymentDetails {
        nextPaymentId++,
        "M-pesa",
        amount,
        transactionTime(),
        success ? "Completed" : "Failed",
        generateAuthorizationCode()
    };

    if (success){
        cout << "\n✅ M-Pesa Transaction Successful" << endl;
        cout << "Authorization Code: " << currentPayment->authorizationCode << endl;
        
    }else{
        cout << "M-pesa Transaction Failed.Try again Later! " << endl;
    }
    displayPaymentReceipt();

    #ifdef _WIN32
        system("pause");
    #else
        cout << "Press Enter to continue...";
        cin.get();
    #endif


    return success; 
}

    /*#ifdef HAVE_CURL
        
        CURL *curl;
        CURLcode res;
        curl = curl_easy_init();
        struct curl_slist *headers = NULL;
        headers = curl_slist_append(headers, "Authorization: Basic U3o2bWl5YTU2UlVEeEtQeXJzRzR5aUtHVGdkcXh6Q25wd3BaNkx0ckJCTVBtMEZ1OnQ3TTN3UmUxU1l4eExEWG1DZVdQeFlBZlJkR0dQU0pDcVZPdlVHZlVuN1hEOVVxd2xZckpicGQxNW90YmRncEE=");
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "GET");
        curl_easy_setopt(curl, CURLOPT_URL, "https://sandbox.safaricom.co.ke/oauth/v1/generate?grant_type=client_credentials");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        ​
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
        cout << "Payment request sent successfull! " << endl;
        return true;


    };*/
    
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
    double tendered = 300.0; // passed directly as parameter
    p.processCashPayment(amount, tendered);

    PaymentProcessor p2;
    p2.processMobilePayment(200.0, "254727951049");

    return 0;
}



