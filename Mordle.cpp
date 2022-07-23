/**
 * @file    Mordle.cpp
 * @author  Mike DeKoker (dekoker.mike@gmail.com)
 * @brief   Implements Mordle; a terminal-based Wordle clone and solver
 *
 * @copyright Copyright (c) 2022 Mike DeKoker
 *
 */
#include <fmt/format.h>
#include <fmt/ostream.h>
#include <fmt/color.h>
#include <stdexcept>
#include <algorithm>
#include <iostream>
#include <fstream>

#include "Mordle.h"
#include "util.h"

Mordle::Mordle(const std::string_view word_file)
    : m_prng_gen(std::random_device()())
{
    if (word_file.empty())
        InitWordListInternal();
    else
        InitWordListFile(word_file);
}

/// Initialize word list from a file
void Mordle::InitWordListFile(std::string_view word_file)
{
    constexpr size_t alloc_chunk = 25;

    m_words.clear();

    std::ifstream ifs{std::string(word_file)};
    if (!ifs.is_open()) {
        fmt::print(std::cerr, "wordle: Failed to open word file: {}\n", word_file);
        return;
    }

    std::string word;
    size_t word_len = 0;

    while(std::getline(ifs, word)) {

        // Grow vector in spurts
        if (m_words.size() == m_words.capacity())
            m_words.reserve(m_words.capacity() + alloc_chunk);

        // Trim whitespace from the word
        string_trim(word);

        // All words must be the same length
        if (0 == word_len)
            word_len = word.length();
        if (word.length() != word_len) {
            fmt::print(std::cerr, "Invalid word file: Inconsistent word length: {}\n", word);
            m_words.clear();
            return;
        }

        // Ensure consistent case
        string_to_lower(word);

        // Move into our vector
        m_words.emplace_back(std::move(word));
    }

    // Make sure list is sorted so we can quickly search
    std::sort(m_words.begin(), m_words.end());
}

// Defined in word_list.cpp
extern std::string_view default_words_blob;
extern size_t           default_word_size;

/// Initialize word list from internal word list
void Mordle::InitWordListInternal()
{
    // The default word blob is just a blob of words with all whitespace
    // removed. Since we know the word length, it's easy to pull them out
    // and put them into our vector.
    size_t word_count = default_words_blob.length() / default_word_size;

    m_words.resize(word_count);

    size_t idx = 0;
    for (size_t i = 0; i<word_count; ++i, idx += default_word_size)
        m_words[i].assign(default_words_blob.substr(idx, default_word_size));
}

const std::string& Mordle::GetRandomWord() const
{
    std::uniform_int_distribution<size_t> dist(0, m_words.size() - 1);
    return m_words[dist(GetPrngGenerator())];
}

/// Returns true if given word is in the word list
bool Mordle::IsWordInList(const std::string& word) const
{
    // Our list is sorted; use a binary search to find it
    auto it = std::lower_bound(m_words.begin(), m_words.end(), word);
    return ((it != m_words.end()) && (0 == it->compare(word)));
}

/**
 * @brief       Check a guessed word against the secret word
 *
 * @param secret_word   The secret word
 * @param guess_word    The guessed word
 * @param result        A string containing the results of each letter in
 *  the guessed word. Character N correponds to character N in the guessed
 *  word. A '!' means that the letter is in the correct spot. A 'x' means
 *  that the letter is not in the word. A '~' means the character is in the
 *  wrong spot. The output result is only valid if this function returns
 *  true.
 *
 * @return Returns true if guess_word is a valid word
 */
bool Mordle::CheckWordGuess(const std::string& secret_word,
    const std::string& guess_word, std::string& result)
{
    if (!IsWordInList(guess_word))
        return false;

    result.resize(secret_word.length());

    size_t i, n = secret_word.length();

    // First pass: Find exact matches
    for (i=0; i<n; ++i)
        result[i] = (guess_word[i] == secret_word[i]) ? res_matched : res_unknown;

    // Next pass: Handle the rest
    for (i=0; i<n; ++i) {
        if (result[i] != res_unknown) continue;

        size_t offset = 0;
        while (1) {
            // Check if current letter is in the word
            auto p = secret_word.find(guess_word[i], offset);
            if (p == std::string::npos) {
                result[i] = res_missing;
                break;
            }

            // Letter is in the word. If it's a letter that we already
            // marked as correct, then loop back around and try again.
            if (result[p] != res_matched) {
                result[i] = res_mislaid;
                break;
            }

            offset = p+1;
        }
    }

    return true;
}

/// Play a game of wordle in the current terminal
bool Mordle::TerminalPlay(std::string secret_word)
{
    constexpr int max_guesses = 6;
    int guess_number = 1;

    std::string guess, result;
    if (secret_word.empty())
        secret_word.assign(GetRandomWord());        // Pick a random word

    // Main game loop
    while (1) {

        // Get user's input
        fmt::print("{}: ", guess_number);
        if (!std::getline(std::cin, guess))
            break;
        if (string_trim(guess).empty())
            continue;
        string_to_lower(guess);

        // Check the guess against the word
        if (!CheckWordGuess(secret_word, guess, result)) {
            fmt::print("Not a word\n");
            continue;
        }

        // Display the guess results
        DisplayGuessResult(guess, result);

        // Are we done?
        if (guess.compare(secret_word) == 0) {
            fmt::print("{}\n", GetWinExclamatory(guess_number));
            return true;
        }
        if (++guess_number > max_guesses) {
            fmt::print("{}\nThe word was: {}\n", GetLoseInsult(), secret_word);
            return false;
        }
    }

    return false;       // Should be unreachable
}

void Mordle::DisplayGuessResult(const std::string& guess, const std::string& result)
{
    if (!m_no_color) {

        // Use colorized output

        for (size_t i = 0; i<guess.length(); ++i) {
            fmt::color c;
            switch(result[i]) {
                case Mordle::res_matched: c = static_cast<fmt::color>(color_matched); break;
                case Mordle::res_mislaid: c = static_cast<fmt::color>(color_mislaid); break;
                case Mordle::res_missing: c = static_cast<fmt::color>(color_missing); break;
                default : c = fmt::color::white;
            }
            fmt::print("{:^3}", fmt::styled(guess[i],
                fmt::fg(fmt::color::white) | fmt::bg(c)));
        }
        fmt::print("\n");
    }
    else {

        // Don't use colorized output

        // Display guess
        for (size_t i = 0; i<guess.length(); fmt::print("{}", guess[i++]));
        fmt::print("\n");
        // Display results underneath
        for (size_t i = 0; i<guess.length(); fmt::print("{}", result[i++]));
        fmt::print("\n");
    }
}

/// Returns the string to use when player wins
std::string_view Mordle::GetWinExclamatory(int guess_count) const
{
    switch (guess_count) {
    case 1:  return("Genius!\n"); break;
    case 2:  return("Magnificent\n"); break;
    case 3:  return("Impressive\n"); break;
    case 4:  return("Splendid\n"); break;
    case 5:  return("Great\n"); break;
    case 6:  return("Phew\n"); break;
    default: return("Meh\n"); break;
    }
}

/// Returns the string to use when player loses
std::string_view Mordle::GetLoseInsult() const
{
    std::uniform_int_distribution dist(0, 25);

    switch (dist(GetPrngGenerator())) {
    case 0:  return "Wow, that was embarrassing.";
    case 1:  return "At least your head can serve as a hat rack.";
    case 2:  return "Were you dropped on your head as a child?";
    case 3:  return "Stupid is as stupid does.";
    case 4:  return "Don't quit your day job.";
    case 5:  return "You are terrible at this.";
    case 6:  return "Sorry, you suck.";
    default: return "You lose.";
    }
}

/// Display word statistics
int Mordle::DisplayWordStats()
{
    fmt::print("MOOMOO: Implement DisplayWordStats()\n");
    return 0;
}

/// List words with optional hints to filter output
int Mordle::ListWords(const HintVect& hints)
{
    // Validate hints

    // Generare a string containing all valid result codes
    std::string res_chars;
    res_chars.append(1, res_matched);
    res_chars.append(1, res_missing);
    res_chars.append(1, res_mislaid);

    for (auto&& [word,result] : hints) {
        bool valid = true;      // Optimism

        // word and result length must match the game word size
        if ((word.length() != GetWordSize()) || (result.length() != GetWordSize()))
            valid = false;
        // result string must be comprised of res_* chars
        if (result.find_first_not_of(res_chars) != std::string::npos)
            valid = false;

        if (!valid) {
            fmt::print(std::cerr, "Invalid hint: {} {}\n", word, result);
            return 1;
        }
    }

    // For all words in our word list...
    bool words_displayed = false;
    for (const auto& w : m_words) {

        // For each hint...
        bool drop = false;
        for (const auto& h : hints) {
            if (!CheckWordAgainstHint(w, h)) {
                // Current word is not a possible solution given this hint
                drop = true;
                break;
            }
        }
        if (drop)
            continue;

        fmt::print("{}\n", w);
        words_displayed = true;
    }

    if (!words_displayed)
        fmt::print("<No words matched>\n");

    return 0;
}

// Determine if a word is a possible solution given a hint
bool Mordle::CheckWordAgainstHint(const std::string& word, const HintPair& hint)
{
    const std::string& hword = hint.first;
    const std::string& hres  = hint.second;
    const auto npos = std::string::npos;

    for (size_t i=0; i<GetWordSize(); ++i) {
        switch(hres[i]) {
        case res_matched: if (word[i] != hword[i])         return false; break;
        case res_missing: if (word.find(hword[i]) != npos) return false; break;
        case res_mislaid: if (word[i] == hword[i])         return false; break;
        default: break;
        }
    }

    return true;
}
