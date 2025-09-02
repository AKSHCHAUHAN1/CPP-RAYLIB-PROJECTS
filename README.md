# 🎯 C++ GUI Projects with Raylib

A collection of interactive GUI applications built with **C++17** and **Raylib**.  
These projects include a **Sudoku Solver**, a **CGPA Calculator**, and a **Login & Registration System**, each with simple form-based interfaces.

---

## 🧩 Sudoku Solver GUI

An interactive Sudoku Solver built with Raylib.  
You can play Sudoku, input numbers, and let the solver automatically solve the puzzle.

### ✨ Features
- Play Sudoku with keyboard inputs (1–9 to enter, 0/Backspace to clear).
- Arrow keys for navigation.
- Automatic Sudoku solving algorithm.
- Clean and responsive grid layout.

### ▶️ How to Use
1. Click on a Sudoku cell to select it.
2. Enter numbers using **1–9**, use **0/Backspace** to clear.
3. Use arrow keys to navigate cells.
4. Press the **Solve button** to auto-complete the puzzle.

### ⚡ Build & Run

#### macOS / Linux
```bash
g++ SudokuGUI.cpp -o sudoku -std=c++17 -I/opt/homebrew/include \
  -L/opt/homebrew/lib -lraylib -framework OpenGL -framework Cocoa -framework IOKit
./sudoku
```

#### Windows (MinGW)
```bash
g++ SudokuGUI.cpp -o sudoku.exe -std=c++17 -lraylib -lopengl32 -lgdi32 -lwinmm
sudoku.exe
```

---

## 🎓 CGPA Calculator GUI

A form-style CGPA Calculator for students.  
Enter your courses, credits, and grades across multiple semesters and calculate both **SGPA** (per semester) and **CGPA** (overall).

### ✨ Features
- Form-based input: enter **Credit** and **Grade** for each course.
- Add/remove courses dynamically (`+` / `-`).
- Add/remove semesters (`N` / `M`).
- Keyboard navigation between input boxes.
- Press **Enter** to calculate results instantly.
- Scroll support for multiple semesters on one page.

### ▶️ How to Use
- Navigate between fields using **Arrow keys**.
- Use `+` / `-` to add or remove courses.
- Use `N` / `M` to add or remove semesters.
- Press **Enter** to calculate SGPA and CGPA.

### ⚡ Build & Run

#### macOS / Linux
```bash
g++ CGPACalculatorGUI.cpp -o cgpa -std=c++17 -I/opt/homebrew/include \
  -L/opt/homebrew/lib -lraylib -framework OpenGL -framework Cocoa -framework IOKit
./cgpa
```

#### Windows (MinGW)
```bash
g++ CGPACalculatorGUI.cpp -o cgpa.exe -std=c++17 -lraylib -lopengl32 -lgdi32 -lwinmm
cgpa.exe
```

---

## 🔐 Login & Registration System GUI

A simple login and registration form GUI.  
Allows users to register with a username and password and then log in with the saved credentials.

### ✨ Features
- **Registration system**: create new user accounts.
- **Login system**: authenticate existing users.
- Secure file storage for credentials (`users.txt`).
- Passwords masked with `*`.
- Switch between **Login** and **Register** using **TAB**.
- Press **Enter** to submit.

### ▶️ How to Use
1. Enter **username** and **password** in the form fields.
2. Press **Enter** to register or login.
3. Press **TAB** to toggle between Register and Login mode.
4. Messages are shown for success, errors, or invalid credentials.

### ⚡ Build & Run

#### macOS / Linux
```bash
g++ LoginSystemGUI.cpp -o login -std=c++17 -I/opt/homebrew/include \
  -L/opt/homebrew/lib -lraylib -framework OpenGL -framework Cocoa -framework IOKit
./login
```

#### Windows (MinGW)
```bash
g++ LoginSystemGUI.cpp -o login.exe -std=c++17 -lraylib -lopengl32 -lgdi32 -lwinmm
login.exe
```

---

## 🛠️ Technologies Used
- **C++17**
- **Raylib** (for graphics & input handling)
- File I/O for persistent storage

---

## 🚀 Getting Started
1. Install Raylib  
   - macOS:  
     ```bash
     brew install raylib
     ```  
   - Windows: Download from [Raylib GitHub Releases](https://github.com/raysan5/raylib/releases) and add to MinGW include/lib paths.  

2. Clone the repository:
   ```bash
   git clone https://github.com/AKSHCHAUHAN1/CPP-RAYLIB-PROJECTS.git
   ```

3. Navigate to the project folder:
   ```bash
   cd CPP-RAYLIB-PROJECTS
   ```

4. Compile and run any project using the commands above.

---

## 📌 Notes
- Ensure **Raylib** is installed on your system.  
- Tested on **macOS** (Apple Silicon) and **Windows (MinGW)**.  
- Each project is independent and can be compiled separately.  