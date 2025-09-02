#include <raylib.h>
#include <algorithm>
#include <string>
#include <vector>
#include <sstream>
#include <cctype>
#include <cmath>
#include <cstdlib>

// ------------ Data ------------
struct Course { std::string credit, grade; };
struct Semester { std::vector<Course> courses; };

// ------------ Helpers ------------
static bool IsNumericLike(const std::string& s) {
    if (s.empty()) return false;
    int dots = 0;
    for (char ch : s) {
        if (ch >= '0' && ch <= '9') continue;
        if (ch == '.' && ++dots <= 1) continue;
        return false;
    }
    return true;
}
static bool TryParseFloat(const std::string& s, float& out) {
    if (!IsNumericLike(s)) return false;
    char* end = nullptr;
    out = std::strtof(s.c_str(), &end);
    if (!end || *end != '\0') return false;
    if (!std::isfinite(out)) return false;
    return true;
}

static void AppendNumericChar(std::string& field, int ch, int maxLen = 8) {
    if ((ch >= '0' && ch <= '9') || ch == '.') {
        if ((int)field.size() < maxLen) {
            if (ch == '.') {
                if (field.find('.') == std::string::npos) field.push_back((char)ch);
            } else field.push_back((char)ch);
        }
    }
}

int main() {
    const int W = 900, H = 720;
    InitWindow(W, H, "CGPA Calculator (Multi-Semester)");
    SetTargetFPS(60);

    std::vector<Semester> semesters(1);
    semesters[0].courses.push_back(Course{});

    int activeSem = 0;
    int activeCourse = 0;
    bool editingCredit = true;

    std::vector<float> lastGPA;
    float lastCGPA = 0.0f;
    bool hasResults = false;

    float scrollY = 0.0f;
    const float rowH = 54.0f;
    const float semHeaderH = 40.0f;
    const float semFooterGap = 28.0f;
    const float topPad = 140.0f;
    const float bottomPad = 80.0f;

    while (!WindowShouldClose()) {
        // ---------- INPUT: actions ----------
        if (IsKeyPressed(KEY_RIGHT)) editingCredit = false;
        if (IsKeyPressed(KEY_LEFT))  editingCredit = true;

        if (IsKeyPressed(KEY_DOWN)) {
            activeCourse++;
            if (activeCourse >= (int)semesters[activeSem].courses.size()) {
                activeCourse = 0;
                activeSem = (activeSem + 1) % (int)semesters.size();
            }
        }
        if (IsKeyPressed(KEY_UP)) {
            activeCourse--;
            if (activeCourse < 0) {
                activeSem = (activeSem - 1 + (int)semesters.size()) % (int)semesters.size();
                activeCourse = (int)semesters[activeSem].courses.size() - 1;
            }
        }

        if (IsKeyPressed(KEY_KP_ADD) || IsKeyPressed(KEY_EQUAL)) {  // "+" key (main or numpad)
            semesters[activeSem].courses.push_back(Course{});
            activeCourse = (int)semesters[activeSem].courses.size() - 1;
        }
        if (IsKeyPressed(KEY_MINUS) || IsKeyPressed(KEY_KP_SUBTRACT)) {  // "-" key (main or numpad)
            if ((int)semesters[activeSem].courses.size() > 1) {
                semesters[activeSem].courses.erase(semesters[activeSem].courses.begin() + activeCourse);
                activeCourse = std::max(0, activeCourse - 1);
            }
        }

        if (IsKeyPressed(KEY_N)) {
            semesters.push_back(Semester{});
            semesters.back().courses.push_back(Course{});
            activeSem = (int)semesters.size() - 1;
            activeCourse = 0;
        }
        if (IsKeyPressed(KEY_M)) {
            if ((int)semesters.size() > 1) {
                semesters.erase(semesters.begin() + activeSem);
                activeSem = std::min(activeSem, (int)semesters.size() - 1);
                activeCourse = std::min(activeCourse, (int)semesters[activeSem].courses.size() - 1);
            }
        }

        int ch = GetCharPressed();
        while (ch > 0) {
            if (editingCredit)  AppendNumericChar(semesters[activeSem].courses[activeCourse].credit, ch);
            else                AppendNumericChar(semesters[activeSem].courses[activeCourse].grade,  ch);
            ch = GetCharPressed();
        }
        if (IsKeyPressed(KEY_BACKSPACE)) {
            std::string& field = editingCredit
                ? semesters[activeSem].courses[activeCourse].credit
                : semesters[activeSem].courses[activeCourse].grade;
            if (!field.empty()) field.pop_back();
        }

        if (IsKeyPressed(KEY_ENTER)) {
            hasResults = true;
            lastGPA.assign(semesters.size(), 0.0f);
            float cgCredits = 0.0f, cgPoints = 0.0f;

            for (int s = 0; s < (int)semesters.size(); ++s) {
                float semCr = 0.0f, semPts = 0.0f;
                for (const auto& c : semesters[s].courses) {
                    float cr, gr;
                    if (TryParseFloat(c.credit, cr) && TryParseFloat(c.grade, gr)) {
                        semCr  += cr;
                        semPts += cr * gr;
                    }
                }
                lastGPA[s] = (semCr > 0.0f) ? (semPts / semCr) : 0.0f;
                cgCredits += semCr;
                cgPoints  += semPts;
            }
            lastCGPA = (cgCredits > 0.0f) ? (cgPoints / cgCredits) : 0.0f;
        }

        scrollY -= GetMouseWheelMove() * 40.0f; 
        if (IsKeyPressed(KEY_PAGE_DOWN)) scrollY += 200.0f;
        if (IsKeyPressed(KEY_PAGE_UP))   scrollY -= 200.0f;
        if (scrollY < 0.0f) scrollY = 0.0f;

        // ---------- DRAW ----------
        BeginDrawing();
        ClearBackground(RAYWHITE);

        DrawText("CGPA Calculator", 20, 20, 28, DARKBLUE);
        DrawText("Controls: UP/DOWN move | LEFT/RIGHT switch box", 20, 60, 18, DARKGRAY);
        DrawText("+ add course | - remove course", 20, 82, 18, DARKGRAY);
        DrawText("N add sem | M remove sem | Enter calculate", 20, 104, 18, DARKGRAY);

        float y = topPad - scrollY;
        float contentHeight = 0.0f;

        for (const auto& sem : semesters) {
            contentHeight += semHeaderH;
            contentHeight += (float)sem.courses.size() * rowH;
            contentHeight += semFooterGap;
        }
        float viewportHeight = (float)H - topPad - bottomPad;
        float maxScroll = std::max(0.0f, contentHeight - viewportHeight);
        if (scrollY > maxScroll) scrollY = maxScroll;

        for (int s = 0; s < (int)semesters.size(); ++s) {
            DrawText(TextFormat("Semester %d", s + 1), 20, (int)y, 22, BLACK);
            y += semHeaderH;

            float semCr = 0.0f, semPts = 0.0f;
            for (int c = 0; c < (int)semesters[s].courses.size(); ++c) {
                float rowY = y + c * rowH;

                DrawText("Credit:", 40, (int)rowY + 8, 20, BLACK);
                Rectangle creditBox = { 120.0f, rowY, 120.0f, 32.0f };
                DrawRectangleRec(creditBox, LIGHTGRAY);

                DrawText("Grade:", 270, (int)rowY + 8, 20, BLACK);
                Rectangle gradeBox  = { 340.0f, rowY, 120.0f, 32.0f };
                DrawRectangleRec(gradeBox, LIGHTGRAY);

                if (s == activeSem && c == activeCourse) {
                    if (editingCredit) DrawRectangleLinesEx(creditBox, 2, RED);
                    else               DrawRectangleLinesEx(gradeBox, 2, RED);
                }

                DrawText(semesters[s].courses[c].credit.c_str(), creditBox.x + 8, (int)rowY + 7, 20, BLACK);
                DrawText(semesters[s].courses[c].grade.c_str(),  gradeBox.x + 8,  (int)rowY + 7, 20, BLACK);

                float cr, gr;
                bool okCr = TryParseFloat(semesters[s].courses[c].credit, cr);
                bool okGr = TryParseFloat(semesters[s].courses[c].grade,  gr);
                if (!okCr && !semesters[s].courses[c].credit.empty()) {
                    DrawRectangleLinesEx(creditBox, 2, ORANGE);
                }
                if (!okGr && !semesters[s].courses[c].grade.empty()) {
                    DrawRectangleLinesEx(gradeBox, 2, ORANGE);
                }
                if (okCr && okGr) { semCr += cr; semPts += cr * gr; }
            }

            float preview = (semCr > 0.0f) ? (semPts / semCr) : 0.0f;
            float previewY = y + 7;               // baseline of first course row
            DrawText(TextFormat("Preview GPA: %.2f", preview), 500, (int)previewY, 20, DARKGREEN);

            y += (float)semesters[s].courses.size() * rowH;
            y += semFooterGap;
        }

        if (hasResults) {
            int ry = H - 64;
            int x = 20;
            for (int s = 0; s < (int)lastGPA.size(); ++s) {
                DrawText(TextFormat("Sem %d GPA: %.2f", s + 1, lastGPA[s]), x, ry, 20, DARKGREEN);
                x += 180;
            }
            DrawText(TextFormat("Final CGPA: %.2f", lastCGPA), W - 260, ry, 24, DARKPURPLE);
        } else {
            DrawText("Press Enter to calculate SGPA & CGPA", 20, H - 64, 20, GRAY);
        }

        if (contentHeight > viewportHeight + 1.0f) {
            float trackX = (float)W - 16.0f, trackY = topPad, trackW = 8.0f, trackH = viewportHeight;
            DrawRectangle(trackX, trackY, trackW, trackH, Color{230,230,230,255});
            float thumbH = std::max(30.0f, trackH * (viewportHeight / contentHeight));
            float thumbY = trackY + (trackH - thumbH) * (scrollY / maxScroll);
            DrawRectangle(trackX, thumbY, trackW, thumbH, GRAY);
            DrawRectangleLines(trackX, trackY, trackW, trackH, DARKGRAY);
        }

        DrawText("Tip: Grades are grade points (e.g., 7.5/10), not percentages.", 20, H - 34, 18, DARKGRAY);
        EndDrawing();
    }

    CloseWindow();
    return 0;
}