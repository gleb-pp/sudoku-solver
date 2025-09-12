#include <iostream>
#include <vector>
#include <random>
#include <algorithm>
#include <unordered_set>
#include <unordered_map>
#include <set>

using namespace std;

const int POPULATION_SIZE = 100;
const int MAX_REPEATING_RESULT = 300;
const int CRAZY_MUTATION_INITIAL_PERCENT = 2;
const int CRAZY_MUTATION_MAXIMUM_PERCENT = 40;
// const int REMOVE_WORST_PERCENT = 80;

int crazy_mutation = CRAZY_MUTATION_INITIAL_PERCENT;

default_random_engine generator;

int random_number(int a, int b) {
    uniform_int_distribution<int> distribution(a, b);
    return distribution(generator);
}

struct VectorHasher {
    size_t operator()(const vector<vector<int>>& rows) const {
        size_t hash = 0;
        for (const auto& row : rows) {
            for (int val : row) {
                hash ^= hash * 31 + std::hash<int>{}(val);
            }
        }
        return hash;
    }
};

class SudokuSolver;

class SudokuField {
    friend class SudokuSolver;
private:
    vector<vector<int>> rows;
    vector<vector<int>> grid;
    vector<unordered_set<int>> allowed_in_row;
    static unordered_set<vector<vector<int>>, VectorHasher> rows_checked;

    int generate_random(int row_ind) {
        vector<int> allowed(allowed_in_row[row_ind].begin(), allowed_in_row[row_ind].end());
        uniform_int_distribution<int> distribution(0, allowed.size() - 1);
        int chosen = allowed[distribution(generator)];
        allowed_in_row[row_ind].erase(chosen);
        return chosen;
    }

    void fillInitial() {
        for (int i = 0; i < 9; ++i) {
            for (int j = 0; j < 9; ++j) {
                if (!initial[i][j]) {
                    int new_num = generate_random(i);
                    rows[i][j] = new_num;
                    grid[i / 3 * 3 + j / 3][i % 3 * 3 + j % 3] = new_num;
                }
            }
        }
    }

    bool rows_correct() {
        for (const vector<int>& r : rows) {
            for (const int& elem : r) {
                int elem_res = 0;
                for (const int& el : r) {
                    if (elem == el) {
                        elem_res += 1;
                    }
                }
                if (elem_res > 1) {
                    return false;
                }
            }
        }
        return true;
    }

    int computeFitness() const {
        int f = 0;
        for (int j = 0; j < 9; ++j) {
            int count[9] = {0};
            for (int i = 0; i < 9; ++i) {
                count[rows[i][j] - 1]++;
            }
            for (int k = 0; k < 9; ++k) {
                if (count[k] > 1) {
                    f += (count[k] - 1) * count[k];
                }
            }
        }
        for (const vector<int>& gr : grid) {
            int count[9] = {0};
            for (int i = 0; i < 9; ++i) {
                count[gr[i] - 1]++;
            }
            for (int k = 0; k < 9; ++k) {
                if (count[k] > 1) {
                    f += (count[k] - 1) * count[k];
                }
            }
        }
        return f * f;
    }

    void swap(int i1, int j1, int i2, int j2) {
        int num1 = rows[i1][j1];
        int num2 = rows[i2][j2];
        rows[i1][j1] = num2;
        grid[i1 / 3 * 3 + j1 / 3][i1 % 3 * 3 + j1 % 3] = num2;
        rows[i2][j2] = num1;
        grid[i2 / 3 * 3 + j2 / 3][i2 % 3 * 3 + j2 % 3] = num1;
        fitnessValue = computeFitness();
    }

    void print() const {
        for (const vector<int>& row : rows) {
            for (int elem : row) {
                cout << elem << " ";
            }
            cout << endl;
        }
    }

protected:
    static vector<vector<bool>> initial;
    int fitnessValue;

    // constructor for initially creating the field
    SudokuField(const vector<vector<int>>& r, const vector<vector<int>>& g, const vector<unordered_set<int>>& a) : rows(r), grid(g), allowed_in_row(a) {
        fillInitial();
        fitnessValue = computeFitness();
    }

    // constructor for mating and mutations
    SudokuField(SudokuField& parent1, SudokuField& parent2) {
        rows = vector<vector<int>>(9, vector<int>(9, 0));
        grid = vector<vector<int>>(9, vector<int>(9, 0));

        // combining parents (row from one, row from other)
        bool first = true;
        for (int i = 0; i < 9; ++i) {
            for (int j = 0; j < 9; ++j) {
                int number;
                if (first) {
                    number = parent1.rows[i][j];
                } else {
                    number = parent2.rows[i][j];
                }
                rows[i][j] = number;
                grid[i / 3 * 3 + j / 3][i % 3 * 3 + j % 3] = number;
            }
            first = -first;
        }

        // mutation — swapping in row
        int i = random_number(0, 8);
        int j_1 = random_number(0, 8);
        int j_2 = random_number(0, 8);
        while (initial[i][j_1] || initial[i][j_2] || j_2 == j_1) {
            i = random_number(0, 8);
            j_1 = random_number(0, 8);
            j_2 = random_number(0, 8);
        }
        swap(i, j_1, i, j_2);

        if (random_number(1, 100) <= crazy_mutation) {
            for (int i = 0; i < 9; ++i) {
                vector<int> free_js;
                for (int j = 0; j < 9; ++j) {
                    if (!initial[i][j]) {
                        free_js.push_back(j);
                    }
                }
                shuffle(free_js.begin(), free_js.end(), generator);
                for (int k = 0; k < free_js.size(); k += 2) {
                    if (k + 1 != free_js.size()) swap(i, free_js[k], i, free_js[k + 1]);
                }
            }
        }
    }

    bool check_final() {
        if (fitnessValue == 0) {
            print();
            return true;
        }
        if (rows_checked.find(rows) != rows_checked.end()) return false;
        rows_checked.insert(rows);
        set<pair<int, int>> problems;

        // searching for conflicts in columns
        for (int j = 0; j < 9; ++j) {
            int is[9] = {-1};
            for (int i = 0; i < 9; ++i) {
                if (is[rows[i][j] - 1] >= 0 && !initial[i][j] && !initial[is[rows[i][j] - 1]][j]) {
                    problems.insert({i, j});
                    problems.insert({is[rows[i][j] - 1], j});
                }
                is[rows[i][j] - 1] = i;
            }
        }

        // searching for conflicts in grids
        for (int i = 0; i < 9; ++i) {
            for (int j = 0; j < 9; ++j) {
                for (int is = i / 3 * 3; is < i / 3 * 3 + 1; ++is) {
                    for (int js = j / 3 * 3; js < j / 3 * 3 + 1; ++js) {
                        if ((is != i || js != j) && rows[i][j] == rows[is][js] && !initial[i][j] && !initial[is][js]) {
                            problems.insert({i, j});
                            problems.insert({is, js});
                        }
                    }
                }
            }
        }

        vector<pair<int, int>> problems_vec;
        for (auto& p : problems) {
            problems_vec.push_back(p);
        }

        if (problems_vec.size() < 2) return false;

        // trying to swap these 4 values by pairs to solve the problem
        swap(problems_vec[0].first, problems_vec[0].second, problems_vec[1].first, problems_vec[1].second);
        if (fitnessValue == 0 && rows_correct()) {
            print();
            return true;
        }
        swap(problems_vec[0].first, problems_vec[0].second, problems_vec[1].first, problems_vec[1].second);

        if (problems_vec.size() == 2) return false;

        swap(problems_vec[0].first, problems_vec[0].second, problems_vec[2].first, problems_vec[2].second);
        if (fitnessValue == 0 && rows_correct()) {
            print();
            return true;
        }
        swap(problems_vec[0].first, problems_vec[0].second, problems_vec[2].first, problems_vec[2].second);

        swap(problems_vec[1].first, problems_vec[1].second, problems_vec[2].first, problems_vec[2].second);
        if (fitnessValue == 0 && rows_correct()) {
            print();
            return true;
        }
        swap(problems_vec[1].first, problems_vec[1].second, problems_vec[2].first, problems_vec[2].second);

        if (problems_vec.size() == 3) return false;

        swap(problems_vec[0].first, problems_vec[0].second, problems_vec[3].first, problems_vec[3].second);
        if (fitnessValue == 0 && rows_correct()) {
            print();
            return true;
        }
        swap(problems_vec[0].first, problems_vec[0].second, problems_vec[3].first, problems_vec[3].second);

        swap(problems_vec[1].first, problems_vec[1].second, problems_vec[3].first, problems_vec[3].second);
        if (fitnessValue == 0 && rows_correct()) {
            print();
            return true;
        }
        swap(problems_vec[1].first, problems_vec[1].second, problems_vec[3].first, problems_vec[3].second);

        swap(problems_vec[2].first, problems_vec[2].second, problems_vec[3].first, problems_vec[3].second);
        if (fitnessValue == 0 && rows_correct()) {
            print();
            return true;
        }
        swap(problems_vec[2].first, problems_vec[2].second, problems_vec[3].first, problems_vec[3].second);

        cout << "AAAAAAAAAA" << endl;
        return false;
    }
};

class SudokuSolver {
private:
    vector<vector<int>> rows;
    vector<vector<int>> grid;
    vector<SudokuField> population;
    vector<unordered_set<int>> allowed_in_row;
    int unknown_numbers = 0;

    void createPopulation() {
        population.clear();
        for (int i = 0; i < POPULATION_SIZE; ++i) {
            SudokuField field = SudokuField(rows, grid, allowed_in_row);
            population.push_back(field);
        }
    }

    void makeChildren() {
        shuffle(population.begin(), population.end(), generator);
        for (int i = 0; i < POPULATION_SIZE; i += 2) {
            SudokuField kid = SudokuField(population[i], population[i+1]);
            population.push_back(kid);
        }
    }

    void removeDregs() {
        auto ind = population.begin() + POPULATION_SIZE / 2;
        nth_element(population.begin(), ind, population.end(), [](const SudokuField& a, const SudokuField& b) { return a.fitnessValue > b.fitnessValue; });
        population.erase(population.begin(), ind);
    }

    int min_fitness() {
        int minn = __INT_MAX__;
        for (SudokuField& f : population) {
            if (f.fitnessValue < minn) {
                minn = f.fitnessValue;
                if (minn <= 16 && f.check_final()) {
                    return 0;
                }
            }
        }
        return minn;
    }

    bool preprocessing_grids() {
        bool changed = false;
        vector<vector<set<pair<int, int>>>> info(9, vector<set<pair<int, int>>>(9, set<pair<int, int>>()));
        for (int i = 0; i < 9; ++i) {
            for (int j = 0; j < 9; ++j) {
                if (rows[i][j] == 0) {
                    for (int num = 0; num < 9; ++num) {
                        info[i / 3 * 3 + j / 3][num].insert({i, j});
                    }
                }
            }
        }
        for (int i = 0; i < 9; ++i) {
            for (int j = 0; j < 9; ++j) {
                if (rows[i][j] != 0) {
                    info[i / 3 * 3 + j / 3][rows[i][j] - 1] = {};
                    for (int i1 = 0; i1 < 9; i1++) {
                        info[i1 / 3 * 3 + j / 3][rows[i][j] - 1].erase({i1, j});
                    }
                    for (int j1 = 0; j1 < 9; j1++) {
                        info[i / 3 * 3 + j1 / 3][rows[i][j] - 1].erase({i, j1});
                    }
                }
            }
        }
        for (int gr = 0; gr < 9; ++gr) {
            for (int num = 0; num < 9; ++num) {
                if (info[gr][num].size() == 1) {
                    int x = (*info[gr][num].begin()).first;
                    int y = (*info[gr][num].begin()).second;
                    rows[x][y] = num + 1;
                    grid[x / 3 * 3 + y / 3][x % 3 * 3 + y % 3] = num + 1;
                    allowed_in_row[x].erase(num + 1);
                    SudokuField::initial[x][y] = true;
                    changed = true;
                    --unknown_numbers;
                }
            }
        }
        return changed;
    }

    bool preprocessing_rows() {
        bool changed = false;
        vector<vector<unordered_set<int>>> info(9, vector<unordered_set<int>>(9, unordered_set<int>()));
        for (int i = 0; i < 9; ++i) {
            for (int j = 0; j < 9; ++j) {
                if (rows[i][j] == 0) {
                    for (int num = 0; num < 9; ++num) {
                        info[i][num].insert(j);
                    }
                }
            }
        }
        for (int i = 0; i < 9; ++i) {
            for (int j = 0; j < 9; ++j) {
                if (rows[i][j] != 0) {
                    info[i][rows[i][j] - 1] = {};
                    for (int i1 = 0; i1 < 9; i1++) {
                        info[i1][rows[i][j] - 1].erase(j);
                        for (int j1 = 0; j1 < 9; j1++) {
                            if (i / 3 * 3 + j / 3 == i1 / 3 * 3 + j1 / 3) {
                                info[i1][rows[i][j] - 1].erase(j1);
                            }
                        }
                    }
                }
            }
        }
        for (int row = 0; row < 9; ++row) {
            for (int num = 0; num < 9; ++num) {
                if (info[row][num].size() == 1) {
                    int x = row;
                    int y = *(info[row][num].begin());
                    rows[x][y] = num + 1;
                    grid[x / 3 * 3 + y / 3][x % 3 * 3 + y % 3] = num + 1;
                    allowed_in_row[x].erase(num + 1);
                    SudokuField::initial[x][y] = true;
                    changed = true;
                    --unknown_numbers;
                }
            }
        }
        return changed;
    }

    bool preprocessing_columns() {
        bool changed = false;
        vector<vector<unordered_set<int>>> info(9, vector<unordered_set<int>>(9, unordered_set<int>()));
        for (int i = 0; i < 9; ++i) {
            for (int j = 0; j < 9; ++j) {
                if (rows[i][j] == 0) {
                    for (int num = 0; num < 9; ++num) {
                        info[j][num].insert(i);
                    }
                }
            }
        }
        for (int i = 0; i < 9; ++i) {
            for (int j = 0; j < 9; ++j) {
                if (rows[i][j] != 0) {
                    info[j][rows[i][j] - 1] = {};
                    for (int j1 = 0; j1 < 9; j1++) {
                        info[j1][rows[i][j] - 1].erase(i);
                        for (int i1 = 0; i1 < 9; i1++) {
                            if (i / 3 * 3 + j / 3 == i1 / 3 * 3 + j1 / 3) {
                                info[j1][rows[i][j] - 1].erase(i1);
                            }
                        }
                    }
                }
            }
        }
        for (int col = 0; col < 9; ++col) {
            for (int num = 0; num < 9; ++num) {
                if (info[col][num].size() == 1) {
                    int x = *(info[col][num].begin());
                    int y = col;
                    rows[x][y] = num + 1;
                    grid[x / 3 * 3 + y / 3][x % 3 * 3 + y % 3] = num + 1;
                    allowed_in_row[x].erase(num + 1);
                    SudokuField::initial[x][y] = true;
                    changed = true;
                    --unknown_numbers;
                }
            }
        }
        return changed;
    }

public:
    SudokuSolver(vector<vector<int>>& r, vector<vector<int>>& g, vector<unordered_set<int>>& a) : rows(r), grid(g), allowed_in_row(a) {
        vector<vector<bool>> initial = vector<vector<bool>>(9, vector<bool>(9, true));
        for (int i = 0; i < 9; ++i) {
            for (int j = 0; j < 9; ++j) {
                if (rows[i][j] == 0) {
                    initial[i][j] = false;
                    ++unknown_numbers;
                }
            }
        }
        SudokuField::initial = initial;
    }

    void solve() {
        int iters_without_changing = 0;
        int current_fitness = 0;
        while (preprocessing_grids() || preprocessing_rows() || preprocessing_columns()) {}
        createPopulation();
        if (unknown_numbers == 0) {
            for (const vector<int>& row : rows) {
                for (int elem : row) {
                    cout << elem << " ";
                }
                cout << endl;
            }
            return;
        }
        while (true) {
            makeChildren();  // спаривание + мутации
            removeDregs();   // сортировка и удаление худших

            int min_fit = min_fitness();
            if (min_fit == 0) {
                return;
            } else if (current_fitness == min_fit) {
                iters_without_changing++;
            } else {
                cout << min_fit << endl;
                current_fitness = min_fit;
                crazy_mutation = CRAZY_MUTATION_INITIAL_PERCENT;
                iters_without_changing = 0;
            }
            if (iters_without_changing >= MAX_REPEATING_RESULT) {
                if (crazy_mutation == CRAZY_MUTATION_INITIAL_PERCENT) {
                    crazy_mutation = CRAZY_MUTATION_MAXIMUM_PERCENT;
                } else {
                    crazy_mutation = CRAZY_MUTATION_INITIAL_PERCENT;
                    createPopulation();
                }
                iters_without_changing = 0;
            }
        }
    }
};

vector<vector<bool>> SudokuField::initial(1, vector<bool>(1, false));
unordered_set<vector<vector<int>>, VectorHasher> SudokuField::rows_checked;

int main() {
    freopen("input.txt", "r", stdin);
    // freopen("output.txt", "w", stdout);

    // preparing to handle hyphen instead of number
    cin.exceptions(ios::failbit);
    
    // input processing
    vector<vector<int>> rows(9, vector<int>(9, 0)), grid(9, vector<int>(9, 0));
    vector<unordered_set<int>> allowed_in_row(9, {1, 2, 3, 4, 5, 6, 7, 8, 9});
    vector<vector<unordered_set<int>>> possible(9, vector<unordered_set<int>>(9, {1, 2, 3, 4, 5, 6, 7, 8, 9}));
    
    for (int i = 0; i < 9; ++i) {
        for (int j = 0; j < 9; ++j) {
            try {
                int num;
                cin >> num;
                rows[i][j] = num;
                grid[i / 3 * 3 + j / 3][i % 3 * 3 + j % 3] = num;
                allowed_in_row[i].erase(num);

                // detect if there is a cell with only one possible number
                for (int i1 = 0; i1 < 9; ++i1) {
                    for (int j1 = 0; j1 < 9; ++j1) {
                        if ((i == i1 || j == j1 || i / 3 * 3 + j / 3 == i1 / 3 * 3 + j1 / 3) && !(i == i1 && j == j1)) {
                            possible[i1][j1].erase(num);
                            if (possible[i1][j1].size() == 1) {
                                int num1 = *(possible[i1][j1].begin());
                                rows[i1][j1] = num1;
                                grid[i1 / 3 * 3 + j1 / 3][i1 % 3 * 3 + j1 % 3] = num1;
                                allowed_in_row[i1].erase(num1);
                            }
                        }
                    }
                }
            } catch (const ios_base::failure& e) {
                // handling hyphen instead of number
                cin.clear(); 
            }
        }
    }
    fclose(stdin);

    // creating the class instance, solving, and printing
    SudokuSolver sudoku = SudokuSolver(rows, grid, allowed_in_row);
    sudoku.solve();
    // fclose(stdout);
    return 0;
}
