#pragma comment(lib, "ws2_32.lib") // 명시적인 라이브러리의 링크. 윈속 라이브러리 참조
#include <iostream>
#include <opencv2/opencv.hpp>
#include <mysql/jdbc.h>
#include <string>
#include <vector>
#include <memory>

void registerUser(const std::string& username, const std::string& password, cv::Mat& faceImage) {
    // OpenCV로 얼굴 이미지 처리 (예: 얼굴 검출, 정규화 등)
    std::vector<uchar> buf;
    cv::imencode(".jpg", faceImage, buf);
    std::string encodedFace(buf.begin(), buf.end());

    try {
        // MySQL 데이터베이스 연결 및 저장
        sql::mysql::MySQL_Driver* driver = sql::mysql::get_mysql_driver_instance();
        std::unique_ptr<sql::Connection> conn(driver->connect("tcp://127.0.0.1:3306", "root", "dbslzhs301"));
        conn->setSchema("chat");

        std::unique_ptr<sql::PreparedStatement> pstmt(conn->prepareStatement("INSERT INTO users (username, password, face) VALUES (?, ?, ?)"));
        pstmt->setString(1, username);
        pstmt->setString(2, password);
        pstmt->setString(3, encodedFace);
        pstmt->execute();

        std::cout << "User registered successfully." << std::endl;
    }
    catch (sql::SQLException& e) {
        std::cerr << "Error registering user: " << e.what() << std::endl;
    }
}

void loginUser(cv::Mat& faceImage) {
    // OpenCV로 얼굴 이미지 처리 및 데이터베이스에서 얼굴 이미지 비교
    std::vector<uchar> buf;
    cv::imencode(".jpg", faceImage, buf);
    std::string encodedFace(buf.begin(), buf.end());

    try {
        sql::mysql::MySQL_Driver* driver = sql::mysql::get_mysql_driver_instance();
        std::unique_ptr<sql::Connection> conn(driver->connect("tcp://127.0.0.1:3306", "root", "dbslzhs301"));
        conn->setSchema("chat");

        std::unique_ptr<sql::Statement> stmt(conn->createStatement());
        std::unique_ptr<sql::ResultSet> res(stmt->executeQuery("SELECT username, password, face FROM users"));

        while (res->next()) {
            std::string dbFace = res->getString("face");
            if (dbFace == encodedFace) {  // 간단한 비교. 실제 구현에서는 얼굴 인식 알고리즘을 사용해야 함
                std::cout << "Login successful! Username: " << res->getString("username") << std::endl;
                return;
            }
        }

        std::cout << "Login failed. Face not recognized." << std::endl;
    }
    catch (sql::SQLException& e) {
        std::cerr << "Error during login: " << e.what() << std::endl;
    }
}

int main() {
    // 카메라 열기
    cv::VideoCapture cap(0);
    if (!cap.isOpened()) {
        std::cerr << "Error opening camera" << std::endl;
        return -1;
    }

    cv::Mat frame;
    std::cout << "Press 'r' to register, 'l' to login." << std::endl;
    char action;
    std::cin >> action;

    cap >> frame;
    if (frame.empty()) {
        std::cerr << "Error capturing frame" << std::endl;
        return -1;
    }

    if (action == 'r') {
        std::string username, password;
        std::cout << "Enter username: ";
        std::cin >> username;
        std::cout << "Enter password: ";
        std::cin >> password;
        registerUser(username, password, frame);
    }
    else if (action == 'l') {
        loginUser(frame);
    }
    else {
        std::cerr << "Invalid action" << std::endl;
    }

    return 0;
}

//int main()
//{
//    sql::mysql::MySQL_Driver* driver = sql::mysql::get_mysql_driver_instance();
//    std::unique_ptr<sql::Connection> conn(driver->connect("tcp://127.0.0.1:3306", "root", "dbslzhs301"));
//
//    std::cout << "abc";
//}