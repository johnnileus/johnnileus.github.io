#include <iostream>
#include <fstream>
#include <random>
#include <thread>
#include <math.h>
#include <chrono>
#include <mutex>
#include <unordered_map>
#include <cstdlib>
#include <string>

using namespace std;

static char emptyChar = '.';
static char fullChar = '#';

static int threadCount = 14;

pair<int, int> directions[8] = {
        { -1, -1 }, { 0, -1 }, { 1, -1 },
        { -1,  0 },                { 1,  0 },
        { -1,  1 }, { 0,  1 }, { 1,  1 }};



class Grid {
public:
    bool* grid;
    int w;
    int h;
    int cellCount;

    template <typename T>
    void initGrid(T cols, T rows){
        w = (int) cols;
        h = (int) rows;
        cellCount = w * h;
        //create 2d array in heap;
        grid = new bool[cellCount];
        for (int i = 0; i < h; ++i) {
            for (int j = 0; j < w; ++j) {
                grid[i*w+j] = false;
            }

        }
    }

    void printGrid() {
        system("cls");
        for (int i = 0; i < h; ++i) {
            for (int j = 0; j < w; ++j) {
                grid[i*w + j] ? cout << fullChar : cout << emptyChar;
                cout << " ";
            }
            cout << "|" <<endl;
        }
    }

    void randomiseGrid(int amt) {
        for (int i = 0; i < amt; ++i) {
            while (true) {
                int randX = rand() % w;
                int randY = rand() % h;
                if (!grid[randY*w + randX]) {
                    grid[randY*w + randX] = true;
                    break;
                }
            }
        }
    }

    bool calculateCell(int x, int y) {
        int cellNumber = y*w + x;
        bool currentVal = grid[cellNumber];

        int neighbours = 0;
        for (int i = 0; i < 8; ++i) {
            int newX = x + directions[i].first;
            int newY = y + directions[i].second;
            //if cell is in grid and is full
            if (newX >= 0 && newX < w && newY >= 0 && newY < h && grid[newY*w + newX]) {
                neighbours++;
            }
        }
        if (currentVal) {
            if (neighbours < 2 || neighbours > 3) {
                return false;
            }
            return true;
        } else {
            if (neighbours == 3) {
                return true;
            }
            return false;
        }


    }

    void operator++(int) {
        stepSimulation();
    }

    void stepSimulation() {
        bool* tempGrid = new bool[cellCount];

        for (int i = 0; i < cellCount; ++i) {
            tempGrid[i] = false;
        }




        for (int y = 0; y < h; ++y) {
            for (int x = 0; x < w; ++x) {
                int cellNumber = y*w + x;
                tempGrid[cellNumber] = calculateCell(x, y);
            }
        }


        for (int i = 0; i < cellCount; ++i) {
            grid[i] = tempGrid[i];
        }
        delete tempGrid;

    }

    void loadFromFile(string fileName) {
        delete grid;


        ifstream file(fileName+".txt");
        if (!file.is_open()) {
            cout << "file failed to open";
            return;
        }

        string line;

        getline(file, line);
        w = stoi(line);
        getline(file, line);
        h = stoi(line);

        cellCount = w * h;
        cout << w << "x" << h << endl;
        grid = new bool[cellCount];
        for (int i = 0; i < cellCount; ++i) {
            grid[i] = false;
        }
        char inp;
        for (int y = 0; y < h; ++y) {
            for (int x = 0; x < w; ++x) {
                file.get(inp);
                bool val = false;
                if (inp == '1') {
                    val = true;
                }
                grid[y*w + x] = val;
            }
            file.get(inp); //discard \n
        }



    }

    void saveToFile(string fileName, int seed) {
        ofstream file(fileName+".txt");
        file << w << endl << h << endl << seed << endl;
        for (int y = 0; y < h; ++y) {
            for (int x = 0; x < w; ++x) {
                file << grid[y*w + x];
            }
            file << endl;
        }
    }
private:

};

class BasePattern {
public:
    int w;
    int h;
    int cellCount;
    vector<bool> pattern;

    BasePattern(vector<bool> newPat, int width, int height) {
        pattern = newPat;
        w = width;
        h = height;
        cellCount = w * h;
    }
};

bool matchCoordinate(Grid grid, BasePattern pat, int x, int y) {
    int totalMatches = 0;
    for (int i = 0; i < pat.cellCount; ++i) {

        int patX = i % pat.w;
        int patY = (i - patX) / pat.w;

        int newX = patX + x;
        int newY = patY + y;

        if (newX < grid.w && newY < grid.h) {
            if (grid.grid[newY * grid.w + newX] == pat.pattern[i]) {
                totalMatches++;
            } else { break; }
        }

    }
    if (totalMatches == pat.cellCount) {
        totalMatches = 0;
        for (int a = -1; a < pat.w + 1; ++a) {

            //top row calculations
            bool topRow;
            if (y - 1 < 0) {
                topRow = 0;
            } else {
                topRow = grid.grid[(y - 1) * grid.w + x + a];
            }

            //bottom row calculations
            bool bottomRow;
            if (y + pat.h >= grid.h) {
                bottomRow = 0;
            } else {
                bottomRow = (grid.grid[(y + pat.h) * grid.w + x + a]);
            }

            if (!topRow && !bottomRow) {
                totalMatches++;
            } else {
                break;
            }
        }

        for (int b = 0; b < pat.h; ++b) {
            bool leftCol;
            if (x - 1 < 0) {
                leftCol = 0;
            } else {
                leftCol = grid.grid[(y + b) * grid.w + x - 1];
            }
            bool rightCol;
            if (x - 1 < 0) {
                rightCol = 0;
            } else {
                rightCol = grid.grid[(y + b) * grid.w + x + pat.w];
            }
            if (!leftCol && !rightCol) {
                totalMatches++;
            } else {
                break;
            }
        }

        if (totalMatches == pat.w + 2 + pat.h) {
//            cout << "MATCH: " << x << " " << y << endl;
            return true;
        }
    }
    return false;
}

vector<pair<int,int>>* findInGrid(Grid grid, BasePattern pat) {
    vector<pair<int, int>>* matches = new vector<pair<int,int>>();

    for (int y = 0; y < grid.h -pat.h + 1; ++y) {
        for (int x = 0; x < grid.w - pat.w + 1; ++x) {

            if (matchCoordinate(grid, pat, x, y)) {
                matches->push_back({x, y});
            }

        }
    }

    return matches;
}

class Pattern;

class MatchedPattern {
public:
    int originalx;
    int originaly;
    int x;
    int y;
    int stage;
    int streak;
    bool toDelete;
    Pattern* pattern;

    MatchedPattern(int newx, int newy, int newstage, Pattern* ptrn) {
        x = newx;
        y = newy;
        originalx = newx;
        originaly = newy;
        stage = newstage;
        streak = 1;
        pattern = ptrn;
        toDelete = false;
    }

private:
};



class Pattern{
public:
    string Name;
    int period;
    vector<pair<int, int>> offsets;
    vector<BasePattern> patterns;
    Pattern(string name, vector<BasePattern> pats, vector<pair<int,int>> offs) {
        Name = name;
        period = pats.size();
        patterns = pats;
        offsets = offs;
    }

    void search(Grid grid, vector<MatchedPattern> *matchedPats) {
        vector<MatchedPattern>* newPats = new vector<MatchedPattern>;
        for (int i = 0; i < period; ++i) {
            vector<pair<int, int>>* matches = findInGrid(grid, patterns[i]);
            for (auto match : *matches) {
                newPats->push_back(MatchedPattern(match.first, match.second, i, this));
            }
            delete matches;
        }

        for (auto pat: *newPats) {
            bool pass = true;
            for (auto existingPat: *matchedPats) {
                if (pat.x == existingPat.x || pat.y == existingPat.y) {
                    pass = false;
                }
            }
            if (pass) {
                matchedPats->push_back(pat);
            }
        }
        delete newPats;

    }
};



vector<bool> rotateVector90(vector<bool> inp, int w, int h){
    vector<bool> arr;
    for (int i = 0; i < inp.size(); ++i) {
        int oldx = i % w;
        int oldy = i / w;

        int newx =  oldy;
        int newy = w - 1 - oldx;

        int newIndex = newy * h + newx;
        arr.push_back(inp[newIndex]);
    }
    return arr;
}

vector<bool> rotateVector180(vector<bool> inp, int w, int h){
    vector<bool> arr;
    for (int i = 0; i < inp.size(); ++i) {
        int oldx = i % w;
        int oldy = i / w;

        int newx = w - 1 - oldx;
        int newy = h - 1 - oldy;

        int newIndex = newy * w + newx;
        arr.push_back(inp[newIndex]);
    }
    return arr;
}

vector<bool> rotateVector270(vector<bool> inp, int w, int h){
    vector<bool> arr;
    for (int i = 0; i < inp.size(); ++i) {
        int oldx = i % w;
        int oldy = i / w;

        int newx = h - 1 - oldy;
        int newy = oldx;

        int newIndex = newy * h + newx;
        arr.push_back(inp[newIndex]);
    }
    return arr;
}



// ---------- you're now entering the spaghetti zone -----------

//block
vector<bool> arr1 = {1,1,1,1};
vector<pair<int,int>> offsets1 = {{0,0}};
BasePattern block1 = BasePattern(arr1, 2, 2);
Pattern block = Pattern("BLOCK", {block1}, offsets1);

//behive
vector<bool> arr2 = {0,1,0,
       1,0,1,
       1,0,1,
       0,1,0};
BasePattern beehiveNS1 = BasePattern(arr2, 3, 4);
BasePattern beehiveEW1 = BasePattern(rotateVector90(arr2, 3, 4), 4,3);
Pattern beehiveNS = Pattern("BEEHIVE", {beehiveNS1}, {{0,0}});
Pattern beehiveEW = Pattern("BEEHIVE", {beehiveEW1}, {{0,0}});

//toad
vector<bool> arr3 = {0,1,1,1,1,1,1,0};
BasePattern toadN1 = BasePattern(arr3, 4,2);
BasePattern toadE1 = BasePattern(rotateVector90(arr3, 4, 2), 2, 4);
vector<bool> arr4 = {0,0,1,0,1,0,0,1,1,0,0,1,0,1,0,0};
BasePattern toadN2 = BasePattern(arr4, 4,4);
BasePattern toadE2 = BasePattern(rotateVector90(arr4, 4, 4), 4, 4);
Pattern toadN = Pattern("TOAD", {toadN1, toadN2}, {{0,1}, {0,-1}});
Pattern toadE = Pattern("TOAD", {toadE1, toadE2}, {{1,0}, {-1,1}});



//blinker
vector<bool> arr5 = {1,1,1};
vector<pair<int,int>> offsets2 = {{1,-1}, {-1, 1}};
Pattern blinker = Pattern("BLINKER", {BasePattern(arr5, 1, 3),
                                      BasePattern(arr5, 3, 1)}, offsets2);



//glider
vector<bool> arr6 = {0,1,0,0,0,1,1,1,1};
BasePattern gliderSE1 = BasePattern(arr6, 3, 3);
BasePattern gliderSW1 = BasePattern(rotateVector90(arr6, 3, 3), 3, 3);
BasePattern gliderNW1 = BasePattern(rotateVector180(arr6, 3, 3), 3, 3);
BasePattern gliderNE1 = BasePattern(rotateVector270(arr6, 3, 3), 3, 3);
vector<bool> arr7 = {1,0,1,0,1,1,0,1,0};
BasePattern gliderSE2 = BasePattern(arr7, 3, 3);
BasePattern gliderSW2 = BasePattern(rotateVector90(arr7, 3, 3), 3, 3);
BasePattern gliderNW2 = BasePattern(rotateVector180(arr7, 3, 3), 3, 3);
BasePattern gliderNE2 = BasePattern(rotateVector270(arr7, 3, 3), 3, 3);
vector<bool> arr8 = {0,0,1,1,0,1,0,1,1};
BasePattern gliderSE3 = BasePattern(arr8, 3, 3);
BasePattern gliderSW3 = BasePattern(rotateVector90(arr8, 3, 3), 3, 3);
BasePattern gliderNW3 = BasePattern(rotateVector180(arr8, 3, 3), 3, 3);
BasePattern gliderNE3 = BasePattern(rotateVector270(arr8, 3, 3), 3, 3);
vector<bool> arr9 = {1,0,0,0,1,1,1,1,0};
BasePattern gliderSE4 = BasePattern(arr9, 3, 3);
BasePattern gliderSW4 = BasePattern(rotateVector90(arr9, 3, 3), 3, 3);
BasePattern gliderNW4 = BasePattern(rotateVector180(arr9, 3, 3), 3, 3);
BasePattern gliderNE4 = BasePattern(rotateVector270(arr9, 3, 3), 3, 3);
vector<pair<int,int>> offsets3 = {{0,0}, {0,1},{0,0},{1,0}};
Pattern gliderSE = Pattern("GLIDER", {gliderSE1, gliderSE2, gliderSE3, gliderSE4}, offsets3);
vector<pair<int,int>> offsets4 = {{0,0}, {-1,0},{0,0},{0,1}};
Pattern gliderSW = Pattern("GLIDER", {gliderSW1, gliderSW2, gliderSW3, gliderSW4}, offsets4);
vector<pair<int,int>> offsets5 = {{0,0}, {0,-1},{0,0},{-1,0}};
Pattern gliderNW = Pattern("GLIDER", {gliderNW1, gliderNW2, gliderNW3, gliderNW4}, offsets5);
vector<pair<int,int>> offsets6 = {{0,0}, {1,0},{0,0},{0,-1}};
Pattern gliderNE = Pattern("GLIDER", {gliderNE1, gliderNE2, gliderNE3, gliderNE4}, offsets6);

//LWSS
vector<bool> arr10 = {0,1,1,0,0,1,1,1,1,0,1,1,0,1,1,0,0,1,1,0};
BasePattern LWSSE1 = BasePattern(arr10, 5,4);
BasePattern LWSSS1 = BasePattern(rotateVector90(arr10, 5,4), 4, 5);
BasePattern LWSSW1 = BasePattern(rotateVector180(arr10, 5,4), 5, 4);
BasePattern LWSSN1 = BasePattern(rotateVector270(arr10, 5,4), 4, 5);
vector<bool> arr11 = {1,0,0,1,0,0,0,0,0,1,1,0,0,0,1,0,1,1,1,1};
BasePattern LWSSE2 = BasePattern(arr11, 5, 4);
BasePattern LWSSS2 = BasePattern(rotateVector90(arr11, 5, 4), 4, 5);
BasePattern LWSSW2 = BasePattern(rotateVector180(arr11, 5, 4), 5, 4);
BasePattern LWSSN2 = BasePattern(rotateVector270(arr11, 5, 4), 4, 5);
vector<bool> arr12 = {0,0,1,1,0,1,1,0,1,1,1,1,1,1,0,0,1,1,0,0};
BasePattern LWSSE3 = BasePattern(arr12, 5, 4);
BasePattern LWSSS3 = BasePattern(rotateVector90(arr12, 5, 4), 4, 5);
BasePattern LWSSW3 = BasePattern(rotateVector180(arr12, 5, 4), 5, 4);
BasePattern LWSSN3 = BasePattern(rotateVector270(arr12, 5, 4), 4, 5);
vector<bool> arr13 = {0,1,1,1,1,1,0,0,0,1,0,0,0,0,1,1,0,0,1,0};
BasePattern LWSSE4 = BasePattern(arr13, 5, 4);
BasePattern LWSSS4 = BasePattern(rotateVector90(arr13, 5, 4), 4, 5);
BasePattern LWSSW4 = BasePattern(rotateVector180(arr13, 5, 4), 5, 4);
BasePattern LWSSN4 = BasePattern(rotateVector270(arr13, 5, 4), 4, 5);

vector<pair<int,int>> offsets7 = {{1,-1}, {0,0}, {1,1}, {0,0}};
Pattern LWSSE = Pattern("LWSS", {LWSSE1, LWSSE2,LWSSE3,LWSSE4}, offsets7);
vector<pair<int,int>> offsets8 = {{1,1}, {0,0}, {-1, 1}, {0,0}};
Pattern LWSSS = Pattern("LWSS", {LWSSS1,LWSSS2,LWSSS3,LWSSS4}, offsets8);
vector<pair<int,int>> offsets9 = {{-1, 1}, {0,0}, {-1, -1}, {0,0}};
Pattern LWSSW = Pattern("LWSS", {LWSSW1,LWSSW2,LWSSW3,LWSSW4}, offsets9);
vector<pair<int,int>> offsets10 = {{-1,-1}, {0,0}, {1, -1},{0,0}};
Pattern LWSSN = Pattern("LWSS", {LWSSN1,LWSSN2,LWSSN3,LWSSN4}, offsets10);


void step(Grid &grid, vector<MatchedPattern> &matchedPatterns) {
    auto t1 = chrono::high_resolution_clock::now();
    grid++;
    for (auto& pat: matchedPatterns) {
        pat.stage = (pat.stage+1) % pat.pattern->period;
    }


    //grid.printGrid();

    //------------------------
    //search for matches
    for (int i = 0; i < matchedPatterns.size(); ++i) {
        MatchedPattern *pat = &matchedPatterns[i];

        pair<int,int> offset = pat->pattern->offsets[pat->stage];

        if (matchCoordinate(grid, pat->pattern->patterns[pat->stage],
                            pat->x + offset.first, pat->y + offset.second)) {
            pat->streak++;
            pat->x += offset.first;
            pat->y += offset.second;
        } else {
            pat->toDelete=true;
        }
    }
    // add new pats to existing pat vector if coordinates are unique

    for (int i = matchedPatterns.size() - 1; i >= 0 ; --i) {
        if (matchedPatterns[i].toDelete) {
            matchedPatterns.erase(matchedPatterns.begin() + i);
        }
    }

    blinker.search(grid, &matchedPatterns);
    block.search(grid, &matchedPatterns);
    beehiveNS.search(grid, &matchedPatterns);
    beehiveEW.search(grid, &matchedPatterns);
    gliderSE.search(grid, &matchedPatterns);
    gliderSW.search(grid, &matchedPatterns);
    gliderNW.search(grid, &matchedPatterns);
    gliderNE.search(grid, &matchedPatterns);
    toadN.search(grid, &matchedPatterns);
    toadE.search(grid, &matchedPatterns);
    LWSSE.search(grid, &matchedPatterns);
    LWSSS.search(grid, &matchedPatterns);
    LWSSW.search(grid, &matchedPatterns);
    LWSSN.search(grid, &matchedPatterns);



}

unordered_map<string, int> ERNMap = {
        {"BLOCK", 0},
        {"BEEHIVE", 1},
        {"BLINKER", 2},
        {"TOAD", 3},
        {"GLIDER", 4},
        {"LWSS", 5},
};


class baseERN{
public:
    mutex mut;
    string patternName;
    int lowestERN = 999999;
    int lowestSeed = 0;
    int lowestWidth = 0;
    int lowestHeight = 0;
    int aliveCells = 0;

    baseERN(string name) {
        patternName = name;
    }

    virtual void displayInfo() {
        cout << patternName << " baseERN: " << lowestERN;
        cout << " | Seed:" << lowestSeed;
        cout << " | Grid width and height:" << lowestWidth<<","<< lowestHeight;
        cout << " | Alive Cells: " << aliveCells << endl;
    }

    virtual void updateValues(int newERN, int newSeed, int newWidth, int newHeight, int newAliveCells) {
        lowestERN = newERN;
        lowestSeed = newSeed;
        lowestWidth = newWidth;
        lowestHeight = newHeight;
        aliveCells = newAliveCells;
    }
};

class ERN : public baseERN {
public:

    int step;

    ERN(string name) : baseERN(name) {

    }

    void displayInfo() override {
        cout << patternName << " baseERN: " << lowestERN;
        cout << " | Seed:" << lowestSeed;
        cout << " | Grid width and height:" << lowestWidth<<","<< lowestHeight;
        cout << " | Step: " << step;
        cout << " | Alive Cells: " << aliveCells << endl;

    }

    void updateStep(int newStep) {
        step = newStep;
    }

};

ERN ERNarr[6] = {ERN("BLOCK"), ERN("BEEHIVE"), ERN("BLINKER"),
                 ERN("TOAD"), ERN("GLIDER"), ERN("LWSS")};

void threadCalculator(int id, int gridCount){
    srand(id);
    for (int i = 0; i < gridCount; ++i) {
        cout << ".";
        Grid grid = Grid();
        int w = rand()%35 + 5;
        int h = rand()%35 + 5;
        int cellCount = w*h;
        int aliveCells = rand()%(cellCount - 10);
        int ERNval = w + h + cellCount;

        grid.initGrid(w, h);
        grid.randomiseGrid(aliveCells);
        vector<MatchedPattern> matchedPatterns;
        for (int j = 0; j < 100; ++j) {
            step(grid, matchedPatterns);

            bool found = false;
            for (auto pat: matchedPatterns) {
                string patName = pat.pattern->Name;
                ERN *temp = &ERNarr[ERNMap[patName]];
                if (temp->lowestERN > ERNval) {
                    lock_guard<mutex> lock(temp->mut);
                    temp->updateValues(ERNval, id, w, h, aliveCells);
                    temp->updateStep(j);
                    cout << endl << "New lowest ERN: " << ERNval << " " << patName << " threadid:" << id << endl;
                }
            }

        }




    }
}

int main() {
    //setup
    int seed = 0;
    srand(seed);
    Grid grid = Grid();

    // if a grid is loaded;
    bool simulating = false;

    vector<MatchedPattern> matchedPatterns;
    while (true) {
        string input;

        int currentStep = 0;

        if (simulating) {
            cout << "------------------------------------------\n";
            cout << "step - step grid by X steps\nsave -> save grid to file\n exit -> exit grid to main menu";
            cout << "cmds: step, save, exit: ";
            cin >> input;
            if (input.substr(0,4) == "exit") {
                simulating = false;
                matchedPatterns.clear();
            }
            else if (input.substr(0,4) == "save") {
                cout << "name of file:";
                string name;
                cin >> name;


                grid.saveToFile(name, seed);
            }
            else if (input.substr(0,4) == "step") {
                int steps;
                cout << "How many steps: ";
                cin >> steps;

                for (int i = 0; i < steps; ++i) {
                    cout <<"Current step: " << currentStep << endl;

                    step(grid, matchedPatterns);

//                    cout << "Pattern count: " << matchedPatterns.size() << endl;
                    bool found = false;
                    grid.printGrid();

                    for (auto pat: matchedPatterns) {
                    cout << "PATTERN " << pat.pattern->Name <<  ", x:" << pat.x << " y:" << pat.y << " streak:" <<pat.streak << " stage:" << pat.stage << endl;
                        if (pat.pattern->Name == "LWSS" && pat.streak > pat.pattern->period) {
                            found = true;
                        }
                    }
                    currentStep++;

                }
            }

        } else {
            cout << "-------------------------------------------------\n";
            cout << "stop -> end program\nexperiments -> show questions 2-4\nnew -> create new grid\nload -> load grid from file";
            cout << "thread -> find lowest ERN using threads\n";
            cout << "--- CMDS: stop, experiments, new, load, thread: ";
            cin >> input;
            cout << endl;
            if (input.substr(0,3) == "new") {
                int x, y, cellAmt;
                cout << "x: ";

                cin >> x;
                cout << "y: ";
                cin >> y;
                cout << "cell#: ";
                cin >> cellAmt;


                grid.initGrid(x, y);
                grid.randomiseGrid(cellAmt);

                grid.printGrid();

                simulating = true;
            }
            else if (input.substr(0,4) == "load") {
                cout << "name of file to load:";
                string name;
                cin >> name;

                grid.loadFromFile(name);
                grid.printGrid();
                simulating = true;
            }
            else if (input.substr(0,4) == "stop") {
                break;
            }
            else if (input.substr(0,11) == "experiments") {

                cout << "Load the following files for the corresponding questions: "<< endl;
                cout << "   (patterns stay alive for longer than their period)" << endl;
                cout << "- QUESTION 2: File name: 'block', block appears at step 5. Took 1 experiment" << endl;
                cout << "- QUESTION 3: File name: 'blinker', blinker appears at step 3. Took 1 experiment" << endl;
                cout << "- QUESTION 4: File name: 'glider', blinker appears at step 67. Took 8 experiments" << endl;
                cout << "File name to load: ";
                string name;
                cin >> name;
                grid.loadFromFile(name);
                grid.printGrid();
                simulating = true;
            }
            else if (input.substr(0,6) == "thread") {
                vector<thread> threads;
                cout << "how many grids?: ";
                int gridCount;
                cin >> gridCount;
                for (int i = 0; i < threadCount; ++i) {
                    threads.push_back(thread(threadCalculator, i, gridCount));
                }
                for (thread& t: threads) {
                    t.join();
                }
                cout << endl;
                for (auto& x :ERNarr) {
                    string name = x.patternName;
                    if (x.lowestERN != 999999) {
                        x.displayInfo();
                    } else {
                        cout << name << " wasnt found." << endl;
                    }
                }
            }
        }


    }


    return 0;
}
