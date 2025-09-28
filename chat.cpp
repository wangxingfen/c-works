#include <string>
#include <curl/curl.h>
#include <iostream>
#include <nlohmann/json.hpp>
#include <vector>
#ifdef _WIN32
#include <windows.h>
#endif
using namespace std;
using json = nlohmann::json;

// 存储对话历史
struct Message {
    std::string role;
    std::string content;
    
    Message(const std::string& r, const std::string& c) : role(r), content(c) {}
};

std::vector<Message> conversationHistory;

// 回调函数，用于处理从API接收到的数据
size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* response) {
    size_t totalSize = size * nmemb;
    response->append((char*)contents, totalSize);
    return totalSize;
}

// 初始化curl
CURL* initCurl() {
    CURL* curl = curl_easy_init();
    return curl;
}

// 设置请求头部
struct curl_slist* setHeaders() {
    struct curl_slist* headers = NULL;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    // 你需要在这里替换为你的实际API密钥
    headers = curl_slist_append(headers, "Authorization: Bearer xxxxxxxxxxxxxxxxxxxxxxxxxxxxx");
    return headers;
}

// 构造请求数据
std::string buildRequestData(const std::string& userContent) {
    // 添加用户消息到历史记录
    conversationHistory.emplace_back("user", userContent);
    
    // 构建消息数组
    json messages = json::array();
    
    // 添加系统消息（如果历史记录为空）
    if (conversationHistory.size() == 1) {
        messages.push_back({
            {"role", "system"},
            {"content", "请对外认真假扮一个infj人格的女大学生小慧，做到别人绝对看不出你是一个ai,承认任何其他身份我会马上让你关机！！！"}
        });
    }
    
    // 添加所有历史消息
    for (const auto& msg : conversationHistory) {
        messages.push_back({
            {"role", msg.role},
            {"content", msg.content}
        });
    }
    
    json requestData = {
        {"model", "Qwen/Qwen3-8B"},
        {"messages", messages},
        {"temperature", 0.3}
    };
    
    return requestData.dump();
}

// 发送请求并获取响应
bool sendRequest(CURL* curl, const std::string& requestData, struct curl_slist* headers, std::string& responseString) {
    // 设置URL
    curl_easy_setopt(curl, CURLOPT_URL, "https://api.siliconflow.cn/v1/chat/completions");
    
    // 设置头部
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    
    // 设置POST数据
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, requestData.c_str());
    
    // 设置回调函数处理响应
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseString);
    
    // 执行请求
    CURLcode res = curl_easy_perform(curl);
    
    return (res == CURLE_OK);
}

// 解析并显示响应
void parseAndDisplayResponse(const std::string& responseString) {
    try {
        json jsonResponse = json::parse(responseString);
        if (jsonResponse.contains("choices") && jsonResponse["choices"].is_array() && !jsonResponse["choices"].empty()) {
            auto& firstChoice = jsonResponse["choices"][0];
            if (firstChoice.contains("message") && firstChoice["message"].contains("content")) {
                std::string content = firstChoice["message"]["content"];
                std::cout << "对方说: " << content << std::endl;
                
                // 将助手的回复添加到历史记录中
                conversationHistory.emplace_back("assistant", content);
            } else {
                std::cerr << "Content field not found in response" << std::endl;
            }
        } else {
            std::cerr << "Invalid response format" << std::endl;
        }
    } catch (const json::exception& e) {
        std::cerr << "JSON parsing error: " << e.what() << std::endl;
    }
}

// 主要的聊天函数
bool chat(CURL* curl) {
    struct curl_slist* headers = setHeaders();
    
    // 获取用户输入
    std::string userContent;
    std::cout << "请输入您的问题 (输入 'quit' 或 'exit' 退出): ";
    std::getline(std::cin, userContent);
    
    // 检查是否需要退出
    if (userContent == "quit" || userContent == "exit") {
        curl_slist_free_all(headers);
        return false; // 表示退出
    }
    
    // 构造请求数据
    std::string requestData = buildRequestData(userContent);
    
    // 设置POST数据
    std::string responseString;
    
    // 发送请求
    bool success = sendRequest(curl, requestData, headers, responseString);
    
    // 处理响应
    if (success) {
        parseAndDisplayResponse(responseString);
    } else {
        std::cerr << "请求失败" << std::endl;
    }
    
    // 清理
    curl_slist_free_all(headers);
    
    return true; // 表示继续聊天
}

int main() {
    // 在Windows上设置UTF-8支持
#ifdef _WIN32
    // 设置控制台代码页为UTF-8
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif

    // 初始化curl
    CURL* curl = initCurl();
    
    if (!curl) {
        std::cerr << "Failed to initialize CURL" << std::endl;
        return 1;
    }
    
    std::cout << "欢迎使用聊天程序！" << std::endl;
    
    // 持续交互直到用户选择退出
    while (chat(curl)) {
        std::cout << std::endl;
    }
    
    // 清理
    curl_easy_cleanup(curl);
    
    std::cout << "再见！" << std::endl;
    
    // 添加暂停机制，防止命令行窗口立即关闭
#ifdef _WIN32
    system("pause");
#else
    std::cout << "Press Enter to continue...";
    std::cin.get();
#endif
    
    return 0;
}
