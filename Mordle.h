/**
 * @file    Mordle.h
 * @author  Mike DeKoker (dekoker.mike@gmail.com)
 * @brief   Declares Mordle; a terminal-based Wordle clone and solver
 *
 * @copyright Copyright (c) 2022 Mike DeKoker
 *
 */
#ifndef swordle__header_included
#define swordle__header_included

#include <string_view>
#include <cstdint>
#include <vector>
#include <string>
#include <random>

class Mordle {
public:

    // -- Construction

    Mordle(const std::string& word_file);

    // -- Methods

    /// Play a game of wordle in the current terminal
    bool TerminalPlay();

    /// Returns total number of words in word list
    size_t GetWordListCount() const noexcept { return m_words.size(); }
    /// Returns a random word from the word list
    const std::string& GetRandomWord() const;
    /// Returns true if given word is in the word list
    bool IsWordInList(const std::string& word) const;

    // Check a guessed word against the secret word
    bool CheckWordGuess(const std::string& secret_word, const std::string& guess_word,
        std::string& result);

    // - Character result codes (res_*)
    static constexpr char res_matched = '!';    ///< Letter is in correct spot
    static constexpr char res_missing = 'x';    ///< Letter is not in word
    static constexpr char res_mislaid = '~';    ///< Letter is in the wrong spot
    static constexpr char res_unknown = ' ';    ///< Letter hasn't been processed yet

    // - RGB color values for colorful guess results (color_*)
    static constexpr uint32_t color_matched = 0x00538D4E;   // Green
    static constexpr uint32_t color_missing = 0x003A3A3C;   // Gray
    static constexpr uint32_t color_mislaid = 0x00B59F3B;   // Yellow

protected:

    /// Display the results of a guess to standard output
    void DisplayGuessResult(const std::string& guess, const std::string& result,
        bool colorful = true);

    /// Returns the string to use when player wins
    std::string_view GetWinExclamatory(int guess_count) const;
    /// Returns the string to use when player loses
    std::string_view GetLoseInsult() const;

    /// Returns our pseudorandom number generator object
    std::mt19937& GetPrngGenerator() const noexcept { return m_prng_gen; }

    /// Data type of our word set; an ordered list of lower case words
    using word_set = std::vector<std::string>;

private:

    word_set                m_words;            ///< Set of all words
    mutable std::mt19937    m_prng_gen;         ///< PRNG generator
};


#endif // ifndef swordle__header_included
