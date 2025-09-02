#include <raylib.h>
#include <fstream>
#include <string>
#include <cctype>

// Save credentials to file
bool RegisterUser(const std::string &username, const std::string &password) {
    std::ifstream infile("users.txt");
    std::string u, p;
    while (infile >> u >> p) {
        if (u == username) return false; 
    }
    infile.close();

    std::ofstream outfile("users.txt", std::ios::app);
    outfile << username << " " << password << "\n";
    outfile.close();
    return true;
}

// Verify credentials
bool LoginUser(const std::string &username, const std::string &password) {
    std::ifstream infile("users.txt");
    std::string u, p;
    while (infile >> u >> p) {
        if (u == username && p == password) return true;
    }
    return false;
}

int main() {
    InitWindow(600, 400, "Login & Registration System");
    SetTargetFPS(60);

    std::string username = "";
    std::string password = "";
    bool usernameActive = false;
    bool passwordActive = false;

    bool showRegister = true;
    std::string message = "";

    while (!WindowShouldClose()) {
        // --- Handle input focus ---
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            Vector2 mouse = GetMousePosition();
            usernameActive = CheckCollisionPointRec(mouse, {200, 120, 200, 30});
            passwordActive = CheckCollisionPointRec(mouse, {200, 170, 200, 30});
        }

        // --- Handle typing ---
        if (usernameActive) {
        int key = GetCharPressed();
        while (key > 0) {
            if (isprint(key)) username.push_back((char)key);
            key = GetCharPressed();
        }
        if (IsKeyPressed(KEY_BACKSPACE) && !username.empty()) {
            username.pop_back();
        }
    }

        if (passwordActive) {  
        int key = GetCharPressed();
        while (key > 0) {
            if (isprint(key)) password.push_back((char)key);
            key = GetCharPressed();
        }
        if (IsKeyPressed(KEY_BACKSPACE) && !password.empty()) {
            password.pop_back();
        }
    }

        // --- Handle Enter key ---
        if (IsKeyPressed(KEY_ENTER)) {
            if (showRegister) {
                if (!username.empty() && !password.empty()) {
                    if (RegisterUser(username, password)) {
                        message = "Registered successfully!";
                        username.clear();
                        password.clear();
                    } else {
                        message = "Username already exists!";
                    }
                } else {
                    message = "Fill both fields!";
                }
            } else {
                if (LoginUser(username, password)) {
                    message = "Login successful!";
                    username.clear();
                    password.clear();
                } else {
                    message = "Invalid credentials!";
                }
            }
        }

        // --- Switch Register/Login ---
        if (IsKeyPressed(KEY_TAB)) {
            showRegister = !showRegister;
            message = showRegister ? "Switched to Register" : "Switched to Login";
        }

        BeginDrawing();
        ClearBackground(RAYWHITE);

        DrawText("Login & Registration System", 140, 40, 20, DARKBLUE);
        DrawText(showRegister ? "[TAB] Switch to Login" : "[TAB] Switch to Register", 180, 80, 16, GRAY);

        DrawText("Username:", 100, 125, 18, BLACK);
        DrawRectangleLines(200, 120, 200, 30, usernameActive ? RED : BLACK);
        DrawText(username.c_str(), 210, 125, 18, DARKGRAY);

        DrawText("Password:", 100, 175, 18, BLACK);
        DrawRectangleLines(200, 170, 200, 30, passwordActive ? RED : BLACK);
        std::string hidden(password.size(), '*');
        DrawText(hidden.c_str(), 210, 175, 18, DARKGRAY);

        DrawText("Press ENTER to submit", 200, 220, 16, DARKGREEN);

        if (!message.empty()) {
            DrawText(message.c_str(), 200, 260, 18, MAROON);
        }

        EndDrawing();
    }

    CloseWindow();
    return 0;
}