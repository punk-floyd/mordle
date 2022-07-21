#include "Mordle.h"

int main(int argc, char* argv[])
{
    const char* word_file = "/home/mike/sw/mike/mordle/WORDS";
    if (argc > 1)
        word_file = argv[1];

    Mordle ws(word_file);
    ws.TerminalPlay();
}
