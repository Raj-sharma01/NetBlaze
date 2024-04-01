#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <fstream>
#include <sstream>
#include <curl/curl.h>
#include <condition_variable> // Add this line

// Get the number of CPU cores
unsigned int numCores = std::thread::hardware_concurrency();
    
// Set MAX_CONCURRENT_DOWNLOADS to the number of CPU cores
const int MAX_CONCURRENT_DOWNLOADS = numCores;

// Semaphore implementation
class Semaphore {
public:
    Semaphore(int count = 0) : count(count) {}

    void notify() {
        std::unique_lock<std::mutex> lock(mutex);
        count++;
        cv.notify_one();
    }

    void wait() {
        std::unique_lock<std::mutex> lock(mutex);
        while (count == 0) {
            cv.wait(lock);
        }
        count--;
    }

private:
    std::mutex mutex;
    std::condition_variable cv;
    int count;
};

Semaphore semaphore(MAX_CONCURRENT_DOWNLOADS);
std::mutex fileMutex;

// Get filename from URL
std::string getFilenameFromURL(const std::string& url) {
    size_t found = url.find_last_of("/");
    if (found != std::string::npos) {
        return url.substr(found + 1);
    }
    return "file";
}

// Download function called by each thread
void downloadFile(const std::string& url, const std::string& downloadDirectory) {
    semaphore.wait(); // Wait for semaphore

    CURL *curl = curl_easy_init();
    if (curl) {
        std::string filename = getFilenameFromURL(url);
        std::string filepath = downloadDirectory + "/" + filename;

        FILE *fp = fopen(filepath.c_str(), "wb");
        if (fp) {
            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
            curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1L); // Enable progress meter
            curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, NULL); // Disable default progress meter

            CURLcode res = curl_easy_perform(curl);
            if (res != CURLE_OK) {
                std::cerr << "Failed to download file from URL: " << url << std::endl;
            }
            fclose(fp);
        } else {
            std::cerr << "Failed to open file for writing: " << filepath << std::endl;
        }
        curl_easy_cleanup(curl);
    }

    semaphore.notify(); // Release semaphore
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " <download_directory> <URL1> <URL2> <URL3> ..." << std::endl;
        return 1;
    }

    std::string downloadDirectory = argv[1];

    // Start a thread for each download
    std::vector<std::thread> threads;
    for (int i = 2; i < argc; ++i) {
        threads.emplace_back(downloadFile, argv[i], downloadDirectory);
    }

    // Wait for all threads to finish
    for (auto& thread : threads) {
        thread.join();
    }

    std::cout << "All downloads completed successfully." << std::endl;

    return 0;
}
