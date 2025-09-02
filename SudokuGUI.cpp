#include <raylib.h>
#include <array>
#include <string>
#include <optional>
#include <random>

static constexpr int N = 9;
using Grid = std::array<std::array<int, N>, N>;

// ------------------- Sudoku Core -------------------
bool HasConflictAt(const Grid &g, int r, int c) {
    int val = g[r][c];
    if (val == 0) return false;
    for (int i = 0; i < N; i++) {
        if (i != c && g[r][i] == val) return true;
    }
    for (int i = 0; i < N; i++) {
        if (i != r && g[i][c] == val) return true;
    }
    int br = (r/3)*3, bc = (c/3)*3;
    for (int i=0; i<3; i++)
        for (int j=0; j<3; j++) {
            int rr = br+i, cc = bc+j;
            if (rr==r && cc==c) continue;
            if (g[rr][cc] == val) return true;
        }
    return false;
}

bool IsValidPlacement(const Grid &g, int r, int c, int val) {
    if (val == 0) return true;
    for (int i=0; i<N; i++) {
        if (g[r][i] == val) return false;
        if (g[i][c] == val) return false;
    }
    int br=(r/3)*3, bc=(c/3)*3;
    for (int i=0;i<3;i++)
        for (int j=0;j<3;j++)
            if (g[br+i][bc+j]==val) return false;
    return true;
}

std::optional<std::pair<int,int>> FindEmpty(const Grid &g) {
    for (int r=0;r<N;r++)
        for (int c=0;c<N;c++)
            if (g[r][c]==0) return std::make_pair(r,c);
    return std::nullopt;
}

bool SolveSudoku(Grid &g) {
    auto empty = FindEmpty(g);
    if (!empty) return true;
    auto [r,c] = *empty;
    for (int num=1; num<=9; num++) {
        if (IsValidPlacement(g,r,c,num)) {
            g[r][c] = num;
            if (SolveSudoku(g)) return true;
            g[r][c] = 0;
        }
    }
    return false;
}

// ------------------- Utilities -------------------
Grid ParsePuzzle(const std::string &s) {
    Grid g{}; for (auto &row:g) row.fill(0);
    int idx=0;
    for (char ch: s) {
        if (ch=='\n'||ch==' '||ch=='\r'||ch=='\t') continue;
        if (idx>=81) break;
        int r=idx/9, c=idx%9;
        if (ch>='1'&&ch<='9') g[r][c]=ch-'0';
        else g[r][c]=0;
        idx++;
    }
    return g;
}

Grid GenerateRandomPuzzle() {
    Grid g{}; for (auto &row: g) row.fill(0);
    std::random_device rd; std::mt19937 gen(rd());
    std::uniform_int_distribution<> dist(1,9);
    // fill diagonal boxes
    for (int k=0;k<9;k+=3) {
        for (int i=0;i<3;i++)
            for (int j=0;j<3;j++) {
                int val;
                do { val=dist(gen);} while(!IsValidPlacement(g,k+i,k+j,val));
                g[k+i][k+j]=val;
            }
    }
    Grid solved=g;
    SolveSudoku(solved);
    std::uniform_int_distribution<> dcell(0,80);
    for (int k=0;k<40;k++) {
        int idx=dcell(gen);
        solved[idx/9][idx%9]=0;
    }
    return solved;
}

// ------------------- GUI State -------------------
struct GUIState {
    Grid grid{};
    Grid given{};
    Grid solution{};
    bool hasSolution=false;
    int selRow=-1, selCol=-1;
};

bool PointInRect(Vector2 p, Rectangle r) {
    return p.x>=r.x && p.x<=r.x+r.width && p.y>=r.y && p.y<=r.y+r.height;
}

// ------------------- GUI Functions -------------------
void DrawGrid(const GUIState &st, Rectangle board) {
    float cellW=board.width/9.0f, cellH=board.height/9.0f;
    DrawRectangleRec(board, RAYWHITE);

    for (int br=0;br<3;br++) for (int bc=0;bc<3;bc++) {
        if ((br+bc)%2==1) {
            Rectangle box{board.x+bc*3*cellW, board.y+br*3*cellH,3*cellW,3*cellH};
            DrawRectangleRec(box, Fade(LIGHTGRAY,0.2f));
        }
    }

    for (int r=0;r<9;r++) for (int c=0;c<9;c++) {
        if (HasConflictAt(st.grid,r,c)) {
            Rectangle cell{board.x+c*cellW, board.y+r*cellH, cellW, cellH};
            DrawRectangleRec(cell, Color{255,200,200,255});
        }
    }

    if (st.selRow>=0&&st.selCol>=0) {
        Rectangle sel{board.x+st.selCol*cellW,board.y+st.selRow*cellH,cellW,cellH};
        DrawRectangleRec(sel, Fade(LIGHTGRAY,0.3f));
    }

    Font f=GetFontDefault();
    for (int r=0;r<9;r++) for (int c=0;c<9;c++) {
        int val=st.grid[r][c]; if (!val) continue;
        std::string s(1,char('0'+val));
        Vector2 sz=MeasureTextEx(f,s.c_str(),28,2);
        float cx=board.x+c*cellW+(cellW-sz.x)/2;
        float cy=board.y+r*cellH+(cellH-sz.y)/2-2;
        Color col=(st.given[r][c]==0)? BLUE: BLACK;
        if (HasConflictAt(st.grid,r,c)) col=RED;
        DrawTextEx(f,s.c_str(),{cx,cy},28,2,col);
    }

    for (int i=0;i<=9;i++) {
        float x=board.x+i*cellW, y=board.y+i*cellH;
        int thick=(i%3==0)?3:1;
        DrawLineEx({x,board.y},{x,board.y+board.height},thick,BLACK);
        DrawLineEx({board.x,y},{board.x+board.width,y},thick,BLACK);
    }
}

void HandleMouse(GUIState &st, Rectangle board) {
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        Vector2 m=GetMousePosition();
        if (PointInRect(m,board)) {
            float cellW=board.width/9.0f, cellH=board.height/9.0f;
            int c=(int)((m.x-board.x)/cellW);
            int r=(int)((m.y-board.y)/cellH);
            st.selRow=r; st.selCol=c;
        }
    }
}

void SetCell(GUIState &st,int r,int c,int val) {
    if (r<0||c<0||r>=9||c>=9) return;
    if (st.given[r][c]!=0) return;
    st.grid[r][c]=val;
    st.hasSolution=false;
}

void HandleKeyboard(GUIState &st) {
    if (st.selRow>=0&&st.selCol>=0) {
        for (int num=1;num<=9;num++) {
            int key=KEY_ZERO+num;
            if (IsKeyPressed(key)) SetCell(st,st.selRow,st.selCol,num);
        }
        if (IsKeyPressed(KEY_ZERO)||IsKeyPressed(KEY_BACKSPACE)||IsKeyPressed(KEY_DELETE))
            SetCell(st,st.selRow,st.selCol,0);
    }
    if (IsKeyPressed(KEY_UP)) st.selRow = st.selRow>0?st.selRow-1:st.selRow;
    if (IsKeyPressed(KEY_DOWN)) st.selRow = st.selRow<8?st.selRow+1:st.selRow;
    if (IsKeyPressed(KEY_LEFT)) st.selCol = st.selCol>0?st.selCol-1:st.selCol;
    if (IsKeyPressed(KEY_RIGHT)) st.selCol = st.selCol<8?st.selCol+1:st.selCol;
}

bool SolveAndSnapshot(GUIState &st) {
    Grid tmp=st.grid;
    if (SolveSudoku(tmp)) {
        st.solution=tmp;
        st.grid=tmp;
        st.hasSolution=true;
        return true;
    }
    st.hasSolution=false;
    return false;
}

void ClearEditable(GUIState &st) {
    for (int r=0;r<9;r++) for (int c=0;c<9;c++)
        if (st.given[r][c]==0) st.grid[r][c]=0;
    st.hasSolution=false;
}

// ------------------- Main -------------------
int main() {
    const char *puzzle=
        "530070000\n"
        "600195000\n"
        "098000060\n"
        "800060003\n"
        "400803001\n"
        "700020006\n"
        "060000280\n"
        "000419005\n"
        "000080079";

    GUIState st{};
    st.grid=ParsePuzzle(puzzle);
    st.given=st.grid;

    const int screenW=600, screenH=820;
    InitWindow(screenW,screenH,"Sudoku");
    SetTargetFPS(60);

    Rectangle board{50,120,500,500}; 

    Rectangle btnNew{50,640,500,40};
    Rectangle btnSolve{50,690,140,40};
    Rectangle btnClear{230,690,140,40};
    Rectangle btnReset{410,690,140,40};

    while (!WindowShouldClose()) {
        HandleMouse(st,board);
        HandleKeyboard(st);

        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            Vector2 m=GetMousePosition();
            if (PointInRect(m,btnNew)) {
                st.grid=GenerateRandomPuzzle();
                st.given=st.grid;
                st.hasSolution=false;
            } else if (PointInRect(m,btnSolve)) {
                SolveAndSnapshot(st);
            } else if (PointInRect(m,btnClear)) {
                ClearEditable(st);
            } else if (PointInRect(m,btnReset)) {
                st.grid=st.given;
                st.hasSolution=false;
            }
        }

        BeginDrawing();
        ClearBackground(Color{245,245,245,255});
        DrawText("Sudoku Solver (Backtracking)",50,15,24,BLACK);
        DrawText("Click a cell, type 1-9 to enter. 0/Backspace to clear.",50,50,18,DARKGRAY);
        DrawText("Use Arrow keys to navigate.",50,75,18,DARKGRAY);

        DrawGrid(st,board);

        auto drawBtn=[&](Rectangle r,const char*lbl){
            Color base=LIGHTGRAY;
            if (PointInRect(GetMousePosition(),r)) base=GRAY;
            DrawRectangleRounded(r,0.2f,6,base);
            DrawRectangleRoundedLines(r,0.2f,6,BLACK);
            int tw=MeasureText(lbl,18);
            DrawText(lbl,(int)(r.x+(r.width-tw)/2),(int)(r.y+(r.height-18)/2),18,BLACK);
        };

        drawBtn(btnNew,"New Puzzle");
        drawBtn(btnSolve,"Solve");
        drawBtn(btnClear,"Clear");
        drawBtn(btnReset,"Reset");

        if (st.hasSolution) {
            DrawText("Solved!",50,750,20,DARKGREEN);
        } else {
            bool full=true, valid=true;
            for (int r=0;r<9;r++) for (int c=0;c<9;c++) {
                if (st.grid[r][c]==0) full=false;
                else if (HasConflictAt(st.grid,r,c)) valid=false;
            }
            if (full && valid) DrawText("Completed (Valid)",50,750,20,DARKGREEN);
            else DrawText("Press Solve or keep entering numbers.",50,750,20,DARKGRAY);
        }
        EndDrawing();
    }

    CloseWindow();
    return 0;
}