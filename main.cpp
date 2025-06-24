#include <bits/stdc++.h>
#include <unordered_map>

using namespace std;

// ANSI Color Codes
const string RED_COLOR = "\033[31m";
const string RESET_COLOR = "\033[0m";

// --- Optimized Trie with memory pooling ---
class TrieNode {
public:
    unordered_map<char, TrieNode*> children;
    bool is_end;

    TrieNode() : is_end(false) {}
};

class OptimizedTrie {
private:
    TrieNode* root;
    vector<TrieNode*> node_pool;

public:
    OptimizedTrie() { 
        root = new TrieNode(); 
        node_pool.reserve(100000); // Pre-allocate memory
    }

    void insert(const string& word) {
        if (word.empty()) return;
        TrieNode* curr = root;
        for (char c : word) {
            if (curr->children.find(c) == curr->children.end()) {
                curr->children[c] = new TrieNode();
                node_pool.push_back(curr->children[c]);
            }
            curr = curr->children[c];
        }
        curr->is_end = true;
    }

    bool search(const string& word) {
        if (word.empty()) return false;
        TrieNode* curr = root;
        for (char c : word) {
            if (curr->children.find(c) == curr->children.end()) {
                return false;
            }
            curr = curr->children[c];
        }
        return curr->is_end;
    }

    // Optimized BK-Tree approach for suggestions
    vector<string> getSuggestions(const string& word, int max_dist = 2) {
        vector<string> suggestions;
        string current = "";
        dfs(root, word, current, max_dist, suggestions);

        // Limit suggestions to avoid performance issues
        if (suggestions.size() > 10) {
            suggestions.resize(10);
        }
        return suggestions;
    }

private:
    void dfs(TrieNode* node, const string& target, string& current, 
             int max_dist, vector<string>& suggestions) {

        if (suggestions.size() >= 10) return; // Early termination

        if (node->is_end && !current.empty()) {
            int dist = editDistance(current, target);
            if (dist <= max_dist) {
                suggestions.push_back(current);
            }
        }

        // Pruning: if current string is already too different, skip
        if (current.length() > target.length() + max_dist) return;

        for (auto& [ch, child] : node->children) {
            current.push_back(ch);
            dfs(child, target, current, max_dist, suggestions);
            current.pop_back();
        }
    }

    // Optimized edit distance with early termination
    int editDistance(const string& s1, const string& s2) {
        int m = s1.length(), n = s2.length();
        if (abs(m - n) > 2) return 3; // Early termination for large differences

        vector<vector<int>> dp(2, vector<int>(n + 1));

        for (int j = 0; j <= n; j++) dp[0][j] = j;

        for (int i = 1; i <= m; i++) {
            int curr = i % 2, prev = 1 - curr;
            dp[curr][0] = i;

            for (int j = 1; j <= n; j++) {
                if (s1[i-1] == s2[j-1]) {
                    dp[curr][j] = dp[prev][j-1];
                } else {
                    dp[curr][j] = 1 + min({dp[prev][j], dp[curr][j-1], dp[prev][j-1]});
                }
            }
        }

        return dp[m % 2][n];
    }

public:
    ~OptimizedTrie() {
        for (TrieNode* node : node_pool) {
            delete node;
        }
        delete root;
    }
};

// Fast string normalization with lookup table
class FastNormalizer {
private:
    static bool lookup_table[256];
    static bool initialized;

public:
    static string normalize(const string& s) {
        if (!initialized) {
            for (int i = 0; i < 256; i++) {
                lookup_table[i] = isalpha(i);
            }
            initialized = true;
        }

        string result;
        result.reserve(s.length());

        for (unsigned char c : s) {
            if (lookup_table[c]) {
                result += tolower(c);
            }
        }
        return result;
    }
};

bool FastNormalizer::lookup_table[256];
bool FastNormalizer::initialized = false;

// --- Singleton Dictionary Manager ---
class DictionaryManager {
private:
    OptimizedTrie* dictionary;
    unordered_set<string> word_set; // For O(1) lookups

    DictionaryManager() : dictionary(new OptimizedTrie()) {
        loadDictionary();
    }

    void loadDictionary() {
        ifstream file("dictionary_111.txt");
        if (!file) {
            cerr << "Error: Cannot open dictionary_111.txt" << endl;
            exit(1);
        }

        string word;
        word_set.reserve(50000); // Pre-allocate hash table

        while (file >> word) {
            string normalized = FastNormalizer::normalize(word);
            if (!normalized.empty()) {
                dictionary->insert(normalized);
                word_set.insert(normalized);
            }
        }
        file.close();
    }

public:
    static DictionaryManager& getInstance() {
        static DictionaryManager instance;
        return instance;
    }

    bool isCorrect(const string& word) {
        return word_set.count(word) > 0; // O(1) lookup
    }

    vector<string> getSuggestions(const string& word) {
        return dictionary->getSuggestions(word, 1);
    }

    ~DictionaryManager() {
        delete dictionary;
    }
};

// Optimized text processor with single-pass parsing
class TextProcessor {
private:
    struct WordInfo {
        string word;
        size_t start_pos;
        size_t end_pos;
        bool is_correct;
    };

    vector<WordInfo> words;
    string original_text;

public:
    void processFile(const string& filename) {
        ifstream file(filename);
        if (!file) {
            cerr << "Error: Cannot open " << filename << endl;
            return;
        }

        // Read entire file at once
        file.seekg(0, ios::end);
        size_t size = file.tellg();
        original_text.resize(size);
        file.seekg(0);
        file.read(&original_text[0], size);
        file.close();

        parseText();
    }

private:
    void parseText() {
        words.reserve(1000);
        DictionaryManager& dict = DictionaryManager::getInstance();

        size_t i = 0;
        while (i < original_text.length()) {
            if (isalpha(original_text[i])) {
                size_t start = i;
                string word;

                // Extract word
                while (i < original_text.length() && isalpha(original_text[i])) {
                    word += original_text[i++];
                }

                string normalized = FastNormalizer::normalize(word);
                bool correct = normalized.empty() || dict.isCorrect(normalized);

                words.push_back({word, start, i, correct});
            } else {
                i++;
            }
        }
    }

public:
    void displayWithHighlights() {
        size_t pos = 0;

        for (const auto& wordInfo : words) {
            // Print text before word
            cout << original_text.substr(pos, wordInfo.start_pos - pos);

            // Print word with highlighting if incorrect
            if (!wordInfo.is_correct) {
                cout << RED_COLOR << wordInfo.word << RESET_COLOR;
            } else {
                cout << wordInfo.word;
            }

            pos = wordInfo.end_pos;
        }

        // Print remaining text
        cout << original_text.substr(pos) << endl;
    }

    void interactiveFix() {
        DictionaryManager& dict = DictionaryManager::getInstance();
        unordered_map<string, string> replacements;

        // Process unique incorrect words only
        unordered_set<string> processed;

        for (const auto& wordInfo : words) {
            if (!wordInfo.is_correct && processed.find(wordInfo.word) == processed.end()) {
                processed.insert(wordInfo.word);

                cout << "\nIncorrect word: " << wordInfo.word << endl;

                string normalized = FastNormalizer::normalize(wordInfo.word);
                vector<string> suggestions = dict.getSuggestions(normalized);

                if (!suggestions.empty()) {
                    cout << "Suggestions:" << endl;
                    for (size_t i = 0; i < suggestions.size(); i++) {
                        cout << i << ". " << suggestions[i] << endl;
                    }
                    cout << "Enter choice (number), 'c' for custom, 'i' to ignore: ";
                } else {
                    cout << "No suggestions found." << endl;
                    cout << "Enter 'c' for custom spelling, 'i' to ignore: ";
                }

                string choice;
                cin >> choice;

                if (choice == "c") {
                    cout << "Enter replacement: ";
                    string replacement;
                    cin >> replacement;
                    replacements[wordInfo.word] = replacement;
                } else if (choice != "i" && !suggestions.empty() && isdigit(choice[0])) {
                    int idx = stoi(choice);
                    if (idx >= 0 && idx < suggestions.size()) {
                        replacements[wordInfo.word] = suggestions[idx];
                    }
                }
            }
        }

        // Apply replacements and save
        saveWithReplacements(replacements);
    }

private:
    void saveWithReplacements(const unordered_map<string, string>& replacements) {
        string result = original_text;
        int offset = 0;

        for (const auto& wordInfo : words) {
            auto it = replacements.find(wordInfo.word);
            if (it != replacements.end()) {
                size_t pos = wordInfo.start_pos + offset;
                result.replace(pos, wordInfo.word.length(), it->second);
                offset += it->second.length() - wordInfo.word.length();
            }
        }

        ofstream outFile("input.txt");
        if (outFile) {
            outFile << result;
            outFile.close();
            cout << "\nCorrected text saved to input.txt (original file overwritten)" << endl;
        } else {
            cout << "\nError: Could not save to input.txt" << endl;
        }
    }
};

int main() {
    TextProcessor processor;
    processor.processFile("input.txt");
    processor.displayWithHighlights();
    processor.interactiveFix();

    return 0;
}
