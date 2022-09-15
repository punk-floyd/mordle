/**
 * @file    PlayerStats.h
 * @author  Mike DeKoker (dekoker.mike@gmail.com)
 * @brief   Declares PlayerStats; player statistics for mrdle
 *
 * @copyright Copyright (c) 2022 Mike DeKoker
 *
 */

#include <string>
#include <vector>

class PlayerStats {

public:

    PlayerStats(std::string name = "", size_t word_size = 5, size_t max_guesses = 6)
        : m_guess(max_guesses), m_name(std::move(name)), m_word_size(word_size)
    { }

    /// Load stats from file
    int Load();
    /// Save stats to file
    int Save() const;
    /// Report current player stats to standard output
    void Report(bool no_color = false, size_t guess_highlight = 0) const;

    /// Add a new game attempt to player stats
    void Attempt();
    /// Update stats for a win
    void Win(size_t guesses);
    /// Update stats for a loss
    void Lose();

protected:

    using guess_vect_t = std::vector<uint64_t>;

    /// Returns the pathname to use for the stats file
    std::string GeneratePathname() const;

private:

    std::string     m_name;             ///< Optional user name
    size_t          m_word_size;        ///< Word length that stats are based on

    uint64_t        m_play_count{ 0 };  ///< Total play attempts
    uint64_t        m_win_count{ 0 };   ///< Total wins
    uint64_t        m_cur_streak{ 0 };  ///< Current winning streak
    uint64_t        m_max_streak{ 0 };  ///< Maximum winning streak
    time_t          m_last_win{ 0 };    ///< Time of last win
    guess_vect_t    m_guess;            ///< Guess distribution
};
