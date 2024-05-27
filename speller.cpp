
// Implements a dictionary's functionality
#include <array>
#include <fstream>
#include <functional>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

// Default dictionary
#define DICTIONARY "dictionaries/large"

// Number of buckets in hash table
#define N 150000

using namespace std;

// Represents a node in a hash table
class List_Node
{
  public:
    string word;
    List_Node *next;

    List_Node(string _word) : word(_word)
    {
        next = nullptr;
    }
    ~List_Node(void)
    {
        if (next != nullptr)
        {
            delete next;
        }
    }

    bool search(string _word_lower)
    {
        string lower_word = word;
        string lower_target = _word_lower;

        transform(lower_word.begin(), lower_word.end(), lower_word.begin(), ::tolower);

        if (lower_word == lower_target)
        {
            return true;
        }
        else if (next == nullptr)
        {
            return false;
        }
        else
        {
            return next->search(_word_lower);
        }
    }

    void add(List_Node *new_node)
    {
        if (next == nullptr)
        {
            next = new_node;
        }
        else
        {
            new_node->next = next;
            next = new_node;
        }
    }
};

// Represents the hash table itself
class Hash_Table
{
  public:
    array<List_Node *, N> table;
    unsigned int word_count = 0;
    // Loads dictionary into memory, returning true if successful, else false
    Hash_Table(string dictionary)
    {
        table.fill(nullptr);
        // Open the dictionary file
        ifstream dict_file(DICTIONARY, ios::in);

        if (!dict_file.is_open())
        {
            cout << "Unable to open dictionary file";
        }
        string word = "";

        // Read a word at a time from dictionary file until you reach EOF
        while (dict_file.peek() != EOF)
        {
            getline(dict_file, word);

            // Look for the right place for the word using the hash() function
            int place = hash_function(word);

            // Add the word to the hash table directly if place is empty
            if (table[place] == nullptr)
            {
                table[place] = new List_Node(word);
            }
            // Solve collision if it is not empty
            else
            {
                List_Node *new_node = new List_Node(word);
                table[place]->add(new_node);
            }
            // At this point there is another word
            word_count++;
        }
        dict_file.close();
    }

    ~Hash_Table(void)
    {
        // For every bucket in the hash table
        for (int i = 0; i < N; i++)
        {
            // Signal to free first node. It's destructor will take care of the rest
            delete table[i];
        }
    }

    // Returns true if word is in dictionary, else false
    bool check(const string word)
    {
        // Use a recursive helper function to iterate the corresponding linked list and find a word
        string lower_target = word;
        transform(lower_target.begin(), lower_target.end(), lower_target.begin(), ::tolower);
        int place = hash_function(lower_target);

        if (table[place] != nullptr)
        {
            return table[place]->search(lower_target);
        }
        else
        {
            return false;
        }
    }

    // Returns words loaded into dictionary
    unsigned int size(void)
    {
        return word_count;
    }

    // Hashes word to a number
    unsigned int hash_function(const string word)
    {
        hash<string> myhash;
        int hash_value = myhash(word) % N;

        return hash_value;
    }
};

class Miss
{
  public:
    unsigned int id;
    string misspelled_word;
    unsigned int line_number;
    array<string, 3> suggestions;

    Miss(unsigned int _id, string _misspelled_word, unsigned int _line_number) : id(_id), line_number(_line_number)
    {
        misspelled_word = _misspelled_word;
        transform(misspelled_word.begin(), misspelled_word.end(), misspelled_word.begin(), ::tolower);
    }

    void fill_suggestions(const Hash_Table& table)
    {
        int comparisons = 0;
        int best_score = 0;
        int second_to_best_score = 0;
        int third_to_best_score = 0;

        string best_suggestion = "";
        string second_to_best_suggestion = "";
        string third_to_best_suggestion = "";

        for (int i = 0; i < N; i++)
        {
            if (table.table[i] != nullptr)
            {

                List_Node *list = table.table[i];
                while (list != nullptr)
                {
                    string word = list->word;
                    if (word.length()) {
                        int score = get_similarity(word);
                        comparisons++;

                        if (score > best_score)
                        {
                            best_score = score;
                            best_suggestion = word;
                        }
                        else if (score > second_to_best_score)
                        {
                            second_to_best_score = score;
                            second_to_best_suggestion = word;
                        }
                        else if (score > third_to_best_score)
                        {
                            third_to_best_score = score;
                            third_to_best_suggestion = word;
                        }

                        if (list->next != nullptr)
                        {
                            list = list->next;
                        } else {
                            break;
                        }
                    }
                }
            }
        }

        suggestions[0] = best_suggestion;
        suggestions[1] = second_to_best_suggestion;
        suggestions[2] = third_to_best_suggestion;
    }

    int get_similarity(const string _target_word)
    {

        int score = 0;

        int lenght_difference = misspelled_word.length() - _target_word.length();

        switch (lenght_difference)
        {
        case 0:
            score += 10;
            break;
        case 1:
        case -1:
            score += 5;
            break;
        case 2:
        case -2:
            score += 2;
            break;
        default:
            break;
        }

        int smaller_length =
            (misspelled_word.length() < _target_word.length()) ? misspelled_word.length() : _target_word.length();

        for (int i = 0; i < smaller_length; i++)
        {
            if (misspelled_word[i] == _target_word[i])
            {
                score += 4;
            }
        }

        return score;
    }
};

int main(int argc, char *argv[])
{
    // Check for correct number of args
    if (argc != 2 && argc != 3)
    {
        cout << "Usage: ./speller [DICTIONARY] text\n";
        return 1;
    }

    // Determine dictionary to use if provided
    const string dictionary = (argc == 3) ? argv[1] : DICTIONARY;

    Hash_Table table(dictionary);

    // Try to open text
    string text = (argc == 3) ? argv[2] : argv[1];
    ifstream file(text, ios::in);
    if (!file.is_open())
    {
        cout << "Could not open text" << text << "\n";
        return 1;
    }
    else
    {
        cout << "Opened text correctly\n";
    }

    // Prepare to report misspellings
    cout << "\nMISSPELLED WORDS\n\n";

    // Prepare to spell-check
    vector<Miss> misses;
    int index = 0, misspellings = 0, words = 0;
    string word = "";

    // Spell-check each word in text
    char c;
    string line;
    unsigned int line_number = 0;
    while (file.peek() != EOF)
    {
        getline(file, line, '\n');
        istringstream line_stream(line);
        line_number++;

        while (line_stream.peek() != EOF)
        {
            getline(line_stream, word, ' ');

            bool ignore_word = false;

            if (word.length() == 1)
            {
                ignore_word = true;
            }

            for (int i = 0; i < word.length(); i++)
            {
                char c = word[i];
                if (isdigit(c))
                {
                    ignore_word = true;
                    break;
                }
                if (!isalpha(c))
                {
                    ignore_word = true;
                    break;
                }
            }

            words++;
            if (!ignore_word && word.length() != 0)
            {
                bool misspelled = !table.check(word);

                // Print word if misspelled
                if (misspelled)
                {
                    misspellings++;
                    misses.push_back(Miss(misspellings, word, line_number));
                    cout << setw(5) << misspellings << ": " << setw(20) << word << setw(15)
                              << "Line: " << line_number << "\n";
                }
            }
        }
    }

    // Check whether there was an error
    if (file.bad())
    {
        file.close();
        cout << "Error reading " << text << "\n";
        return 1;
    }

    // Determine dictionary's size
    unsigned int n = table.size();

    // Report results
    cout << "\nWORDS MISSPELLED:     " << misspellings << "\n";
    cout << "WORDS IN DICTIONARY:  " << n << "\n";
    cout << "WORDS IN TEXT:        " << words << "\n";
    cout << "LINES IN TEXT:        " << line_number << "\n";

    while (true)
    {
        cout << endl;
        cout << "Select a misspelling ID from above (First column) to get suggestions, type any letter to exit: ";

            string id_candidate = "";
            int id = -1;

            getline(cin, id_candidate);

            try
            {
                id = stoi(id_candidate) - 1;
            }
            catch (const invalid_argument &e)
            {
                cout << "Closing the program now" << endl;
                break;    
            }
            if (id < 0)
            {
                cout << "Invalid input. ID must be positive. Please try again\n";
                continue;
            }

            if (id < misses.size())
            {
                misses[id].fill_suggestions(table);
            }
            else
            {
                cout << "Invalid input. ID must be smaller than number of misspellings";
                continue;
            }

            cout << endl;
            cout << "Suggestions: " << endl;
            cout << "1. " << misses[id].suggestions[0] << endl;
            cout << "2. " << misses[id].suggestions[1] << endl;
            cout << "3. " << misses[id].suggestions[2] << endl;
        }
    file.close();
    return 0;
}
