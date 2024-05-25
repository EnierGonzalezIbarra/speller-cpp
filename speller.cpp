
// Implements a dictionary's functionality
#include <string>
#include <functional>
#include <iostream>
#include <fstream>
#include <algorithm>

// Default dictionary
#define DICTIONARY "dictionaries/large"

// Maximum length for a word
#define LENGTH 45

// Choose number of buckets in hash table
#define N 150000

using namespace std;

// Represents a node in a hash table
class node {
    public:
    string word;
    node* next;

};

// Represents the hash table itself 
class hash_table {
    private:
        node *table[N];
        unsigned int word_count = 0;
    public:

    // Returns true if word is in dictionary, else false
    bool check(const string word)
    {
        // Use a recursive helper function to iterate the corresponding linked list and find a word
        return find_word(table[hash_function(word)], word);
    }

    // Loads dictionary into memory, returning true if successful, else false
    bool load_dict(string dictionary)
    {
        // Open the dictionary file
        ifstream dict_file("./dictionaries/large", std::ios::in);

        if (!dict_file.is_open()) 
        {
            std::cout << "Unable to open dictionary file";
            return false;
        }
        string word = "";

        // Read a word at a time from dictionary file until you reach EOF
        while (getline(dict_file, word))
        {
            node *new_node = new node;
            if (new_node == NULL)
            {
                std::cout << "Memory error while loading dict\n";
                dict_file.close();
                return false;
            }

            new_node->word = word;
            new_node->next = NULL;

            // Look for the right place for the word using the hash() function
            int place = hash_function(new_node->word);

            // Add the word to the hash table directly if place is empty
            if (table[place] == NULL)
            {
                table[place] = new_node;
            }
            // Solve collision if it is not empty
            else
            {
                add(&(table[place]), &new_node);
            }
            // At this point there is another word
            word_count++;
        }
        dict_file.close();
        return true;
    }

    // Looks for a word in a bucket
    bool find_word(node *head_ptr, const string word)
    {
        node *ptr = head_ptr;
        while (ptr != NULL)
        {
            node *next = ptr->next;
            string lower_head_ptr = ptr->word;
            string lower_word = word;

            std::transform(lower_head_ptr.begin(), lower_head_ptr.end(), lower_head_ptr.begin(), ::tolower);
            std::transform(lower_word.begin(), lower_word.end(), lower_word.begin(), ::tolower);
            
            if (lower_head_ptr == lower_word)
            {
                return true;
            }
            ptr = next;
        }
        return false;
    }

    // Unloads dictionary from memory, returning true if successful, else false
    bool unload(void)
    {
        // For every bucket in the hash table
        for (int i = 0; i < N; i++)
        {
            // Use a recursive helper function to destroy_linked_node the corresponding linked list
            // and free all nodes from end to start
            destroy_linked_node(table[i]);
            if (i == N - 1)
            {
                return true;
            }
        }
        return false;

    }

    void add(node **head_ptr, node **new_ptr)
    {
        (*new_ptr)->next = *head_ptr;
        *head_ptr = *new_ptr;
    }

    // destroys linked lists looking to add or free nodes
    void destroy_linked_node(node *head_ptr)
    {
        node *ptr = head_ptr;
        while (ptr != NULL)
        {
            node *next = ptr->next;
            delete ptr;
            ptr = next;
        }
    }
    unsigned int size(void)
    {
        return word_count;
    }

    // Hashes word to a number
    unsigned int hash_function(const string word)
    {
        hash<string> myhash;
        int hash_value = ((int) myhash(word)) % 150'000;

        return hash_value;
    }
};


int main(int argc, char* argv[])
{
    hash_table table;
    // Check for correct number of args
    if (argc != 2 && argc != 3)
    {
        std::cout << "Usage: ./speller [DICTIONARY] text\n";
        return 1;
    }

    // Determine dictionary to use if provided 
    const string dictionary = (argc == 3) ? argv[1] : DICTIONARY;

    bool loaded = table.load_dict(dictionary);

    // Exit if dictionary not loaded
    if (!loaded)
    {
        std::cout<< "Could not load" << dictionary << "\n";
        return 1;
    }

    // Try to open text
    string text = (argc == 3) ? argv[2] : argv[1];
    ifstream file (text, std::ios::in);
    if (!file.is_open())
    {
        std::cout << "Could not open" <<  text << "\n";
        table.unload();
        return 1;
    }

    // Prepare to report misspellings
    std::cout << "\nMISSPELLED WORDS\n\n";

    // Prepare to spell-check
    int index = 0, misspellings = 0, words = 0;
    string word;

    // Spell-check each word in text
    char c;
    while (file.good())
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
        }
    }

    // Check whether there was an error
    if (file.bad())
    {
        file.close();
        std::cout << "Error reading " << text << "\n";
        table.unload();
        return 1;
    }

    // Close text
    file.close();

    // Determine dictionary's size
    unsigned int n = table.size();


    // Unload dictionary
    bool unloaded = table.unload();

    // Abort if dictionary not unloaded
    if (!unloaded)
    {
        std::cout << "Could not unload " << dictionary << "\n";
        return 1;
    }


    // Report results
    std::cout << "\nWORDS MISSPELLED:     " << misspellings << "\n";
    std::cout << "WORDS IN DICTIONARY:  " << n << "\n";
    std::cout << "WORDS IN TEXT:        " << words << "\n";

    // Success
    return 0;
}





