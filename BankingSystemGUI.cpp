#include <raylib.h>
#include <string>
#include <vector>
#include <ctime>
#include <cctype>

// ---------- Data Models ----------
struct Transaction {
    std::string type;
    float amount;
    std::string timestamp;
    std::string details;
};

struct Account {
    int id;
    float balance;
    std::vector<Transaction> history;

    // Explicit constructor for robust object creation
    Account(int account_id = 0) : id(account_id), balance(0.0f) {}

    void AddTransaction(const std::string& type, float amt, const std::string& det) {
        time_t now = time(nullptr);
        char buf[64];
        strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M", localtime(&now));
        history.push_back(Transaction{type, amt, buf, det});
    }

    void Deposit(float amt) {
        balance += amt;
        AddTransaction("Deposit", amt, "");
    }

    bool Withdraw(float amt, std::string& msg) {
        if (amt <= 0) { msg = "Invalid withdrawal amount"; return false; }
        if (amt > balance) { msg = "Unable to withdraw: Insufficient funds"; return false; }
        balance -= amt;
        AddTransaction("Withdrawal", amt, "");
        msg = "Withdrawal successful";
        return true;
    }

    void ReceiveFunds(float amt, int from_account_id) {
        balance += amt;
        AddTransaction("Transfer In", amt, "From Acc " + std::to_string(from_account_id));
    }

    bool Transfer(Account& to, float amt, std::string& msg) {
        if (amt <= 0) { msg = "Invalid transfer amount"; return false; }
        if (amt > balance) { msg = "Unable to transfer: Insufficient funds"; return false; }
        balance -= amt;
        AddTransaction("Transfer Out", amt, "To Acc " + std::to_string(to.id));
        to.ReceiveFunds(amt, id);
        msg = "Transfer successful";
        return true;
    }
};

struct Customer {
    int id{};
    std::string name;
    Account account;
};

// ---------- Globals ----------
std::vector<Customer> customers;
int nextID = 1;
int activeCustomer = -1;
int transferSourceIndex = -1;
std::string inputName;
std::string inputAmount;
bool inputNameActive = false;
bool inputAmountActive = false;
std::string message;
float historyScrollY = 0.0f;
bool transferMode = false;

// ---------- Helpers ----------
float ToFloat(const std::string& s) {
    try { return s.empty() ? 0.0f : std::stof(s); }
    catch (...) { return 0.0f; }
}

void AddCustomer(const std::string& name) {
    Customer c;
    c.id = nextID++;
    c.name = name;
    c.account = Account(c.id);
    customers.push_back(c);
    if (activeCustomer == -1) activeCustomer = 0;
}

// ⭐ FIX: Replaced the old Button function with two specialized ones.

// This function ONLY draws the button.
void DrawButton(const char* text, Rectangle bounds, Color color, bool enabled = true) {
    Vector2 m = GetMousePosition();
    bool hover = enabled && CheckCollisionPointRec(m, bounds);
    Color fill = color;
    
    if (!enabled) {
        fill = Color{(unsigned char)(color.r / 2), (unsigned char)(color.g / 2), (unsigned char)(color.b / 2), 180};
    } else if (hover) {
        fill = Color{(unsigned char)Clamp((int)color.r - 30, 0, 255),
                     (unsigned char)Clamp((int)color.g - 30, 0, 255),
                     (unsigned char)Clamp((int)color.b - 30, 0, 255), 255};
    }
    
    DrawRectangleRec(bounds, fill);
    DrawRectangleLinesEx(bounds, 2, BLACK);
    int tw = MeasureText(text, 20);
    DrawText(text, bounds.x + (bounds.width - tw) / 2, bounds.y + 8, 20, BLACK);
}

// This function ONLY checks if the button was clicked.
bool IsButtonClicked(Rectangle bounds, bool enabled = true) {
    if (!enabled) return false;
    return CheckCollisionPointRec(GetMousePosition(), bounds) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
}


// ---------- Main ----------
int main() {
    const int W = 1000, H = 720;
    InitWindow(W, H, "Banking System (Raylib GUI)");
    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        // ===================================================================
        //  1. INPUT & LOGIC UPDATE PHASE
        // ===================================================================

        // --- Text Input ---
        if (!transferMode) {
            if (inputAmountActive) {
                for (int ch = GetCharPressed(); ch > 0; ch = GetCharPressed())
                    if ((ch >= '0' && ch <= '9') || ch == '.')
                        if (inputAmount.size() < 12) inputAmount.push_back((char)ch);
                if (IsKeyPressed(KEY_BACKSPACE) && !inputAmount.empty()) inputAmount.pop_back();
            }
            if (inputNameActive) {
                for (int ch = GetCharPressed(); ch > 0; ch = GetCharPressed())
                    if (std::isalpha(ch) || ch == ' ')
                        if (inputName.size() < 20) inputName.push_back((char)ch);
                if (IsKeyPressed(KEY_BACKSPACE) && !inputName.empty()) inputName.pop_back();
            }
        }

        // --- Button Input ---
        if (transferMode) {
            // --- Overlay Input Logic ---
            Rectangle closeBtnBounds = {740, 150, 40, 40};
            if (IsButtonClicked(closeBtnBounds)) {
                transferMode = false;
                transferSourceIndex = -1;
                message = "Transfer cancelled";
            }

            int ty = 220;
            float amtNow = ToFloat(inputAmount);
            for (int i = 0; i < (int)customers.size(); ++i) {
                if (i == transferSourceIndex) continue;
                std::string label = customers[i].name + " (Acc " + std::to_string(customers[i].id) + ")";
                int wBtn = MeasureText(label.c_str(), 20) + 40;
                Rectangle targetBtnBounds = {240, (float)ty, (float)wBtn, 40};

                if (IsButtonClicked(targetBtnBounds)) {
                    std::string msg;
                    if (transferSourceIndex >= 0) {
                        Account& from = customers[transferSourceIndex].account;
                        Account& to = customers[i].account;
                        if (amtNow > 0) {
                            from.Transfer(to, amtNow, msg);
                            message = msg + " (to " + customers[i].name + ")";
                        } else message = "Invalid transfer amount";
                    }
                    inputAmount.clear();
                    transferMode = false;
                    transferSourceIndex = -1;
                }
                ty += 60;
            }
        } else {
            // --- Main Screen Input Logic ---
            float amt = ToFloat(inputAmount);
            Rectangle nameBox = {220, 65, 200, 40};
            Rectangle amountBox = {160, 285, 160, 40};
            Rectangle addCustomerBtn = {440, 65, 160, 40};
            Rectangle depositBtn = {340, 285, 120, 40};
            Rectangle withdrawBtn = {470, 285, 120, 40};
            Rectangle transferBtn = {600, 285, 120, 40};

            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                if (CheckCollisionPointRec(GetMousePosition(), nameBox)) {
                    inputNameActive = true; inputAmountActive = false;
                } else if (CheckCollisionPointRec(GetMousePosition(), amountBox)) {
                    inputAmountActive = true; inputNameActive = false;
                }
            }

            if (IsButtonClicked(addCustomerBtn)) {
                if (!inputName.empty()) { AddCustomer(inputName); message = "Customer added: " + inputName; inputName.clear(); } 
                else message = "Please enter a valid name";
            }
            
            int x = 20, yBtn = 130;
            for (int i = 0; i < (int)customers.size(); ++i) {
                std::string label = customers[i].name + " (Acc " + std::to_string(customers[i].id) + ")";
                int width = MeasureText(label.c_str(), 20) + 40;
                if (IsButtonClicked({(float)x, (float)yBtn, (float)width, 40})) {
                    activeCustomer = i; message = "Switched to " + customers[i].name;
                }
                x += width + 20; if (x > W - 200) { x = 20; yBtn += 50; }
            }

            if (activeCustomer != -1) {
                Customer& c = customers[activeCustomer];
                if (IsButtonClicked(depositBtn)) {
                    if (amt > 0) { c.account.Deposit(amt); message = "Deposit successful"; } 
                    else message = "Invalid deposit amount";
                    inputAmount.clear();
                }
                if (IsButtonClicked(withdrawBtn)) {
                    std::string msg;
                    if (amt > 0) c.account.Withdraw(amt, msg);
                    else msg = "Invalid withdrawal amount";
                    message = msg;
                    inputAmount.clear();
                }
                if (IsButtonClicked(transferBtn)) {
                    if (customers.size() > 1) {
                        transferMode = true; transferSourceIndex = activeCustomer; message = "Select target account";
                    } else message = "No other account to transfer";
                }
            }
        }


        // ===================================================================
        //  2. DRAWING PHASE
        // ===================================================================
        BeginDrawing();
        ClearBackground(RAYWHITE);

        DrawText("Banking System", 20, 20, 30, DARKBLUE);
        
        // --- Draw Main Screen ---
        DrawText("New Customer Name:", 20, 70, 20, BLACK);
        Rectangle nameBox = {220, 65, 200, 40};
        DrawRectangleRec(nameBox, LIGHTGRAY);
        DrawRectangleLinesEx(nameBox, 2, inputNameActive ? RED : BLACK);
        DrawText(inputName.c_str(), nameBox.x + 8, nameBox.y + 10, 20, BLACK);
        DrawButton("Add Customer", {440, 65, 160, 40}, Color{200, 230, 200, 255}, !transferMode);

        int x = 20, yBtn = 130;
        for (int i = 0; i < (int)customers.size(); ++i) {
            std::string label = customers[i].name + " (Acc " + std::to_string(customers[i].id) + ")";
            int width = MeasureText(label.c_str(), 20) + 40;
            DrawButton(label.c_str(), {(float)x, (float)yBtn, (float)width, 40}, LIGHTGRAY, !transferMode);
            x += width + 20; if (x > W - 200) { x = 20; yBtn += 50; }
        }

        if (activeCustomer != -1) {
            Customer& c = customers[activeCustomer];
            DrawText(TextFormat("Active Customer: %s (ID %d)", c.name.c_str(), c.id), 20, 220, 22, BLACK);
            DrawText(TextFormat("Balance: %.2f", c.account.balance), 20, 250, 22, DARKGREEN);

            Rectangle amountBox = {160, 285, 160, 40};
            DrawRectangleRec(amountBox, LIGHTGRAY);
            DrawRectangleLinesEx(amountBox, 2, inputAmountActive ? RED : BLACK);
            DrawText(inputAmount.c_str(), amountBox.x + 8, amountBox.y + 10, 20, BLACK);
            DrawText("Enter Amount:", 20, 290, 20, BLACK);

            DrawButton("Deposit", {340, 285, 120, 40}, Color{173, 216, 230, 255}, !transferMode);
            DrawButton("Withdraw", {470, 285, 120, 40}, Color{255, 182, 193, 255}, !transferMode);
            DrawButton("Transfer", {600, 285, 120, 40}, Color{144, 238, 144, 255}, !transferMode);

            DrawText("Transaction History:", 20, 350, 22, BLACK);
            Rectangle panel = {20, 380, 960, 250};
            DrawRectangleLinesEx(panel, 2, BLACK);

            Rectangle panelContent = { panel.x, panel.y, panel.width, (float)c.account.history.size() * 28.0f };
            if(CheckCollisionPointRec(GetMousePosition(), panel)) historyScrollY -= GetMouseWheelMove() * 30.0f;
            if(historyScrollY > panelContent.height - panel.height) historyScrollY = panelContent.height - panel.height;
            if(historyScrollY < 0) historyScrollY = 0;
            
            BeginScissorMode((int)panel.x, (int)panel.y, (int)panel.width, (int)panel.height);
                float y = panel.y + 5 - historyScrollY;
                for (const auto& t : c.account.history) {
                    DrawText(TextFormat("[%s] %s %.2f %s", t.timestamp.c_str(), t.type.c_str(), t.amount, t.details.c_str()), (int)panel.x + 10, (int)y, 18, DARKGRAY);
                    y += 28.0f;
                }
            EndScissorMode();
        }

        // --- Draw Overlay ---
        if (transferMode) {
            DrawRectangle(0, 0, W, H, Color{0, 0, 0, 150});
            DrawRectangle(200, 150, 600, 400, RAYWHITE);
            DrawRectangleLines(200, 150, 600, 400, BLACK);
            DrawText("Select Account to Transfer", 220, 170, 24, DARKBLUE);
            DrawButton("X", {740, 150, 40, 40}, Color{255, 180, 180, 255});
            
            int ty = 220;
            for (int i = 0; i < (int)customers.size(); ++i) {
                if (i == transferSourceIndex) continue;
                std::string label = customers[i].name + " (Acc " + std::to_string(customers[i].id) + ")";
                int wBtn = MeasureText(label.c_str(), 20) + 40;
                DrawButton(label.c_str(), {240, (float)ty, (float)wBtn, 40}, LIGHTGRAY);
                ty += 60;
            }
        }

        if (!message.empty()) DrawText(message.c_str(), 20, H - 40, 22, RED);
        EndDrawing();
    }

    CloseWindow();
    return 0;
}