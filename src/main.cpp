/*
 * Title:       SMS Sender Pro
 * Version:     1.0.0
 * Author:      Paulo Muniz
 * GitHub:      https://github.com/paulomunizdev/sms-sender-pro
 * Description: A professional bulk SMS messaging tool using the Twilio API, featuring
 *             phone number validation, progress tracking, and detailed reporting.
 */

// Required header files for the application
#include <iostream>     // For input/output operations
#include <fstream>      // For file handling
#include <string>       // For string operations
#include <vector>       // For dynamic arrays
#include <curl/curl.h>  // For HTTP requests
#include <nlohmann/json.hpp>  // For JSON parsing
#include <iomanip>      // For output formatting
#include <chrono>       // For time operations
#include <thread>       // For thread operations
#include <sstream>      // For string stream operations

// Using the JSON library with an alias
using json = nlohmann::json;

/*
 * Namespace containing ANSI color codes for terminal output formatting
 * These codes are used to add colors and styling to the console output
 */
namespace Color {
    const std::string RESET   = "\033[0m";    // Resets all formatting
    const std::string RED     = "\033[31m";    // Red text
    const std::string GREEN   = "\033[32m";    // Green text
    const std::string YELLOW  = "\033[33m";    // Yellow text
    const std::string BLUE    = "\033[34m";    // Blue text
    const std::string MAGENTA = "\033[35m";    // Magenta text
    const std::string CYAN    = "\033[36m";    // Cyan text
    const std::string BOLD    = "\033[1m";     // Bold text
}

/*
 * Structure to hold Twilio configuration data
 * Contains the essential credentials needed for Twilio API authentication
 */
struct TwilioConfig {
    std::string account_sid;    // Twilio account SID
    std::string auth_token;     // Twilio authentication token
    std::string phone_number;   // Sender phone number
};

/*
 * @brief Displays the application banner in the console
 * Creates a visually appealing header using ASCII characters and colors
 */
void displayBanner() {
    std::cout << Color::CYAN << Color::BOLD << "\n"
              << "╔════════════════════════════════════════╗\n"
              << "║          Twilio SMS Sender Pro         ║\n"
              << "║             by: Paulo Muniz            ║\n"
              << "╚════════════════════════════════════════╝\n"
              << Color::RESET << "\n";
}

/*
 * @brief Displays a progress bar in the console
 * @param current Current progress value
 * @param total Total number of items to process
 */
void displayProgress(int current, int total) {
    float percentage = (float)current / total * 100;
    int barWidth = 30;
    int pos = barWidth * current / total;

    std::cout << "[";
    for (int i = 0; i < barWidth; ++i) {
        if (i < pos) std::cout << Color::GREEN << "█" << Color::RESET;
        else std::cout << " ";
    }
    std::cout << "] " << std::fixed << std::setprecision(1) << percentage << "%" << "\r";
    std::cout.flush();
}

/*
 * @brief Reads Twilio configuration from a file
 * @return TwilioConfig structure containing the configuration
 * @throws std::runtime_error if configuration file is missing or invalid
 */
TwilioConfig readConfig() {
    TwilioConfig config;
    std::ifstream config_file("twilio_config.txt");
    
    if (!config_file.is_open()) {
        throw std::runtime_error(
            "Error: twilio_config.txt not found!\n"
            "Please create twilio_config.txt with the following format:\n"
            "ACCOUNT_SID=your_account_sid\n"
            "AUTH_TOKEN=your_auth_token\n"
            "PHONE_NUMBER=your_phone_number"
        );
    }
    
    // Read configuration file line by line
    std::string line;
    while (std::getline(config_file, line)) {
        if (line.find("ACCOUNT_SID=") == 0) {
            config.account_sid = line.substr(12);
        } else if (line.find("AUTH_TOKEN=") == 0) {
            config.auth_token = line.substr(11);
        } else if (line.find("PHONE_NUMBER=") == 0) {
            config.phone_number = line.substr(12);
        }
    }
    
    // Validate configuration
    if (config.account_sid.empty() || config.auth_token.empty() || config.phone_number.empty()) {
        throw std::runtime_error(Color::RED + "Invalid configuration in twilio_config.txt" + Color::RESET);
    }
    
    return config;
}

/*
 * Main SMS Sender class
 * Handles all SMS sending operations and phone number management
 */
class SMSSender {
private:
    TwilioConfig config;
    
    /*
     * @brief Callback function for CURL to write received data
     * @return Size of processed data
     */
    static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
        ((std::string*)userp)->append((char*)contents, size * nmemb);
        return size * nmemb;
    }

    /*
     * @brief Normalizes phone numbers to a standard format
     * @param number Phone number to normalize
     * @return Normalized phone number string
     */
    std::string normalizePhoneNumber(const std::string& number) {
        std::string cleaned = "";
        // Remove all non-digit characters
        for (char c : number) {
            if (isdigit(c)) {
                cleaned += c;
            }
        }
        
        // Add international prefix if not present
        if (!cleaned.empty()) {
            return "+" + cleaned;
        }
        return "";
    }

    /*
     * @brief Validates phone number format
     * @param number Phone number to validate
     * @return bool indicating if number is valid
     */
    bool validatePhoneNumber(const std::string& number) {
        std::string normalized = normalizePhoneNumber(number);
        
        // Enhanced validation rules
        if (normalized.empty()) return false;
        if (normalized.length() < 10 || normalized.length() > 15) return false;
        if (normalized.substr(1, 1) == "0") return false;  // Invalid country code
        
        return true;
    }

    /*
     * @brief URL encodes a string for HTTP transmission
     * @param value String to encode
     * @return URL encoded string
     */
    std::string urlEncode(const std::string& value) {
        std::ostringstream escaped;
        escaped.fill('0');
        escaped << std::hex;

        for (char c : value) {
            if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
                escaped << c;
            } else {
                escaped << '%' << std::setw(2) << int((unsigned char)c);
            }
        }

        return escaped.str();
    }

    /*
     * @brief Formats phone number for display
     * @param number Phone number to format
     * @return Formatted phone number string
     */
    std::string formatPhoneNumber(const std::string& number) {
        if (number.length() >= 12) {
            return number.substr(0, 3) + " " + number.substr(3, 2) + " " + 
                   number.substr(5, 4) + " " + number.substr(9);
        }
        return number;
    }

public:
    // Constructor
    SMSSender(const TwilioConfig& cfg) : config(cfg) {}

    /*
     * Structure to hold SMS sending result
     */
    struct SendResult {
        bool success;           // Indicates if send was successful
        std::string message;    // Result message or error description
        std::string sid;        // Twilio message SID
    };

    /*
     * @brief Loads phone numbers from file
     * @return Vector of validated phone numbers
     * @throws std::runtime_error if numbers file is missing
     */
    std::vector<std::string> loadPhoneNumbers() {
        std::vector<std::string> numbers;
        std::vector<std::string> invalid_numbers;
        std::ifstream file("numbers.txt");
        std::string line;

        if (!file.is_open()) {
            throw std::runtime_error(
                Color::RED + "Error: numbers.txt not found!\n" + Color::RESET +
                "Please create numbers.txt with one phone number per line.\n"
                "Format: [country_code][number] (Example: 5511999999999)"
            );
        }

        std::cout << Color::CYAN << "\nReading phone numbers from numbers.txt...\n" << Color::RESET;
        int line_number = 0;
        
        // Process each line in the file
        while (std::getline(file, line)) {
            line_number++;
            line.erase(remove_if(line.begin(), line.end(), isspace), line.end());
            
            if (!line.empty()) {
                std::string normalizedNumber = normalizePhoneNumber(line);
                if (validatePhoneNumber(normalizedNumber)) {
                    numbers.push_back(normalizedNumber);
                    std::cout << Color::GREEN << "✓ " << Color::RESET << 
                             "Valid number: " << formatPhoneNumber(normalizedNumber) << std::endl;
                } else {
                    invalid_numbers.push_back(line);
                    std::cout << Color::RED << "✗ " << Color::RESET << 
                             "Invalid number on line " << line_number << ": " << line << std::endl;
                }
            }
        }

        // Report invalid numbers if any
        if (!invalid_numbers.empty()) {
            std::cout << Color::YELLOW << "\nWarning: Found " << invalid_numbers.size() 
                     << " invalid numbers!\n" << Color::RESET;
            std::cout << "Numbers should include country code (e.g., +5511999999999)\n\n";
        }

        return numbers;
    }

    /*
     * @brief Sends an SMS message using Twilio API
     * @param recipient Recipient phone number
     * @param message Message content
     * @return SendResult structure containing the result
     */
    SendResult sendSMS(const std::string& recipient, const std::string& message) {
        CURL* curl = curl_easy_init();
        SendResult result{false, "", ""};

        if (curl) {
            std::string readBuffer;
            std::string url = "https://api.twilio.com/2010-04-01/Accounts/" + 
                            config.account_sid + "/Messages.json";

            // Prepare POST data
            std::string postData = "From=" + urlEncode(config.phone_number) +
                                 "&To=" + urlEncode(recipient) +
                                 "&Body=" + urlEncode(message);

            // Configure CURL options
            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postData.c_str());
            curl_easy_setopt(curl, CURLOPT_USERNAME, config.account_sid.c_str());
            curl_easy_setopt(curl, CURLOPT_PASSWORD, config.auth_token.c_str());
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

            // Perform the request
            CURLcode res = curl_easy_perform(curl);

            if (res == CURLE_OK) {
                try {
                    json response = json::parse(readBuffer);
                    if (response.contains("sid")) {
                        result.success = true;
                        result.sid = response["sid"].get<std::string>();
                        result.message = "Message sent successfully";
                    } else if (response.contains("error_message")) {
                        result.message = "Twilio Error: " + response["error_message"].get<std::string>();
                    } else {
                        result.message = "Unknown response: " + readBuffer;
                    }
                } catch (const std::exception& e) {
                    result.message = "Error parsing response: " + std::string(e.what());
                }
            } else {
                result.message = "Connection failed: " + std::string(curl_easy_strerror(res));
            }

            curl_easy_cleanup(curl);
        }

        return result;
    }
};

/*
 * Main function
 * Handles the program flow and user interaction
 */
int main() {
    std::string message;
    displayBanner();

    try {
        std::cout << Color::CYAN << "Initializing SMS sender..." << Color::RESET << std::endl;
        auto config = readConfig();
        std::cout << Color::GREEN << "✓ " << Color::RESET << "Configuration loaded successfully\n";

        // Initialize SMS sender and load phone numbers
        SMSSender sender(config);
        auto numbers = sender.loadPhoneNumbers();

        // Check if any valid numbers were found
        if (numbers.empty()) {
            std::cout << Color::RED << "\nError: No valid phone numbers found in numbers.txt\n" << Color::RESET;
            std::cout << "Please check the file and try again.\n";
            std::cout << "\nPress Enter to exit...";
            std::cin.get();
            return 1;
        }

        // Get message from user
        std::cout << Color::CYAN << "\n=== Message Configuration ===" << Color::RESET << "\n";
        std::cout << "Enter the SMS message to send (max 1600 characters):\n"
                  << Color::YELLOW << "Message: " << Color::RESET;
        std::getline(std::cin, message);

        // Validate message
        while (message.empty()) {
            std::cout << Color::RED << "Message cannot be empty. Please enter a message:\n" << Color::RESET;
            std::cout << Color::YELLOW << "Message: " << Color::RESET;
            std::getline(std::cin, message);
        }

        // Show confirmation details
        std::cout << Color::CYAN << "\n=== Confirmation ===" << Color::RESET << "\n";
        std::cout << "Ready to send messages:\n";
        std::cout << "- From: " << Color::YELLOW << config.phone_number << Color::RESET << "\n";
        std::cout << "- Recipients: " << Color::YELLOW << numbers.size() << Color::RESET << "\n";
        std::cout << "- Message length: " << Color::YELLOW << message.length() << "/1600" << Color::RESET << " characters\n";
        std::cout << "- Message preview: " << Color::YELLOW << message << Color::RESET << "\n\n";
        
        // Get user confirmation
        std::cout << "Send messages? (y/n): ";
        char confirm;
        std::cin >> confirm;
        
        // Check if user wants to proceed
        if (confirm != 'y' && confirm != 'Y') {
            std::cout << Color::YELLOW << "Operation cancelled by user.\n" << Color::RESET;
            std::cout << "\nPress Enter to exit...";
            std::cin.get();
            std::cin.get();
            return 0;
        }

        // Start sending messages
        std::cout << Color::CYAN << "\n=== Sending Messages ===" << Color::RESET << "\n";
        int success_count = 0;
        int fail_count = 0;
        int total = numbers.size();
        int current = 0;

        // Process each number in the list
        for (const auto& number : numbers) {
            current++;
            displayProgress(current, total);
            auto result = sender.sendSMS(number, message);

            // Clear progress bar line
            std::cout << "\r" << std::string(80, ' ') << "\r";  
            std::cout << "[" << current << "/" << total << "] Sending to " << number << "... ";
            
            // Display result
            if (result.success) {
                std::cout << Color::GREEN << "✓ SUCCESS" << Color::RESET << 
                         " (SID: " << result.sid << ")" << std::endl;
                success_count++;
            } else {
                std::cout << Color::RED << "✗ FAILED: " << Color::RESET << 
                         result.message << std::endl;
                fail_count++;
            }
            
            // Implement rate limiting with visual feedback
            for (int i = 0; i < 11; ++i) {
                std::cout << "\rWaiting for rate limit... " << 
                         std::string(11 - i, '.') << std::flush;
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
            std::cout << "\r" << std::string(80, ' ') << "\r";
        }

        // Display final report with statistics
        std::cout << Color::CYAN << "\n=== Final Report ===" << Color::RESET << "\n";
        std::cout << "Total messages: " << Color::YELLOW << total << Color::RESET << "\n";
        std::cout << Color::GREEN << "✓ Successful: " << success_count << Color::RESET << "\n";
        std::cout << Color::RED << "✗ Failed: " << fail_count << Color::RESET << "\n";
        
        // Show troubleshooting information if there were failures
        if (fail_count > 0) {
            std::cout << Color::YELLOW << "\nPossible reasons for failures:" << Color::RESET << "\n";
            std::cout << "- Invalid Twilio credentials\n";
            std::cout << "- Phone number not properly configured\n";
            std::cout << "- Network connection issues\n";
            std::cout << "- Insufficient Twilio balance\n";
            std::cout << "- Message content restrictions\n";
            std::cout << Color::CYAN << "Check the Twilio dashboard for detailed message status.\n" << Color::RESET;
        }

    } catch (const std::exception& e) {
        // Handle any exceptions that occurred during execution
        std::cerr << Color::RED << "\nError: " << e.what() << Color::RESET << std::endl;
        std::cout << "\nPress Enter to exit...";
        std::cin.get();
        return 1;
    }

    // Program completion
    std::cout << Color::GREEN << "\nProgram finished successfully!" << Color::RESET;
    std::cout << "\nPress Enter to exit...";
    std::cin.get();
    std::cin.get();
    return 0;
}