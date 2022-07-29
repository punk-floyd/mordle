/**
 * @file    mrdle.h
 * @author  Mike DeKoker (dekoker.mike@gmail.com)
 * @brief   Declares mrdle; a terminal-based Wordle clone and solver
 *
 * @copyright Copyright (c) 2022 Mike DeKoker
 *
 */
#ifndef mrdle__header_included
#define mrdle__header_included

#include <string_view>
#include <cstdint>
#include <vector>
#include <string>
#include <random>

class mrdle {
public:

    /// First item is a guessed word, second item is response string for that guess
    using HintPair = std::pair<std::string, std::string>;
    /// A list of hints
    using HintVect = std::vector<HintPair>;

    // -- Construction

    /// Construct from a path to a word list file
    mrdle(const std::string_view word_file = "");

    // -- Methods

    // - Top level operations

    /// Play a game of wordle in the current terminal
    bool TerminalPlay(std::string secret_word);
    /// List words with optional hints to filter output
    int ListWords(const HintVect& hints = HintVect());

    /// Returns total number of words in word list
    size_t GetWordListCount() const noexcept { return m_words.size(); }
    /// Returns word size
    size_t GetWordSize() const
        { return m_words.empty() ? 0 : m_words[0].length(); }

    /// Returns a random word from the word list
    const std::string& GetRandomWord() const;
    /// Returns true if given word is in the word list
    bool IsWordInList(const std::string& word) const;

    // Check a guessed word against the secret word
    bool CheckWordGuess(const std::string& secret_word, const std::string& guess_word,
        std::string& result);
    // Determine if a word is a possible solution given a hint
    bool CheckWordAgainstHint(const std::string& word, const HintPair& hint);

    void SetNoColorMode(bool no_color) noexcept
        { m_no_color = no_color; }

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

    /// Data type of our word set; an ordered list of lower case words
    using word_list = std::vector<std::string>;
    // Map an alphabet character to its state (res_*)
    using GameCharMap = std::map<char, char>;

    /// Display the results of a guess to standard output
    void DisplayGuessResult(const std::string& guess, const std::string& result,
        const GameCharMap& cmap);

    /// Returns the string to use when player wins
    std::string_view GetWinExclamatory(int guess_count) const;
    /// Returns the string to use when player loses
    std::string_view GetLoseInsult() const;

    /// Returns our pseudorandom number generator object
    std::mt19937& GetPrngGenerator() const noexcept { return m_prng_gen; }

    /// Initialize word list from a file
    void InitWordListFile(std::string_view word_file);
    /// Initialize word list from internal word list
    void InitWordListInternal();

private:

    word_list               m_words;            ///< Set of all words
    mutable std::mt19937    m_prng_gen;         ///< PRNG generator
    bool                    m_no_color{false};  ///< Don't use colorized output
};


#endif // ifndef mrdle__header_included
