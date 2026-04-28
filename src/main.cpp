
// library 
// library khusus raylib
#include "raylib.h"
// library untuk string
#include <string>
// library untuk array dinamis
#include <vector>
// library untuk error handling saat queue dan stack kosong
#include <stdexcept>
#include <iostream>
// library untuk menggabungkan angka dan string
#include <sstream>
#include <algorithm>
#include <functional>

// namespace
using namespace std;

// mengelompokan variabel luar agar rapi
namespace GameConfig {
    // ukuran windows 
    const int SCREEN_W = 1280;
    const int SCREEN_H = 720;
    // Judul
    const char* TITLE = "WHO IS THIS? | Greyford City Detective Bureau";
    // batasi kecepatan gameloop
    const int TARGET_FPS = 60;
    // jumlah suspect character
    const int SUSPECT_COUNT = 5;
    // jumlah suspect terpilih
    const int CORRECT_SUSPECT = 1;

}

// mengelompokan warna grafis yang akan digunakan
namespace Colors {
/*
Color = {R, G, B, A}
R = Red
G = green 
B = blue
A = Alpha = 
    A full = 255
    A transparan = 0
*/ 
    const Color BG_DARK      = {10, 12, 18, 255};
    const Color PANEL_BG     = {18, 22, 34, 255};
    const Color PANEL_BORDER = {40, 55, 80, 255};
    const Color ACCENT_GOLD  = {200, 160, 60, 255};
    const Color ACCENT_RED   = {180, 50, 50, 255};
    const Color TEXT_WHITE   = {230, 228, 220, 255};
    const Color TEXT_GREY    = {130, 128, 120, 255};
    const Color TEXT_DIM     = {70, 68, 65, 255};
    const Color CHECKBOX_ON  = {60, 180, 100, 255};
    const Color CHECKBOX_OFF = {50, 55, 70, 255};
    const Color BTN_NORMAL   = {30, 45, 70, 255};
    const Color BTN_HOVER    = {50, 75, 120, 255};
    const Color BTN_PRESS    = {200, 160, 60, 255};
    const Color HIGHLIGHT    = {255, 220, 80, 30};
    const Color SUSPECT_SEL  = {200, 160, 60, 80};
    const Color CORRECT_CLR  = {40, 160, 80, 255};
    const Color WRONG_CLR    = {180, 40, 40, 255};
}

// ENUM DAN STRUCT
enum class Screen {
    // state layar game
    MAIN_MENU,
    INTRO,
    GAMEPLAY,
    CHOOSE_SUSPECT,
    WIN,
    LOSE,
    CREDITS
};

// data
// isi checklist investigasi 
struct CheckItem {
    // teks pertanyaan
    std::string label;  
    bool checked;  // mengecek apakah player sudah mencentang form nya
    bool correct;  // mengecek apakah ini jawaban yang benar
};

// data suspect character
struct Suspect {
    std::string id;
    std::string name;
    int age;
    std::string gender;
    std::string height; 
    std::string job;
    std::string relation;
    std::string alibi;    
    bool isGuilty;
    std::vector<CheckItem> form;
};

// LINKED LIST
// mengatur alur screen
namespace LinkedList {
    struct Node {
        Screen screen;
        Node* next;
        Node(Screen s) : screen(s), next(nullptr) {}
    };

    /*
    main > intro > game > balik ke awal
    sambungkan screen ke head agar circular
    */

    struct CircularList {
        Node* head;
        Node* current;
        CircularList() : head(nullptr), current(nullptr) {}

        // menambah screen ke list
        void add(Screen s) {
            Node* n = new Node(s);
            if (!head) {
                head = n;
                // node terakhir balik ke awal
                n->next = head;
                current = head;
            } else {
                Node* tmp = head;
                while (tmp->next != head) tmp = tmp->next;
                tmp->next = n;
                n->next = head;
            }
        }

        void next() {
            // pindah ke screen berikutnya
            if (current) current = current->next;
        }

        // ambil screen sekarang
        Screen getCurrent() {
            return current ? current->screen : Screen::MAIN_MENU;
        }

        ~CircularList() {
            if (!head) return;
            Node* cur = head->next;
            while (cur != head) {
                Node* tmp = cur->next;
                delete cur;
                cur = tmp;
            }
            delete head;
        }
    };
}

// STACK
// untuk history navigasi dan tombol back
/*
push: main > intro > game
pop: balik ke intro
*/

namespace StackNS {
    struct Stack {
        // menggunakan vector sebagai stack
        std::vector<Screen> data;
        void push(Screen s) { data.push_back(s); }
        // mengambil screen terakhir lalu hapus
        Screen pop() {
            if (data.empty()) throw std::underflow_error("Stack is empty!");
            Screen s = data.back();
            data.pop_back();
            return s;
        }

        // melihat screen keatas tanpa dihapus
        Screen peek() {
            if (data.empty()) throw std::underflow_error("Stack is empty!");
            return data.back();
        }
        bool empty() { return data.empty(); }
    };
}

// QUEUE
// menyimpan log pesan / event
namespace QueueNS {
    // antrian 
    struct Queue {
        // tempat penyimpanan data
        // vector = array dinamis; string = isi datanya
        std::vector<std::string> data;
        // menambah data antrian dengan memasukan dari belakang
        void enqueue(const std::string& msg ) { // parameter teks yang mau dimasukkan
            //  jumlah isi sekarang dimana cuma 10 data lalu tambah ke belakang, kalau data penuh tidak ditambahkan
            if (data.size() < 10) data.push_back(msg);
        }
        // fungsi untuk mengambil dan menghapus data paling depan 
        std::string dequeue() {
            // cek apakah data kosong, jika iya error (throw)
            if (data.empty()) throw std::underflow_error("Queue is empty!");
            // ambil data paling depan tanpa hapus
            std::string msg = data.front();
            // happus elemen pertama di vector
            data.erase(data.begin());
            return msg;
        }
        // mengecek apakah antrian kosong
        bool empty() { return data.empty(); }
        std::string front() { return data.empty() ? "" : data.front(); }
    };
}

// BINARY TREE 
// menyimpan dan mengelompokkan bukti (eidence)
namespace BinaryTree {
    struct TreeNode {
        std::string evidence;
        std::string category;
        bool pointsToSuspect;
        TreeNode* left;
        TreeNode* right;
        TreeNode(std::string e, std::string c, bool p)
            : evidence(e), category(c), pointsToSuspect(p), left(nullptr), right(nullptr) {}
    };

    struct EvidenceTree {
        TreeNode* root;
        EvidenceTree() : root(nullptr) {}

        void insert(TreeNode* &node, std::string e, std::string c, bool p) {
            if (!node) {
                node = new TreeNode(e, c, p);
            } else if (e < node->evidence) {
                insert(node->left, e, c, p);
            } else {
                insert(node->right, e, c, p);
            }
        }

        // masukkan data ke tree
        void insert(std::string e, std::string c, bool p) {
            insert(root, e, c, p);
        }

        std::vector<std::string> inorder(TreeNode* node) {
            std::vector<std::string> result;
            if (!node) return result;
            // ambil data dari urutan kiri > root > kanan
            auto left = inorder(node->left);
            result.insert(result.end(), left.begin(), left.end());
            result.push_back("[" + node->category + "] " + node->evidence);
            auto right = inorder(node->right);
            result.insert(result.end(), right.begin(), right.end());
            return result;
        }

        std::vector<std::string> getAllEvidence() {
            return inorder(root);
        }

        // menghapus semua node rekursif
        ~EvidenceTree() { destroyTree(root); }
        void destroyTree(TreeNode* node) {
            if (!node) return;
            destroyTree(node->left);
            destroyTree(node->right);
            delete node;
        }
    };
}

// status game pointer based
struct GameState {
    Screen                    currentScreen;
    int                       currentSuspectIndex;
    int                       selectedSuspect;
    bool                      shouldExit;
    float                     timer;
    QueueNS::Queue*           messageLog;
    StackNS::Stack*           navHistory;
    BinaryTree::EvidenceTree* evidenceTree;
    Texture2D                 suspectPhotos[5];
    Texture2D                 fingerprintPhotos[5]; // <-- tambah
    Texture2D                 crimeFingerprint; 
};


// INLINE FUNCTIONS
// meningkatkan performa dan mengurangi overhead
// mengecek apakah mouse sedang berjalan di atas kotak (rectangle)
inline bool IsMouseOverRect(Rectangle rect) {
    return CheckCollisionPointRec(GetMousePosition(), rect);
}

// fungsi untuk menghasilkan warna pertengahan antara A dan B
inline Color Lerp(Color a, Color b, float t) {
    return {
        (unsigned char)(a.r + (b.r - a.r) * t),
        (unsigned char)(a.g + (b.g - a.g) * t),
        (unsigned char)(a.b + (b.b - a.b) * t),
        255
    };
}

// menampilkan teks ditengah layar
inline void DrawTextCentered(const char* text, int y, int fontSize, Color color) {
    // menghitung lebar teks
    int w = MeasureText(text, fontSize);
    DrawText(text, (GameConfig::SCREEN_W - w) / 2, y, fontSize, color);
}

// TEMPLATE FUNCTION
// fungsi generic untuk membandingkan dua nilai
template <typename T>
T MaxValue(T a, T b) {
    return (a > b) ? a : b;
}

// main 
// overloading drawpanel
// menggambar kotak dan kasih border
void DrawPanel(Rectangle rect, Color bg, Color border) {
    DrawRectangleRec(rect, bg);
    DrawRectangleLinesEx(rect, 1.5f, border);
}

// menggambar kotak menggunakan title dan ukuran font
void DrawPanel(Rectangle rect, Color bg, Color border,
               const char* title, int titleSize) {
    DrawRectangleRec(rect, bg);
    DrawRectangleLinesEx(rect, 1.5f, border);
    DrawText(title, (int)rect.x + 12, (int)rect.y + 10, titleSize, Colors::ACCENT_GOLD);
    DrawLine((int)rect.x, (int)rect.y + titleSize + 20,
             (int)(rect.x + rect.width), (int)rect.y + titleSize + 20,
             Colors::PANEL_BORDER);
}

// menggambar kotak dengan icon dan title
void DrawPanel(Rectangle rect, Color bg, Color border,
               const char* title, int titleSize, const char* icon) {
    DrawRectangleRec(rect, bg);
    DrawRectangleLinesEx(rect, 1.5f, border);
    DrawText(icon,  (int)rect.x + 12, (int)rect.y + 10, titleSize, Colors::ACCENT_GOLD);
    int iconW = MeasureText(icon, titleSize);
    DrawText(title, (int)rect.x + 18 + iconW, (int)rect.y + 10, titleSize, Colors::ACCENT_GOLD);
    DrawLine((int)rect.x, (int)rect.y + titleSize + 20,
             (int)(rect.x + rect.width), (int)rect.y + titleSize + 20,
             Colors::PANEL_BORDER);
}

// overloading drawbutton
// menggambar normal draw button
bool DrawButton(Rectangle rect, const char* label, int fontSize) {
    bool  hover   = IsMouseOverRect(rect);
    bool  pressed = hover && IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
    Color bg      = pressed ? Colors::BTN_PRESS
                            : (hover ? Colors::BTN_HOVER : Colors::BTN_NORMAL);
    DrawRectangleRec(rect, bg);
    DrawRectangleLinesEx(rect, 1.5f, Colors::PANEL_BORDER);
    int tw = MeasureText(label, fontSize);
    DrawText(label,
             (int)(rect.x + (rect.width  - tw)      / 2),
             (int)(rect.y + (rect.height - fontSize) / 2),
             fontSize, Colors::TEXT_WHITE);
    return pressed;
}

// menggambar normal draw button dengan accent color ketika warna di klik, border berubah warna
bool DrawButton(Rectangle rect, const char* label, int fontSize, Color accent) {
    // logika untuk mengeklik tombol saat mouse berada di atas tombol
    bool  hover   = IsMouseOverRect(rect);
    bool  pressed = hover && IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
    Color bg      = pressed ? accent
                            : (hover ? Colors::BTN_HOVER : Colors::BTN_NORMAL);
    DrawRectangleRec(rect, bg);
    DrawRectangleLinesEx(rect, 2.0f, accent);
    int   tw     = MeasureText(label, fontSize);
    Color txtClr = hover ? accent : Colors::TEXT_WHITE;
    DrawText(label,
             (int)(rect.x + (rect.width  - tw)      / 2),
             (int)(rect.y + (rect.height - fontSize) / 2),
             fontSize, txtClr);
    return pressed;
}

// CALLBACK FUNCTION
bool DrawButton(Rectangle rect, const char* label, int fontSize, 
                Color accent, std::function<void()> onClick) {
    bool hover   = IsMouseOverRect(rect);
    bool pressed = hover && IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
    if (pressed) onClick(); // callback
    return pressed;
}


// suspect data
std::vector<Suspect> InitSuspects() {
    std::vector<Suspect> s;

    s.push_back({"S-001","Jeremiah",25,"Male","1.8 m","Plumber",
        "No relation to victim",
        "Fixing a pipe leak at the apartment building", false,
        {{"Matches description (male, 170-180cm, brown hair)?", false, true },
         {"Appearance matches witness sketch?",                  false, false},
         {"Fingerprints match crime scene?",                     false, false},
         {"Has connection to victim?",                           false, false},
         {"Alibi is unproven / suspicious?",                    false, false},
         {"Evidence links to suspect?",                         false, false}}});

    s.push_back({"S-002","Ryan",18,"Male","1.76 m","Football Player / Secret Cult Member",
        "Neighbor of victim",
        "Claims traveling - refuses to disclose destination", true,
        {{"Matches description (male, 170-180cm, brown hair)?",   false, true},
         {"Appearance matches witness sketch?",                    false, true},
         {"Fingerprints match crime scene?",                       false, true},
         {"Has connection to victim?",                             false, true},
         {"Alibi is unproven / suspicious?",                      false, true},
         {"Evidence links to suspect?",                           false, true}}});

    s.push_back({"S-003","Chris",19,"Male","1.8 m","University Student",
        "Ex of victim",
        "Returning home after visiting victim", false,
        {{"Matches description (male, 170-180cm, brown hair)?", false, true },
         {"Appearance matches witness sketch?",                  false, true },
         {"Fingerprints match crime scene?",                     false, false},
         {"Has connection to victim?",                           false, true },
         {"Alibi is unproven / suspicious?",                    false, true },
         {"Evidence links to suspect?",                         false, false}}});

    s.push_back({"S-004","Sebastian",19,"Male","1.6 m","Violin Musician",
        "Neighbor of victim",
        "Inside apartment playing violin (heard by neighbors)", false,
        {{"Matches description (male, 170-180cm, brown hair)?", false, true },
         {"Appearance matches witness sketch?",                  false, false},
         {"Fingerprints match crime scene?",                     false, false},
         {"Has connection to victim?",                           false, true },
         {"Alibi is unproven / suspicious?",                    false, false},
         {"Evidence links to suspect?",                         false, false}}});

    s.push_back({"S-005","Scott",27,"Male","1.8 m","Police Officer",
        "No relation to victim",
        "On patrol duty around the apartment complex", false,
        {{"Matches description (male, 170-180cm, brown hair)?", false, true },
         {"Appearance matches witness sketch?",                  false, false},
         {"Fingerprints match crime scene?",                     false, false},
         {"Has connection to victim?",                           false, false},
         {"Alibi is unproven / suspicious?",                    false, false},
         {"Evidence links to suspect?",                         false, false}}});

    return s;
}

// draw helper
void DrawBackground() {
    ClearBackground(Colors::BG_DARK);
    for (int y = 0; y < GameConfig::SCREEN_H; y += 4)
        DrawRectangle(0, y, GameConfig::SCREEN_W, 1, {255, 255, 255, 5});
    DrawRectangleLinesEx(
        {0, 0, (float)GameConfig::SCREEN_W, (float)GameConfig::SCREEN_H},
        3, {40, 55, 80, 120});
}

void DrawHeader(const char* title, const char* subtitle) {
    DrawText("GREYFORD CITY DETECTIVE BUREAU", 20, 12, 12, Colors::TEXT_DIM);
    DrawText("CASE: C-420", GameConfig::SCREEN_W - 130, 12, 12, Colors::TEXT_DIM);
    DrawLine(0, 30, GameConfig::SCREEN_W, 30, Colors::PANEL_BORDER);
    DrawText(title,  20, 40, 22, Colors::ACCENT_GOLD);
    DrawText(subtitle, 20, 66, 14, Colors::TEXT_GREY);
    DrawLine(0, 88, GameConfig::SCREEN_W, 88, Colors::PANEL_BORDER);
}

// avatar: suspect character
void DrawSuspectAvatar(int idx, Texture2D textures[]) {
   Rectangle av = {60, 145, 150, 150}; 
    Texture2D tex = textures[idx];

    float size = (tex.width < tex.height) ? tex.width : tex.height;

    Rectangle src = {
        (tex.width  - size) / 2.0f,
        (tex.height - size) / 2.0f,
        size,
        size
    };

    Rectangle dest = av;

    DrawTexturePro(tex, src, dest, {0,0}, 0.0f, WHITE);
    DrawRectangleLinesEx(av, 2, Colors::PANEL_BORDER);
}

// SCREEN
// 1. main menu
void DrawMainMenu() {
    DrawBackground();
    DrawTextCentered("WHO IS THIS?", 120, 70, Colors::ACCENT_GOLD);
    DrawTextCentered("GREYFORD CITY DETECTIVE BUREAU", 200, 18, Colors::TEXT_GREY);
    DrawTextCentered("CASE C-420  |  THE MURDER OF SHERYL IRIS (19)", 228, 14, Colors::TEXT_DIM);

    DrawLine(200, 310, GameConfig::SCREEN_W - 200, 310, Colors::PANEL_BORDER);
    DrawLine(200, 312, GameConfig::SCREEN_W - 200, 312, Colors::ACCENT_GOLD);
    DrawLine(200, 314, GameConfig::SCREEN_W - 200, 314, Colors::PANEL_BORDER);

    DrawButton({490, 340, 300, 56}, "[ START INVESTIGATION ]", 16, Colors::ACCENT_GOLD);
    DrawButton({490, 412, 300, 56}, "[ CREDITS ]",             16);
    DrawButton({490, 484, 300, 56}, "[ EXIT ]",                16, Colors::ACCENT_RED);

    DrawTextCentered("September 30, 2005  |  Clest Residence Apartments",
                     620, 14, Colors::TEXT_DIM);
}

// 0=none  1=start  2=credits  3=exit
int MainMenuInput() {
    if (IsMouseOverRect({490, 340, 300, 56}) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) return 1;
    if (IsMouseOverRect({490, 412, 300, 56}) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) return 2;
    if (IsMouseOverRect({490, 484, 300, 56}) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) return 3;
    return 0;
}

// 2. intro
void DrawIntro() {
    DrawBackground();
    DrawTextCentered("BRIEFING", 100, 36, Colors::ACCENT_GOLD);

    Rectangle panel = {100, 160, (float)(GameConfig::SCREEN_W - 200), 440};
    DrawPanel(panel, Colors::PANEL_BG, Colors::PANEL_BORDER, "CASE OVERVIEW", 16, "//");

    const char* lines[] = {
        "CASE ID  : C-420",
        "TITLE    : The Murder of Sheryl Iris (19)",
        "DATE     : September 30th, 2005",
        "LOCATION : Clest Residence Apartments, Greyford City",
        "EXACT    : Apartment #402, Kitchen",
        "",
        "On the evening of September 30, 2005, a murder occurred at the",
        "Clest Residence Apartments. The victim was identified as Sheryl",
        "Iris, 19 years old. You are assigned as the lead detective.",
        "",
        "MISSION  : Analyze File A (Case Data) and File B (Suspects),",
        "           fill in the Investigation Form for each of 5 suspects,",
        "           then identify the true killer.",
        "",
        "WEAPON   : Rusty Blunt Knife",
        "EVIDENCE : Blood near knife, symbol-shaped stone chips, fingerprints",
        "WITNESS  : Male, 170-180cm, brown hair, fair skin, dark blue t-shirt,",
        "           visible tattoos, firm chin, high nose. Eyes: brown/hazel/green."
    };

    int y = 210;
    for (const char* line : lines) {
        DrawText(line, 130, y, 14, Colors::TEXT_GREY);
        y += 20;
    }

    DrawButton({(float)(GameConfig::SCREEN_W - 260), (float)(GameConfig::SCREEN_H - 72), 220, 48},
               "BEGIN  >>", 16, Colors::ACCENT_GOLD);
}

bool IntroInput() {
    return IsMouseOverRect({(float)(GameConfig::SCREEN_W - 260),
                            (float)(GameConfig::SCREEN_H - 72), 220, 48})
           && IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
}

// 3. gameplay
void DrawGameplay(const std::vector<Suspect>& suspects, int idx,
                  Texture2D suspectPhotos[],
                  Texture2D fingerprintPhotos[],
                  Texture2D crimeFingerprint) {

    DrawBackground();

    char hdr[80];
    snprintf(hdr, sizeof(hdr), "SUSPECT %d / 5  |  %s  [%s]",
             idx + 1, suspects[idx].name.c_str(), suspects[idx].id.c_str());
    DrawHeader("INVESTIGATION", hdr);

    // pemakaian template
    int panelHeight = MaxValue(580, 580);

    // FILE A - kanan
    DrawPanel({820, 100, 430, 600}, Colors::PANEL_BG, Colors::PANEL_BORDER,
              "FILE A  |  CASE DATA", 14, ">>");

    int ay = 145;
    auto rowA = [&](const char* lbl, const char* val, Color vc = Colors::TEXT_WHITE) {
        DrawText(lbl, 835, ay,      12, Colors::TEXT_DIM);
        DrawText(val, 835, ay + 14, 13, vc);
        ay += 38;
    };
    rowA("CASE ID",   "C-420");
    rowA("TITLE",     "The Murder of Sheryl Iris (19)");
    rowA("DATE",      "September 30, 2005");
    rowA("LOCATION",  "Clest Residence Apt #402, Kitchen");
    rowA("WITNESS", "Male, 170-180cm, brown hair, fair skin,", Colors::ACCENT_GOLD);
    DrawText("dark blue t-shirt, visible tattoos,",           835, ay,      13, Colors::ACCENT_GOLD); // hapus ay-22
    DrawText("firm chin, high nose, brown/hazel/green eyes.", 835, ay + 16, 13, Colors::ACCENT_GOLD); // hapus ay-6
    ay += 36;
    DrawLine(835, ay, 1235, ay, Colors::PANEL_BORDER); ay += 14;
    DrawText("EVIDENCE:",                          835, ay, 13, Colors::TEXT_GREY);  ay += 20;
    DrawText("- Rusty blunt knife with blood",     835, ay, 13, Colors::TEXT_WHITE); ay += 18;
    DrawText("- Symbol-shaped stone chips (cult)", 835, ay, 13, Colors::TEXT_WHITE); ay += 18;
    DrawText("- Fingerprints at scene",            835, ay, 13, Colors::TEXT_WHITE); ay += 18;
    DrawText("- Cult illuminati necklace", 835, ay, 13, Colors::TEXT_WHITE); ay += 24; 

DrawLine(835, ay, 1235, ay, Colors::PANEL_BORDER); ay += 14;
DrawText("CRIME SCENE FINGERPRINT:", 835, ay, 13, Colors::TEXT_GREY); ay += 18;
    Rectangle fpSrc  = {0, 0, (float)crimeFingerprint.width,
                            (float)crimeFingerprint.height};
    Rectangle fpDest = {985, (float)ay + 15, 100, 100};
    DrawTexturePro(crimeFingerprint, fpSrc, fpDest, {0,0}, 0.0f, WHITE);
    DrawRectangleLinesEx(fpDest, 1, Colors::PANEL_BORDER);
    // FILE B - kiri
    DrawPanel({20, 100, 380, 580}, Colors::PANEL_BG, Colors::PANEL_BORDER,
              "FILE B  |  SUSPECT DATA", 14, ">>");

    DrawSuspectAvatar(idx, suspectPhotos);

    int bx = 270, by = 140;
    auto rowB = [&](const char* lbl, const std::string& val) {
        DrawText(lbl,         bx, by,      11, Colors::TEXT_DIM);
        DrawText(val.c_str(), bx, by + 13, 13, Colors::TEXT_WHITE);
        by += 34;
    };
    rowB("ID",         suspects[idx].id);
    rowB("NAME",       suspects[idx].name);
    rowB("AGE",        std::to_string(suspects[idx].age));
    rowB("GENDER",     suspects[idx].gender);
    rowB("HEIGHT",     suspects[idx].height);

    // sidik jari suspect
     int by2 = 305;
    DrawText("RELATION TO VICTIM:", 35, by2, 11, Colors::TEXT_DIM);                     by2 += 16;
    DrawText(suspects[idx].relation.c_str(), 35, by2, 13, Colors::TEXT_WHITE);          by2 += 30;
    DrawText("ALIBI:", 35, by2, 11, Colors::TEXT_DIM);                                  by2 += 16;
    DrawText(suspects[idx].alibi.c_str(), 35, by2, 12, Colors::ACCENT_GOLD);            by2 += 30;

    // OCCUPATION 
    DrawText("OCCUPATION:", 35, by2, 11, Colors::TEXT_DIM);            by2 += 16;
    std::string job = suspects[idx].job;
    size_t slash = job.find(" / ");
    if (slash != std::string::npos) {
        DrawText(job.substr(0, slash).c_str(),           35, by2, 12, Colors::TEXT_WHITE); by2 += 18;
        DrawText(("/ " + job.substr(slash + 3)).c_str(), 35, by2, 12, Colors::TEXT_WHITE); by2 += 24;
    } else {
        DrawText(job.c_str(), 35, by2, 12, Colors::TEXT_WHITE);        by2 += 24;
    }

    // sidik jari suspect - setelah alibi
    DrawLine(35, by2, 385, by2, Colors::PANEL_BORDER);                                  by2 += 14;
    DrawText("SUSPECT FINGERPRINT:", 35, by2, 11, Colors::TEXT_GREY);                   by2 += 16;
    Rectangle sfSrc  = {0, 0, (float)fingerprintPhotos[idx].width,
                               (float)fingerprintPhotos[idx].height};
    Rectangle sfDest = {130, (float)by2 + 30, 150, 150};
    DrawTexturePro(fingerprintPhotos[idx], sfSrc, sfDest, {0,0}, 0.0f, WHITE);
    DrawRectangleLinesEx(sfDest, 1, Colors::PANEL_BORDER);

// Investigation Form - TENGAH (antara File B dan File A)
DrawPanel({410, 100, 390, 580}, Colors::PANEL_BG, Colors::PANEL_BORDER,
          "INVESTIGATION FORM", 14, "?>");

// Nama dan ID suspect di atas form
DrawText(suspects[idx].id.c_str(),   425, 140, 11, Colors::TEXT_DIM);
DrawText(suspects[idx].name.c_str(), 425, 155, 16, Colors::ACCENT_GOLD);
DrawLine(425, 178, 788, 178, Colors::PANEL_BORDER);

// Checklist items
int fy = 190;
for (int i = 0; i < (int)suspects[idx].form.size(); i++) {
    bool      chk = suspects[idx].form[i].checked;
    Rectangle box = {425, (float)fy, 18, 18};
    DrawRectangleRec(box, chk ? Colors::CHECKBOX_ON : Colors::CHECKBOX_OFF);
    DrawRectangleLinesEx(box, 1, Colors::PANEL_BORDER);
    if (chk) DrawText("x", 430, fy + 2, 14, Colors::BG_DARK);
    DrawText(suspects[idx].form[i].label.c_str(), 452, fy + 3, 12,
             chk ? Colors::TEXT_WHITE : Colors::TEXT_GREY);
    fy += 34;
}

// Tombol navigasi
    if (idx < 4)
        DrawButton({(float)(GameConfig::SCREEN_W - 260), (float)(GameConfig::SCREEN_H - 62), 220, 48},
                   "NEXT SUSPECT  >>", 15, Colors::ACCENT_GOLD);
    else
        DrawButton({(float)(GameConfig::SCREEN_W - 260), (float)(GameConfig::SCREEN_H - 62), 240, 48},
                   "CHOOSE SUSPECT  >>", 14, Colors::ACCENT_GOLD);

    if (idx > 0)
        DrawButton({(float)(GameConfig::SCREEN_W - 510), (float)(GameConfig::SCREEN_H - 62), 220, 48},
                   "<<  PREV", 15);
}

// 1=next  -1=prev  0=none
int GameplayInput(std::vector<Suspect>& suspects, int idx) {
    // Checkbox toggle koordinat disesuaikan dengan form di tengah
    int fy = 190;
    for (int i = 0; i < (int)suspects[idx].form.size(); i++) {
        if (IsMouseOverRect({425, (float)fy, 18, 18}) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
            suspects[idx].form[i].checked = !suspects[idx].form[i].checked;
        fy += 34;
    }

    if (IsMouseOverRect({(float)(GameConfig::SCREEN_W - 260), (float)(GameConfig::SCREEN_H - 62), 240, 48})
        && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) return 1;

    if (idx > 0 &&
        IsMouseOverRect({(float)(GameConfig::SCREEN_W - 510), (float)(GameConfig::SCREEN_H - 62), 220, 48})
        && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) return -1;

    return 0;
}

// 4. choose suspect
void DrawChooseSuspect(const std::vector<Suspect>& suspects,
                       int selectedIdx,
                       Texture2D suspectPhotos[]) {
    DrawBackground();
    DrawHeader("CHOOSE YOUR SUSPECT",
               "Select who you believe committed the murder of Sheryl Iris");
    DrawText("Review your investigation forms. Select the suspect you believe is guilty.",
             20, 100, 15, Colors::TEXT_GREY);

    const int cardW  = 200, cardH = 300;
    const int startX = (GameConfig::SCREEN_W - (5 * cardW + 4 * 20)) / 2;
    const int cardY  = 180;

    for (int i = 0; i < 5; i++) {
        int       cx   = startX + i * (cardW + 20);
        Rectangle card = {(float)cx, (float)cardY, (float)cardW, (float)cardH};
        bool      hover = IsMouseOverRect(card);
        bool      sel   = (selectedIdx == i);

        DrawRectangleRec(card, Colors::PANEL_BG);
        DrawRectangleLinesEx(card, sel ? 3.0f : 1.5f,
            sel ? Colors::ACCENT_GOLD : (hover ? Colors::BTN_HOVER : Colors::PANEL_BORDER));
        if (sel)
            DrawRectangleRec({(float)cx + 2, (float)cardY + 2,
                              (float)cardW - 4, (float)cardH - 4},
                             Colors::SUSPECT_SEL);

        // Foto suspect
        float size = (suspectPhotos[i].width < suspectPhotos[i].height)
             ? suspectPhotos[i].width : suspectPhotos[i].height;
        Rectangle src = {
            (suspectPhotos[i].width  - size) / 2.0f,
            (suspectPhotos[i].height - size) / 2.0f,
            size, size
        };
        Rectangle dest = {(float)cx + 50, (float)cardY + 15, 100, 100};
        DrawTexturePro(suspectPhotos[i], src, dest, {0, 0}, 0.0f, WHITE);
        DrawRectangleLinesEx(dest, 2,
            sel ? Colors::ACCENT_GOLD : Colors::PANEL_BORDER);

        DrawText(suspects[i].id.c_str(), cx + 10, cardY + 122, 11, Colors::TEXT_DIM);
        int nw = MeasureText(suspects[i].name.c_str(), 17);
        DrawText(suspects[i].name.c_str(),
                 cx + (cardW - nw) / 2, cardY + 138, 17,
                 sel ? Colors::ACCENT_GOLD : Colors::TEXT_WHITE);
        DrawText(suspects[i].job.c_str(),  cx + 10, cardY + 162, 10, Colors::TEXT_GREY);
        DrawText(("Age: " + std::to_string(suspects[i].age)).c_str(),
                 cx + 10, cardY + 178, 11, Colors::TEXT_GREY);

        int  checked = 0;
        for (auto& f : suspects[i].form) if (f.checked) checked++;
        char cbuf[32];
        snprintf(cbuf, sizeof(cbuf), "Flags: %d / %d", checked, (int)suspects[i].form.size());
        DrawText(cbuf, cx + 10, cardY + 204, 12,
                 checked > 3 ? Colors::ACCENT_RED : Colors::TEXT_DIM);
        if (sel) DrawText("[ SELECTED ]", cx + 10, cardY + 228, 13, Colors::ACCENT_GOLD);
    }

    const char* confirmLabel = selectedIdx >= 0 ? "CONFIRM SUSPECT >>>" : "SELECT A SUSPECT FIRST";
    Color       confirmClr   = selectedIdx >= 0 ? Colors::ACCENT_GOLD  : Colors::TEXT_DIM;
    DrawButton({(float)(GameConfig::SCREEN_W / 2 - 140), (float)(GameConfig::SCREEN_H - 72), 280, 52},
               confirmLabel, 15, confirmClr);
    DrawButton({20, (float)(GameConfig::SCREEN_H - 72), 160, 52}, "<< REVIEW", 15);
}

// -2=back  -1=none  0-4=pilih  100=confirm
int ChooseSuspectInput(int& selectedIdx) {
    const int cardW  = 200, cardH = 300;
    const int startX = (GameConfig::SCREEN_W - (5 * cardW + 4 * 20)) / 2;
    const int cardY  = 180;

    for (int i = 0; i < 5; i++) {
        int cx = startX + i * (cardW + 20);
        if (IsMouseOverRect({(float)cx, (float)cardY, (float)cardW, (float)cardH})
            && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            selectedIdx = i;
            return i;
        }
    }

    if (selectedIdx >= 0 &&
        IsMouseOverRect({(float)(GameConfig::SCREEN_W / 2 - 140),
                         (float)(GameConfig::SCREEN_H - 72), 280, 52})
        && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) return 100;

    if (IsMouseOverRect({20, (float)(GameConfig::SCREEN_H - 72), 160, 52})
        && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) return -2;

    return -1;
}

// 5. screen win
void DrawWin() {
    DrawBackground();
    DrawTextCentered("CASE SOLVED!", 120, 60, Colors::CORRECT_CLR);
    DrawTextCentered("Congratulations, Detective!", 200, 28, Colors::ACCENT_GOLD);

    DrawPanel({200, 260, 880, 290}, Colors::PANEL_BG, Colors::CORRECT_CLR);
    DrawText("You identified RYAN (S-002) as the murderer of Sheryl Iris.", 230, 285, 17, Colors::TEXT_WHITE);
    DrawText("EVIDENCE SUMMARY:", 230, 320, 13, Colors::TEXT_GREY);
    DrawText("- Secret cult member: matches cult stone chip evidence",        230, 344, 13, Colors::TEXT_WHITE);
    DrawText("- Fingerprints matched those found at the crime scene",         230, 362, 13, Colors::TEXT_WHITE);
    DrawText("- Refused to reveal alibi destination (suspicious)",            230, 380, 13, Colors::TEXT_WHITE);
    DrawText("- Neighbor of victim, had access to apartment #402",            230, 398, 13, Colors::TEXT_WHITE);
    DrawText("- Appearance matches witness: tattoos, dark blue t-shirt",      230, 416, 13, Colors::TEXT_WHITE);
    DrawTextCentered("Justice has been served for Sheryl Iris.", 560, 18, Colors::ACCENT_GOLD);

    DrawButton({(float)(GameConfig::SCREEN_W / 2 - 150), (float)(GameConfig::SCREEN_H - 80), 300, 52},
               "RETURN TO MAIN MENU", 16, Colors::CORRECT_CLR);
}

bool WinInput() {
    return IsMouseOverRect({(float)(GameConfig::SCREEN_W / 2 - 150),
                            (float)(GameConfig::SCREEN_H - 80), 300, 52})
           && IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
}

// 6. screen lose
void DrawLose() {
    DrawBackground();
    DrawTextCentered("WRONG SUSPECT!", 120, 60, Colors::WRONG_CLR);
    DrawTextCentered("The Real Criminal Remains Free...", 200, 28, Colors::ACCENT_RED);

    DrawPanel({200, 260, 880, 260}, Colors::PANEL_BG, Colors::WRONG_CLR);
    DrawText("You failed to identify the true murderer.",                      230, 285, 17, Colors::TEXT_WHITE);
    DrawText("The real killer, [----], has escaped justice.",              230, 320, 15, Colors::TEXT_GREY);
    DrawText("Key clues you may have missed:", 230, 358, 13, Colors::TEXT_GREY);
    DrawText("- Secret cult membership links to stone chip evidence",          230, 380, 13, Colors::TEXT_WHITE);
    DrawText("- Ryan refused to reveal alibi destination",                     230, 398, 13, Colors::TEXT_WHITE);
    DrawText("- Fingerprint match and tattoo matching witness description",     230, 416, 13, Colors::TEXT_WHITE);
    DrawTextCentered("Sheryl Iris deserved justice. Try again.", 560, 18, Colors::ACCENT_RED);

    DrawButton({(float)(GameConfig::SCREEN_W / 2 - 150), (float)(GameConfig::SCREEN_H - 80), 300, 52},
               "RETURN TO MAIN MENU", 16, Colors::WRONG_CLR);
}

bool LoseInput() {
    return IsMouseOverRect({(float)(GameConfig::SCREEN_W / 2 - 150),
                            (float)(GameConfig::SCREEN_H - 80), 300, 52})
           && IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
}

// 7. screen edit
void DrawCredits() {
    DrawBackground();
    DrawTextCentered("CREDITS", 100, 50, Colors::ACCENT_GOLD);
    DrawLine(200, 170, GameConfig::SCREEN_W - 200, 170, Colors::PANEL_BORDER);

    DrawPanel({300, 190, 680, 380}, Colors::PANEL_BG, Colors::PANEL_BORDER, "KELOMPOK 7", 18);

    int my = 250;
    DrawText("WHO IS THIS? - Detective Game", 340, my, 16, Colors::TEXT_WHITE);  my += 34;
    DrawLine(340, my, 960, my, Colors::PANEL_BORDER);                             my += 18;
    DrawText("Developed by:", 340, my, 15, Colors::ACCENT_GOLD);        my += 36;

    const char* roles[] = {
        "2510511040 - Farika Aulia Putri",
        "2510511041 - Nur Izzati Zafira",
        "2510511056 - Intan Yusriyaturizki",
        "2510511066 - Adinda Nurul Athiyyah",
        "2510511070 - Byan Azriel"
    };
    for (const char* r : roles) {
        DrawText("o", 340, my, 14, Colors::ACCENT_GOLD);
        DrawText(r,   360, my, 14, Colors::TEXT_GREY);
        my += 28;
    }
    DrawLine(340, my + 8, 960, my + 8, Colors::PANEL_BORDER);
    DrawText("Powered by: C++ | Raylib", 340, my + 22, 13, Colors::TEXT_DIM);

    DrawButton({(float)(GameConfig::SCREEN_W / 2 - 100), (float)(GameConfig::SCREEN_H - 80), 200, 52},
               "<< BACK", 16);
}

bool CreditsInput() {
    return IsMouseOverRect({(float)(GameConfig::SCREEN_W / 2 - 100),
                            (float)(GameConfig::SCREEN_H - 80), 200, 52})
           && IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
}

int main() {
    try {
        InitWindow(GameConfig::SCREEN_W, GameConfig::SCREEN_H, GameConfig::TITLE);
        SetTargetFPS(GameConfig::TARGET_FPS);
        SetExitKey(KEY_NULL);
    } catch (std::exception& e) {
        std::cerr << "Init error: " << e.what() << std::endl;
        return 1;
    }

    // Data suspect + pointer
    std::vector<Suspect> suspects   = InitSuspects();
    Suspect*             suspectPtr = suspects.data();

    // Game state (heap)
    GameState* state              = new GameState();
    state->currentScreen          = Screen::MAIN_MENU;
    state->currentSuspectIndex    = 0;
    state->selectedSuspect        = -1;
    state->shouldExit             = false;
    state->timer                  = 0.0f;
    state->messageLog             = new QueueNS::Queue();
    state->navHistory             = new StackNS::Stack();
    state->evidenceTree           = new BinaryTree::EvidenceTree();

    // Load foto suspect
    state->suspectPhotos[0] = LoadTexture("Assets/Jeremiah.png");
    state->suspectPhotos[1] = LoadTexture("Assets/Ryan.png");
    state->suspectPhotos[2] = LoadTexture("Assets/Chris.png");
    state->suspectPhotos[3] = LoadTexture("Assets/Sebasian.png");
    state->suspectPhotos[4] = LoadTexture("Assets/Scott.png");

    // sidik jari tiap suspect
    state->fingerprintPhotos[0] = LoadTexture("Assets/fp_jeremiah.jpeg");
    state->fingerprintPhotos[1] = LoadTexture("Assets/fp_ryan.jpeg");
    state->fingerprintPhotos[2] = LoadTexture("Assets/fp_chris.jpeg");
    state->fingerprintPhotos[3] = LoadTexture("Assets/fp_sebastian.jpeg");
    state->fingerprintPhotos[4] = LoadTexture("Assets/fp_scott.jpeg");

    // sidik jari TKP
    state->crimeFingerprint = LoadTexture("Assets/fp_crime.jpeg");

    // Binary tree - barang bukti
    state->evidenceTree->insert("Blood near knife",        "Physical",  true);
    state->evidenceTree->insert("Cult stone chips",        "Symbolic",  true);
    state->evidenceTree->insert("Fingerprints",            "Biometric", true);
    state->evidenceTree->insert("Illuminati necklace",     "Symbolic",  true);
    state->evidenceTree->insert("Dark blue t-shirt fiber", "Trace",     true);

    // Circular linked list - alur screen
    LinkedList::CircularList screenFlow;
    screenFlow.add(Screen::MAIN_MENU);
    screenFlow.add(Screen::INTRO);
    screenFlow.add(Screen::GAMEPLAY);
    screenFlow.add(Screen::CHOOSE_SUSPECT);
    screenFlow.add(Screen::WIN);

    state->messageLog->enqueue("Investigation started. Case C-420 opened.");

    // GAME LOOP 
    while (!WindowShouldClose() && !state->shouldExit) {
        state->timer += GetFrameTime();

        BeginDrawing();

        try {
            switch (state->currentScreen) {

                // callback function
                case Screen::MAIN_MENU: {
                DrawMainMenu();

                DrawButton({490, 340, 300, 56}, "[ START INVESTIGATION ]", 16, Colors::ACCENT_GOLD, [&]() {
                    state->navHistory->push(Screen::MAIN_MENU);
                    state->currentScreen = Screen::INTRO;
                    state->currentSuspectIndex = 0;
                    state->selectedSuspect = -1;

                    for (auto& s : suspects)
                        for (auto& f : s.form)
                            f.checked = false;
                });

                DrawButton({490, 412, 300, 56}, "[ CREDITS ]", 16, Colors::ACCENT_GOLD, [&]() {
                    state->navHistory->push(Screen::MAIN_MENU);
                    state->currentScreen = Screen::CREDITS;
                });

                DrawButton({490, 484, 300, 56}, "[ EXIT ]", 16, Colors::ACCENT_RED, [&]() {
                    state->shouldExit = true;
                });

                break;
            }

                case Screen::INTRO: {
                    DrawIntro();
                    if (IntroInput()) {
                        state->navHistory->push(Screen::INTRO);
                        state->currentScreen = Screen::GAMEPLAY;
                        state->messageLog->enqueue("Reviewing suspect 1: Jeremiah");
                    }
                    break;
                }

                case Screen::GAMEPLAY: {
                    DrawGameplay(suspects, state->currentSuspectIndex,
                    state->suspectPhotos,
                    state->fingerprintPhotos,
                    state->crimeFingerprint);
                    int action = GameplayInput(suspects, state->currentSuspectIndex);
                    if (action == 1) {
                        if (state->currentSuspectIndex < 4) {
                            state->currentSuspectIndex++;
                            state->messageLog->enqueue(
                                "Reviewing suspect " +
                                std::to_string(state->currentSuspectIndex + 1) +
                                ": " + suspects[state->currentSuspectIndex].name);
                        } else {
                            state->navHistory->push(Screen::GAMEPLAY);
                            state->currentScreen = Screen::CHOOSE_SUSPECT;
                        }
                    } else if (action == -1 && state->currentSuspectIndex > 0) {
                        state->currentSuspectIndex--;
                    }
                    break;
                }

                case Screen::CHOOSE_SUSPECT: {
                    DrawChooseSuspect(suspects, state->selectedSuspect, state->suspectPhotos);
                    int action = ChooseSuspectInput(state->selectedSuspect);
                    if (action == 100) {
                        state->navHistory->push(Screen::CHOOSE_SUSPECT);
                        bool guilty = (suspectPtr + state->selectedSuspect)->isGuilty;
                        state->currentScreen = guilty ? Screen::WIN : Screen::LOSE;
                    } else if (action == -2) {
                        state->currentScreen       = Screen::GAMEPLAY;
                        state->currentSuspectIndex = 4;
                    }
                    break;
                }

                case Screen::WIN: {
                    DrawWin();
                    if (WinInput()) {
                        state->currentScreen = Screen::MAIN_MENU;
                        state->timer = 0.0f;
                    }
                    break;
                }

                case Screen::LOSE: {
                    DrawLose();
                    if (LoseInput()) {
                        state->currentScreen = Screen::MAIN_MENU;
                        state->timer = 0.0f;
                    }
                    break;
                }

                case Screen::CREDITS: {
                    DrawCredits();
                    if (CreditsInput()) {
                        state->currentScreen = state->navHistory->empty()
                            ? Screen::MAIN_MENU
                            : state->navHistory->pop();
                    }
                    break;
                }

                default:
                    throw std::runtime_error("Unknown screen state!");
            }

        } catch (std::underflow_error&) {
         // stack / queue kosong
        } catch (std::runtime_error& e) {
        std::cerr << "Runtime error: " << e.what() << std::endl;
        state->currentScreen = Screen::MAIN_MENU;
        }

        // ESC = kembali ke layar sebelumnya
        if (IsKeyPressed(KEY_ESCAPE) && state->currentScreen != Screen::MAIN_MENU) {
            try {
                state->currentScreen = state->navHistory->empty()
                    ? Screen::MAIN_MENU
                    : state->navHistory->pop();
            } catch (...) {
                state->currentScreen = Screen::MAIN_MENU;
            }
        }

        EndDrawing();
    }

    // Cleanup foto
    for (int i = 0; i < 5; i++)
        UnloadTexture(state->suspectPhotos[i]);

    for (int i = 0; i < 5; i++)
    UnloadTexture(state->fingerprintPhotos[i]);
    UnloadTexture(state->crimeFingerprint);
    delete state->messageLog;
    delete state->navHistory;
    delete state->evidenceTree;
    delete state;

    // menutup window/screen
    CloseWindow();
    return 0;
}