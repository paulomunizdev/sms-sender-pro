# SMS Sender Pro

A professional bulk SMS messaging tool using the Twilio API, featuring phone number validation, progress tracking, and detailed reporting.

![License](https://img.shields.io/badge/license-MIT-blue.svg)
![Version](https://img.shields.io/badge/version-1.0.0-green.svg)

## Features

- Bulk SMS messaging using Twilio API
- Phone number validation and formatting 
- Real-time progress tracking with visual feedback
- Detailed success/failure reporting
- Rate limiting implementation
- Color-coded console output
- Configuration file support

## Quick Start (Pre-compiled Binary)

If you just want to use the tool without compiling it, follow these steps:

1. Clone the repository:
```bash
git clone https://github.com/paulomunizdev/sms-sender-pro.git
```

2. Navigate to the binary folder:
```bash
cd sms-sender-pro
```

3. Configure your Twilio credentials:
   - Create a file named `twilio_config.txt` with the following content:
   ```
   ACCOUNT_SID=your_account_sid
   AUTH_TOKEN=your_auth_token
   PHONE_NUMBER=your_twilio_phone_number
   ```

4. Create your recipients list:
   - Create a file named `numbers.txt` with one phone number per line
   - Format: [country_code][number] (Example: 5511999999999)
   ```
   5511999999999
   5511888888888
   ```

5. Set execution permissions:
```bash
chmod +x sms_sender
```

6. Run the application:
```bash
./sms_sender
```

## Building from Source

If you want to compile the tool yourself, follow these steps:

1. Clone the repository:
```bash
git clone https://github.com/paulomunizdev/sms-sender-pro.git
```

2. Navigate to the binary folder:
```bash
cd sms-sender-pro
```

2. Install dependencies (Ubuntu/Debian):
```bash
sudo apt-get update
sudo apt-get install g++ libcurl4-openssl-dev nlohmann-json3-dev
```

For other distributions, install equivalent packages for:
- G++ compiler
- libcurl development files
- nlohmann-json library

3. Navigate to the source folder:
```bash
cd src
```

4. Compile the code:
```bash
g++ -o sms_sender main.cpp -lcurl -pthread
```

5. Configure your Twilio credentials:
   - Create a file named `twilio_config.txt` with the following content:
   ```
   ACCOUNT_SID=your_account_sid
   AUTH_TOKEN=your_auth_token
   PHONE_NUMBER=your_twilio_phone_number
   ```

6. Create your recipients list:
   - Create a file named `numbers.txt` with one phone number per line
   - Format: [country_code][number] (Example: 5511999999999)
   ```
   5511999999999
   5511888888888
   ```

7. Run the application:
```bash
./sms_sender
```

## Usage

1. When you run the application, it will:
   - Load your Twilio configuration
   - Validate the phone numbers from numbers.txt
   - Prompt you to enter your message
   - Show a confirmation with message details
   - Ask for confirmation before sending
   - Display real-time progress while sending
   - Show a final report with success/failure statistics

2. The application will validate your phone numbers and show:
   - ✓ Valid numbers in green
   - ✗ Invalid numbers in red

3. After sending, you'll get a detailed report showing:
   - Total messages sent
   - Successful deliveries
   - Failed deliveries
   - Troubleshooting information if there were any failures

## Error Handling

The application includes comprehensive error handling for:
- Missing configuration files
- Invalid phone numbers
- Network connectivity issues
- API response errors
- Rate limiting

## License

This project is licensed under the MIT License.

## Author

Paulo Muniz - [GitHub](https://github.com/paulomunizdev)
