#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <fstream>
#include <cctype>
#include <algorithm>

using namespace std;

const string RED_COLOR = "\033[31m";
const string RESET_COLOR = "\033[0m";

class TrieNode {
public:
    unordered_map<char, TrieNode*> children;
    bool is_end;

    TrieNode() : is_end(false) {}
};

class Trie {
private:
    TrieNode* root;

    void deleteNodes(TrieNode* node) {
        if (!node) return;
        for (auto& pair : node->children) {
            deleteNodes(pair.second);
        }
        delete node;
    }

    int editDistance(const string& s1, const string& s2) {
        int m = s1.length();
        int n = s2.length();

        vector<vector<int>> dp(m + 1, vector<int>(n + 1));

        for (int i = 0; i <= m; i++) {
            dp[i][0] = i;
        }
        for (int j = 0; j <= n; j++) {
            dp[0][j] = j;
        }

        for (int i = 1; i <= m; i++) {
            for (int j = 1; j <= n; j++) {
                if (s1[i - 1] == s2[j - 1]) {
                    dp[i][j] = dp[i - 1][j - 1];
                } else {
                    dp[i][j] = 1 + min({dp[i - 1][j], dp[i][j - 1], dp[i - 1][j - 1]});
                }
            }
        }
        return dp[m][n];
    }

    void dfs(TrieNode* node, const string& target, string& current, int max_dist, vector<string>& suggestions) {
        if (node->is_end && !current.empty()) {
            int dist = editDistance(current, target);
            if (dist <= max_dist) {
                suggestions.push_back(current);
            }
        }

        for (auto& pair : node->children) {
            current.push_back(pair.first);
            dfs(pair.second, target, current, max_dist, suggestions);
            current.pop_back();
        }
    }

public:
    Trie() {
        root = new TrieNode();
    }

    ~Trie() {
        deleteNodes(root);
    }

    void insert(const string& word) {
        if (word.empty()) return;
        TrieNode* curr = root;
        for (char c : word) {
            if (curr->children.find(c) == curr->children.end()) {
                curr->children[c] = new TrieNode();
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

    vector<string> getSuggestions(const string& word, int max_dist = 2) {
        vector<string> suggestions;
        string current = "";
        dfs(root, word, current, max_dist, suggestions);

        if (suggestions.size() > 10) {
            suggestions.resize(10);
        }
        return suggestions;
    }
};

class Normalizer {
public:
    static string normalize(const string& s) {
        string result;
        for (char c : s) {
            if (isalpha(c)) {
                result += tolower(c);
            }
        }
        return result;
    }
};

class DictionaryManager {
private:
    Trie* dictionary;

    DictionaryManager() {
        dictionary = new Trie();
        loadDictionary();
    }

    void loadDictionary() {
        ifstream file("dictionary_111.txt");
        if (!file) {
            cerr << "Error: Cannot open dictionary_111.txt" << endl;
            exit(1);
        }

        string word;
        while (file >> word) {
            string normalized = Normalizer::normalize(word);
            if (!normalized.empty()) {
                dictionary->insert(normalized);
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
        return dictionary->search(word);
    }

    vector<string> getSuggestions(const string& word) {
        return dictionary->getSuggestions(word, 1);
    }

    ~DictionaryManager() {
        delete dictionary;
    }
};

class TextProcessor {
private:
    vector<pair<string, bool>> words; 

public:
    void processFile(const string& filename) {
        ifstream file(filename);
        if (!file) {
            cerr << "Error: Cannot open " << filename << endl;
            return;
        }

        DictionaryManager& dict = DictionaryManager::getInstance();
        string word;
        while (file >> word) {
            string normalized = Normalizer::normalize(word);
            bool correct = normalized.empty() || dict.isCorrect(normalized);
            words.push_back({word, correct});
        }
        file.close();
    }

    void displayWithHighlights() {
        for (const auto& word_pair : words) {
            if (!word_pair.second) {
                cout << RED_COLOR << word_pair.first << RESET_COLOR << " ";
            } else {
                cout << word_pair.first << " ";
            }
        }
        cout << endl;
    }

    void interactiveFix(const string& filename) {
        DictionaryManager& dict = DictionaryManager::getInstance();
        unordered_map<string, string> replacements;
        
        for (const auto& word_pair : words) {
            string original_word = word_pair.first;
            bool is_correct = word_pair.second;

            if (!is_correct && replacements.find(original_word) == replacements.end()) {
                cout << "\nIncorrect word: " << original_word << endl;
                string normalized = Normalizer::normalize(original_word);
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
                    replacements[original_word] = replacement;
                } else if (choice != "i" && !suggestions.empty() && isdigit(choice[0])) {
                    int idx = stoi(choice);
                    if (idx >= 0 && idx < suggestions.size()) {
                        replacements[original_word] = suggestions[idx];
                    }
                } else {
                    replacements[original_word] = original_word;
                }
            }
        }
        
        saveWithReplacements(filename, replacements);
    }

    void saveWithReplacements(const string& filename, const unordered_map<string, string>& replacements) {
        ofstream outFile(filename);
        if (!outFile) {
            cout << "\nError: Could not save to " << filename << endl;
            return;
        }

        for (size_t i = 0; i < words.size(); ++i) {
            const auto& word_pair = words[i];
            auto it = replacements.find(word_pair.first);
            if (it != replacements.end()) {
                outFile << it->second;
            } else {
                outFile << word_pair.first;
            }

            if (i < words.size() - 1) {
                outFile << " ";
            }
        }
        outFile.close();
        cout << "\nCorrected text saved to " << filename << endl;
    }
};

int main() {
    string filename = "input.txt";
    TextProcessor processor;

    processor.processFile(filename);
    processor.displayWithHighlights();
    processor.interactiveFix(filename);

    return 0;
}
