#include <iostream>
#include <string>
#include <curl/curl.h>

// Callback function to handle data received from curl
size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

void configureOSPF(const std::string& routerIp, const std::string& authToken) {
    CURL* curl;
    CURLcode res;
    std::string readBuffer;

    // JSON payload to reset and configure OSPF
    std::string jsonData = R"({
        "ospf": {
            "action": "reset",
            "config": {
                "routerId": "1.1.1.1",
                "networks": [
                    {
                        "network": "192.168.1.0",
                        "wildcardMask": "0.0.0.255",
                        "area": "0"
                    },
                    {
                        "network": "10.0.0.0",
                        "wildcardMask": "0.0.0.255",
                        "area": "0"
                    }
                ]
            }
        }
    })";

    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();
    if (curl) {
        // Set the URL for the API request
        std::string url = "https://" + routerIp + "/api/v1/ospf/configure";

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

        // Set the HTTP headers
        struct curl_slist* headers = NULL;
        headers = curl_slist_append(headers, ("Authorization: Bearer " + authToken).c_str());
        headers = curl_slist_append(headers, "Content-Type: application/json");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        // Set the HTTP POST method and data
        curl_easy_setopt(curl, CURLOPT_POST, 1L);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, jsonData.c_str());

        // Skip peer verification for SSL (not recommended for production)
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);

        // Perform the request and check for errors
        res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
        } else {
            // Print the response from the server
            std::cout << "Response: " << readBuffer << std::endl;
        }

        // Clean up
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
    }
    curl_global_cleanup();
}

int main() {
    std::string routerIp, authToken;

    std::cout << "Enter the router IP address: ";
    std::getline(std::cin, routerIp);
    std::cout << "Enter the authorization token: ";
    std::getline(std::cin, authToken);

    configureOSPF(routerIp, authToken);

    return 0;
}
