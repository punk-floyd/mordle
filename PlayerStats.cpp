#include <system_error>
#include <fmt/format.h>
#include <fmt/ostream.h>
#include <fmt/chrono.h>
#include <fmt/color.h>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <chrono>
#include <vector>

#include "PlayerStats.h"
#include "mrdle.h"
#include "os.h"

int PlayerStats::Save() const
{
    if (int res = EnsureStatsDir())
        return res;

    std::ofstream ofs(GeneratePathname());
    if (!ofs.is_open())
        return -1;

    ofs << m_play_count << '\n';
    ofs << m_win_count  << '\n';
    ofs << m_cur_streak << '\n';
    ofs << m_max_streak << '\n';
    ofs << m_last_win   << '\n';
    for (auto g : m_guess)
        ofs << g << '\n';

    return 0;
}

int PlayerStats::Load()
{
    std::ifstream ifs(GeneratePathname());
    if (!ifs.is_open())
        return -1;

    ifs >> m_play_count;
    ifs >> m_win_count;
    ifs >> m_cur_streak;
    ifs >> m_max_streak;
    ifs >> m_last_win;
    for (auto &g : m_guess)
        ifs >> g;

    return ifs.good() ? 0 : -1;
}

void PlayerStats::Attempt()
{
    m_play_count++;
}

int PlayerStats::Win(size_t guesses, bool save)
{
    auto now = std::chrono::system_clock::now();
    m_last_win = std::chrono::system_clock::to_time_t(now);

    ++m_win_count;

    if (m_max_streak < ++m_cur_streak)
        m_max_streak =   m_cur_streak;

    // Index N contains the number of wins in N+1 guesses
    if (guesses && ((guesses - 1) < m_guess.size()))
        m_guess[guesses - 1]++;

    return save ? Save() : 0;
}

int PlayerStats::Lose(bool save)
{
    m_cur_streak = 0;

    return save ? Save() : 0;
}

void PlayerStats::Report(bool no_color, size_t guess_highlight) const
{
    constexpr int fw = 15;  // field width

    fmt::print("{:<{}} {}\n", "Played:", fw, m_play_count);
    if (0 == m_play_count)
        return;

    double win_pct_d = static_cast<double>(m_win_count) / static_cast<double>(m_play_count);
    int win_pct = static_cast<int>((win_pct_d * 100) + 0.5);
    fmt::print("{:<{}} {}\n", "Win %:", fw, win_pct);
    fmt::print("{:<{}} ", "Last win:", fw);
    if (m_win_count)
        fmt::print("{:%Y-%m-%d %H:%M:%S}\n", fmt::localtime(m_last_win));
    else
        fmt::print("Never. So sad.\n");

    fmt::print("{:<{}} {}\n", "Current Streak:", fw, m_cur_streak);
    fmt::print("{:<{}} {}\n", "Max Streak:", fw, m_max_streak);

    fmt::print("Guess distribution:\n");

    // Scale to the maximum guess count
    size_t max_item = 0;
    for (auto g : m_guess) {
        if (max_item < g)
            max_item = g;
    }
    double max_item_d = static_cast<double>(max_item);

    constexpr int max_range = 50;
    for (size_t idx = 1;  auto g : m_guess) {
        fmt::print("{} ", idx);

        // Length of bar is proportional to max item
        int n = max_item ? static_cast<int>((static_cast<double>(g) / max_item_d) * max_range) : 0;
        std::string s(std::to_string(g));
        s.insert(0, n + 1, ' ');
        s.append(1, ' ');

        if (!no_color) {
            auto c = idx == guess_highlight ?
                static_cast<fmt::color>(mrdle::color_matched) :
                static_cast<fmt::color>(mrdle::color_missing);
            fmt::print("{}\n", fmt::styled(s, fmt::fg(fmt::color::white) | fmt::bg(c)));
        }
        else
            fmt::print("{}\n", s);  // no color

        idx++;
    }
}

/// Returns directory to use for stats, or empty string
std::string PlayerStats::GetStatsDirectory() const
{
    std::string stat_dir(GetUserHomeDirectory());
    if (!stat_dir.empty())
        stat_dir.append("/.mrdle");

    return stat_dir;
}

std::string PlayerStats::GeneratePathname() const
{
    std::string pn(GetStatsDirectory());
    if (pn.empty())
        return pn;

    // File name: stats[-name].word_size.max_guesses
    std::ostringstream oss;
    oss << pn << "/stats";
    if  (!m_name.empty())
        oss << '-' << m_name;
    oss << '.' << m_word_size << '.' << m_guess.size();

    //return std::string("c:\\tmp\\dev_stats.txt");
    return std::string(oss.str());
}

/// Ensures stats directory exists, creating if necessary
int PlayerStats::EnsureStatsDir() const
{
    std::string stat_dir(GetStatsDirectory());
    if (stat_dir.empty())
        return -1;

    std::error_code ec;
    if (std::filesystem::exists(stat_dir, ec) && !ec)
        return 0;   // Directory exists

    // Directory does not exist. Create it.
    return std::filesystem::create_directory(stat_dir, ec) && !ec;
}
