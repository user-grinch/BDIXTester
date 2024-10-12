#include <vector>
#include <chrono>
#include <thread>
#include <algorithm>
#include <atomic>
#include "imgui/imgui.h"
#include "curl/curl.h"

class BDIXTester {
private:
    std::string localFile;
    std::string remoteUrl;
    int timeoutMs;
    std::vector<std::string> bdixList;
    std::vector<std::string> validBdixList;
    std::string currentUrl;
    float currentProgress;
    std::atomic<bool> testInProgress;
    std::atomic<bool> stopTest;
    std::thread testThread;
    char urlInputBuffer[256] = "";
    bool showPopup;
    std::string errorMessage;

public:
    BDIXTester(const std::string& localFile, const std::string& remoteUrl, int timeout) {
        this->localFile = localFile;
        this->remoteUrl = remoteUrl;
        this->timeoutMs = timeout * 1000;
        this->currentProgress = 0.0f;
        this->testInProgress = false;
        this->stopTest = false;
        this->showPopup = false;
        LoadDB();  
    }

    void RenderUI(bool popup = true) {
        ImVec2 windowSize = {ImGui::GetWindowContentRegionMax().x - ImGui::GetWindowContentRegionMin().x,
                                ImGui::GetWindowContentRegionMax().y - ImGui::GetWindowContentRegionMin().y};

        float buttonWidth = windowSize.x;
        float buttonHeight = ImGui::GetFrameHeight();
        const char* buttonText = "Stop";
        ImVec2 buttonTextSize = ImGui::CalcTextSize(buttonText);
        ImVec2 buttonPadding = ImGui::GetStyle().FramePadding;

        float stopWidth = buttonTextSize.x + buttonPadding.x * 2.0f;
        float stopHeight = buttonTextSize.y + buttonPadding.y * 2.0f;
        float progressBarWidth = windowSize.x - stopWidth - ImGui::GetStyle().ItemSpacing.x; 
        ImGui::ProgressBar(currentProgress, ImVec2(progressBarWidth, stopHeight), currentUrl.c_str());
        ImGui::SameLine();
        if (ImGui::Button(buttonText, ImVec2(stopWidth, stopHeight))) {
            stopTest = true;
            currentUrl = "Stopped by User";
            currentProgress = 0.0f;
        }

        ImVec2 btnSz = ImVec2(buttonWidth/2 - ImGui::GetStyle().ItemSpacing.x, buttonHeight);
        if (ImGui::Button("Scan DB", btnSz)) {
            if (!testInProgress) {
                stopTest = false;
                testInProgress = true;
                validBdixList.clear();

                testThread = std::thread([this]() {
                    TestURLs(); 
                });
                testThread.detach();
            } else {
                ShowError("Stop/ Wait for the scan to finish");
            }
        }
        ImGui::SameLine();
        ImGui::Spacing();
        ImGui::SameLine();
        if (ImGui::Button("Update DB", btnSz)) {
            if (!testInProgress) {
                testThread = std::thread([this]() {
                    UpdateDB();
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                    LoadDB();
                });
                testThread.detach();
            } else {
                ShowError("Stop/ Wait for the scan to finish");
            }
        }

        ImGui::Dummy({0, 5});
        std::vector<std::string> *pVec = nullptr;

        if (ImGui::BeginTabBar("Tbabar")) {
            if (ImGui::BeginTabItem("BDIX URLs")) {
                pVec = &bdixList;
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Working URLs")) {
                pVec = &validBdixList;
                ImGui::EndTabItem();
            }
            ImGui::EndTabBar();
        }
        if (pVec->empty()) {
            ImGui::Text("No URLs available");
        } else {
            ImGui::BeginChild("TEST");
            ImGuiListClipper clipper;
            static ImGuiTextFilter filter;
            clipper.Begin(static_cast<int>(pVec->size()));
            ImGui::SetNextItemWidth(ImGui::GetWindowContentRegionWidth());
            ImGui::InputTextWithHint("##Filter", (std::string("Search (Total: ") + std::to_string(pVec->size()) + ")").c_str(),
                 filter.InputBuf, IM_ARRAYSIZE(filter.InputBuf));
            filter.Build();

            if (ImGui::BeginTable("URLTable", 3, ImGuiTableFlags_Sortable | ImGuiTableFlags_Borders | ImGuiTableFlags_ScrollY | ImGuiTableFlags_RowBg)) {
                ImGui::TableSetupColumn("URL", ImGuiTableColumnFlags_WidthStretch | ImGuiTableColumnFlags_DefaultSort);
                ImGui::TableSetupColumn("Type", ImGuiTableColumnFlags_WidthFixed, ImGui::GetWindowContentRegionWidth() * 0.125f); // Server Type column
                ImGui::TableSetupColumn("Action", ImGuiTableColumnFlags_WidthFixed, ImGui::GetWindowContentRegionWidth() * 0.125f); // Action column for buttons
                ImGui::TableSetupScrollFreeze(0, 1);
                ImGui::TableHeadersRow();

                ImGuiTableSortSpecs* sortSpecs = ImGui::TableGetSortSpecs();
                if (sortSpecs->SpecsDirty) {
                    sortSpecs->SpecsDirty = false;

                    int column = sortSpecs->Specs->ColumnIndex;
                    bool ascending = sortSpecs->Specs->SortDirection == ImGuiSortDirection_Ascending;

                    // Sort the items based on the current specifications
                    std::sort(pVec->begin(), pVec->end(), [column, ascending, this](const std::string& a, const std::string& b) {
                          return SortItems(a, b, column, ascending);
                      });
                }

                std::vector<int> filteredIndices;
                for (int i = 0; i < pVec->size(); ++i) {
                    const std::string& url = pVec->at(i);
                    if (filter.PassFilter(url.c_str())) {
                        filteredIndices.push_back(i); 
                    }
                }

                ImGuiListClipper clipper;
                clipper.Begin(static_cast<int>(filteredIndices.size()));

                while (clipper.Step()) {
                    for (int n = clipper.DisplayStart; n < clipper.DisplayEnd; ++n) {
                        int i = filteredIndices[n]; 
                        const std::string& url = pVec->at(i);
                        ImGui::TableNextRow();
                        ImGui::TableNextColumn();
                        ImGui::Text("%s", url.c_str());
                        ImGui::TableNextColumn();
                        ImGui::Text("%s", GetServerType(url).c_str());
                        ImGui::TableNextColumn();
                        if (ImGui::Button(("Open##" + std::to_string(i)).c_str())) {
                            ShellExecuteA(0, "open", url.c_str(), 0, 0, SW_SHOWNORMAL);
                        }
                    }
                }

                // End the table
                ImGui::EndTable();
            }
            clipper.End();
            ImGui::EndChild();
        }

        if (showPopup && popup) {
            ImGui::OpenPopup("Error");
            ImGui::SetNextWindowPos(ImVec2(windowSize.x / 2, windowSize.y / 2), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
            ImGui::SetNextWindowSize(ImVec2(windowSize.x / 2, 0), ImGuiCond_Appearing);
            if (ImGui::BeginPopupModal("Error", NULL, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_AlwaysAutoResize)) {
                ImGui::TextWrapped("%s", errorMessage.c_str());
                ImGui::NewLine();
                if (ImGui::Button("OK", ImVec2(ImGui::GetWindowContentRegionWidth(), 0))) {
                    showPopup = false;
                    ImGui::CloseCurrentPopup();
                }
                ImGui::EndPopup();
            }
        }
    }

    bool SortItems(const std::string& a, const std::string& b, int column, bool ascending) {
        if (column == 0) { 
            return ascending ? a < b : a > b;
        } else if (column == 1) { 
            return ascending ? GetServerType(a) < GetServerType(a) : GetServerType(a) > GetServerType(b);
        }
        return false; // Default case
    }

    bool DownloadDB(const std::string& url, const std::string& output) {
        CURL *curl;
        CURLcode res;
        FILE *file;

        fopen_s(&file, output.c_str(), "wb");
        if (!file) {
            ShowError("Failed to create database");
            return false;
        }

        curl_global_init(CURL_GLOBAL_DEFAULT);
        curl = curl_easy_init();
        if (!curl) {
            ShowError("Failed to initialize curl");
            fclose(file);
            return false;
        }

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, fwrite);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, file);
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L); 

        // HTTPS SSL/TLS
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);
        res = curl_easy_perform(curl);

        fclose(file);
        curl_easy_cleanup(curl);
        curl_global_cleanup();

        if (res != CURLE_OK) {
            ShowError("Failed to download database\n" + std::string(curl_easy_strerror(res)));
            return false;
        }
        return true;
    }

    void UpdateDB() {
        if (!DownloadDB(remoteUrl, localFile)) {
            ShowError("Failed to download the BDIX URL list.");
            testInProgress = false;
            return;
        }
    }

    void LoadDB() {
        FILE *file = nullptr;
        fopen_s(&file, localFile.c_str(), "r");
        if (!file) {
            ShowError("Failed to open database, try updating");
            testInProgress = false;
            return;
        }

        char line[1024];
        bdixList.clear();
        while (fgets(line, sizeof(line), file)) {
            std::string url(line);
            url.erase(std::remove(url.begin(), url.end(), '\n'), url.end());
            if (!url.empty()) {
                bdixList.push_back(url);
            }
        }

        if (bdixList.empty()) {
            ShowError("No URLs found in the database, try updating");
            testInProgress = false;
        }

        fclose(file);

        // Load valid URLs
        fopen_s(&file, "valid.db", "r");
        if (file) {
            validBdixList.clear();
            while (fgets(line, sizeof(line), file)) {
                std::string url(line);
                url.erase(std::remove(url.begin(), url.end(), '\n'), url.end());
                if (!url.empty()) {
                    validBdixList.push_back(url);
                }
            }
            fclose(file);
        }
    }

    void TestURLs() {
        size_t count = bdixList.size();
        if (bdixList.empty()) {
            ShowError("Update the database first");
        }

        FILE *out = nullptr;
        fopen_s(&out, "valid.db", "w");
        if (!out) {
            ShowError("Failed to create valid.db");
            return;
        }

        for (size_t i = 0; i < count; ++i) {
            if (stopTest) {
                testInProgress = false;
                return;  
            }

            currentUrl = bdixList[i];
            currentProgress = static_cast<float>(i + 1) / count;

            if (CheckURL(currentUrl)) {
                validBdixList.push_back(currentUrl);
                if (out) {
                    fprintf(out, "%s\n", currentUrl.c_str());
                }
            }
        }
        if (out) {
            fclose(out);
        }
        testInProgress = false;
    }

    bool CheckURL(const std::string& url, int timeout = 2) {
        CURL *curl;
        CURLcode res;

        curl_global_init(CURL_GLOBAL_DEFAULT);
        curl = curl_easy_init();
        if(curl) {
            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
            curl_easy_setopt(curl, CURLOPT_TIMEOUT, timeout);
            curl_easy_setopt(curl, CURLOPT_NOBODY, 1L); // Send HEAD
            curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L); // Follow redirects

            // HTTPS SSL/TLS verify
            curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L); 
            curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L); 

            res = curl_easy_perform(curl);
            if(res == CURLE_OK) {
                long http_code;
                curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
                return http_code >= 200 && http_code < 400;
            } else if (res == CURLE_OPERATION_TIMEDOUT) {
                return false;
            }
            curl_easy_cleanup(curl);
        }
        curl_global_cleanup();
        return false;
    }

    void ShowError(const std::string& message) {
        errorMessage = message;
        showPopup = true;
    }

    std::string GetServerType(const std::string& url) {
        std::vector<std::pair<std::string, std::string>> serverTypeMapping = {
            {"ftp", "FTP"},
            {"file", "FTP"},
            {"game", "FTP"},
            {"games", "FTP"},
            {"software", "FTP"},
            {"mov", "MOV"},
            {"movie", "MOV"},
            {"flix", "MOV"},
            {"anime", "MOV"},
            {"stream", "MOV"},
            {"media", "MOV"},
            {"cartoon", "MOV"}
        };

        std::string serverType = "MISC"; 
        for (const auto& [keyword, type] : serverTypeMapping) {
            if (url.find(keyword) != std::string::npos) {
                serverType = type;
                break; 
            }
        }
        return serverType;
    }
};