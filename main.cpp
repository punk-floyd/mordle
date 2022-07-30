/**
 * @file    main.cpp
 * @author  Mike DeKoker (dekoker.mike@gmail.com)
 * @brief   Program entry point and command line processing
 *
 * @copyright Copyright (c) 2022
 *
 */
#include <fmt/core.h>
#include <fmt/ostream.h>
#include <fmt/color.h>
#include <filesystem>
#include <stdexcept>
#include <iostream>
#include <map>
#include "mrdle.h"
#include "util.h"

static constexpr std::string_view program_version("0.3");

/// Program options data
struct ProgOpts {

    bool                version{false};         ///< --version
    bool                help{false};            ///< --help
    bool                list{false};            ///< --list
    bool                rules{false};           ///< --rules
    bool                player_stats{false};    ///< --player-stats
    bool                play{true};             ///< --play
    bool                no_color{false};        ///< --no-color

    std::string         secret_word;            ///< --secret-word
    std::string         word_file;              ///< --word-file

    mrdle::HintVect     hint_vect;              ///< --hint
};

static int ProcessCommandLine(int argc, char* argv[], ProgOpts& opts);
static int DisplayPlayerStats(const ProgOpts& opts);
static int DisplayVersion(const ProgOpts& opts);
static int DisplayRules(const ProgOpts& opts);
static int DisplayHelp(const ProgOpts& opts);

int main(int argc, char* argv[])
{
    try {
        // Parse and digest the command line
        ProgOpts opts;
        if ((argc > 1) && ProcessCommandLine(argc, argv, opts))
            return -1;

        // Handle options that do not require a mrdle instance
        if (opts.version)
            return DisplayVersion(opts);
        if (opts.help)
            return DisplayHelp(opts);
        if (opts.rules)
            return DisplayRules(opts);
        if (opts.player_stats)
            return DisplayPlayerStats(opts);

        // Instantiate the mrdle object
        mrdle ws(opts.word_file);
        if (0 == ws.GetWordListCount()) {
            // Something failed; should have been reported
            return 1;
        }
        ws.SetNoColorMode(opts.no_color);

        if (opts.list)
            return ws.ListWords(opts.hint_vect);

        // Validate secret word length if necessary
        auto swl = opts.secret_word.length();
        if (swl && swl != ws.GetWordSize()) {
            fmt::print(std::cerr, "mrdle: Invalid secret word length\n");
            return 1;
        }

        ws.TerminalPlay(opts.secret_word);
    }
    catch (std::runtime_error& e) {
        fmt::print(std::cerr, "Well this is emabarrassing: {}\n", e.what());
        return 1;
    }

    return 0;
}

int ProcessCommandLine(int argc, char* argv[], ProgOpts& opts)
{
    // Quick and dirty implementation. Not using getopt because it's not
    // portable. To simplify things, we only support "--foo" arguments.

    /// Map a "boolean" argument to its value
    std::map<std::string, bool*, std::less<>> bool_map;
    bool_map["version"]      = &opts.version;
    bool_map["help"]         = &opts.help;
    bool_map["list"]         = &opts.list;
    bool_map["rules"]        = &opts.rules;
    bool_map["player-stats"] = &opts.player_stats;
    bool_map["play"]         = &opts.play;
    bool_map["no-color"]     = &opts.no_color;
    /// Map a "string" argument to its value
    std::map<std::string, std::string*, std::less<>> str_map;
    str_map["secret-word"]   = &opts.secret_word;
    str_map["word-file"]     = &opts.word_file;

    // For all command line arguments...
    for (int a=1; a<argc; ++a) {

        // Verify basic argument shape: --foo
        std::string_view arg(argv[a]);
        if ((arg.length() < 3) || (!arg.starts_with("--"))) {
            fmt::print(std::cerr, "mrdle: Invalid argument: {}\n", arg);
            return -1;
        }
        arg.remove_prefix(2);       // Remove the leading "--"

        // Does this parameter correspond to a boolean flags?
        const auto bit = bool_map.find(arg);
        if (bit != bool_map.cend()) {
            *bit->second = true;
            continue;
        }

        // Does this parameter correspond to a string value?
        const auto sit = str_map.find(arg);
        if (sit != str_map.cend()) {

            // Make sure we have the argument value; value validated later
            if (a + 1 >= argc) {
                fmt::print(std::cerr, "mrdle: Missing required value for argument: {}\n", arg);
                return -1;
            }

            sit->second->assign(argv[++a]);
            continue;
        }

        // Only other valid parameter is --hint which takes two arguments
        if (0 == arg.compare("hint")) {

            // Make sure we have the argument values; values validated later
            if (a + 2 >= argc) {
                fmt::print(std::cerr, "mrdle: Missing required value for argument: {}\n", arg);
                return -1;
            }

            opts.hint_vect.emplace_back(std::make_pair(argv[a+1], argv[a+2]));
            a += 2;
            opts.list = true;   // Any --hint implies --list
            continue;
        }

        fmt::print(std::cerr, "mrdle: Unknown argument: {}\n", arg);
        return -1;
    }

    // Convert any input words to lower case
    string_to_lower(opts.secret_word);
    for (auto& h : opts.hint_vect)
        string_to_lower(h.first);

    return 0;
}

static int DisplayVersion(const ProgOpts& opts)
{
    constexpr std::string_view prog_name("mrdle");

    if (opts.no_color)
        fmt::print("{}\n", prog_name);
    else {
        auto c1 = static_cast<fmt::color>(mrdle::color_matched);
        auto c2 = static_cast<fmt::color>(mrdle::color_missing);
        for (size_t i = 0; i<prog_name.length(); ++i) {
            fmt::color c;
            switch(prog_name[i]) {
                case 'm': case 'd': c = c2; break;
                default:            c = c1; break;
            }
            fmt::print("{:^3}", fmt::styled(prog_name[i],
                fmt::fg(fmt::color::white) | fmt::bg(c)));
        }
        fmt::print("\n");
    }

    fmt::print(" Version {}, Built:{}\n", program_version, __DATE__);
    fmt::print(" By Mike DeKoker (dekoker.mike@gmail.com)\n");
    return 0;
}

static int DisplayHelp(const ProgOpts& opts)
{
    static constexpr std::string_view name("mrdle");

    fmt::print("{}: Wordle clone and solution helper\n", name);
    fmt::print("Usage: {} [ACTION] [OPTIONS...]\n", name);

    fmt::print("\nActions:\n");
    fmt::print("  --play              Shall we play a game? (default action)\n");
    fmt::print("  --list              List words from word list (see --hint)\n");
    fmt::print("  --rules             Display game rules and exit\n");
  //fmt::print("  --player-stats      Display stats for the current user\n");
    fmt::print("\n");
    fmt::print("Game options:\n");
    fmt::print("  --secret-word WORD  Uses WORD as the secret word.\n");
    fmt::print("List words options:\n");
    fmt::print("  --hint WORD HINT    Implies --list. Filters listed words by excluding words\n");
    fmt::print("                      that do not satisfy the game hint. WORD is a word that\n");
    fmt::print("                      was played and HINT is the encoded results of that word\n");
    fmt::print("                      See Finding Solutions below.\n");
    fmt::print("Common options:\n");
    fmt::print("  --word-file FILE    Use words listed in FILE. Words can be of any length\n");
    fmt::print("                      but they must all be the same length.\n");
    fmt::print("  --no-color          Do not use colored output\n");
    fmt::print("  --version           Display version information and exit\n");
    fmt::print("  --help              Display usage information and exit\n");
    fmt::print("\n");

    return 0;
}

static int DisplayRules(const ProgOpts& opts)
{
    fmt::print("mrdle: MOOMOO: Display rules\n");
    return 0;
}

static int DisplayPlayerStats(const ProgOpts& opts)
{
    fmt::print("mrdle: MOOMOO: Display player stats\n");
    return 0;
}
