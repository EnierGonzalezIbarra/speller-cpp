
// Implements a dictionary's functionality
#include <string>
#include <functional>
#include <iostream>
#include <fstream>
#include <array>
// #include <algorithm>

// Default dictionary
#define DICTIONARY "dictionaries/large"

// Maximum length for a word
#define LENGTH 45

// Choose number of buckets in hash table
#define N 150000

using namespace std;

// Represents a node in a hash table
class List_Node {
    public:
    string word;
    List_Node* next;

    List_Node(string _word): word(_word) {
         next = nullptr;
    }
    ~List_Node(void) {
        if (next != nullptr) {
            delete next;
        }
    }
    
    bool search(string _word_lower)
    {
        string lower_word = word;
        string lower_target = _word_lower;

        std::transform(lower_word.begin(), lower_word.end(), lower_word.begin(), ::tolower);
        
        // cout << endl;
        // cout << "Comparing: " << lower_word << " & " << lower_target << "\n";
        // cout << "From: " << word << " & " << _word << "\n";

        if (lower_word == lower_target)
        {
            // cout << "Matched. Now returning true\n";
            return true;
        } else if (next == nullptr) {
            // cout << "Any matched. Now returning false\n";
            return false;
        } else {
            // cout << "Not matched. Now looking forward\n";
            return next->search(_word_lower);
        }
    }

    void add(List_Node* new_node)
    {
        if (next == nullptr) {
            next = new_node;
        } else {
            new_node->next = next;    
            next = new_node;
        }
    }
};

// Represents the hash table itself 
class Hash_Table {
    private:
        array<List_Node*, N> table;
        unsigned int word_count = 0;
    public:


    // Loads dictionary into memory, returning true if successful, else false
    Hash_Table(string dictionary)
    {
        table.fill(nullptr);
        // Open the dictionary file
        ifstream dict_file(DICTIONARY, std::ios::in);

        if (!dict_file.is_open()) 
        {
            std::cout << "Unable to open dictionary file";
        } else {
            cout << "Dictionary file is open\n";
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
                List_Node* new_node = new List_Node(word);
                table[place]->add(new_node);
            }
            // At this point there is another word
            word_count++;
        }
        cout << "Closing dictfile\n";
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
        std::transform(lower_target.begin(), lower_target.end(), lower_target.begin(), ::tolower);
        int place = hash_function(lower_target);
        // cout << place << "\n";
        if (table[place] != nullptr) {
            return table[place]->search(lower_target);
        } else {
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


int main(int argc, char* argv[])
{
    // Check for correct number of args
    if (argc != 2 && argc != 3)
    {
        std::cout << "Usage: ./speller [DICTIONARY] text\n";
        return 1;
    }

    // Determine dictionary to use if provided 
    const string dictionary = (argc == 3) ? argv[1] : DICTIONARY;

    Hash_Table table(dictionary);

    // Try to open text
    string text = (argc == 3) ? argv[2] : argv[1];
    ifstream file (text, std::ios::in);
    if (!file.is_open())
    {
        std::cout << "Could not open text" <<  text << "\n";
        return 1;
    } else {
        std:: cout << "Opened text correctly\n";
    }

    // Prepare to report misspellings
    std::cout << "\nMISSPELLED WORDS\n\n";

    // Prepare to spell-check
    int index = 0, misspellings = 0, words = 0;
    string word;

    // Spell-check each word in text
    char c;
    while (file.peek() != EOF)
    {
        file.get(c);
        // Allow only alphabetical characters and apostrophes
        if (isalpha(c) || (c == '\'' && index > 0))
        {
            // Append character to word
            string string_from_c(1, c);
            word.append(string_from_c);
            index++;

            // Ignore alphabetical strings too long to be words
            if (index > LENGTH)
            {
                // Consume remainder of alphabetical string
                while (file.get(c)) 
                {
                    if (!isalpha(c))
                    {
                        break;
                    }
                }

                // Prepare for new word
                index = 0;
            }
        }

        // Ignore words with numbers (like MS Word can)
        else if (isdigit(c))
        {
            // Consume remainder of alphanumeric string
            while (file.get(c)) 
            {
                if (!isalnum(c))
                {
                    break;
                }
            }
            // Prepare for new word
            index = 0;
        }

        // We must have found a whole word
        else if (index > 0)
        {
            // Update counter
            words++;

            // Check word's spelling
            bool misspelled = !table.check(word);

            // Print word if misspelled
            if (misspelled)
            {
                std::cout << word << "\n";
                misspellings++;
            }

            // Prepare for next word
            index = 0;
            word.clear();
        }
    }

    // Check whether there was an error
    if (file.bad())
    {
        file.close();
        std::cout << "Error reading " << text << "\n";
        return 1;
    }

    //
    // // Determine dictionary's size
    // unsigned int n = table.size();
    //
    //
    // // Unload dictionary
    // bool unloaded = table.unload();
    //
    // // Abort if dictionary not unloaded
    // if (!unloaded)
    // {
    //     std::cout << "Could not unload " << dictionary << "\n";
    //     return 1;
    // }
    //
    //
    // // Report results
    // std::cout << "\nWORDS MISSPELLED:     " << misspellings << "\n";
    // std::cout << "WORDS IN DICTIONARY:  " << n << "\n";
    // std::cout << "WORDS IN TEXT:        " << words << "\n";
    //
    // Close text
    file.close();
    // Return Success
    return 0;
}





